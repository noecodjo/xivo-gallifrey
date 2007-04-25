#!/usr/bin/python
# -*- coding: iso-8859-15 -*-
"""Autoprovisioning daemon for Xivo

Copyright (C) 2007, Proformatique

"""
# Dependencies : python-sqlite

import os, cgi, sys, sqlite, thread, threading, traceback

import syslog
from syslog import syslog      as syslogf
from syslog import LOG_EMERG   as SYSLOG_EMERG
from syslog import LOG_ALERT   as SYSLOG_ALERT
from syslog import LOG_CRIT    as SYSLOG_CRIT
from syslog import LOG_ERR     as SYSLOG_ERR
from syslog import LOG_WARNING as SYSLOG_WARNING
from syslog import LOG_NOTICE  as SYSLOG_NOTICE
from syslog import LOG_INFO    as SYSLOG_INFO
from syslog import LOG_DEBUG   as SYSLOG_DEBUG

from getopt import getopt

import BaseHTTPServer
from BaseHTTPServer import HTTPServer
from BaseHTTPServer import BaseHTTPRequestHandler

import SocketServer
from SocketServer import socket

# personnal imports:
import provsup
from Phones import * # package containing one module per vendor
from moresynchro import RWLock
from moresynchro import ListLock
from daemonize import daemonize

DB	    = "/var/lib/asterisk/astsqlite"
# sqlite timeout is in ms
SQLITE_BUSY_TIMEOUT = 100

# === used all over this file: ===
TABLE	    = "phone"
UF_TABLE    = "userfeatures"
SIP_TABLE   = "usersip"

# right now the only allowed tech is SIP:
TECH        = "sip"

# deletion (W) / provisioning (R) lock timeout in secs
DEL_OR_PROV_TIMEOUT = 45

def name_from_first_last(first, last):
	"Construct full name from first and last."
	if first and last:
		return first + ' ' + last
	if first:
		return first
	if last:
		return last
	return ''

