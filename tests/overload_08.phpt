--TEST--
Function overrideing test for localtime
--SKIPIF--
<?php 
	extension_loaded('timecop') or die('skip timecop not available');
	$required_func = array("timecop_strtotime", "timecop_freeze", "localtime", "timecop_orig_localtime");
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
$val = localtime();
var_dump(($val[5]+1900).($val[4]+1).$val[3].$val[2].$val[1].$val[0]);
--EXPECT--
string(12) "201222912345"
