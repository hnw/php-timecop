--TEST--
Class overrideing test for datetime
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
date.timezone=America/Los_Angeles
--FILE--
<?php
timecop_freeze(timecop_orig_strtotime("2012-02-29 01:23:45"));
$dt = new DateTime();
var_dump($dt->format("c"));
$dt2 = new DateTime();
$dt2->modify("+3days");
var_dump($dt2->format("c"));
$dt2->modify("-1hours -11mins -11secs");
var_dump($dt2->format("c"));
--EXPECT--
string(25) "2012-02-29T01:23:45-08:00"
string(25) "2012-03-03T01:23:45-08:00"
string(25) "2012-03-03T00:12:34-08:00"