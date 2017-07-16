--TEST--
Check for Timecop::scale()
--SKIPIF--
<?php
$required_func = array();
$required_class = array("TimecopDateTime");
$required_method = array(array("Timecop", "travel"), array("Timecop", "scale"));
include(__DIR__."/tests-skipcheck.inc.php");
--INI--
date.timezone=America/Los_Angeles
--FILE--
<?php
$dt1 = new TimecopDateTime("1970-01-01 00:00:00.900 GMT");
Timecop::travel($dt1);
Timecop::scale(10);
usleep(111000); // 1ms margin
$dt2 =new TimecopDateTime();
Timecop::scale(20);
usleep(101000); // 1ms margin
$dt3 =new TimecopDateTime();
var_dump($dt2->format("Y-m-d H:i:s"));
var_dump($dt3->format("Y-m-d H:i:s"));
--EXPECT--
string(19) "1969-12-31 16:00:02"
string(19) "1969-12-31 16:00:04"
