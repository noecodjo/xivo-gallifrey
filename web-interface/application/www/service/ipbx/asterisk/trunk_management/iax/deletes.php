<?php

$param['page'] = $page;

if(($arr = xivo_issa_val('peers',$_QR)) === false)
	$_QRY->go($_HTML->url('service/ipbx/trunk_management/iax'),$param);

$generaliax = &$ipbx->get_module('generaliax');
$outcalltrunk = &$ipbx->get_module('outcalltrunk');

$nb = count($arr);

$tfeatures_where = array();
$tfeatures_where['trunk'] = 'iax';

for($i = 0;$i < $nb;$i++)
{
	$tfeatures_where['trunkid'] = strval($arr[$i]);

	if(($info['tfeatures'] = $tfeatures->get_where($tfeatures_where)) === false
	|| $trunkiax->delete($info['tfeatures']['trunkid']) === false)
		continue;

	if($tfeatures->delete($info['tfeatures']['id']) === false)
	{
		$trunkiax->add_origin();
		continue;
	}

	if($info['tfeatures']['registerid'] !== 0
	&& $generaliax->delete($info['tfeatures']['registerid']) === false)
	{
		$trunkiax->add_origin();
		$tfeatures->add_origin();
		continue;
	}

	$outcalltrunk->delete_where(array('trunkid' => $info['tfeatures']['id']));
}

$_QRY->go($_HTML->url('service/ipbx/trunk_management/iax'),$param);

?>
