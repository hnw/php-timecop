--TEST--
Function overrideing test for gmmktime
--SKIPIF--
<?php
	extension_loaded('timecop') or die('skip timecop not available');
	$required_func = array("timecop_freeze", "gmmktime", "timecop_orig_gmmktime");
	foreach ($required_func as $func_name) {
		if (!function_exists($func_name)) {
			die("skip $func_name() function is not available.");
		}
	}
--INI--
--FILE--
<?php
timecop_freeze(0);
var_dump(gmmktime(0));
--EXPECT--
int(0)
