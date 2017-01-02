--TEST--
Function overrideing test for unixtojd
--SKIPIF--
<?php
$required_func = array("timecop_strtotime", "timecop_freeze", "unixtojd");
include(__DIR__."/../tests-skipcheck.inc.php");
--ENV--
TZ=UTC
--INI--
date.timezone=America/Los_Angeles
timecop.func_override=1
--FILE--
<?php
timecop_freeze(timecop_strtotime("1995-10-08 23:59:59 GMT"));
var_dump(unixtojd());
timecop_freeze(timecop_strtotime("1995-10-09 00:00:00 GMT"));
var_dump(unixtojd());
--EXPECT--
int(2449999)
int(2450000)
