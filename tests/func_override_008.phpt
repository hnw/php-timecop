--TEST--
Function overrideing test for localtime
--SKIPIF--
<?php
$required_func = array("timecop_strtotime", "timecop_freeze");
include(__DIR__."/tests-skipcheck.inc.php");
--INI--
date.timezone=America/Los_Angeles
timecop.func_override=1
--FILE--
<?php
timecop_freeze(timecop_strtotime("2012-02-29 01:23:45"));
$val = localtime();
var_dump(($val[5]+1900).($val[4]+1).$val[3].$val[2].$val[1].$val[0]);
--EXPECT--
string(12) "201222912345"
