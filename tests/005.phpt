--TEST--
Check for timecop_gmmktime
--SKIPIF--
<?php
	extension_loaded('timecop') or die('skip timecop not available');
	$required_func = array("timecop_freeze", "timecop_gmmktime");
	foreach ($required_func as $func_name) {
		if (!function_exists($func_name)) {
			die("skip $func_name() function is not available.");
		}
	}
?>
--FILE--
<?php
timecop_freeze(0);
var_dump(timecop_gmmktime(0,0,0));
var_dump(timecop_gmmktime(16,0,0));
var_dump(timecop_gmmktime(16,0,0,1,1,1970));
--EXPECT--
int(0)
int(57600)
int(57600)
