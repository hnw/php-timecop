--TEST--
Function overloading test 3
--SKIPIF--
<?php 
	extension_loaded('timecop') or die('skip timecop not available');
	if (!function_exists("time")) {
		die('skip time() function is not available.');
	}
?>
--INI--
timecop.func_overload=1
--FILE--
<?php
$_SERVER['REQUEST_TIME']=timecop_orig_strtotime("2012-02-29");
var_dump(timecop_orig_date("Y-m-d", strtotime("+1 month")));
--EXPECT--
string(10) "2012-03-29"
