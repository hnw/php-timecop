--TEST--
Check for clone TimecopDateTime instance
--SKIPIF--
<?php
$required_class = array("timecopdatetime");
include(__DIR__."/tests-skipcheck.inc.php");
--INI--
date.timezone=America/Los_Angeles
timecop.func_override=0
--FILE--
<?php
$dt1 = new TimecopDateTime("2012-01-01 12:00:00 GMT");
$dt2 = clone $dt1;
var_dump($dt2->format("c"));
--EXPECT--
string(25) "2012-01-01T12:00:00+00:00"
