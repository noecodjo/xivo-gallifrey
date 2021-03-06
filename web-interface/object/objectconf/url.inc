<?php

#
# XiVO Web-Interface
# Copyright (C) 2006-2010  Proformatique <technique@proformatique.com>
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
#

$array = array();
$array['index'] = 'index.php';

$array['xivo'] = 'xivo/index.php';
$array['xivo/configuration'] = 'xivo/configuration/index.php';
$array['xivo/configuration/manage/user'] = 'xivo/configuration/index.php/manage/user/';
$array['xivo/configuration/manage/entity'] = 'xivo/configuration/index.php/manage/entity/';
$array['xivo/configuration/manage/server'] = 'xivo/configuration/index.php/manage/server/';
$array['xivo/configuration/manage/ldapserver'] = 'xivo/configuration/index.php/manage/ldapserver/';
$array['xivo/configuration/manage/directories'] = 'xivo/configuration/index.php/manage/directories/';
$array['xivo/configuration/manage/accesswebservice'] = 'xivo/configuration/index.php/manage/accesswebservice/';
$array['xivo/configuration/network/interface'] = 'xivo/configuration/index.php/network/interface/';
$array['xivo/configuration/network/iproute'] = 'xivo/configuration/index.php/network/iproute/';
$array['xivo/configuration/network/resolvconf'] = 'xivo/configuration/index.php/network/resolvconf/';
$array['xivo/configuration/network/mail'] = 'xivo/configuration/index.php/network/mail/';
$array['xivo/configuration/network/dhcp'] = 'xivo/configuration/index.php/network/dhcp/';
$array['xivo/configuration/network/ha'] = 'xivo/configuration/index.php/network/ha/';
$array['xivo/configuration/network/ha_stop'] = 'xivo/configuration/index.php/network/ha_stop/';

$array['xivo/configuration/support/xivo'] = 'xivo/configuration/index.php/support/xivo/';
$array['xivo/configuration/support/alerts'] = 'xivo/configuration/index.php/support/alerts/';
$array['xivo/configuration/support/limits'] = 'xivo/configuration/index.php/support/limits/';

$array['xivo/configuration/provisioning/general'] = 'xivo/configuration/index.php/provisioning/general/';

$array['xivo/configuration/controlsystem/commonconf'] = 'xivo/configuration/index.php/controlsystem/commonconf/';
$array['xivo/configuration/controlsystem/ha'] = 'xivo/configuration/index.php/controlsystem/ha/';
$array['xivo/configuration/controlsystem/ha_stop'] = 'xivo/configuration/index.php/controlsystem/ha_stop/';

$array['xivo/preferences'] = 'xivo/preferences.php';
$array['xivo/help'] = 'xivo/help.php';
$array['xivo/contact'] = 'xivo/contact.php';
$array['xivo/logoff'] = 'xivo/logoff.php';

$array['xivo/configuration/json/restricted/manage/entity'] = 'xivo/configuration/json.php/restricted/manage/entity/';
$array['xivo/configuration/json/restricted/network/interface'] = 'xivo/configuration/json.php/restricted/network/interface/';
$array['xivo/configuration/json/restricted/network/iproute'] = 'xivo/configuration/json.php/restricted/network/iproute/';
$array['xivo/configuration/json/restricted/network/resolvconf'] = 'xivo/configuration/json.php/restricted/network/resolvconf/';
$array['xivo/configuration/json/restricted/network/mail'] = 'xivo/configuration/json.php/restricted/network/mail/';
$array['xivo/configuration/json/restricted/network/dhcp'] = 'xivo/configuration/json.php/restricted/network/dhcp/';
$array['xivo/configuration/json/restricted/network/ha'] = 'xivo/configuration/json.php/restricted/network/ha/';
$array['xivo/configuration/json/restricted/network/ha_stop'] = 'xivo/configuration/json.php/restricted/network/ha_stop/';
$array['xivo/configuration/json/restricted/support/monitoring'] = 'xivo/configuration/json.php/restricted/support/monitoring/';
$array['xivo/configuration/json/restricted/provisioning/general'] = 'xivo/configuration/json.php/restricted/provisioning/general/';


