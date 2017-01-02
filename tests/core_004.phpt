--TEST--
Check for timecop_return()
--SKIPIF--
<?php
$required_func = array("timecop_travel", "timecop_return");
$required_class = array("TimecopDateTime");
$required_method = array();
include(__DIR__."/../tests-skipcheck.inc.php");
--INI--
date.timezone=America/Los_Angeles
timecop.func_override=0
--FILE--
<?php
$dt1 = new TimecopDateTime("+1 week");
timecop_travel($dt1);
$dt2 = new TimecopDateTime();
timecop_return();
$dt3 = new TimecopDateTime();
var_dump($dt2 == $dt3);
var_dump($dt2 < $dt3);
var_dump($dt2 > $dt3);
--EXPECT--
bool(false)
bool(false)
bool(true)
