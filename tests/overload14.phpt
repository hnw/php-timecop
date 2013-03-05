--TEST--
Function overrideing test for date_create
--SKIPIF--
<?php 
	extension_loaded('timecop') or die('skip timecop not available');
	$required_func = array("timecop_freeze", "timecop_orig_strtotime", "date_create");
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

// constuctor with 0 argument
$dt = date_create();
var_dump($dt->format("c"));

// constuctor with 1 argument
$dt2 = date_create("+3days");
var_dump($dt2->format("c"));
--EXPECT--
string(25) "2012-02-29T01:23:45-08:00"
string(25) "2012-03-03T01:23:45-08:00"
