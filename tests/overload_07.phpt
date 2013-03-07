--TEST--
Function overrideing test for getdate
--SKIPIF--
<?php 
	extension_loaded('timecop') or die('skip timecop not available');
	$required_func = array("timecop_strtotime", "timecop_freeze", "getdate", "timecop_orig_getdate");
	foreach ($required_func as $func_name) {
		if (!function_exists($func_name)) {
			die("skip $func_name() function is not available.");
		}
	}
--INI--
date.timezone=America/Los_Angeles
--FILE--
<?php
timecop_freeze(timecop_strtotime("2012-02-29 01:23:45"));
$val = getdate();
var_dump($val['year'].$val['mon'].$val['mday'].$val['hours'].$val['minutes'].$val['seconds']);
--EXPECT--
string(12) "201222912345"
