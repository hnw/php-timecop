/*
  +----------------------------------------------------------------------+
  | PHP Version 5                                                        |
  +----------------------------------------------------------------------+
  | Copyright (c) 1997-2012 The PHP Group                                |
  +----------------------------------------------------------------------+
  | This source file is subject to version 3.01 of the PHP license,      |
  | that is bundled with this package in the file LICENSE, and is        |
  | available through the world-wide-web at the following url:           |
  | http://www.php.net/license/3_01.txt                                  |
  | If you did not receive a copy of the PHP license and are unable to   |
  | obtain it through the world-wide-web, please send a note to          |
  | license@php.net so we can mail you a copy immediately.               |
  +----------------------------------------------------------------------+
  | Author:                                                              |
  +----------------------------------------------------------------------+
*/

/* $Id$ */

#ifndef PHP_TIMECOP_H
#define PHP_TIMECOP_H

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

PHP_MINIT_FUNCTION(timecop);
PHP_MSHUTDOWN_FUNCTION(timecop);
PHP_RINIT_FUNCTION(timecop);
PHP_RSHUTDOWN_FUNCTION(timecop);
PHP_MINFO_FUNCTION(timecop);

PHP_FUNCTION(timecop_time);
PHP_FUNCTION(timecop_date);
PHP_FUNCTION(timecop_strtotime);

ZEND_BEGIN_MODULE_GLOBALS(timecop)
	long func_overload;

ZEND_END_MODULE_GLOBALS(timecop)

struct timecop_overload_def {
        char *orig_func;
        char *ovld_func;
        char *save_func;
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

#ifdef ZTS
#define TIMECOP_G(v) TSRMG(timecop_globals_id, zend_timecop_globals *, v)
#else
#define TIMECOP_G(v) (timecop_globals.v)
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
