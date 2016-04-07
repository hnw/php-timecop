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

$dts = array(
    // constuctor with 0 argument
    new DateTime(),

    // constuctor with 1 argument(null)
    new DateTime(NULL),

    // constuctor with 1 argument(false)
    new DateTime(false),

    // constuctor with 1 argument(empty string)
    new DateTime(""),

    // constuctor with 1 argument(absolute format)
    new DateTime("2012-03-31 12:34:56"),

    // constuctor with 1 argument(relative format)
    new DateTime("+3days"),

    // constuctor with 1 argument(including timezone info)
    new DateTime("1970-01-01 19:00:00 EST"),

    // constuctor with 1 argument(unix time)
    new DateTime("@86400"),

    // constuctor with 2 argument
    new DateTime("now", new DateTimezone("Asia/Tokyo")),
);

foreach ($dts as $dt) {
    var_dump($dt->format("c"));
}

--EXPECT--
string(8) "DateTime"
string(25) "2012-02-29T01:23:45-08:00"
string(25) "2012-02-29T01:23:45-08:00"
string(25) "2012-02-29T01:23:45-08:00"
string(25) "2012-02-29T01:23:45-08:00"
string(25) "2012-03-31T12:34:56-07:00"
string(25) "2012-03-03T01:23:45-08:00"
string(25) "1970-01-01T19:00:00-05:00"
string(25) "1970-01-02T00:00:00+00:00"
string(25) "2012-02-29T18:23:45+09:00"
