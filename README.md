# php-timecop

[![Build Status](https://travis-ci.org/hnw/php-timecop.png)](https://travis-ci.org/hnw/php-timecop)

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
```

## SYSTEM REQUIREMENTS

- OS: Linux, FreeBSD, MacOSX
- PHP: 5.2.x, 5.3.x, 5.4.x, 5.5.x
  - Tested only on 5.2.17, 5.3.21, 5.4.11, and 5.5.0alpha4
- SAPI: Apache, CLI
  - Other SAPIs are not tested, but there is no SAPI-dependent code.
- non-ZTS(recommended), ZTS

## FEATURES

- Freeze time to a specific point.
- Travel back to a specific point in time, but allow time to continue moving forward from there.
- Override the following PHP stock functions and class, which supports freezing or traveling time.
  - time()
  - mktime()
  - gmmktime()
  - date()
  - gmdate()
  - idate()
  - getdate()
  - localtime()
  - strtotime()
  - strftime()
  - gmstrftime()
  - unixtojd()
  - DateTime
- Rewrite value of the following global variables when the time has been moved.
  - $_SERVER['REQUEST_TIME']


## USAGE

```php
<?php
var_dump(date("Y-m-d")); // todays date
timecop_freeze(0);
var_dump(gmdate("Y-m-d H:i:s")); // string(19) "1970-01-01 00:00:00"
var_dump(strtotime("+100000 sec")); // int(10000)
```

## LICENSE
#
The MIT License

Copyright (c) 2012 Yoshio HANAWA

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
