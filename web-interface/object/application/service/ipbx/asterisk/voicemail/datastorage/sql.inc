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

require_once(dwho_file::joinpath(XIVO_PATH_OBJECT,'application','abstract','datastorage','sql.inc'));

class xivo_application_service_asterisk_voicemail_sql extends xivo_application_abstract_datastorage_sql
{
	var $_table	= array('voicemail'	=> '');

	function search($value,$disable=null,$order=null,$limit=null)
	{
		$this->_cnt = 0;

		if(dwho_has_len($value) === false)
			return(false);

		$this->_dso->new_select($this->_table['voicemail']);

		$this->_dso->wherecontain('fullname',$value,true,$this->_table['voicemail']);
		$this->_dso->orwherecontain('mailbox',$value,true,$this->_table['voicemail']);
		$this->_dso->orwherecontain('email',$value,true,$this->_table['voicemail']);

		$this->_dso->new_order($order,null,$this->_table['voicemail']);
		$this->_dso->limit($limit);

		$r = $this->_dso->select_all();

		$this->_dso->reset_order();
		$this->_dso->reset_limit();

		if(($this->_cnt = $this->_dso->select_count(null,'uniqueid',$this->_table['voicemail'])) === false
		|| isset($r[0]) === false)
			$r = false;

		return($r);
	}
}

?>
