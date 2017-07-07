--TEST--
Test for timecop_microtime
--SKIPIF--
<?php
$required_func = array("timecop_freeze", "timecop_travel", "timecop_return", "timecop_microtime");
include(__DIR__."/tests-skipcheck.inc.php");
--INI--
date.timezone=America/Los_Angeles
timecop.func_override=0
--FILE--
<?php
timecop_freeze(172800);
var_dump(timecop_microtime());
var_dump(timecop_microtime(true));
timecop_travel(172800);
var_dump(timecop_microtime());
var_dump(timecop_microtime(true));
timecop_return();
var_dump(timecop_microtime());
var_dump(timecop_microtime(true));
--EXPECTREGEX--
string\(17\) "0.00000000 172800"
float\(172800\)
string\(17\) "0.\d{8} 1728\d{2}"
float\(1728\d{2}.*\)
string\(21\) "0.\d{8} \d{10}"
float\(\d{10}.*\)
