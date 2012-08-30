--TEST--
Function overloading test for strftime
--SKIPIF--
<?php 
	extension_loaded('timecop') or die('skip timecop not available');
	$required_func = array("timecop_strtotime", "timecop_freeze", "strftime", "timecop_orig_strftime");
	foreach ($required_func as $func_name) {
		if (!function_exists($func_name)) {
			die("skip $func_name() function is not available.");
		}
	}
--INI--
date.timezone=America/Los_Angeles
--FILE--
<?php
timecop_freeze(timecop_strtotime("2012-02-29 01:23:45"));
var_dump(strftime("%Y-%m-%d %H:%M:%S"));
--EXPECT--
string(19) "2012-02-29 01:23:45"
