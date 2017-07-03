--TEST--
Function overrideing test for date()
--SKIPIF--
<?php
$required_func = array("timecop_freeze", "timecop_orig_time", "timecop_orig_date");
$required_class = array("TimecopOrigDateTime");
include(__DIR__."/../tests-skipcheck.inc.php");
--INI--
date.timezone=America/Los_Angeles
timecop.func_override=1
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

$dt = new TimecopOrigDateTime("2010-01-02 03:04:05.678000");

foreach ($tests as $format) {
    timecop_freeze($dt);
    $date1 = date($format);
    var_dump($date1);

    /* test for equality between timecop_date() and date() */
    while (true) {
        $start_time = timecop_orig_time();
        timecop_freeze(new TimecopOrigDateTime());
        $date2 = date($format);
        $date3 = timecop_orig_date($format);
        if ($start_time === timecop_orig_time()) {
            if ($date2 && $date3 && $date2 != $date3) {
                printf("date('%s') is differ from timecop_orig_date() : %s !== %s\n",
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