class SQLiteDB:
	"""An information backend for this provisioning daemon,
	using the Xivo SQLite database.
	
	"""
	def __init__(self, db):
		self.__db = db

	# === INTERNAL FUNCTIONS ===
	
	def generic_sqlite_request(self, foobar, *remain):
		"""Generic function to generate a safe context to issue any
		SQLite request.
		
		"""
		conn = sqlite.connect(self.__db)
		conn.db.sqlite_busy_timeout(SQLITE_BUSY_TIMEOUT)
		try:
			a = foobar(conn, *remain)
		except:
			provsup.log_current_exception()
			a = None
		conn.close()
		return a

	def sqlite_select_one(self, request, parameters_tuple, mapping):
		"Does a SELECT SQLite query and returns only one row."
		return self.generic_sqlite_request(self.method_select_one, request, parameters_tuple, mapping)
	def method_select_one(self, conn, request, parameters_tuple, mapping):
		"""Internally called in a safe context to execute SELECT
		SQLite queries and return their result (only one row).
		
		"""
		cursor = conn.cursor()
		cursor.execute(request, parameters_tuple)
		r = cursor.fetchone()
		if not r:
			return None
		return dict([(k,provsup.elem_or_none(r,v))
		             for k,v in mapping.iteritems()])

	def sqlite_select_all(self, request, parameters_tuple, mapping):
		"Does a SELECT SQLite query and returns all rows."
		return self.generic_sqlite_request(self.method_select_all, request, parameters_tuple, mapping)
	def method_select_all(self, conn, request, parameters_tuple, mapping):
		"""Internally called in a safe context to execute SELECT
		SQLite queries and return their result (all rows).
		
		"""
		cursor = conn.cursor()
		cursor.execute(request, parameters_tuple)
		return map(lambda row: dict([(k,provsup.elem_or_none(row,v))
		                             for k,v in mapping.iteritems()]),
                           cursor.fetchall())

	def sqlite_modify(self, request, parameters_tuple):
		"Does a SQLite query that is going to modify the database."
		return self.generic_sqlite_request(self.method_commit, request, parameters_tuple)
	def method_commit(self, conn, request, parameters_tuple):
		"""Internally called in a safe context to commit the result of
		SQLite queries that modify the database content.
		
		"""
		cursor = conn.cursor()
		cursor.execute(request, parameters_tuple)
		conn.commit()
		return None

	# === ENTRY POINTS ===

	def type_by_macaddr(self, macaddr):
		"""Lookup vendor and model of the phone in the database,
		by mac address.
		
		Returns a dictionary with the following keys:
		        'macaddr', 'vendor', 'model'
		or None

		"""
		query = "SELECT * FROM %s WHERE macaddr=%s" % (TABLE, '%s')
		mapping = dict(map(lambda x: (x,x), ("macaddr", "vendor", "model")))
		return self.sqlite_select_one(query, (macaddr,), mapping)

	def phone_by_macaddr(self, macaddr):
		"""Lookup a phone description by Mac Address in the database.
		
		Returns a dictionary with the following keys:
			'macaddr', 'vendor', 'model', 'proto', 'iduserfeatures'
		or None
		
		"""
		query = "SELECT * FROM %s WHERE macaddr=%s" % (TABLE, '%s')
		mapping = dict(map(lambda x: (x,x),
		                   ('macaddr', 'vendor', 'model', 'proto',
			            'iduserfeatures')))
		return self.sqlite_select_one(query, (macaddr,), mapping)

	def config_by_something_proto(self, something_column, something_content, proto):
		"""Query the database to return a phone configuration.
		
		something_column - name of the column of the table 'userfeatures'
		                   used to select the right phone
		something_content - content to be match against the content of
				    the column identified by something_column
		
		Returns a dictionary with the following keys:
		        'firstname', 'lastname', 'name': user civil status
				note that the name is constructed from the
				first and last name
		        'iduserfeatures': user id in the userfeatures table
		        'provcode': provisioning code
		        'ident': protocol specific identification
		        'passwd': protocol specific password
		        'number': extension number
		        'proto': protocol
		
		Right now proto must evaluate to 'sip'
		
		"""
		if proto != TECH:
			raise ValueError, "proto must be 'sip' for now"
		
		query = ("SELECT %s.*,%s.* " +
			  "FROM %s LEFT OUTER JOIN %s " +
				"ON %s.protocolid=%s.id AND %s.protocol=%s " +
			  "WHERE %s." + something_column + "=%s")\
			% ( UF_TABLE, SIP_TABLE,
			    UF_TABLE, SIP_TABLE,
			    UF_TABLE, SIP_TABLE, UF_TABLE, '%s',
			    UF_TABLE, '%s')
		mapping = {
			"firstname":		UF_TABLE+".firstname",
			"lastname":		UF_TABLE+".lastname",
			"ident":		SIP_TABLE+".name",
			"passwd":		SIP_TABLE+".secret",
			"number":		UF_TABLE+".number",
			"iduserfeatures":	UF_TABLE+".id",
			"provcode":		UF_TABLE+".provisioningid",
			"proto":		UF_TABLE+".protocol"
		}
		confdico = self.sqlite_select_one(
			query, (proto, something_content), mapping)
		if not confdico:
			return None
		confdico["name"] = name_from_first_last(confdico["firstname"],
							confdico["lastname"])
		return confdico

	def config_by_iduserfeatures_proto(self, iduserfeatures, proto):
		"""Lookup the configuration information of the phone in the
		database, by ID of the 'userfeatures' table, and protocol.
		
		Right now proto must evaluate to 'sip'
		
		"""
		return self.config_by_something_proto(
				"id", iduserfeatures, proto)

	def config_by_provcode_proto(self, provcode, proto):
		"""Lookup the configuration information of the phone in the
		database, by provisioning code and protocol.
		
		Right now proto must evaluate to 'sip'
		
		"""
		return self.config_by_something_proto(
				"provisioningid", provcode, proto)

	def save_phone(self, phone):
		"""Save phone informations in the database.
		phone must be a dictionary and contain the following keys:
		
		'macaddr', 'vendor', 'model', 'proto', 'iduserfeatures'
		
		"""
		self.sqlite_modify(
			"REPLACE INTO %s (macaddr, vendor, model, proto, iduserfeatures) VALUES (%s, %s, %s, %s, %s)" \
			% (TABLE, '%s', '%s', '%s', '%s', '%s'),
			map(lambda x: phone[x], ('macaddr', 'vendor', 'model',
			                         'proto', 'iduserfeatures')))

	def phone_by_iduserfeatures(self, iduserfeatures):
		"""Lookup phone information by user information (iduserfeatures)
		Right now this is limited to the 'sip' protocol, so the result
		is a single phone description in the form of a dictionary
		containing the classic keys.
		
		'macaddr', 'vendor', 'model', 'proto', 'iduserfeatures'
		
		"""
		query = ("SELECT * FROM %s " +
		         "WHERE iduserfeatures=%s AND proto=%s") \
			% ( TABLE, '%s', '%s' )
		mapping = dict(map(lambda x: (x,x), ("macaddr", "vendor",
		                          "model", "proto", "iduserfeatures")))
		return self.sqlite_select_one(
			query, (iduserfeatures, TECH), mapping)

	def delete_phone_by_iduserfeatures(self, iduserfeatures):
		"""Delete any phone in the database having the given
		iduserfeatures.
		
		"""
		self.sqlite_modify(
			("DELETE FROM %s WHERE iduserfeatures=%s")
		        % ( TABLE, '%s'),
			(iduserfeatures,))

	def find_orphan_phones(self):
		"""Find every phones that do not have a corresponding user
		anymore, but that are neither provisioned in state GUEST. Used
		at startup to maintain the coherency of the whole provisioning
		subsystem.
		
		Returns a list of dictionaries, each of the latter representing
		informations stored in the base about a phone with the classic
		keys :
		
		'macaddr', 'vendor', 'model', 'proto', 'iduserfeatures'
		
		"""
		return self.sqlite_select_all(
			("SELECT %s.* " + 
			 	"FROM %s LEFT JOIN %s " +
				"ON %s.iduserfeatures = %s.id " +
				"WHERE %s.iduserfeatures != 0 AND %s.id is NULL")
			% (TABLE, TABLE, UF_TABLE,
			   TABLE, UF_TABLE, TABLE, UF_TABLE),
			(),
			dict([(x,TABLE+'.'+x)
			      for x in ("macaddr", "vendor", "model",
			                "proto", "iduserfeatures")]))

	def delete_guest_by_mac(self, macaddr):
		"""Delete every GUEST phone from the database having the given
		Mac Address.
		
		"""
		self.sqlite_modify(
			("DELETE FROM %s WHERE macaddr = %s " +
			                      "AND iduserfeatures = %s")
			% (TABLE, '%s', '%s'),
			(macaddr, 0))

