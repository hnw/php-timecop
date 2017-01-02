--TEST--
Check for timecop_travel()
--SKIPIF--
<?php
$required_func = array("timecop_travel");
$required_class = array("DateTime", "TimecopDateTime");
$required_method = array();
include(__DIR__."/../tests-skipcheck.inc.php");
--INI--
date.timezone=America/Los_Angeles
timecop.func_override=0
--FILE--
<?php
$dt1 = new DateTime("1970-01-01 00:00:00.900 GMT");
timecop_travel($dt1);
usleep(100000);
$dt2 =new TimecopDateTime();

if (class_exists("DateTimeImmutable")) {
    $dt3 = new DateTimeImmutable("1970-01-01 09:00:00.900 GMT");
    timecop_travel($dt3);
    usleep(100000);
    $dt4 =new TimecopDateTime();
} else {
    $dt3 = new DateTime("1970-01-01 01:00:00.900");
    $dt4 = new DateTime("1970-01-01 01:00:01.050");
}

$dt5 = new TimecopDateTime("2040-01-01 00:00:00.900 GMT");
timecop_travel($dt5);
usleep(100000);
if (PHP_INT_SIZE === 8) {
    $dt6 =new TimecopDateTime();
} else {
    // always pass the test on 32bit environment
    $dt6 = clone $dt5;
    $dt6->modify("+1second");
    $dt6->setTimeZone(new DateTimezone("America/Los_Angeles"));
}

timecop_travel(1);
sleep(1);
$dt7 =new TimecopDateTime();

var_dump($dt2->format("Y-m-d H:i:s"));
var_dump($dt4->format("Y-m-d H:i:s"));
var_dump($dt6->format("Y-m-d H:i:s"));
var_dump($dt1 < $dt2);
var_dump($dt3 < $dt4);
var_dump($dt5 < $dt6);
var_dump($dt7->format("Y-m-d H:i:s"));
--EXPECT--
string(19) "1969-12-31 16:00:01"
string(19) "1970-01-01 01:00:01"
string(19) "2039-12-31 16:00:01"
bool(true)
bool(true)
bool(true)
string(19) "1969-12-31 16:00:02"
