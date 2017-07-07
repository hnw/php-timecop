--TEST--
Test for timecop_strtotime
--SKIPIF--
<?php
$required_func = array("timecop_freeze", "timecop_strtotime");
include(__DIR__."/tests-skipcheck.inc.php");
--INI--
date.timezone=America/Los_Angeles
timecop.func_override=0
--FILE--
<?php
timecop_freeze(new DateTime("2012-02-29"));
var_dump(date("Y-m-d", timecop_strtotime("+1 month")));
--EXPECT--
string(10) "2012-03-29"
