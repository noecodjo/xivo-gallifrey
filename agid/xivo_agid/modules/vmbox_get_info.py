__version__ = "$Revision$ $Date$"
__license__ = """
    Copyright (C) 2006, 2007, 2008  Proformatique <technique@proformatique.com>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
"""

from xivo_agid import agid
from xivo_agid import objects

def vmbox_get_info(agi, cursor, args):
    vmboxid = agi.get_variable('XIVO_VMBOXID')

    try:
        vmbox = objects.VMBox(agi, cursor, int(vmboxid))
    except (ValueError, LookupError), e:
        agi.dp_break(str(e))

    if vmbox.skipcheckpass:
        agi.set_variable('XIVO_VMOPTIONS', "s")

    agi.set_variable('XIVO_MAILBOX', vmbox.mailbox)
    agi.set_variable('XIVO_MAILBOX_CONTEXT', vmbox.context)

agid.register(vmbox_get_info)
