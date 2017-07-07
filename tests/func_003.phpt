--TEST--
Test for timecop_gmmktime()
--SKIPIF--
<?php
$required_func = array("timecop_freeze", "timecop_gmmktime");
include(__DIR__."/tests-skipcheck.inc.php");
--INI--
date.timezone=America/Los_Angeles
timecop.func_override=0
--FILE--
<?php
timecop_freeze(86440); // "1970-01-02T00:00:40+00:00"
var_dump(timecop_gmmktime());
var_dump(timecop_gmmktime(12));
var_dump(timecop_gmmktime(12,6));
var_dump(timecop_gmmktime(12,6,41));
var_dump(timecop_gmmktime(12,6,41,1));
var_dump(timecop_gmmktime(12,6,41,1,1));
var_dump(timecop_gmmktime(19,33,20,5,3,1976));
--EXPECTREGEX--
(Strict Standards|Deprecated): timecop_gmmktime\(\): You should be using the time\(\) function instead in [^ ]+\/tests\/func_003\.php on line 3
int\(86440\)
int\(129640\)
int\(130000\)
int\(130001\)
int\(130001\)
int\(43601\)
int\(200000000\)
