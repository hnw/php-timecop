--TEST--
Check for issue #9 (Issue with using timecop constructor)
--SKIPIF--
<?php
$required_version = "5.3";
$required_func = array("timecop_freeze");
$required_class = array("TimecopDateTime");
include(__DIR__."/../tests-skipcheck.inc.php");
--INI--
date.timezone=GMT
timecop.func_override=1
--FILE--
<?php
class Date extends \DateTime
{
    public function __construct($time='now', \DateTimeZone $timezone=null)
    {
        parent::__construct($time, $timezone);
        $this->setTime(0, 0, 0);
    }

    public function __toString() {
        return $this->format('Y-m-d');
    }
}
timecop_freeze(strtotime("2010-01-01"));
$dt1 = new Date();
echo $dt1, "\n";
$dt2 = new Date("2020-12-31");
echo $dt2, "\n";
$dt3 = new Date("2030-06-15", new \DateTimeZone("UTC"));
echo $dt3, "\n";
--EXPECT--
2010-01-01
2020-12-31
2030-06-15
