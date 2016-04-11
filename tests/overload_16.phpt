--TEST--
Check for DateTime/TimecopDateTime/TimecopOrigDateTime instance
--SKIPIF--
<?php
extension_loaded('timecop') or die('skip timecop not available');
$required_class = array("datetime", "timecopdatetime", "timecoporigdatetime");
foreach ($required_class as $class_name) {
    if (!class_exists($class_name)) {
        die("skip $class_name class is not available.");
    }
}
--INI--
date.timezone=America/Los_Angeles
timecop.func_override=1
--FILE--
<?php
timecop_freeze(timecop_orig_strtotime("2012-02-29 01:23:45"));

$dt1 = new DateTime();
var_dump(get_class($dt1));
$dt2 = new TimecopDateTime();
var_dump(get_class($dt2));
$dt3 = new TimecopOrigDateTime();
var_dump(get_class($dt3));
var_dump($dt2 instanceof $dt1);
var_dump($dt3 instanceof $dt1);
var_dump($dt2 instanceof $dt3);
var_dump($dt3 instanceof $dt2);
--EXPECT--
string(8) "DateTime"
string(15) "TimecopDateTime"
string(19) "TimecopOrigDateTime"
bool(true)
bool(true)
bool(false)
bool(false)
