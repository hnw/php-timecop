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
usleep(130000); // 130ms*10=1.3s
$dt2 =new TimecopDateTime();
// ideal virtual time: 1970-01-01 00:00:02.200 GMT

timecop_scale(20);
usleep(100000); // 100ms*20=2.0s
$dt3 =new TimecopDateTime();
// ideal virtual time: 1970-01-01 00:00:04.200 GMT

echo $dt2->format("Y-m-d H:i:s.u\n");
echo $dt3->format("Y-m-d H:i:s.u\n");
--EXPECTREGEX--
1969-12-31 16:00:02\.\d{6}
1969-12-31 16:00:04\.\d{6}
