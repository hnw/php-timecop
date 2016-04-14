--TEST--
Check for timecop_mktime
--SKIPIF--
<?php
	extension_loaded('timecop') or die('skip timecop not available');
	$required_func = array("timecop_freeze", "timecop_freeze", "timecop_mktime");
	foreach ($required_func as $func_name) {
		if (!function_exists($func_name)) {
			die("skip $func_name() function is not available.");
		}
	}
?>
--INI--
date.timezone=America/Los_Angeles
--FILE--
<?php
timecop_travel(172800);
var_dump(timecop_mktime(16,0,0));
var_dump(timecop_mktime(16,0,0,1,1,1970));
--EXPECT--
int(172800)
int(86400)
