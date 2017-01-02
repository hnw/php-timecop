--TEST--
Test for timecop_gmdate
--SKIPIF--
<?php
$required_func = array("timecop_freeze", "timecop_gmdate");
include(__DIR__."/../tests-skipcheck.inc.php");
--INI--
date.timezone=America/Los_Angeles
timecop.func_override=0
--FILE--
<?php
timecop_freeze(strtotime("2012-02-29 01:23:45 GMT"));
var_dump(timecop_gmdate("Y-m-d H:i:s"));
--EXPECT--
string(19) "2012-02-29 01:23:45"
