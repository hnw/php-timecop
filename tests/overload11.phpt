--TEST--
Function overloading test for unixtojd
--SKIPIF--
<?php 
	extension_loaded('timecop') or die('skip timecop not available');
	$required_func = array("timecop_strtotime", "timecop_freeze", "unixtojd", "timecop_orig_unixtojd");
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
var_dump(unixtojd());
--EXPECT--
int(2455987)
