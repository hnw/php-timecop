--TEST--
Check for timecop_date_create_immutable_from_format
--SKIPIF--
<?php
$required_version = "5.5";
$required_func = array("timecop_date_create_immutable_from_format");
include(__DIR__."/../tests-skipcheck.inc.php");
--INI--
date.timezone=America/Los_Angeles
timecop.func_override=0
--FILE--
<?php
// checking class name of instance
$dt0 = timecop_date_create_immutable_from_format("","");
var_dump(get_class($dt0));

$dts = array(
    // constuctor with 2 argument(absolute format)
    timecop_date_create_immutable_from_format("Y-m-d H:i:s", "2012-03-31 12:34:56"),

    // constuctor with 2 argument(including timezone info)
    timecop_date_create_immutable_from_format("Y-m-d H:i:s T", "1970-01-01 19:00:00 EST"),

    // constuctor with 2 argument(unix time)
    timecop_date_create_immutable_from_format("U", "86400"),

    // constuctor with 3 argument
    timecop_date_create_immutable_from_format("Y-m-d H:i:s", "2012-04-01 00:00:00", new DateTimezone("Asia/Tokyo")),

);

foreach ($dts as $dt) {
    var_dump($dt->format("c"));
    var_dump($dt->getTimezone()->getName());
}

--EXPECT--
string(17) "DateTimeImmutable"
string(25) "2012-03-31T12:34:56-07:00"
string(19) "America/Los_Angeles"
string(25) "1970-01-01T19:00:00-05:00"
string(3) "EST"
string(25) "1970-01-02T00:00:00+00:00"
string(6) "+00:00"
string(25) "2012-04-01T00:00:00+09:00"
string(10) "Asia/Tokyo"
