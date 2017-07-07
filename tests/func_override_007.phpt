--TEST--
Function overrideing test for getdate
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
$val = getdate();
var_dump($val['year'].$val['mon'].$val['mday'].$val['hours'].$val['minutes'].$val['seconds']);
--EXPECT--
string(12) "201222912345"
