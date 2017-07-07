--TEST--
Function overrideing test for gmstrftime
--SKIPIF--
<?php
$required_func = array("timecop_strtotime", "timecop_freeze");
include(__DIR__."/tests-skipcheck.inc.php");
--INI--
date.timezone=America/Los_Angeles
timecop.func_override=1
--FILE--
<?php
timecop_freeze(timecop_strtotime("2012-02-29 01:23:45 GMT"));
var_dump(gmstrftime("%Y-%m-%d %H:%M:%S"));
--EXPECT--
string(19) "2012-02-29 01:23:45"
