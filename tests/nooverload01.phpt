--TEST--
Check for timecop_orig_time presence when timecop.func_override=0
--SKIPIF--
<?php 
	extension_loaded('timecop') or die('skip timecop not available');
	$required_func = array("timecop_time");
	foreach ($required_func as $func_name) {
		if (!function_exists($func_name)) {
			die("skip $func_name() function is not available.");
		}
	}
--INI--
timecop.func_override=0
--FILE--
<?php
var_dump(function_exists("timecop_orig_time"));
--EXPECT--
bool(false)
