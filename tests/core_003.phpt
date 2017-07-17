--TEST--
Check for timecop_scale()
--SKIPIF--
<?php
$required_func = array("timecop_travel", "timecop_scale");
$required_class = array("TimecopDateTime");
$required_method = array();
include(__DIR__."/tests-skipcheck.inc.php");
--INI--
date.timezone=America/Los_Angeles
timecop.func_override=0
--FILE--
<?php
$dt1 = new TimecopDateTime("1970-01-01 00:00:00.900 GMT");
timecop_travel($dt1);
timecop_scale(10);
usleep(130000); // 20ms margin
$dt2 =new TimecopDateTime();

timecop_travel(new TimecopDateTime("1970-01-01 00:00:02 GMT"));
timecop_scale(20);
usleep(120000); // 20ms margin
$dt3 =new TimecopDateTime();
var_dump($dt2->format("Y-m-d H:i:s"));
var_dump($dt3->format("Y-m-d H:i:s"));
--EXPECT--
string(19) "1969-12-31 16:00:02"
string(19) "1969-12-31 16:00:04"
