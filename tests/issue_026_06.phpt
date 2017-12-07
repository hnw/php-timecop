--TEST--
Check for issue #26 (Unexpected segmentation fault after homebrew installation #26)
--SKIPIF--
<?php
$required_version = "5.3";
include(__DIR__."/tests-skipcheck.inc.php");
--INI--
date.timezone=GMT
timecop.func_override=1
--FILE--
<?php
class Carbon extends DateTime
{
    public static function createFromFormat($format, $time, $tz = null)
    {
        return "foobar\n";
    }
}
echo Carbon::createFromFormat(null,null);
--EXPECT--
foobar
