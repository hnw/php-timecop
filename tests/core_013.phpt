--TEST--
Test for phpinfo()
--SKIPIF--
<?php
include(__DIR__."/tests-skipcheck.inc.php");
--INI--
timecop.func_override=1
timecop.sync_request_time=1
--FILE--
<?php
ini_set("timecop.func_override", 0); // can't set
ini_set("timecop.sync_request_time", 0); // can't set
phpinfo();
--EXPECTREGEX--
.*
timecop => enabled
Version => \d+\.\d+\.\d+

Directive => Local Value => Master Value
timecop\.func_override => 1 => 1
timecop\.sync_request_time => 1 => 1
.*
