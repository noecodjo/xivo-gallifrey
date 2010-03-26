from __future__ import with_statement
"""commonconf module
"""
__version__ = "$Revision$ $Date$"
__author__  = "Guillaume Bour <gbour@proformatique.com>"
__license__ = """
    Copyright (C) 2010  Proformatique

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

import os, logging, subprocess
from datetime import datetime

from xivo import http_json_server
from xivo.http_json_server import CMD_RW, CMD_R

from xivo_sysconf import helpers, jsondb


class CommonConf(jsondb.JsonDB):
    """
    """
    def __init__(self):
        super(CommonConf, self).__init__()
        self.log = logging.getLogger('xivo_sysconf.modules.commonconf')

        http_json_server.register(self.get      , CMD_RW, safe_init=self.safe_init,
            name='commonconf_get')
        http_json_server.register(self.set      , CMD_RW      , name='commonconf_set')
        http_json_server.register(self.generate , CMD_R       , name='commonconf_generate')
        

        
    def safe_init(self, options):
        super(CommonConf, self).safe_init(options)

        self.file       = options.configuration.get('commonconf', 'commonconf_file')
        self.cmd        = options.configuration.get('commonconf', 'commonconf_cmd')
   
    SECTIONS  = {
        '1. VoIP'       : ['xivo.voip.ifaces', 'xivo.voip.vlan.id'],
        '2. Network'    : [
            'xivo.hostname', 'xivo.domain', 'xivo.net4.ip', 
            'xivo.net4.netmask', 'xivo.net4.broadcast', 'xivo.net4.subnet', 
            'xivo.extra.dns.search', 'xivo.nameservers'
         ],
        '3. DHCP'       : [
            'xivo.dhcp.active', 'xivo.dhcp.pool', 'xivo.dhcp.extra_ifaces'
         ],
        '4. Mail'       : [
            'xivo.smtp.origin', 'xivo.smtp.relayhost', 
            'xivo.smtp.fallback_relayhost', 'xivo.smtp.canonical'
         ],
        '5. Maintenance': ['xivo.maintenance'],
        '6. Alerts'     : ['alert_emails', 'dahdi_monitor_ports', 'max_call_duration'],
    }
    KEYSELECT = ''
    
    ## overriden generators
    def _gen_bool(self, f, key, value):
        value = 1 if value else 0
        f.write("%s=%d\n" % (key, value))
    ## /

    """
        try:
            p = subprocess.Popen([self.cmd])
            ret = p.wait()
        except OSError:
            raise HttpReqError(500, "can't execute '%s'" % self.configexec)

        if ret != 0:
            raise HttpReqError(500, "'%s' process return error %d" % (self.cmd, ret))

        return True
    """
        
        
commonconf = CommonConf()
