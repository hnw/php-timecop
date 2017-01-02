--TEST--
Test for timecop_unixtojd
--SKIPIF--
<?php
$required_func = array("timecop_freeze", "unixtojd", "timecop_unixtojd");
include(__DIR__."/../tests-skipcheck.inc.php");
--ENV--
TZ=UTC
--INI--
date.timezone=America/Los_Angeles
timecop.func_override=0
--FILE--
<?php
timecop_freeze(strtotime("1995-10-08 23:59:59 GMT"));
var_dump(timecop_unixtojd());
timecop_freeze(strtotime("1995-10-09 00:00:00 GMT"));
var_dump(timecop_unixtojd());
--EXPECT--
int(2449999)
int(2450000)
