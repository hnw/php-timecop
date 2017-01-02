--TEST--
Test for timecop_localtime
--SKIPIF--
<?php
$required_func = array("timecop_freeze", "timecop_localtime");
include(__DIR__."/../tests-skipcheck.inc.php");
--INI--
date.timezone=America/Los_Angeles
timecop.func_override=0
--FILE--
<?php
timecop_freeze(new DateTime("2012-02-29 01:23:45"));
$val = timecop_localtime();
var_dump(($val[5]+1900).($val[4]+1).$val[3].$val[2].$val[1].$val[0]);
--EXPECT--
string(12) "201222912345"
