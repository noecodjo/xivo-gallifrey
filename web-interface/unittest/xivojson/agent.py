# -*- coding: utf-8 -*-
from __future__ import with_statement

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
import unittest, pprint, cjson
from xivojson import *
# define global variables
from globals  import *


class Test02Agent(unittest.TestCase):
    def setUp(self):
        global IP, PORT, SSL, USERNAME, PASSWORD

        self.client = JSONClient(IP, PORT, SSL, USERNAME, PASSWORD)
        self.obj    = 'agents'

    def test_01_user(self):
        (resp, data) = self.client.list(self.obj)
        print resp.status, resp.reason
        self.assertEqual(resp.status, 200)
        
#        pprint.pprint(data)
        data = cjson.decode(data)
        pprint.pprint(data[0])
        count = len(data)
        
        # ADD
        with open('xivojson/agent.json') as f:
            content = cjson.decode(f.read())
            
#        print content
        (resp, data) = self.client.add(self.obj, content)
        print resp.reason, data
        pprint.pprint(data)
        self.assertEqual(resp.status, 200)

