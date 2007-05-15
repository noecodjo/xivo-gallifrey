# -*- coding: iso-8859-15 -*-
"""Backend support for SQLite for anysql

Copyright (C) 2007, Proformatique

"""

__version__ = "$Revision$ $Date$"
__license__ = """
    Copyright (C) 2007, Proformatique

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
"""

import anysql
import sqlite
import urisup
from urisup import SCHEME, AUTHORITY, PATH, QUERY, FRAGMENT, uri_help_split

def __dict_from_query(query):
	if not query:
		return {}
	return dict(query)

def connect_by_uri(uri):
	puri = urisup.uri_help_split(uri)
	opts = __dict_from_query(puri[QUERY])
	con = sqlite.connect(puri[PATH])
	if "timeout_ms" in opts:
		con.db.sqlite_busy_timeout(int(opts["timeout_ms"]))
	return con

anysql.register_uri_backend('sqlite', connect_by_uri, sqlite)
