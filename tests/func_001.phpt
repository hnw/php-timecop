--TEST--
Test for timecop_time
--SKIPIF--
<?php
$required_func = array("timecop_freeze", "timecop_time");
include(__DIR__."/tests-skipcheck.inc.php");
--INI--
timecop.func_override=0
--FILE--
<?php
timecop_freeze(0);
var_dump(timecop_time());
--EXPECT--
int(0)
