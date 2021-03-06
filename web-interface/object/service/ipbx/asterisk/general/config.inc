<?php

#
# XiVO Web-Interface
# Copyright (C) 2010  Proformatique <technique@proformatique.com>
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

$array['element'] = array();

$array['element']['timezone'] = array();
$array['element']['timezone']['default'] = '';

$array['element']['externip'] = array();
$array['element']['externip']['default'] = '';

dwho::load_class('dwho_network');

$array['filter'] = array();
$array['filter']['timezone']         = array('set' => false, 'chk' => 2);
$array['filter']['externip'] = array('set'		=> false,
				       'chk'		=> 2,
				       'callback'	=> 'dwho_network::valid_host',
				       'callbackarg'	=> array((DWHO_NETWORK_HOST_IPV4_DOTDEC |
								  DWHO_NETWORK_HOST_HOSTNAME)));

?>
