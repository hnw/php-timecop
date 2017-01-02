--TEST--
Function overrideing test for date
--SKIPIF--
<?php
$required_func = array("timecop_strtotime", "timecop_freeze");
include(__DIR__."/../tests-skipcheck.inc.php");
--INI--
date.timezone=America/Los_Angeles
timecop.func_override=1
--FILE--
<?php
timecop_freeze(timecop_strtotime("2012-02-29"));
var_dump(date("Y-m-d"));
--EXPECT--
string(10) "2012-02-29"
