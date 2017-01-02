--TEST--
Function overrideing test for mktime
--SKIPIF--
<?php
$required_func = array("timecop_freeze");
include(__DIR__."/../tests-skipcheck.inc.php");
--INI--
date.timezone=America/Los_Angeles
timecop.func_override=1
--FILE--
<?php
timecop_freeze(0);
var_dump(mktime(0));
--EXPECT--
int(-57600)
