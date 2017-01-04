--TEST--
Check for issue #14 (All PHPUnit assertions involving DateTime comparison fail with PHP 7.1)
--SKIPIF--
<?php
$required_version = "5.3";
$required_func = array("timecop_freeze");
include(__DIR__."/../tests-skipcheck.inc.php");
--INI--
date.timezone=GMT
timecop.func_override=1
--FILE--
<?php
    class CustomerInvoice
    {
        private $voidDate;
        public function void()
        {
            $this->voidDate = new \DateTime();
        }
        public function getvoidDate()
        {
            return $this->voidDate;
        }
    }
    function test_invoice_void()
    {
        $now = new \DateTime();
        timecop_freeze($now);

        $invoice = new CustomerInvoice();
        $invoice->void();

        var_dump($now == $invoice->getVoidDate());
    }
test_invoice_void();
--EXPECT--
bool(true)
