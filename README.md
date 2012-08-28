# php-timecop

[![Build Status](https://secure.travis-ci.org/hnw/php-timecop.png?branch=master)](http://travis-ci.org/hnw/php-timecop)

## DESCRIPTION

A PHP extension providing "time travel" and "time freezing" capabilities, inspired by [ruby timecop gem](http://github.com/jtrupiano/timecop).

## INSTALL

```
git clone https://github.com/hnw/php-timecop.git
cd php-timecop
phpize
./configure
make
make install
```

After install, add these lines to your php.ini

```ini
extension=timecop.so
timecop.func_overload=1
```

## FEATURES

- Freeze time to a specific point.
- Travel back to a specific point in time, but allow time to continue moving forward from there.
- Provide following functions, which correspond to the PHP stock ones.
  - timecop_time()
  - timecop_mktime()
  - timecop_gmmktime()
  - timecop_date()
  - timecop_gmdate()
  - timecop_idate()
  - timecop_getdate()
  - timecop_localtime()
  - timecop_strtotime()
  - timecop_strftime()
  - timecop_gmstrftime()
  - timecop_unixtojd()
- Provide following class, which correspond to the stock DateTime.
  - TimecopDateTime
- Override the PHP stock functions and class.

## USAGE

```php
<?php
var_dump(timecop_date("Y-m-d")); // todays date
timecop_freeze(0);
var_dump(timecop_gmdate("Y-m-d H:i:s")); // string(19) "1970-01-01 00:00:00"
var_dump(timecop_strtotime("+100000 sec")); // int(10000)
```

When ```timecop.func_overload=1```, below example can be rewritten as follows:

```php
<?php
var_dump(date("Y-m-d")); // todays date
timecop_freeze(0);
var_dump(gmdate("Y-m-d H:i:s")); // string(19) "1970-01-01 00:00:00"
var_dump(strtotime("+100000 sec")); // int(10000)
```
