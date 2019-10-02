--TEST--
Check for issue #43 (Overridden functions ignore declare(strict_types=1))
--SKIPIF--
<?php
$required_version = "7.0";
$required_func = array("strtotime");
include(__DIR__."/tests-skipcheck.inc.php");
--INI--
date.timezone=GMT
timecop.func_override=1
--FILE--
<?php
declare(strict_types=1);

try {
	strtotime(null);
	echo "No error thrown!";
} catch (TypeError $e) {
	echo $e->getMessage();
}
--EXPECT--
timecop_strtotime() expects parameter 1 to be string, null given
