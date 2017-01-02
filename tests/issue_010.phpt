--TEST--
Check for issue #10 (Timecop segfaults when set_error_handler throws an exception)
--SKIPIF--
<?php
include(__DIR__."/../tests-skipcheck.inc.php");
--INI--
date.timezone=GMT
timecop.func_override=1
--FILE--
<?php
error_reporting(-1);
function handler($errno, $errstr, $errfile, $errline) {
    throw new Exception($errstr, $errno);
}
set_error_handler('handler');

print "About to call mktime:\n";
mktime();
print "Called mktime\n";
--EXPECTREGEX--
About to call mktime:

Fatal error: Uncaught (exception 'Exception' with message|Exception:) '?(|timecop_)mktime\(\): You should be using the time\(\) function instead(.|\n)*
