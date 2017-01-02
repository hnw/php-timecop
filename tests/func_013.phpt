--TEST--
Test for timecop_gettimeofday
--SKIPIF--
<?php
$required_func = array("timecop_freeze", "timecop_travel", "timecop_return", "timecop_microtime");
include(__DIR__."/../tests-skipcheck.inc.php");
--INI--
date.timezone=America/Los_Angeles
timecop.func_override=0
--FILE--
<?php
timecop_freeze(12300000);
$timeofday1 = timecop_gettimeofday();
var_dump($timeofday1['sec']);
var_dump($timeofday1['usec']);
var_dump($timeofday1['minuteswest']);
var_dump($timeofday1['dsttime']);
var_dump(timecop_gettimeofday(true));
timecop_travel(12300000);
$timeofday2 = timecop_gettimeofday();
var_dump($timeofday2['sec']);
var_dump($timeofday2['usec']);
var_dump($timeofday2['minuteswest']);
var_dump($timeofday2['dsttime']);
var_dump(timecop_gettimeofday(true));
timecop_return();
$timeofday3 = timecop_gettimeofday();
var_dump($timeofday3['sec']);
var_dump($timeofday3['usec']);
var_dump($timeofday3['minuteswest']);
var_dump($timeofday3['dsttime']);
var_dump(timecop_gettimeofday(true));
--EXPECTREGEX--
int\(12300000\)
int\(0\)
int\(420\)
int\(1\)
float\(12300000\)
int\(123000\d{2}\)
int\(\d+\)
int\(420\)
int\([01]\)
float\(123000\d{2}(\.\d+)?\)
int\(\d+\)
int\(\d+\)
int\(4[28]0\)
int\([01]\)
float\(\d+(\.\d+)?\)
