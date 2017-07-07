--TEST--
Test for timecop_strftime
--SKIPIF--
<?php
$required_func = array("timecop_freeze", "timecop_strftime");
include(__DIR__."/tests-skipcheck.inc.php");
--INI--
date.timezone=America/Los_Angeles
timecop.func_override=0
--FILE--
<?php
timecop_freeze(strtotime("2012-02-29 01:23:45"));
var_dump(timecop_strftime("%Y-%m-%d %H:%M:%S"));
--EXPECT--
string(19) "2012-02-29 01:23:45"
