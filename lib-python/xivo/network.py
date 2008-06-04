"""Network related routines for XIVO

Copyright (C) 2007, 2008  Proformatique

WARNING: Linux specific module, needs /sys/
"""

__version__ = "$Revision$ $Date$"
__license__ = """
    Copyright (C) 2007, 2008  Proformatique

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA..
"""

import re
import os
import os.path
import subprocess

from xivo import except_tb
from xivo import StreamedLines
from xivo.pyfunc import *


# CONFIG

SYS_CLASS_NET = "/sys/class/net"

# /sys/class/net/<ifname>/carrier tells us if the interface if plugged
CARRIER = "carrier"


# CODE

DECIMAL_SPLIT = re.compile(r'(\d+)').split

def to_int_if_possible(s):
	try:
		return int(s)
	except ValueError:
		return s

def split_lexdec(lexdec_str):
	"""
	This function splits the non decimal and the decimal parts of lexdec_str
	
	Exemples:
	
	>>> split_iface_name('wazza42sub10')
	('wazza', 42, 'sub', 10)
	>>> split_iface_name('42sub10')
	('', 42, 'sub', 10)
	>>> split_iface_name('a42sub')
	('a', 42, 'sub')
	>>> split_iface_name('')
	('',)
	"""
	lexdec_resplitted = DECIMAL_SPLIT(lexdec_str)
	if len(lexdec_resplitted) > 1 and lexdec_resplitted[-1] == '':
		strs = lexdec_resplitted[:-1]
	else:
		strs = lexdec_resplitted
	return tuple(map(to_int_if_possible, strs))

def unsplit_lexdec(lexdec_seq):
	"""
	Invert of split_lexdec()
	
	WARNING: unsplit_lexdec(split_lexdec("a0001")) == "a1"
	"""
	return ''.join(map(str, lexdec_seq))

def cmp_lexdec(x_str, y_str):
	"""
	Compare the splitted versions of x_str and y_str
	"""
	return cmp(split_lexdec(x_str), split_lexdec(y_str))

def sorted_lst_lexdec(seqof_lexdec_str):
	"""
	Sort ifnames according to their split_lexdec() representations
	Returns a list.
	NOTES:
	* The sorting is NOT done in place.
	* This function do not strip leading zeros in decimal parts; elements
	  are preserved as they are.
	"""
	return sorted(seqof_lexdec_str, cmp = cmp_lexdec)

def get_linux_netdev_list():
	"""
	Get an unfiltered view of network interfaces as seen by Linux
	"""
	return [entry for entry in os.listdir(SYS_CLASS_NET)
	        if os.path.isdir(os.path.join(SYS_CLASS_NET, entry))]

def get_filtered_ifnames(f = lambda x: True):
	"""
	Return the filtered list of network interfaces
	"""
	return filter(f, get_linux_netdev_list())

def get_filtered_phys(f = lambda x: True):
	"""
	Return the filtered list of network interfaces which are not VLANs
	(the interface name does not contain a '.')
	"""
	return [dev for dev in get_filtered_ifnames(f) if not '.' in dev]

def is_interface_plugged(ifname):
	"""
	WARNING: Only works on physical interfaces
	"""
	return int(file(os.path.join(SYS_CLASS_NET, ifname, CARRIER)).read().strip())

def normalize_ipv4_address(addr):
	"""
	Returns a canonical string repr of addr (which is a valid IPv4)
	"""
	return '.'.join([str(int(elt)) for elt in addr.split('.', 3)])

def normalize_mac_address(macaddr):
	"""
	input: mac address, with bytes in hexa, ':' separated
	ouput: mac address in format %02X:%02X:%02X:%02X:%02X:%02X
	"""
	macaddr_split = macaddr.upper().split(':', 6)
	if len(macaddr_split) != 6:
		raise ValueError, "Bad format for mac address " + macaddr
	return ':'.join([('%02X' % int(s, 16)) for s in macaddr_split])

