--TEST--
Check for TimecopDateTime::setTimestampfor PHP 5.2
--SKIPIF--
<?php
	extension_loaded('timecop') or die('skip timecop not available');
	if (version_compare(PHP_VERSION, '5.3.0') >= 0) {
		die("skip this is for PHP 5.2.x");
	}
	$required_class = array("timecopdatetime");
	foreach ($required_class as $class_name) {
		if (!class_exists($class_name)) {
			die("skip $class_name class is not available.");
		}
	}
--INI--
date.timezone=America/Los_Angeles
--FILE--
<?php
$dt = new TimecopDateTime("@1000");
$dt->setTimestamp(2000);
var_dump($dt->format("U"));
--EXPECT--
string(4) "2000"