#	def delete_orphan_phones(self):
#		"""Delete any phone that does not have a corresponding user
#		anymore, but that is not provisioned in state GUEST. Used at
#		startup to maintain the base in a coherent state, because
#		SQLite does not support foreign key constraints.
#		
#		"""
#		self.sqlite_modify(
#			("DELETE FROM %s WHERE macaddr IN "+
#			 "(SELECT %s.macaddr " + 
#			 	"FROM %s LEFT JOIN %s " +
#				"ON %s.iduserfeatures = %s.id " +
#				"WHERE %s.iduserfeatures != 0 AND %s.id is NULL)")
#			% (TABLE, TABLE, TABLE, UF_TABLE,
#			   TABLE, UF_TABLE, TABLE, UF_TABLE), ())

class CommonProvContext:
	def __init__(self, userlocks, maclocks, dbinfos, rwlock):
		# There is no locking order because my ListLocks don't block.
		self.userlocks = userlocks
		self.maclocks = maclocks
		self.dbinfos = dbinfos
		self.rwlock = rwlock

def __provisioning(mode, ctx, phone):
	"Provisioning high level logic for the described phone"
	# I only allow "sip" right now
	if phone["proto"] != TECH:
		syslogf(SYSLOG_ERR,			       \
			"__provisioning(): the only protocol supported " +\
			"right now is SIP, but I got %s" \
			% phone["proto"])
		raise ValueError, "Unknown protocol '%s' != sip" % (phone["proto"],)

	syslogf(SYSLOG_NOTICE, "__provisioning(): handling phone %s" % str(phone))

	if (not phone["vendor"]) or (not phone["model"]):
		syslogf(SYSLOG_ERR, "__provisioning(): Missing model or vendor in phone %s" % str(phone))
		raise ValueError, "Missing model or vendor in phone %s" % str(phone)

	if "provcode" in phone and phone["provcode"] != "0" and \
	   not provsup.well_formed_provcode(phone["provcode"]):
		syslogf(SYSLOG_ERR, "__provisioning(): Invalid provcode %s" % (phone["provcode"],))
		raise ValueError, "Invalid provcode %s" % (phone["provcode"],)

	if phone["actions"] != "no":
		if "ipv4" not in phone:
			syslogf(SYSLOG_DEBUG, "__provisioning(): trying to get IPv4 address from Mac Address %s" % (phone["macaddr"],))
			phone["ipv4"] = provsup.ipv4_from_macaddr(phone["macaddr"], lambda x: syslogf(SYSLOG_ERR, x))
		if phone["ipv4"] is None:
			syslogf(SYSLOG_ERR, "__provisioning(): No IP address found for Mac Address %s" % (phone["macaddr"],))
			raise RuntimeError, "No IP address found for Mac Address %s" % (phone["macaddr"],)

	syslogf(SYSLOG_DEBUG, "__provisioning(): locking phone %s" % (phone["macaddr"],))
	if not ctx.maclocks.try_acquire(phone["macaddr"]):
	    syslogf(SYSLOG_WARNING, "__provisioning(): Operation already in progress for %s" % (phone["macaddr"],))
	    raise RuntimeError, "Operation already in progress for phone %s" % (phone["macaddr"],)
	try:
	    syslogf(SYSLOG_DEBUG, "__provisioning(): phone class from vendor")
	    prov_class = provsup.PhoneClasses[phone["vendor"]] # TODO also use model
	    syslogf(SYSLOG_DEBUG, "__provisioning(): phone instance from class")
	    prov_inst = prov_class(phone)

	    if "provcode" in phone and phone["provcode"] == "0":
		    phone["iduserfeatures"] = "0"

	    if "iduserfeatures" in phone and phone["iduserfeatures"] == "0":
		syslogf("__provisioning(): reinitializing provisioning to GUEST for phone %s" % (str(phone),))
		prov_inst.reinitprov()
	    else:			
		if "iduserfeatures" in phone:
		    syslogf("__provisioning(): getting configuration from iduserfeatures for phone %s" % (str(phone),))
		    config = ctx.dbinfos.config_by_iduserfeatures_proto(phone["iduserfeatures"], phone["proto"])
		else:
		    syslogf("__provisioning(): getting configuration from provcode for phone %s" % (str(phone),))
		    config = ctx.dbinfos.config_by_provcode_proto(phone["provcode"], phone["proto"])
		if config is not None \
		   and 'iduserfeatures' in config \
		   and config['iduserfeatures']:
		    syslogf(SYSLOG_DEBUG, "__provisioning(): locking user %s" % config['iduserfeatures'])
		    if not ctx.userlocks.try_acquire(config['iduserfeatures']):
			syslogf(SYSLOG_WARNING, "__provisioning(): Operation already in progress for user %s" % config['iduserfeatures'])
			raise RuntimeError, "Operation already in progress for user %s" % config['iduserfeatures']
		    try:
			syslogf(SYSLOG_NOTIFY, "__provisioning(): AUTOPROV'isioning phone %s with config %s" % (str(phone),str(config)))
			__mode_dependant_provlogic_locked(mode, ctx, phone, config)
			prov_inst.autoprov(config)
		    finally:
			syslogf(SYSLOG_DEBUG, "__provisioning(): unlocking user %s" % config['iduserfeatures'])
			ctx.userlocks.release(config['iduserfeatures'])
		else:
		    syslogf(SYSLOG_ERR, "__provisioning(): not AUTOPROV'isioning phone %s cause no config found or no iduserfeatures in config" % (str(phone),))

	    if "iduserfeatures" not in phone and config is not None and \
	       "iduserfeatures" in config:
		syslogf(SYSLOG_DEBUG, "__provisioning(): iduserfeatures='%s' from config to phone" % (config["iduserfeatures"],))
		phone["iduserfeatures"] = config["iduserfeatures"]

	    if mode != "informative" and "iduserfeatures" in phone:
		    syslogf("__provisioning(): SAVING phone %s informations to backend" % (str(phone),))
		    ctx.dbinfos.save_phone(phone)
	finally:
	    syslogf(SYSLOG_DEBUG, "__provisioning(): unlocking phone %s" % (phone["macaddr"],))
	    ctx.maclocks.release(phone["macaddr"])

