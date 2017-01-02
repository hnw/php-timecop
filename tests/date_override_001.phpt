--TEST--
Check for DateTime/TimecopDateTime/TimecopOrigDateTime inheritance when function override is enabled
--SKIPIF--
<?php
$required_func = array("timecop_freeze", "timecop_orig_strtotime");
$required_class = array("TimecopDateTime", "TimecopOrigDateTime");
include(__DIR__."/../tests-skipcheck.inc.php");
--INI--
date.timezone=America/Los_Angeles
timecop.func_override=1
--FILE--
<?php
timecop_freeze(timecop_orig_strtotime("2012-02-29 01:23:45"));

$dt1 = new DateTime();
var_dump(get_class($dt1));
$dt2 = new TimecopDateTime();
var_dump(get_class($dt2));
$dt3 = new TimecopOrigDateTime();
var_dump(get_class($dt3));
var_dump($dt1 instanceof $dt2);
var_dump($dt1 instanceof $dt3);
var_dump($dt2 instanceof $dt1);
var_dump($dt2 instanceof $dt3);
var_dump($dt3 instanceof $dt1);
var_dump($dt3 instanceof $dt2);
--EXPECT--
string(8) "DateTime"
string(15) "TimecopDateTime"
string(19) "TimecopOrigDateTime"
bool(false)
bool(false)
bool(true)
bool(false)
bool(true)
bool(false)
