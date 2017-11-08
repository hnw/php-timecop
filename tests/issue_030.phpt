--TEST--
Check for issue #30 (DateTimeImmutable::createFromFormat with ! returns DateTime)
--SKIPIF--
<?php
$required_version = "5.5";
$required_func = array("timecop_freeze");
$required_class = array("TimecopDateTimeImmutable");
include(__DIR__."/tests-skipcheck.inc.php");
--INI--
date.timezone=GMT
timecop.func_override=1
--FILE--
<?php
$dt1 = \DateTimeImmutable::createFromFormat('!Y-m-d', '2017-10-03');
$dt2 = \DateTimeImmutable::createFromFormat('!Y-m-d', '2017-10-03');

var_dump(get_class($dt1));
var_dump($dt1->format('c'));
var_dump(get_class($dt2));
var_dump($dt2->format('c'));
--EXPECT--
string(17) "DateTimeImmutable"
string(25) "2017-10-03T00:00:00+00:00"
string(17) "DateTimeImmutable"
string(25) "2017-10-03T00:00:00+00:00"
