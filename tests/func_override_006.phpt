--TEST--
Function overrideing test for idate
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
var_dump(idate("Y").idate("m").idate("d").idate("H").idate("i").idate("s"));
--EXPECT--
string(12) "201222912345"