def __mode_dependant_provlogic_locked(mode, ctx, phone, config):
	if mode == 'informative':
		syslogf(SYSLOG_DEBUG, "__mode_dependant_provlogic_locked() in informative mode for phone %s and user %s" % (phone['macaddr'], config['iduserfeatures']))
		existing_phone = phone_by_iduserfeatures(config['iduserfeatures'])
		if existing_phone:
			syslogf(SYSLOG_WARNING, "__mode_dependant_provlogic_locked(): User %s already has a locally provisioned phone, not trying to provision a remote one" % config['iduserfeatures'])
			raise RuntimeError, "User %s already has a locally provisioned phone, not trying to provision a remote one" % config['iduserfeatures']
		ctx.dbinfos.delete_guest_by_mac(phone['macaddr'])
		existing_phone = ctx.dbinfos.phone_by_macaddr(phone['macaddr'])
		if existing_phone:
			syslogf(SYSLOG_WARNING, "__mode_dependant_provlogic_locked(): Phone %s already locally provisioned, not trying to provision it for remote operations" % phone['macaddr'])
			raise RuntimeError, "Phone %s already locally provisioned, not trying to provision it for remote operations" % phone['macaddr']
	elif mode == 'authoritative':
		syslogf(SYSLOG_DEBUG, "__mode_dependant_provlogic_locked() in authoritative mode for phone %s and user %s" % (phone['macaddr'], config['iduserfeatures']))
		existing_phone = phone_by_iduserfeatures(config['iduserfeatures'])
		if existing_phone and existing_phone['macaddr'] != phone['macaddr']:
			syslogf(SYSLOG_NOTIFY, "__mode_dependant_provlogic_locked(): phone %s to be put back in guest state, because another one (%s) is being provisioned for the same user" % (existing_phone['macaddr'], phone['macaddr']))
			existing_phone['mode'] = 'authoritative'
			existing_phone['actions'] = 'no'
			existing_phone['iduserfeatures'] = '0'
			__provisioning('__internal_to_guest', ctx, phone)