$array['xivo/configuration/json/private/manage/entity'] = 'xivo/configuration/json.php/restricted/manage/entity/';
$array['xivo/configuration/json/private/network/interface'] = 'xivo/configuration/json.php/private/network/interface/';
$array['xivo/configuration/json/private/network/iproute'] = 'xivo/configuration/json.php/private/network/iproute/';
$array['xivo/configuration/json/private/network/resolvconf'] = 'xivo/configuration/json.php/private/network/resolvconf/';
$array['xivo/configuration/json/private/network/mail'] = 'xivo/configuration/json.php/private/network/mail/';
$array['xivo/configuration/json/private/network/dhcp'] = 'xivo/configuration/json.php/private/network/dhcp/';
$array['xivo/configuration/json/private/network/ha_stop'] = 'xivo/configuration/json.php/private/network/ha_stop/';
$array['xivo/configuration/json/private/support/monitoring'] = 'xivo/configuration/json.php/private/support/monitoring/';
$array['xivo/configuration/json/private/provisioning/general'] = 'xivo/configuration/json.php/private/provisioning/general/';

$array['service/ipbx'] = 'service/ipbx/index.php';
$array['service/ipbx/general_settings/sip'] = 'service/ipbx/index.php/general_settings/sip/';
$array['service/ipbx/general_settings/iax'] = 'service/ipbx/index.php/general_settings/iax/';
$array['service/ipbx/general_settings/sccp'] = 'service/ipbx/index.php/general_settings/sccp/';
#$array['service/ipbx/general_settings/cti'] = 'service/ipbx/index.php/general_settings/cti/'; // TO BE REMOVED
$array['service/ipbx/general_settings/voicemail'] = 'service/ipbx/index.php/general_settings/voicemail/';
$array['service/ipbx/general_settings/phonebook'] = 'service/ipbx/index.php/general_settings/phonebook/';
$array['service/ipbx/general_settings/advanced'] = 'service/ipbx/index.php/general_settings/advanced/';



$array['service/ipbx/pbx_settings/users'] = 'service/ipbx/index.php/pbx_settings/users/';
$array['service/ipbx/pbx_settings/groups'] = 'service/ipbx/index.php/pbx_settings/groups/';
$array['service/ipbx/pbx_settings/meetme'] = 'service/ipbx/index.php/pbx_settings/meetme/';
$array['service/ipbx/pbx_settings/voicemail'] = 'service/ipbx/index.php/pbx_settings/voicemail/';
$array['service/ipbx/call_management/incall'] = 'service/ipbx/index.php/call_management/incall/';
$array['service/ipbx/call_management/outcall'] = 'service/ipbx/index.php/call_management/outcall/';
$array['service/ipbx/call_management/rightcall'] = 'service/ipbx/index.php/call_management/rightcall/';
$array['service/ipbx/call_management/callfilter'] = 'service/ipbx/index.php/call_management/callfilter/';
$array['service/ipbx/call_management/schedule'] = 'service/ipbx/index.php/call_management/schedule/';
$array['service/ipbx/call_management/voicemenu'] = 'service/ipbx/index.php/call_management/voicemenu/';
$array['service/ipbx/call_management/cdr'] = 'service/ipbx/index.php/call_management/cdr/';
$array['service/ipbx/trunk_management/sip'] = 'service/ipbx/index.php/trunk_management/sip/';
$array['service/ipbx/trunk_management/iax'] = 'service/ipbx/index.php/trunk_management/iax/';
$array['service/ipbx/trunk_management/custom'] = 'service/ipbx/index.php/trunk_management/custom/';
$array['service/ipbx/pbx_services/sounds'] = 'service/ipbx/index.php/pbx_services/sounds/';
$array['service/ipbx/pbx_services/musiconhold'] = 'service/ipbx/index.php/pbx_services/musiconhold/';
$array['service/ipbx/pbx_services/extenfeatures'] = 'service/ipbx/index.php/pbx_services/extenfeatures/';
$array['service/ipbx/pbx_services/handynumbers'] = 'service/ipbx/index.php/pbx_services/handynumbers/';
$array['service/ipbx/pbx_services/phonebook'] = 'service/ipbx/index.php/pbx_services/phonebook/';
$array['service/ipbx/system_management/backupfiles'] = 'service/ipbx/index.php/system_management/backupfiles/';
$array['service/ipbx/system_management/configfiles'] = 'service/ipbx/index.php/system_management/configfiles/';
$array['service/ipbx/system_management/context'] = 'service/ipbx/index.php/system_management/context/';
$array['service/ipbx/system_management/ldapfilter'] = 'service/ipbx/index.php/system_management/ldapfilter/';
$array['service/ipbx/call_center/queues'] = 'service/ipbx/index.php/call_center/queues/';
$array['service/ipbx/call_center/agents'] = 'service/ipbx/index.php/call_center/agents/';
$array['service/ipbx/call_center/queueskills'] = 'service/ipbx/index.php/call_center/queueskills/';
$array['service/ipbx/call_center/queueskillrules'] = 'service/ipbx/index.php/call_center/queueskillrules/';
$array['service/ipbx/control_system/logfiles'] = 'service/ipbx/index.php/control_system/logfiles/';
$array['service/ipbx/control_system/reload'] = 'service/ipbx/index.php/control_system/reload/';
$array['service/ipbx/control_system/restart'] = 'service/ipbx/index.php/control_system/restart/';
$array['service/ipbx/web_services/phonebook/search'] = 'service/ipbx/web_services.php/phonebook/search/';
$array['service/ipbx/web_services/phonebook/menu'] = 'service/ipbx/web_services.php/phonebook/menu/';


