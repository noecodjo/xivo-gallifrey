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

#$sysconfd    = &$_XOBJ->get_module('sysconfd');
#$content     = $sysconfd->request_get('/commonconf_apply');
$commonconf    = &$_XOBJ->get_module('commonconf');
$commonconf->generate();
$content     = $commonconf->apply();
$status      = $commonconf->last_status_code();

if($status != 200)
{
    preg_match('/<pre>(.*)<\/pre>/mis', $content, $matches);
    if(count($matches) > 1)
        $content = $matches[1];
}

$content = str_replace("\n", "<br/>\n", $content);

$_TPL->set_var('status',$status);
$_TPL->set_var('info' , $content);

$menu = &$_TPL->get_module('menu');
$menu->set_top('top/user/'.$_USR->get_info('meta'));
$menu->set_left('left/xivo/configuration');
$_TPL->set_bloc('main','xivo/configuration/controlsystem/commonconf');
$_TPL->set_struct('xivo/configuration');
$_TPL->display('index');

?>
