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

// checking class name of instance
$dt0 = new DateTime();
var_dump(get_class($dt0));

// constuctor with 0 argument
$dt1 = new DateTime();
var_dump($dt1->format("c"));

// constuctor with 1 argument(absolute format)
$dt2 = new DateTime("2012-03-31 12:34:56");
var_dump($dt2->format("c"));

// constuctor with 1 argument(relative format)
$dt3 = new DateTime("+3days");
var_dump($dt3->format("c"));

// constuctor with 1 argument(including timezone info)
$dt4 = new DateTime("1970-01-01 19:00:00 EST");
var_dump($dt4->format("c"));

// constuctor with 1 argument(unix time)
$dt5 = new DateTime("@86400");
var_dump($dt5->format("c"));

// constuctor with 2 argument
$dt6 = new DateTime("now", new DateTimezone("Asia/Tokyo"));
var_dump($dt6->format("c"));

--EXPECT--
string(15) "TimecopDateTime"
string(25) "2012-02-29T01:23:45-08:00"
string(25) "2012-03-31T12:34:56-07:00"
string(25) "2012-03-03T01:23:45-08:00"
string(25) "1970-01-01T19:00:00-05:00"
string(25) "1970-01-02T00:00:00+00:00"
string(25) "2012-02-29T18:23:45+09:00"