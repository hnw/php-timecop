--TEST--
Check for Timecop::freeze()
--SKIPIF--
<?php
$required_func = array();
$required_class = array("DateTime", "TimecopDateTime");
$required_method = array(array("Timecop", "freeze"));
include("skipcheck.inc.php");
--INI--
date.timezone=America/Los_Angeles
--FILE--
<?php
$dt1 = new DateTime("1970-01-01 00:00:00.000 GMT");
Timecop::freeze($dt1);
$dt2 =new TimecopDateTime();

if (class_exists("DateTimeImmutable")) {
    $dt3 = new DateTimeImmutable("1970-01-01 09:00:00.000 GMT");
    Timecop::freeze($dt3);
    $dt4 =new TimecopDateTime();
} else {
    $dt3 = new DateTime("1970-01-01 01:00:00.000");
    $dt4 = new DateTime("1970-01-01 01:00:00.000");
}

$dt5 = new TimecopDateTime("2040-01-01 00:00:00.000 GMT");
Timecop::freeze($dt5);

if (PHP_INT_SIZE === 8) {
    $dt6 = new TimecopDateTime();
} else {
    // always pass the test on 32bit environment
    $dt6 = $dt5;
    $dt6->setTimeZone(new DateTimezone("America/Los_Angeles"));
}

Timecop::freeze(1);
$dt7 =new TimecopDateTime();

var_dump($dt2->format("Y-m-d H:i:s.u"));
var_dump($dt4->format("Y-m-d H:i:s.u"));
var_dump($dt6->format("Y-m-d H:i:s.u"));
var_dump($dt1 == $dt2);
var_dump($dt3 == $dt4);
var_dump($dt5 == $dt6);
var_dump($dt7->format("Y-m-d H:i:s.u"));
--EXPECT--
string(26) "1969-12-31 16:00:00.000000"
string(26) "1970-01-01 01:00:00.000000"
string(26) "2039-12-31 16:00:00.000000"
bool(true)
bool(true)
bool(true)
string(26) "1969-12-31 16:00:01.000000"
