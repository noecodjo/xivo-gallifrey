<?php

#
# XiVO Web-Interface
# Copyright (C) 2009-2010  Proformatique <technique@proformatique.com>
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

$array['accesswebservice'] = array();
$array['accesswebservice']['table'] = 'accesswebservice';
$array['accesswebservice']['primary'] = array('id' => '');

$array['entity'] = array();
$array['entity']['table'] = 'entity';
$array['entity']['primary'] = array('id' => '');

$array['ha'] = array();
$array['ha']['table'] = 'ha';
$array['ha']['primary'] = array('id' => '');

$array['ha_uname'] = array();
$array['ha_uname']['table'] = 'ha_uname_node';
$array['ha_uname']['primary'] = array('id' => '');

$array['ha_ping'] = array();
$array['ha_ping']['table'] = 'ha_ping_ipaddr';
$array['ha_ping']['primary'] = array('id' => '');

$array['ha_virtnet'] = array();
$array['ha_virtnet']['table'] = 'ha_virtual_network';
$array['ha_virtnet']['primary'] = array('id' => '');

$array['ha_peer'] = array();
$array['ha_peer']['table'] = 'ha_peer';
$array['ha_peer']['primary'] = array('id' => '');

$array['iproute'] = array();
$array['iproute']['table'] = 'iproute';
$array['iproute']['primary'] = array('id' => '');

$array['ldapserver'] = array();
$array['ldapserver']['table'] = 'ldapserver';
$array['ldapserver']['primary'] = array('id' => '');

$array['mail'] = array();
$array['mail']['table'] = 'mail';
$array['mail']['primary'] = array('id' => '');

$array['monitoring'] = array();
$array['monitoring']['table'] = 'monitoring';
$array['monitoring']['primary'] = array('id' => '');

$array['netiface'] = array();
$array['netiface']['table'] = 'netiface';
$array['netiface']['primary'] = array('name' => '');

$array['resolvconf'] = array();
$array['resolvconf']['table'] = 'resolvconf';
$array['resolvconf']['primary'] = array('id' => '');

$array['server'] = array();
$array['server']['table'] = 'server';
$array['server']['primary'] = array('id' => '');

$array['dhcp'] = array();
$array['dhcp']['table'] = 'dhcp';
$array['dhcp']['primary'] = array('id' => '');

$array['provisioning'] = array();
$array['provisioning']['table'] = 'provisioning';
$array['provisioning']['primary'] = array('id' => '');

?>
