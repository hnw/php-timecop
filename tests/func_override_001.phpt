--TEST--
Function overrideing test for time
--SKIPIF--
<?php
$required_func = array("timecop_freeze");
include(__DIR__."/../tests-skipcheck.inc.php");
--INI--
timecop.func_override=1
--FILE--
<?php
timecop_freeze(0);
var_dump(time());
--EXPECT--
int(0)