def ipv4_from_macaddr(macaddr, logexceptfunc = None, ifname_filter = lambda x: True, arping_cmd_list = ['sudo', 'arping'], arping_sleep_us = 150000):
	"""
	Given a mac address, get an IPv4 address for an host living on the
	LAN.  This makes use of the tool "arping".  Of course the remote peer
	must respond to ping broadcasts.  Out of the box, some stupid phones
	from well known stupid and expensive brands don't.
	"""
	# -r : will only display the IP address on stdout, or nothing
	# -c 1 : ping once
	# -w <xxx> : wait for the answer during <xxx> microsec after the ping
	# -I <netiface> : the network interface to use is <netiface>
	#    -I is an undocumented option like -i but it works
	#    with alias interfaces too
	for iface in sorted_lst_lexdec(get_filtered_ifnames(ifname_filter)):
		result = None
		try:
			child = subprocess.Popen(arping_cmd_list + ["-r", "-c", "1", "-w", str(arping_sleep_us), '-I', iface, macaddr],
			                         bufsize = 0, stdout = subprocess.PIPE, close_fds = True)
			StreamedLines.makeNonBlocking(child.stdout)
			for (result,) in StreamedLines.rxStreamedLines(fobjs = (child.stdout,), timeout = arping_sleep_us * 10. / 1000000.):
				break
		except:
			result = None
			if logexceptfunc:
				except_tb.log_exception(logexceptfunc)
		if result:
			return result.strip()
	return None

def macaddr_from_ipv4(ipv4, logexceptfunc = None, ifname_filter = lambda x: True, arping_cmd_list = ['sudo', 'arping'], arping_sleep_us = 150000):
	"""
	ipv4_from_macaddr() is indeed a symetrical fonction that can be
	used to retrieve an ipv4 address from a given mac address.  This
	function just call the former.
	
	WARNING: this is of course ipv4_from_macaddr() implementation dependent
	"""
	return ipv4_from_macaddr(ipv4,
	                         logexceptfunc = logexceptfunc,
	                         ifname_filter = ifname_filter,
	                         arping_cmd_list = arping_cmd_list,
				 arping_sleep_us = arping_sleep_us)

def parse_ipv4(straddr):
	"""
	Return an IPv4 address as a 4uple of ints
	* straddr is an IPv4 address stored as a string
	"""
	return tuple(map(int, straddr.split('.', 3)))

def unparse_ipv4(tupaddr):
	"""
	Return a string repr of an IPv4 internal repr
	* tupaddr is an IPv4 address stored as a tuple of 4 ints
	"""
	return '.'.join(map(str, tupaddr))

def mask_ipv4(mask, addr):
	"""
	Binary AND of IPv4 mask and IPv4 addr
	(mask and addr are 4uple of ints)
	"""
	return tuple([m & a for m, a in zip(mask, addr)])

def or_ipv4(mask, addr):
	"""
	Binary OR of IPv4 mask and IPv4 addr
	(mask and addr are 4uple of ints)
	"""
	return tuple([m | a for m, a in zip(mask, addr)])

def netmask_invert(mask):
	"""
	Invert bits in mask
	(mask is 4uple of ints)
	"""
	return tuple([m ^ 0xFF for m in mask])

def plausible_netmask(addr):
	"""
	Check that addr (4uple of ints) makes a plausible netmask
	(set bits first, reset bits last)
	"""
	state = 1
	for addr_part in addr:
		for bitval in (128, 64, 32, 16, 8, 4, 2, 1):
			if state:
				if not (addr_part & bitval):
					state = 0
			else:
				if (addr_part & bitval):
					return False
	return True

# WARNING: the following function does not test the length which must be <= 63
domain_label_ok = re.compile(r'[a-zA-Z]([-a-zA-Z0-9]*[a-zA-Z0-9])?$').match

def plausible_search_domain(search_domain):
	"""
	Returns True if the search_domain is suitable for use in the search
	line of /etc/resolv.conf, else False.
	"""
	# NOTE: 251 comes from FQDN 255 maxi including label length bytes, we
	# do not want to validate search domain beginning or ending with '.',
	# 255 seems to include the final '\0' length byte, so a FQDN is 253
	# char max.  We remove 2 char so that a one letter label requested and
	# prepended to the search domain results in a FQDN that is not too long
	return search_domain and len(search_domain) <= 251 and \
	       all((((len(label) <= 63) and domain_label_ok(label))
	            for label in search_domain.split('.')))

__all__ = (
	'split_lexdec', 'unsplit_lexdec', 'sorted_lst_lexdec', 'cmp_lexdec',
	'get_filtered_ifnames', 'get_filtered_phys', 'is_interface_plugged',
	'normalize_mac_address', 'ipv4_from_macaddr', 'macaddr_from_ipv4',
	'parse_ipv4', 'unparse_ipv4', 'mask_ipv4', 'or_ipv4', 'netmask_invert',
	'plausible_netmask', 'plausible_search_domain',
)
