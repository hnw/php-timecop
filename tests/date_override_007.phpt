--TEST--
Function overrideing test for date_create_from_format()
--SKIPIF--
<?php
$required_version = "5.3.1";
$required_func = array("timecop_freeze", "timecop_orig_date_create_from_format");
$required_class = array("TimecopOrigDateTime");
include(__DIR__."/tests-skipcheck.inc.php");
--INI--
date.timezone=America/Los_Angeles
timecop.func_override=1
--FILE--
<?php
$tests_args = array(
    array("-", "-"),
    array("S", "nd"),
    array("Y", 1990.0),
    array("m", "09"),
    array("jS of F", "24th of December"),
    array("H", "05", new DateTimeZone("Asia/Tokyo")),
    array("s", 59),
    array("u", 654321),
    array("Y-m-d H:i:s", "2012-03-31 12:34:56"),
    array("Y-m-d !H:i:s", "2012-03-31 12:34:56"),
    array("Y-m-d H:i:s T", "1970-01-01 19:00:00 EST"),
    array("U", "86400"),
    array("Y-m-d H:i:s", "2012-04-01 00:00:00", new DateTimeZone("Asia/Tokyo")),
    array("m-d|", "03-31"),
);

$dt0 = date_create_from_format("Y-m-d H:i:s.u", "2010-01-02 03:04:05.678000");
var_dump(get_class($dt0));
var_dump($dt0->format("Y-m-d H:i:s.uP"));
foreach ($tests_args as $args) {
    timecop_freeze($dt0);
    $dt1 = call_user_func_array("date_create_from_format", $args);
    var_dump($dt1->format("Y-m-d H:i:s.uP"));
    while (true) {
        /* test for equality between timecop_date_create_from_format() and date_create_from_format() */
        $start_time = time();
        timecop_freeze(new TimecopOrigDateTime());
        $dt2 = call_user_func_array("date_create_from_format", $args);
        $dt3 = call_user_func_array("timecop_orig_date_create_from_format", $args);
        if ($start_time === time()) {
            if ($dt2 && $dt3 && ($dt2->format("c") !== $dt3->format("c"))) {
                printf("date_create_from_format('%s', '%s') is differ from timecop_orig_date_create_from_format() : %s !== %s\n",
                       $args[0], $args[1], $dt2->format("c"), $dt3->format("c"));
            }
            break;
        }
    }
}
--EXPECTREGEX--
string\(8\) "DateTime"
string\(32\) "2010-01-02 03:04:05\.678000-08:00"
string\(32\) "2010-01-02 03:04:05\.(000|678)000-08:00"
string\(32\) "2010-01-02 03:04:05\.(\1)000-08:00"
string\(32\) "1990-01-02 03:04:05\.000000-08:00"
string\(32\) "2010-09-02 03:04:05\.000000-07:00"
string\(32\) "2010-12-24 03:04:05\.000000-08:00"
string\(32\) "2010-01-02 05:00:00\.000000\+09:00"
string\(32\) "2010-01-02 00:00:59\.000000-08:00"
string\(32\) "2010-01-02 (00|03):(00|04):(00|05)\.654321-08:00"
string\(32\) "2012-03-31 12:34:56\.000000-07:00"
string\(32\) "1970-01-01 12:34:56\.000000-08:00"
string\(32\) "1970-01-01 19:00:00\.000000-05:00"
string\(32\) "1970-01-02 00:00:00\.000000\+00:00"
string\(32\) "2012-04-01 00:00:00\.000000\+09:00"
string\(32\) "1970-03-31 00:00:00\.000000\-08:00"
