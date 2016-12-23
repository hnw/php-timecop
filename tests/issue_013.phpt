--TEST--
Check for issue #13 (Exception: Failed to parse time string ... giving up time traveling)
--SKIPIF--
<?php
extension_loaded('timecop') or die('skip timecop not available');
--INI--
date.timezone=GMT
timecop.func_override=1
--FILE--
<?php
$dt = new DateTime('2044-06-18 00:00:00');
var_dump($dt->format("c"));
--EXPECT--
string(25) "2044-06-18T00:00:00+00:00"
