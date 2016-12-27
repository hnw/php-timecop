--TEST--
Check for Timecop::return()
--SKIPIF--
<?php
$required_func = array();
$required_class = array("TimecopDateTime");
$required_method = array(array("Timecop", "travel"), array("Timecop", "return"));
include("skipcheck.inc.php");
--INI--
date.timezone=America/Los_Angeles
--FILE--
<?php
$dt1 = new TimecopDateTime("+1 week");
Timecop::travel($dt1);
$dt2 = new TimecopDateTime();
call_user_func(array("Timecop", "return")); // trick for PHP 5.x
$dt3 = new TimecopDateTime();
var_dump($dt2 == $dt3);
var_dump($dt2 < $dt3);
var_dump($dt2 > $dt3);
--EXPECT--
bool(false)
bool(false)
bool(true)
