--TEST--
Check for issue #9 (Issue with using timecop constructor)
--SKIPIF--
<?php
extension_loaded('timecop') or die('skip timecop not available');
if (version_compare(PHP_VERSION, '5.3.0') < 0) {
    die("skip requires PHP >= 5.3.0");
}
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
