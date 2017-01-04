--TEST--
Test for date_timestamp_set() on PHP 5.2
--SKIPIF--
<?php
if (version_compare(PHP_VERSION, '5.3.0') >= 0) {
    die("skip this test is for PHP 5.2.x");
}
$required_class = array("timecopdatetime");
include(__DIR__."/../tests-skipcheck.inc.php");
--INI--
date.timezone=America/Los_Angeles
--FILE--
<?php
$dt = new TimecopDateTime("@1000");
date_timestamp_set($dt, 2000);
var_dump($dt->format("U"));
--EXPECT--
string(4) "2000"
