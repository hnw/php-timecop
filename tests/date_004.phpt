--TEST--
Check for compareing TimecopDateTime instance
--SKIPIF--
<?php
extension_loaded('timecop') or die('skip timecop not available');
$required_class = array("timecopdatetime");
foreach ($required_class as $class_name) {
    if (!class_exists($class_name)) {
        die("skip $class_name class is not available.");
    }
}
--INI--
date.timezone=America/Los_Angeles
timecop.func_override=0
--FILE--
<?php
$dt1 = new TimecopDateTime("2012-01-02 12:00:00 JST");
$dt2 = new TimecopDateTime("2012-01-02 12:00:00", new DateTimeZone("Asia/Tokyo"));
var_dump($dt1 == $dt2);
$dt3 = new TimecopDateTime("2012-01-01 23:00:00 EST");
var_dump($dt1 < $dt3);
--EXPECT--
bool(true)
bool(true)