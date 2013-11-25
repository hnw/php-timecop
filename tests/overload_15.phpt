--TEST--
Function overrideing test for date_create_from_format
--SKIPIF--
<?php
	extension_loaded('timecop') or die('skip timecop not available');
	$required_func = array("date_create_from_format");
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
    if (version_compare(PHP_VERSION, "5.3.4", "<")) die("skip PHP 5.3.4+ required for this test");
--INI--
date.timezone=America/Los_Angeles
--FILE--
<?php

// checking class name of instance
$dt0 = new DateTime();
var_dump(get_class($dt0));
$dt1 = DateTime::createFromFormat('U', 0);
var_dump(get_class($dt1));

$dts = array(
    // constuctor with 2 argument(absolute format)
    date_create_from_format("Y-m-d H:i:s", "2012-03-31 12:34:56"),

    // constuctor with 2 argument(including timezone info)
    date_create_from_format("Y-m-d H:i:s T", "1970-01-01 19:00:00 EST"),

    // constuctor with 2 argument(unix time)
    date_create_from_format("U", "86400"),

    // constuctor with 3 argument
    date_create_from_format("Y-m-d H:i:s", "2012-04-01 00:00:00", new DateTimezone("Asia/Tokyo")),

);

foreach ($dts as $dt) {
    var_dump($dt->format("c"));
    var_dump($dt->getTimezone()->getName());
}

--EXPECT--
string(15) "TimecopDateTime"
string(15) "TimecopDateTime"
string(25) "2012-03-31T12:34:56-07:00"
string(19) "America/Los_Angeles"
string(25) "1970-01-01T19:00:00-05:00"
string(3) "EST"
string(25) "1970-01-02T00:00:00+00:00"
string(6) "+00:00"
string(25) "2012-04-01T00:00:00+09:00"
string(10) "Asia/Tokyo"
