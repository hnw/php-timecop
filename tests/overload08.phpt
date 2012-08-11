--TEST--
Function overloading test for idate
--SKIPIF--
<?php 
	extension_loaded('timecop') or die('skip timecop not available');
	$required_func = array("timecop_strtotime", "timecop_freeze", "idate", "timecop_orig_idate");
	foreach ($required_func as $func_name) {
		if (!function_exists($func_name)) {
			die("skip $func_name() function is not available.");
		}
	}
--INI--
timecop.func_overload=1
--FILE--
<?php
timecop_freeze(timecop_strtotime("2012-02-29 01:23:45"));
var_dump(idate("Y").idate("m").idate("d").idate("H").idate("i").idate("s"));
--EXPECT--
string(12) "201222912345"
