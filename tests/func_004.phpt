--TEST--
Test for timecop_date
--SKIPIF--
<?php
$required_func = array("timecop_freeze", "timecop_date");
include(__DIR__."/../tests-skipcheck.inc.php");
--INI--
date.timezone=America/Los_Angeles
timecop.func_override=0
--FILE--
<?php
timecop_freeze(strtotime("2012-02-29"));
var_dump(timecop_date("Y-m-d"));
--EXPECT--
string(10) "2012-02-29"
