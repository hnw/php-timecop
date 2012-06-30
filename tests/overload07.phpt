--TEST--
Class overloading test for datetime
--SKIPIF--
<?php 
	extension_loaded('timecop') or die('skip timecop not available');
	$required_func = array("timecop_freeze", "timecop_orig_strtotime");
	foreach ($required_func as $func_name) {
		if (!function_exists($func_name)) {
			die("skip $func_name() function is not available.");
		}
	}
	$required_class = array("datetime");
	foreach ($required_class as $class_name) {
		if (!class_exists($class_name)) {
			die("skip $class_name class is not available.");
		}
	}
--INI--
timecop.func_overload=1
--FILE--
<?php
timecop_freeze(timecop_orig_strtotime("2012-02-29 01:23:45"));
$dt = new DateTime();
var_dump($dt->format("Y-m-d H:i:s"));
--EXPECT--
string(19) "2012-02-29 01:23:45"
