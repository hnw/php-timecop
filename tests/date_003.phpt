--TEST--
Test for serialize/unserialize TimecopDateTime instance
--SKIPIF--
<?php
$required_version = "5.3"; // See bug #39821 (Unable to Serialize and then Unserialize a PHP DateTime Object)
$required_class = array("TimecopDateTime");
include(__DIR__."/../tests-skipcheck.inc.php");
--INI--
date.timezone=America/Los_Angeles
timecop.func_override=0
--FILE--
<?php
timecop_freeze(123);
$dt1 = new DateTime("2012-01-01 12:00:00 GMT");
$dt2 = unserialize(serialize($dt1));
var_dump($dt1->format("c"));
var_dump($dt2->format("c"));
--EXPECT--
string(25) "2012-01-01T12:00:00+00:00"
string(25) "2012-01-01T12:00:00+00:00"
