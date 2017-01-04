--TEST--
Test for TimecopDateTime::__construct
--SKIPIF--
<?php
$required_func = array("timecop_freeze");
$required_class = array("timecopdatetime", "datetime");
include(__DIR__."/../tests-skipcheck.inc.php");
--INI--
date.timezone=America/Los_Angeles
timecop.func_override=0
--FILE--
<?php
$strs = array(
    "2005-07-14 22:30:41",
    "2005-07-14 22:30:41 GMT",
    "@1121373041",
    "@1121373041 CEST",
    '1 Monday December 2008',
    '2 Monday December 2008',
    '3 Monday December 2008',
    'first Monday December 2008',
    'second Monday December 2008',
    'third Monday December 2008',
    '2013-03-13 03:00:00 -1sec',
    '0',
);
foreach ($strs as $str) {
    try {
        $dt1 = new DateTime($str);
        $e1 = null;
    } catch (Exception $e) {
        $dt1 = null;
        $e1 = $e;
    }
    try {
        $dt2 = new TimecopDateTime($str);
        $e2 = null;
    } catch (Exception $e) {
        $dt2 = null;
        $e2 = $e;
    }
    if ($dt1 === null && $dt2 === null) {
        if ($e1->getmessage() !== $e2->getmessage()) {
            printf("Exception message is differ for str=%s: %s !== %s\n",
                $str, $e1->getmessage(), $e2->getmessage());
        }
    }
    if ($dt1 && $dt2) {
        if ($dt1->format("c") !== $dt2->format("c")) {
            printf("TimecopDatetime::format is differ from Datetime::format for str=%s: %s !== %s\n",
                $str, $dt1->format("c"), $dt2->format("c"));
        }
        if (version_compare(PHP_VERSION, '5.3.0') >= 0) {
            if ($dt1->format("U") != $dt2->getTimestamp()) {
                printf("TimecopDatetime::getTimestamp is differ from Datetime::getTimestamp for str=%s: %s !== %s\n",
                       $str, $dt1->format("U"), $dt2->getTimestamp());
            }
        }
    }
}
--EXPECT--
