--TEST--
Check for clone TimecopDateTime instance
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
$dt1 = new TimecopDateTime("2012-01-01 12:00:00 GMT");
$dt2 = clone $dt1;
var_dump($dt2->format("c"));
--EXPECT--
string(25) "2012-01-01T12:00:00+00:00"
