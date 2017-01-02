--TEST--
Function overrideing test for gettimeofday
--SKIPIF--
<?php
$required_func = array("timecop_freeze", "timecop_travel", "timecop_return");
$required_class = array("TimecopOrigDateTime");
include(__DIR__."/../tests-skipcheck.inc.php");
--INI--
date.timezone=America/Los_Angeles
timecop.func_override=1
--FILE--
<?php
timecop_freeze(new TimecopOrigDateTime("1970-01-01 00:02:03.456 GMT"));
$timeofday1 = gettimeofday();
var_dump($timeofday1['sec']);
var_dump($timeofday1['usec']);
var_dump($timeofday1['minuteswest']);
var_dump($timeofday1['dsttime']);
var_dump(gettimeofday(true));
timecop_freeze(new TimecopOrigDateTime("1970-01-01 00:02:03.456 GMT"));
$timeofday2 = gettimeofday();
var_dump($timeofday2['sec']);
var_dump($timeofday2['usec']);
var_dump($timeofday2['minuteswest']);
var_dump($timeofday2['dsttime']);
var_dump(gettimeofday(true));
timecop_return();
$timeofday3 = gettimeofday();
var_dump($timeofday3['sec']);
var_dump($timeofday3['usec']);
var_dump($timeofday3['minuteswest']);
var_dump($timeofday3['dsttime']);
var_dump(gettimeofday(true));
--EXPECTREGEX--
int\(123\)
int\(456000\)
int\(480\)
int\(0\)
float\(123\.456\)
int\(123\)
int\([45]\d+\)
int\(480\)
int\([01]\)
float\(123\.[45]\d+\)
int\(\d+\)
int\(\d+\)
int\(4[28]0\)
int\([01]\)
float\(\d+(\.\d+)?\)