def __userdeleted(ctx, iduserfeatures):
	"Does what has to be done when a user is deleted."
	syslogf(SYSLOG_NOTICE, "__userdeleted(): handling deletion of user %s" % iduserfeatures)
	if not ctx.userlocks.try_acquire(iduserfeatures):
		syslogf(SYSLOG_WARNING, "__userdeleted(): Operation already in progress for user %s" % iduserfeatures)
		raise RuntimeError, "Operation already in progress for user %s" % iduserfeatures
	try:
		phone = ctx.dbinfos.phone_by_iduserfeatures(iduserfeatures)
		if phone:
			syslogf("__userdeleted(): phone to destroy, because destruction of its owner - %s" % str(phone))
			phone['mode'] = 'authoritative'
			phone['actions'] = 'no'
			phone['iduserfeatures'] = '0'
			__provisioning('userdeleted', ctx, phone)
		# the following line will just destroy non 'sip' provisioning
		# with the same "iduserfeatures", and as they are none for now
		# it's not really useful but might become so in the future
		ctx.dbinfos.delete_phone_by_iduserfeatures(iduserfeatures)
	finally:
		ctx.userlocks.release(iduserfeatures)


# Safe locking API for provisioning and related stuffs

def lock_and_provision(mode, ctx, phone):
	"""Will attempt to provision with a global lock
	hold in shared mode

	"""
	syslogf(SYSLOG_DEBUG, "Entering lock_and_provision(phone=%s)" % str(phone))
	if not ctx.rwlock.acquire_read(DEL_OR_PROV_TIMEOUT):
		raise RuntimeError, "Could not acquire the global lock in shared mode"
	try:
		__provisioning(mode, ctx, phone)
	finally:
		ctx.rwlock.release()
		syslogf(SYSLOG_DEBUG, "Leaving lock_and_provision for Mac Address %s" % phone["macaddr"])

