--TEST--
Check for compareing TimecopDateTimeImmutable instance
--SKIPIF--
<?php
$required_version = "5.5";
$required_class = array("TimecopDateTimeImmutable");
require(__DIR__."/../tests-skipcheck.inc.php");
--INI--
date.timezone=America/Los_Angeles
timecop.func_override=0
--FILE--
<?php
$dt1 = new TimecopDateTimeImmutable("2012-01-02 12:00:00 JST");
$dt2 = new TimecopDateTimeImmutable("2012-01-02 12:00:00", new DateTimezone("Asia/Tokyo"));
var_dump($dt1 == $dt2);
$dt3 = new TimecopDateTimeImmutable("2012-01-01 23:00:00 EST");
var_dump($dt1 < $dt3);
--EXPECT--
bool(true)
bool(true)
