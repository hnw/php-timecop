--TEST--
Function overloading test 2
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
$_SERVER['REQUEST_TIME']=strtotime("2012-02-29");
var_dump(date("Y-m-d"));
--EXPECT--
string(10) "2012-02-29"
