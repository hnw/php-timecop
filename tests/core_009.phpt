--TEST--
Test for date_timestamp_get() on PHP 5.2
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
var_dump(date_timestamp_get($dt));
--EXPECT--
int(1000)
