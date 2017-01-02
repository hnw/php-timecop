--TEST--
Test for timecop_mktime
--SKIPIF--
<?php
$required_func = array("timecop_travel", "timecop_mktime");
include(__DIR__."/../tests-skipcheck.inc.php");
--INI--
date.timezone=America/Los_Angeles
timecop.func_override=0
--FILE--
<?php
timecop_travel(172800);
var_dump(timecop_mktime(16,0,0));
var_dump(timecop_mktime(16,0,0,1,1,1970));
--EXPECT--
int(172800)
int(86400)
