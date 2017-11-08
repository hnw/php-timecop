--TEST--
Check for issue #5 (DateTime reflection)
--SKIPIF--
<?php
$required_version = "5.3";
$required_func = array("timecop_freeze");
$required_class = array("TimecopDateTime");
include(__DIR__."/tests-skipcheck.inc.php");
--INI--
date.timezone=GMT
timecop.func_override=1
--FILE--
<?php
class Test
{
    public function setCreatedAt(DateTime $createdAt)
    {
        $this->createdAt = $createdAt;
    }
}

$test = new Test();

$reflection = new ReflectionClass($test);
$params = $reflection->getMethod('setCreatedAt')->getParameters();

foreach ($params as $param) {
    var_dump($param->getClass());
}
--EXPECT--
object(ReflectionClass)#3 (1) {
  ["name"]=>
  string(8) "DateTime"
}