def lock_and_userdel(ctx, iduserfeatures):
	"""Will attempt to delete informations related to the user identified by
	iduserfeatures from areas we are handling in the information backend,
	with a global lock hold for exclusive access.

	"""
	syslogf(SYSLOG_DEBUG, "Entering lock_and_userdel %s" % iduserfeatures)
	if not ctx.rwlock.acquire_write(DEL_OR_PROV_TIMEOUT):
		raise RuntimeError, "Could not acquire the global lock in exclusive mode"
	try:
		__userdeleted(ctx, iduserfeatures)
	finally:
		ctx.rwlock.release()
		syslogf(SYSLOG_DEBUG, "Leaving lock_and_userdel %s" % iduserfeatures)

def clean_at_startup(ctx):
	"Put back every non guest orphan phones in GUEST state at startup."
	orphans = ctx.dbinfos.find_orphan_phones()
	for phone in orphans:
		syslogf(SYSLOG_NOTIFY, "clean_at_startup(): about to remove orphan %s at startup" % str(phone))
		lock_and_userdel(ctx, phone['iduserfeatures'])

class MissingParam(Exception):
	pass

class ProvHttpHandler(BaseHTTPRequestHandler):
	"""ThreadingHTTPServer will create one instance of this class for the
	handling of each incoming connection received by this daemon, and each
	instance will run in a separated thread.
	Base methods of BaseHTTPRequestHandler are used to do the basic HTTP
	parsing of the HTTP request that is to be handled, and new methods are
	added that implement the high level behavior of autoprovisioning and
	related operations.
	
	"""
	def __init__(self, request, client_address, server):
		self.my_server = server
		self.my_ctx = self.my_server.my_ctx
		
		self.my_infos = self.my_ctx.dbinfos # compability with legacy code in this class
		
		self.posted = None
		BaseHTTPRequestHandler.__init__(self, request, client_address, server)

	# HTTP responses

	def send_response_headers_200(self, content_type = "text/plain"):
		self.send_response(200)
		self.send_header("Content-Type", content_type)
		self.send_header("Cache-Control", "no-cache")
		self.send_header("Pragma", "no-cache")
		self.end_headers()
	def send_response_lines(self, req_lines):
		self.send_response_headers_200()
		self.wfile.writelines(map(lambda x: x+"\r\n", req_lines))
	def answer_404(self, err_str = None):
		syslogf(SYSLOG_NOTICE, "answer_404(): sending not found message to %s" % str(self.client_address))
		self.send_error(404, err_str)
	def send_error_explain(self, errno, perso_message):
		self.send_response(errno)
		self.end_headers()
		self.wfile.write(self.error_message_format % {
			'code': errno,
			'message': perso_message,
			'explain': self.responses[errno][1]
		})

	# Get POSTED informations in self.posted
	def get_posted(self):
		"""Extract a payload of a POST HTTP method that has the
		following format:
		
		name1=value1
		name2=value2
		...
		nameN=valueN
		
		A dictionary {'name1':'value1,
		              'name2':'value2',
			      ...
			      'nameN':'valueN' } is then stored in self.posted
		"""
		lines = []
		# This will raise an exception if not present or not an integer
		# This is the wanted behavior
		datalength = int(self.headers['Content-Length'])
		readbytes = 0
		if not datalength:
			return {}
		line = self.rfile.readline()
		while line and line.strip() and readbytes < datalength:
			lines.append(tuple(line.strip().split('=', 1)))
			readbytes += len(line)
			if readbytes < datalength:
				line = self.rfile.readline()
			else:
				line = None
		self.posted = dict(lines)

	# Extract interesting stuff from self.posted
	def save_posted_vars(self, d, variables):
		"""Store in the dictionary d each variable of the variables
		list present in self.posted, raising an exception if some
		are missing.
		
		"""
		for v in variables:
			if v not in self.posted:
				raise MissingParam, v + " missing in posted command"
			d[v] = self.posted[v]
	def save_macaddr_ipv4(self, phone):
		"""Save 'macaddr' in phone from self.posted or raise a
		MissingParam exception, and optionally save 'ipv4'.
		
		"""
		if "macaddr" not in self.posted:
			raise MissingParam, "Mac Address not given"
		phone["macaddr"] = provsup.normalize_mac_address(self.posted["macaddr"])
		if "ipv4" in self.posted:
			phone["ipv4"] = self.posted["ipv4"]
	def save_iduserfeatures_or_provcode(self, phone):
		"""Save 'iduserfeatures' in phone from self.posted in priority,
		or save 'provcode' if 'iduserfeatures' was not present.
		
		"""
		try:
			self.save_posted_vars(phone, ("iduserfeatures",))
		except MissingParam:
			self.save_posted_vars(phone, ("provcode",))

	def posted_infos(self, *thetuple):
		"""Generic function to retrieve every needed provisioning info
		from the request.
		
		"""
		phone = {}
		self.save_posted_vars(phone, thetuple)
		self.save_iduserfeatures_or_provcode(phone)
		self.save_macaddr_ipv4(phone)
		return phone
	def posted_phone_infos(self):
		"Used in 'authoritative' and 'informative' modes"
		return self.posted_infos("proto", "vendor", "model", "actions")
	def posted_light_infos(self):
		"Used in 'notification' mode"
		return self.posted_infos("proto", "actions")
	def posted_userdeleted_infos(self):
		"Used in 'userdeleted' mode"
		userinfo = {}
		self.save_posted_vars(userinfo, ("iduserfeatures","actions"))
		return userinfo

	def log_message(self, fmt, *args):
		"Override default logging method."
		syslogf(fmt % args)

	# Main handling functions

	def handle_prov(self):
	    "Does whatever action is asked by the peer"
	    phone = None
	    userinfo = None
	    syslogf(SYSLOG_NOTICE, "handle_prov(): handling /prov POST request for peer %s" % (str(self.client_address),))
	    try:
		self.get_posted()
		if "mode" not in self.posted:
			syslogf(SYSLOG_ERR, "handle_prov(): No mode posted")
			raise ValueError, "No mode posted"

		if self.posted["mode"] == "authoritative" or \
		   self.posted["mode"] == "informative":
			syslogf("handle_prov(): creating phone internal representation using full posted infos.")
			phone = self.posted_phone_infos()
			lock_and_provision(self.posted['mode'], self.my_ctx, phone)
		elif self.posted["mode"] == "notification":
			syslogf("handle_prov(): creating phone internal representation using light posted infos.")
			phone = self.posted_light_infos()
			phonetype = self.my_infos.type_by_macaddr(phone["macaddr"])
			phone["vendor"] = provsup.elem_or_none(phonetype, "vendor")
			phone["model"] = provsup.elem_or_none(phonetype, "model")
			lock_and_provision(self.posted['mode'], self.my_ctx, phone)
		elif self.posted["mode"] == "userdeleted":
			syslogf("handle_prov(): user deletion")
			userinfo = self.posted_userdeleted_infos()
			lock_and_userdel(self.my_ctx, userinfo['iduserfeatures'])
		else:
			syslogf(SYSLOG_ERR, "handle_prov(): Unknown mode %s" % (self.posted["mode"],))
			raise ValueError, "Unknown mode %s" % (self.posted["mode"],)

	    except:
		syslogf(SYSLOG_NOTICE, "handle_prov(): action FAILED - phone %s - userinfo %s" % (str(phone),str(userinfo)))
		tb_line_list = traceback.format_exception(*sys.exc_info())
		err_to_send = "<pre>\n" + cgi.escape(''.join(tb_line_list)) + "</pre>\n"
		for line in tb_line_list:
			syslogf(SYSLOG_ERR, line.rstrip())
		self.send_error_explain(500, err_to_send) # XXX
		return
	    syslogf(SYSLOG_NOTICE, "handle_prov(): provisioning OK for phone %s" % (str(phone),))
	    self.send_response_lines(('Ok',))

	def handle_list(self):
		"Respond with the list of supported Vendors / Models"
		syslogf(SYSLOG_NOTICE, "handle_list(): handling /list GET request for peer %s" % (str(self.client_address),))
		self.send_response_headers_200()
		for phonekey,phoneclass in provsup.PhoneClasses.iteritems():
			phonelabel = phoneclass.label
			phones = phoneclass.get_phones()
			self.wfile.write(phonekey + '="' + phonelabel.replace('"','\\"') + '"\r\n')
			self.wfile.writelines(map(lambda x: phonekey + '.' + x[0] + '="' + x[1].replace('"','\\"') + '"\r\n', phones))

	# === ENTRY POINTS (called FROM BaseHTTPRequestHandler) ===

	def do_POST(self):
		syslogf("do_POST(): handling POST request to path %s for peer %s" % (self.path, str(self.client_address)))
		if self.path == '/prov':
			self.handle_prov()
		else: self.answer_404()
	def do_GET(self):
		syslogf("do_POST(): handling GET request to path %s for peer %s" % (self.path, str(self.client_address)))
		if self.path == '/list':
			self.handle_list()
		else: self.answer_404()

