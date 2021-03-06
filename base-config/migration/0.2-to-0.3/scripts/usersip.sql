BEGIN TRANSACTION;

INSERT INTO tmp_usersip (
	id,
	name,
	commented,
	accountcode,
	amaflags,
	callgroup,
	callerid,
	canreinvite,
	context,
	defaultip,
	dtmfmode,
	fromuser,
	fromdomain,
	fullcontact,
	host,
	insecure,
	language,
	mailbox,
	md5secret,
	nat,
	deny,
	permit,
	mask,
	pickupgroup,
	port,
	qualify,
	restrictcid,
	rtptimeout,
	rtpholdtimeout,
	secret,
	type,
	username,
	disallow,
	allow,
	musiconhold,
	regseconds,
	ipaddr,
	regexten,
	cancallforward,
	setvar,
	'call-limit',
	protocol,
	category)
SELECT
	usersip.id,
	usersip.name,
	usersip.commented,
	usersip.accountcode,
	usersip.amaflags,
	usersip.callgroup,
	usersip.callerid,
	usersip.canreinvite,
	usersip.context,
	usersip.defaultip,
	usersip.dtmfmode,
	usersip.fromuser,
	usersip.fromdomain,
	usersip.fullcontact,
	usersip.host,
	usersip.insecure,
	usersip.language,
	usersip.mailbox,
	usersip.md5secret,
	usersip.nat,
	usersip.deny,
	usersip.permit,
	usersip.mask,
	usersip.pickupgroup,
	usersip.port,
	usersip.qualify,
	usersip.restrictcid,
	usersip.rtptimeout,
	usersip.rtpholdtimeout,
	usersip.secret,
	usersip.type,
	usersip.username,
	usersip.disallow,
	usersip.allow,
	usersip.musiconhold,
	usersip.regseconds,
	usersip.ipaddr,
	usersip.regexten,
	usersip.cancallforward,
	usersip.setvar,
	usersip.'call-limit',
	'sip',
	usersip.category
FROM usersip;

COMMIT;
