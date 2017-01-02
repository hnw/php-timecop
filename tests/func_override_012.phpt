--TEST--
Function overrideing test for microtime
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
var_dump(microtime());
var_dump(microtime(true));
timecop_travel(new TimecopOrigDateTime("1970-01-01 00:02:03.456 GMT"));
var_dump(microtime());
var_dump(microtime(true));
timecop_return();
var_dump(microtime());
var_dump(microtime(true));
--EXPECTREGEX--
string\(14\) "0\.45600000 123"
float\(123\.456\)
string\(14\) "0\.[45]\d{7} 123"
float\(123\.[45]\d+\)
string\(21\) "0.\d{8} \d{10}"
float\(\d{10}.*\)
