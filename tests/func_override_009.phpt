--TEST--
Function overrideing test for strtotime
--SKIPIF--
<?php
$required_func = array("timecop_freeze", "timecop_strtotime", "timecop_orig_date");
$required_class = array("TimecopOrigDateTime");
include(__DIR__."/../tests-skipcheck.inc.php");
--INI--
date.timezone=America/Los_Angeles
timecop.func_override=1
--FILE--
<?php
timecop_freeze(new TimecopOrigDateTime("2012-02-29"));
var_dump(timecop_orig_date("Y-m-d", strtotime("+1 month")));
--EXPECT--
string(10) "2012-03-29"
