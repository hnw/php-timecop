/*
The MIT License

Copyright (c) 2012-2016 Yoshio HANAWA

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

/* $Id$ */

#ifndef PHP_TIMECOP_H
#define PHP_TIMECOP_H

#define PHP_TIMECOP_VERSION "1.1.0"

extern zend_module_entry timecop_module_entry;
#define phpext_timecop_ptr &timecop_module_entry

#ifdef PHP_WIN32
#	define PHP_TIMECOP_API __declspec(dllexport)
#elif defined(__GNUC__) && __GNUC__ >= 4
#	define PHP_TIMECOP_API __attribute__ ((visibility("default")))
#else
#	define PHP_TIMECOP_API
#endif

#ifdef ZTS
#include "TSRM.h"
#endif

#include <time.h>
#include "Zend/zend_interfaces.h"

PHP_MINIT_FUNCTION(timecop);
PHP_MSHUTDOWN_FUNCTION(timecop);
PHP_RINIT_FUNCTION(timecop);
PHP_RSHUTDOWN_FUNCTION(timecop);
PHP_MINFO_FUNCTION(timecop);

PHP_FUNCTION(timecop_freeze);
PHP_FUNCTION(timecop_travel);
PHP_FUNCTION(timecop_return);
PHP_FUNCTION(timecop_time);
PHP_FUNCTION(timecop_mktime);
PHP_FUNCTION(timecop_gmmktime);
PHP_FUNCTION(timecop_date);
PHP_FUNCTION(timecop_gmdate);
PHP_FUNCTION(timecop_idate);
PHP_FUNCTION(timecop_getdate);
PHP_FUNCTION(timecop_localtime);
PHP_FUNCTION(timecop_strtotime);
PHP_FUNCTION(timecop_strftime);
PHP_FUNCTION(timecop_gmstrftime);
PHP_FUNCTION(timecop_unixtojd);
PHP_FUNCTION(timecop_date_create);
PHP_FUNCTION(timecop_date_create_from_format);

PHP_METHOD(TimecopDateTime, __construct);

#if !defined(PHP_VERSION_ID) || PHP_VERSION_ID < 50300
PHP_METHOD(TimecopDateTime, getTimestamp);
PHP_METHOD(TimecopDateTime, setTimestamp);
#endif

typedef enum timecop_mode_t {
	TIMECOP_MODE_NORMAL,
	TIMECOP_MODE_FREEZE,
	TIMECOP_MODE_TRAVEL
} timecop_mode_t;

ZEND_BEGIN_MODULE_GLOBALS(timecop)
	long func_override;
	long sync_request_time;
#if PHP_VERSION_ID >= 70000
	zval orig_request_time;
#else
	zval *orig_request_time;
#endif
	timecop_mode_t timecop_mode;
	long freezed_timestamp;
	long travel_offset;
	zend_class_entry *ce_DateTime;
	zend_class_entry *ce_TimecopDateTime;
ZEND_END_MODULE_GLOBALS(timecop)

struct timecop_override_def {
	char *orig_name;
	char *ovld_name;
	char *save_name;
};

/* In every utility function you add that needs to use variables 
   in php_timecop_globals, call TSRMLS_FETCH(); after declaring other 
   variables used by that function, or better yet, pass in TSRMLS_CC
   after the last function argument and declare your utility function
   with TSRMLS_DC after the last declared argument.  Always refer to
   the globals in your function as TIMECOP_G(variable).  You are 
   encouraged to rename these macros something shorter, see
   examples in any other php module directory.
*/

#if PHP_VERSION_ID >= 70000
#  define TIMECOP_G(v) ZEND_MODULE_GLOBALS_ACCESSOR(timecop, v)
#  if defined(ZTS) && defined(COMPILE_DL_TIMECOP)
     ZEND_TSRMLS_CACHE_EXTERN();
#  endif
#else
#  ifdef ZTS
#    define TIMECOP_G(v) TSRMG(timecop_globals_id, zend_timecop_globals *, v)
#  else
#    define TIMECOP_G(v) (timecop_globals.v)
#  endif
#endif

#endif	/* PHP_TIMECOP_H */

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
