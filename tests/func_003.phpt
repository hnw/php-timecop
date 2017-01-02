--TEST--
Test for timecop_gmmktime
--SKIPIF--
<?php
$required_func = array("timecop_freeze", "timecop_gmmktime");
include(__DIR__."/../tests-skipcheck.inc.php");
--INI--
timecop.func_override=0
--FILE--
<?php
timecop_freeze(0);
var_dump(timecop_gmmktime(0,0,0));
var_dump(timecop_gmmktime(16,0,0));
var_dump(timecop_gmmktime(16,0,0,1,1,1970));
--EXPECT--
int(0)
int(57600)
int(57600)
