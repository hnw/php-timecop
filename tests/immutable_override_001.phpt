--TEST--
Test for DateTimeImmutable/TimecopDateTimeImmutable/TimecopOrigDateTimeImmutable inheritance when function override is enabled
--SKIPIF--
<?php
$required_version = "5.5";
$required_func = array("timecop_freeze");
$required_class = array("TimecopOrigDateTime", "TimecopDateTimeImmutable", "TimecopOrigDateTimeImmutable");
include(__DIR__."/../tests-skipcheck.inc.php");
--INI--
date.timezone=America/Los_Angeles
timecop.func_override=1
--FILE--
<?php
$dt0 = new TimecopOrigDateTime("2012-02-29 01:23:45");
timecop_freeze($dt0);

$dt1 = new DateTimeImmutable();
var_dump(get_class($dt1));
$dt2 = new TimecopDateTimeImmutable();
var_dump(get_class($dt2));
$dt3 = new TimecopOrigDateTimeImmutable();
var_dump(get_class($dt3));
var_dump($dt2 instanceof $dt1);
var_dump($dt3 instanceof $dt1);
var_dump($dt1 instanceof $dt2);
var_dump($dt3 instanceof $dt2);
var_dump($dt1 instanceof $dt3);
var_dump($dt2 instanceof $dt3);
echo "===\n";
if (version_compare(PHP_VERSION, '5.5.0') >= 0) {
    var_dump($dt1 instanceof DateTimeInterface);
    var_dump($dt2 instanceof DateTimeInterface);
    var_dump($dt3 instanceof DateTimeInterface);
} else {
    // force pass if PHP < 5.5.0
    var_dump(true);
    var_dump(true);
    var_dump(true);
}
echo "===\n";
var_dump($dt1->format("c") === $dt0->format("c"));
var_dump($dt2->format("c") === $dt0->format("c"));
var_dump($dt3->format("c") === $dt0->format("c"));
--EXPECT--
string(17) "DateTimeImmutable"
string(24) "TimecopDateTimeImmutable"
string(28) "TimecopOrigDateTimeImmutable"
bool(true)
bool(true)
bool(false)
bool(false)
bool(false)
bool(false)
===
bool(true)
bool(true)
bool(true)
===
bool(true)
bool(true)
bool(false)
