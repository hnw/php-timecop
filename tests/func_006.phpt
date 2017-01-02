--TEST--
Test for timecop_idate
--SKIPIF--
<?php
$required_func = array("timecop_freeze", "timecop_idate");
include(__DIR__."/../tests-skipcheck.inc.php");
--INI--
date.timezone=America/Los_Angeles
timecop.func_override=0
--FILE--
<?php
timecop_freeze(strtotime("2012-02-29 01:23:45"));
var_dump(timecop_idate("Y").timecop_idate("m").timecop_idate("d").timecop_idate("H").timecop_idate("i").timecop_idate("s"));
--EXPECT--
string(12) "201222912345"
