--TEST--
Test for DateTime/TimecopDateTime/TimecopOrigDateTime inheritance
--SKIPIF--
<?php
$required_func = array("timecop_freeze");
$required_class = array("TimecopDateTime", "TimecopOrigDateTime");
include(__DIR__."/../tests-skipcheck.inc.php");
--INI--
date.timezone=America/Los_Angeles
timecop.func_override=0
--FILE--
<?php
$freezed_time = "2017-01-03T20:05:06-08:00";
timecop_freeze(strtotime($freezed_time));

$dt1 = new DateTime();
var_dump(get_class($dt1));
$dt2 = new TimecopDateTime();
var_dump(get_class($dt2));
$dt3 = new TimecopOrigDateTime();
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
var_dump($dt1->format("c") === $freezed_time);
var_dump($dt2->format("c") === $freezed_time);
var_dump($dt3->format("c") === $freezed_time);
--EXPECT--
string(8) "DateTime"
string(15) "TimecopDateTime"
string(19) "TimecopOrigDateTime"
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
bool(false)
bool(true)
bool(false)
