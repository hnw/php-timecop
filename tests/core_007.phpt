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
usleep(130000); // 130ms*10=1.3s
$dt2 =new TimecopDateTime();
// ideal virtual time: 1970-01-01 00:00:02.200 GMT

Timecop::scale(20);
usleep(100000); // 100ms*20=2.0s
$dt3 =new TimecopDateTime();
// ideal virtual time: 1970-01-01 00:00:04.200 GMT

echo $dt2->format("Y-m-d H:i:s.u\n");
echo $dt3->format("Y-m-d H:i:s.u\n");
--EXPECTREGEX--
1969-12-31 16:00:02\.\d{6}
1969-12-31 16:00:04\.\d{6}
