--TEST--
Function overloading test for date
--SKIPIF--
<?php 
	extension_loaded('timecop') or die('skip timecop not available');
	$required_func = array("timecop_strtotime", "timecop_freeze", "date", "timecop_orig_date");
	foreach ($required_func as $func_name) {
		if (!function_exists($func_name)) {
			die("skip $func_name() function is not available.");
		}
	}
--INI--
timecop.func_overload=1
--FILE--
<?php
$_SERVER['REQUEST_TIME']=strtotime("2012-02-29");
var_dump(date("Y-m-d"));
--EXPECT--
string(10) "2012-02-29"