$array['service/ipbx/json/phonebook/local'] = 'service/ipbx/json.php/phonebook/local/';


$array['service/ipbx/json/restricted/pbx_settings/users'] = 'service/ipbx/json.php/restricted/pbx_settings/users/';
$array['service/ipbx/json/restricted/pbx_settings/groups'] = 'service/ipbx/json.php/restricted/pbx_settings/groups/';
$array['service/ipbx/json/restricted/pbx_settings/meetme'] = 'service/ipbx/json.php/restricted/pbx_settings/meetme/';
$array['service/ipbx/json/restricted/pbx_settings/voicemail'] = 'service/ipbx/json.php/restricted/pbx_settings/voicemail/';
$array['service/ipbx/json/restricted/call_management/incall'] = 'service/ipbx/json.php/restricted/call_management/incall/';
$array['service/ipbx/json/restricted/call_management/outcall'] = 'service/ipbx/json.php/restricted/call_management/outcall/';
$array['service/ipbx/json/restricted/call_management/cdr'] = 'service/ipbx/json.php/restricted/call_management/cdr/';
$array['service/ipbx/json/restricted/trunk_management/sip'] = 'service/ipbx/json.php/restricted/trunk_management/sip/';
$array['service/ipbx/json/restricted/trunk_management/iax'] = 'service/ipbx/json.php/restricted/trunk_management/iax/';
$array['service/ipbx/json/restricted/trunk_management/custom'] = 'service/ipbx/json.php/restricted/trunk_management/custom/';
$array['service/ipbx/json/restricted/pbx_services/phonebook'] = 'service/ipbx/json.php/restricted/pbx_services/phonebook/';



$array['service/ipbx/json/private/pbx_settings/users'] = 'service/ipbx/json.php/private/pbx_settings/users/';
$array['service/ipbx/json/private/pbx_settings/groups'] = 'service/ipbx/json.php/private/pbx_settings/groups/';
$array['service/ipbx/json/private/pbx_settings/meetme'] = 'service/ipbx/json.php/private/pbx_settings/meetme/';
$array['service/ipbx/json/private/pbx_settings/voicemail'] = 'service/ipbx/json.php/private/pbx_settings/voicemail/';
$array['service/ipbx/json/private/call_management/incall'] = 'service/ipbx/json.php/private/call_management/incall/';

$array['service/ipbx/ui/pbx_settings/users'] = 'service/ipbx/ui.php/pbx_settings/users/';
$array['service/ipbx/ui/pbx_settings/users/search'] = 'service/ipbx/ui.php/pbx_settings/users/search/';
$array['service/ipbx/ui/pbx_settings/users/groups'] = 'service/ipbx/ui.php/pbx_settings/users/groups/';
$array['service/ipbx/ui/pbx_settings/users/groups/search'] = 'service/ipbx/ui.php/pbx_settings/users/groups/search/';
$array['service/ipbx/ui/pbx_settings/users/queues'] = 'service/ipbx/ui.php/pbx_settings/users/queues/';
$array['service/ipbx/ui/pbx_settings/users/queues/search'] = 'service/ipbx/ui.php/pbx_settings/users/queues/search/';
$array['service/ipbx/ui/pbx_settings/users/meetme'] = 'service/ipbx/ui.php/pbx_settings/users/meetme/';
$array['service/ipbx/ui/pbx_settings/users/meetme/search'] = 'service/ipbx/ui.php/pbx_settings/users/meetme/search/';
$array['service/ipbx/ui/pbx_settings/users/voicemail'] = 'service/ipbx/ui.php/pbx_settings/users/voicemail/';
$array['service/ipbx/ui/pbx_settings/users/voicemail/search'] = 'service/ipbx/ui.php/pbx_settings/users/voicemail/search/';
$array['service/ipbx/ui/pbx_settings/users/voicemail/view'] = 'service/ipbx/ui.php/pbx_settings/users/voicemail/view/';
$array['service/ipbx/ui/pbx_settings/groups'] = 'service/ipbx/ui.php/pbx_settings/groups/';
$array['service/ipbx/ui/pbx_settings/meetme'] = 'service/ipbx/ui.php/pbx_settings/meetme/';
$array['service/ipbx/ui/pbx_settings/voicemail'] = 'service/ipbx/ui.php/pbx_settings/voicemail/';

