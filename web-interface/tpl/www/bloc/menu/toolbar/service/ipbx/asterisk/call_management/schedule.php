<?php
	$url = &$this->get_module('url');

	$act = $this->vars('act');

	echo $url->href_html($url->img_html('img/menu/top/toolbar/bt-add.gif',$this->bbf('toolbar_opt_add'),'border="0"'),'service/ipbx/call_management/schedule','act=add',null,$this->bbf('toolbar_opt_add'));

	if($act === 'list'):
?>
<div class="sb-advanced-menu">
	<ul id="advanced-menu" onmouseover="xivo_eid('advanced-menu').style.display = 'block';" onmouseout="xivo_eid('advanced-menu').style.display = 'none';">	
		<li><a href="#" onclick="xivo_fm['fm-schedule-list']['act'].value = 'enables'; xivo_fm['fm-schedule-list'].submit();"><?=$this->bbf('toolbar_adv_menu_enable');?></a></li>
		<li><a href="#" onclick="xivo_fm['fm-schedule-list']['act'].value = 'disables'; xivo_fm['fm-schedule-list'].submit();"><?=$this->bbf('toolbar_adv_menu_disable');?></a></li>
		<li><a href="#" onclick="this.tmp = xivo_fm['fm-schedule-list']['act'].value; xivo_fm['fm-schedule-list']['act'].value = 'deletes'; return(confirm('<?=xivo_stript($this->bbf('toolbar_adv_menu_delete_confirm'));?>') ? xivo_fm['fm-schedule-list'].submit() : xivo_fm['fm-schedule-list']['act'] = this.tmp); "><?=$this->bbf('toolbar_adv_menu_delete');?></a></li>
	</ul>
</div><a href="#" onmouseover="xivo_eid('advanced-menu').style.display = 'block';" onmouseout="xivo_eid('advanced-menu').style.display = 'none';"><?=$url->img_html('img/menu/top/toolbar/bt-more.gif',$this->bbf('toolbar_opt_advanced'),'border="0"');?></a>

<?php
	endif;
?>