class ThreadingHTTPServer(SocketServer.ThreadingTCPServer):
	"""Same as HTTPServer, but derivates from ThreadingTCPServer instead
	of TCPServer so that any instance of ProvHttpHandler created for any
	incoming connection runs in its own thread.
	
	"""
	allow_reuse_address = 1    # Seems to make sense in testing environment
	def server_bind(self):
		"""Override server_bind to store the server name."""
		SocketServer.TCPServer.server_bind(self)
		host, port = self.socket.getsockname()[:2]
		self.server_name = socket.getfqdn(host)
		self.server_port = port

def log_stderr_and_syslog(x):
	"""This function logs the string x to both stderr and the system log.
	The string to be logged doesn't have to end with a trailing '\\n'
	
	"""
	print >> sys.stderr, x
	syslogf(SYSLOG_ERR, x)

def main(log_level, foreground):
	"""log_level - one of SYSLOG_EMERG to SYSLOG_DEBUG
	            nothing will be logged below this limit
	foreground - don't daemonize if true
	
	"""
	syslog.openlog('autoprovisioning', syslog.LOG_PID, syslog.LOG_DAEMON)
	syslog.setlogmask(syslog.LOG_UPTO(log_level))
	if not foreground:
		daemonize(log_stderr_and_syslog)
	http_server = ThreadingHTTPServer((provsup.LISTEN_IPV4, provsup.LISTEN_PORT), ProvHttpHandler)
	http_server.my_ctx = CommonProvContext(
		ListLock(), # userlocks
		ListLock(), # maclocks
		SQLiteDB(DB),
		RWLock()
	)
	clean_at_startup(http_server.my_ctx)
	http_server.serve_forever()
	syslog.closelog()