// call center
$array['service/ipbx/json/restricted/call_center/agents'] = 'service/ipbx/json.php/restricted/call_center/agents/';
$array['service/ipbx/json/restricted/call_center/queues'] = 'service/ipbx/json.php/restricted/call_center/queues/';
$array['service/ipbx/json/restricted/call_center/queueskills'] = 'service/ipbx/json.php/restricted/call_center/queueskills/';
$array['service/ipbx/json/restricted/call_center/queueskillrules'] = 'service/ipbx/json.php/restricted/call_center/queueskillrules/';
$array['service/ipbx/json/restricted/call_center/campaigns'] = 'service/ipbx/json.php/restricted/call_center/campaigns/';

$array['service/ipbx/json/private/call_center/agents'] = 'service/ipbx/json.php/private/call_center/agents/';
$array['service/ipbx/json/private/call_center/queues'] = 'service/ipbx/json.php/private/call_center/queues/';
$array['service/ipbx/json/private/call_center/campaigns'] = 'service/ipbx/json.php/private/call_center/campaigns/';

$array['service/ipbx/ui/call_center/agents'] = 'service/ipbx/ui.php/call_center/agents/';
$array['service/ipbx/ui/call_center/queues'] = 'service/ipbx/ui.php/call_center/queues/';

// CTI
$array['cti']                    = 'cti/index.php';
$array['cti/general']            = 'cti/index.php/general';
$array['cti/presences']          = 'cti/index.php/presences/';
$array['cti/phonehints']         = 'cti/index.php/phonehints/';
$array['cti/profiles']           = 'cti/index.php/profiles/';
$array['cti/directories']        = 'cti/index.php/directories/';
$array['cti/reversedirectories'] = 'cti/index.php/reversedirectories/';
$array['cti/displays']           = 'cti/index.php/displays/';
$array['cti/contexts']           = 'cti/index.php/contexts/';
$array['cti/sheetactions']       = 'cti/index.php/sheetactions/';
$array['cti/sheetevents']        = 'cti/index.php/sheetevents/';
$array['cti/restart']            = 'cti/index.php/restart/';


$array['service/ipbx/json/restricted/ctiserver/configuration'] = 'service/ipbx/json.php/restricted/ctiserver/configuration/';
$array['service/ipbx/json/private/ctiserver/configuration']    = 'service/ipbx/json.php/private/ctiserver/configuration/';

$array['service/ipbx/json/private/queuelogger/configuration']  = 'service/ipbx/json.php/private/queuelogger/configuration';

// callcenter
$array['callcenter']                    = 'callcenter/index.php';
$array['callcenter/general']            = 'callcenter/index.php/general';
$array['callcenter/tags']               = 'callcenter/index.php/tags';
$array['callcenter/campaigns']          = 'callcenter/index.php/campaigns';

// munin stats
$array['graphs'] = 'graphs/index.php';
$array['graphs/munin/apache']       = 'graphs/munin/index.php/apache';
$array['graphs/munin/asterisk']     = 'graphs/munin/index.php/asterisk';
$array['graphs/munin/disk']         = 'graphs/munin/index.php/disk';
$array['graphs/munin/time']         = 'graphs/munin/index.php/time';
$array['graphs/munin/postfix']      = 'graphs/munin/index.php/postfix';
$array['graphs/munin/processes']    = 'graphs/munin/index.php/processes';
$array['graphs/munin/network']      = 'graphs/munin/index.php/network';
$array['graphs/munin/system']       = 'graphs/munin/index.php/system';
$array['graphs/munin/other']        = 'graphs/munin/index.php/other';


?>
