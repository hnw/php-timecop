--TEST--
Test for timecop_date()
--SKIPIF--
<?php
$required_func = array("timecop_freeze", "timecop_date");
include(__DIR__."/tests-skipcheck.inc.php");
--INI--
date.timezone=America/Los_Angeles
timecop.func_override=0
--FILE--
<?php
$tests = array(
    "U",
    "Y-m-d",
    "Y-m-d H:i:s.u",
    "c",
    "r",
    "W",
    DATE_ATOM,
);

$dt = new DateTime("2010-01-02 03:04:05.678000");

foreach ($tests as $format) {
    timecop_freeze($dt);
    $date1 = timecop_date($format);
    var_dump($date1);

    /* test for equality between timecop_date() and date() */
    while (true) {
        $start_time = time();
        timecop_freeze(new DateTime());
        $date2 = timecop_date($format);
        $date3 = date($format);
        if ($start_time === time()) {
            if ($date2 && $date3 && $date2 != $date3) {
                printf("timecop_date('%s') is differ from date() : %s !== %s\n",
                       $format, $date2, $date3);
            }
            break;
        }
    }
}
--EXPECT--
string(10) "1262430245"
string(10) "2010-01-02"
string(26) "2010-01-02 03:04:05.000000"
string(25) "2010-01-02T03:04:05-08:00"
string(31) "Sat, 02 Jan 2010 03:04:05 -0800"
string(2) "53"
string(25) "2010-01-02T03:04:05-08:00"