if sqlite.paramstyle != 'pyformat':
	raise NotImplementedError, "This script expect pysqlite 1 with sqlite.paramstyle != 'pyformat', but sqlite.paramstyle has been detected as %s" % sqlite.paramstyle

dontlauchmain = False
foreground = False
logmap = {
	'emerg':   SYSLOG_EMERG,
	'alert':   SYSLOG_ALERT,
	'crit':    SYSLOG_CRIT,
	'err':     SYSLOG_ERR,
	'warning': SYSLOG_WARNING,
	'notice':  SYSLOG_NOTICE,
	'info':    SYSLOG_INFO,
	'debug':   SYSLOG_DEBUG
}
log_level = SYSLOG_NOTICE

# l: log filter up to EMERG, ALERT, CRIT, ERR, WARNING, NOTICE, INFO or DEBUG
# d: don't launch the main function (useful for python -i invocations)
# f: keep the program on foreground, don't daemonize
# b: override the default sqlite DB filename

opts,args = getopt(sys.argv[1:], 'b:l:df')
for k,v in opts:
	if '-l' == k:
		if v.lower() not in logmap:
			raise ValueError, "Unknown log filter '%s'" % v
		log_level = logmap[v.lower()]
	elif '-d' == k:
		dontlauchmain = True
	elif '-f' == k:
		foreground = True
	elif '-b' == k:
		DB = v

if __name__ == '__main__' and not dontlauchmain:
	main(log_level, foreground)
