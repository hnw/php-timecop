--TEST--
Check for timecop_freeze
--SKIPIF--
<?php 
	extension_loaded('timecop') or die('skip timecop not available');
	$required_func = array("timecop_freeze", "timecop_time");
	foreach ($required_func as $func_name) {
		if (!function_exists($func_name)) {
			die("skip $func_name() function is not available.");
		}
	}
?>
--FILE--
<?php
timecop_freeze(123);
sleep(1);
var_dump(timecop_time());
--EXPECT--
int(123)
