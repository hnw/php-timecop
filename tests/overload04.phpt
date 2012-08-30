--TEST--
Function overloading test for gmdate
--SKIPIF--
<?php 
	extension_loaded('timecop') or die('skip timecop not available');
	$required_func = array("timecop_strtotime", "timecop_freeze", "gmdate", "timecop_orig_gmdate");
	foreach ($required_func as $func_name) {
		if (!function_exists($func_name)) {
			die("skip $func_name() function is not available.");
		}
	}
--INI--
date.timezone=America/Los_Angeles
--FILE--
<?php
timecop_freeze(timecop_strtotime("2012-02-29 01:23:45 GMT"));
var_dump(gmdate("Y-m-d H:i:s"));
--EXPECT--
string(19) "2012-02-29 01:23:45"
