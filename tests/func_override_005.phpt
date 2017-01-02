--TEST--
Function overrideing test for gmdate
--SKIPIF--
<?php
$required_func = array("timecop_strtotime", "timecop_freeze");
include(__DIR__."/../tests-skipcheck.inc.php");
--INI--
date.timezone=America/Los_Angeles
timecop.func_override=1
--FILE--
<?php
timecop_freeze(timecop_strtotime("2012-02-29 01:23:45 GMT"));
var_dump(gmdate("Y-m-d H:i:s"));
--EXPECT--
string(19) "2012-02-29 01:23:45"
