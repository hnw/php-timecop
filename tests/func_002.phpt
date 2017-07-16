--TEST--
Test for timecop_mktime()
--SKIPIF--
<?php
$required_func = array("timecop_freeze", "timecop_mktime");
include(__DIR__."/tests-skipcheck.inc.php");
--INI--
date.timezone=America/Los_Angeles
timecop.func_override=0
--FILE--
<?php
timecop_freeze(172840); // "1970-01-02T16:00:40-08:00"
var_dump(timecop_mktime());
var_dump(timecop_mktime(4));
var_dump(timecop_mktime(4,6));
var_dump(timecop_mktime(4,6,41));
var_dump(timecop_mktime(4,6,41,1));
var_dump(timecop_mktime(4,6,41,1,1));
var_dump(timecop_mktime(12,33,20,5,3,1976));
--EXPECTREGEX--
(Strict Standards|Deprecated): timecop_mktime\(\): You should be using the time\(\) function instead in .+[\/\\]tests[\/\\]func_002\.php on line 3
int\(172840\)
int\(129640\)
int\(130000\)
int\(130001\)
int\(130001\)
int\(43601\)
int\(200000000\)
