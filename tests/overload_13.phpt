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

// constuctor with 0 argument
$dt = new DateTime();
var_dump($dt->format("c"));

// constuctor with 1 argument
$dt2 = new DateTime("+3days");
var_dump($dt2->format("c"));

// 1st argument of constuctor includes timezone info
$dt3 = new DateTime("1970-01-01 19:00:00 EST");
var_dump($dt3->format("c"));

// 1st argument of constuctor is "unix time"
$dt4 = new DateTime("@86400");
var_dump($dt4->format("c"));

// traveling to the past
// constuctor with 2 arguments
$dt5 = new DateTime("now", new DateTimezone("Asia/Tokyo"));
var_dump($dt5->format("c"));

timecop_freeze(timecop_orig_strtotime("2037-01-01 00:00:00"));

// traveling to the future
$dt6 = new DateTime("now");
var_dump($dt6->format("c"));

--EXPECT--
string(25) "2012-02-29T01:23:45-08:00"
string(25) "2012-03-03T01:23:45-08:00"
string(25) "1970-01-01T19:00:00-05:00"
string(25) "1970-01-02T00:00:00+00:00"
string(25) "2012-02-29T18:23:45+09:00"
string(25) "2037-01-01T00:00:00-08:00"