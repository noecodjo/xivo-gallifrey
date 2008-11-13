<?php

if(isset($_SERVER['REMOTE_ADDR']) === false
|| $_SERVER['REMOTE_ADDR'] !== '127.0.0.1')
	xivo_die('Error/403');

$appincall = &$ipbx->get_application('incall',null,false);

switch($_QRY->get_qs('act'))
{
	case 'list':
	default:
		if(($incall = $appincall->get_incalls_list()) === false)
			xivo_die('no-data');

		$_HTML->set_var('incall',$incall);
		$_HTML->set_var('sum',$_QRY->get_qs('sum'));
		$_HTML->display('/service/ipbx/'.$ipbx->get_name().'/call_management/incall');
}

?>
