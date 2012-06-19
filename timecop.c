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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "php_timecop.h"
#include <time.h>

/* If you declare any globals in php_timecop.h uncomment this:
ZEND_DECLARE_MODULE_GLOBALS(timecop)
*/

/* True global resources - no need for thread safety here */
static int le_timecop;

/* {{{ timecop_functions[]
 *
 * Every user visible function must have an entry in timecop_functions[].
 */
const zend_function_entry timecop_functions[] = {
	PHP_FE(timecop_time, NULL)
	PHP_FE_END	/* Must be the last line in timecop_functions[] */
};
/* }}} */

/* {{{ timecop_module_entry
 */
zend_module_entry timecop_module_entry = {
#if ZEND_MODULE_API_NO >= 20010901
	STANDARD_MODULE_HEADER,
#endif
	"timecop",
	timecop_functions,
	PHP_MINIT(timecop),
	PHP_MSHUTDOWN(timecop),
	NULL,
	NULL,
	PHP_MINFO(timecop),
#if ZEND_MODULE_API_NO >= 20010901
	"0.1",
#endif
	STANDARD_MODULE_PROPERTIES
};
/* }}} */

#ifdef COMPILE_DL_TIMECOP
ZEND_GET_MODULE(timecop)
#endif

/* {{{ PHP_INI
 */
/* Remove comments and fill if you need to have entries in php.ini
PHP_INI_BEGIN()
    STD_PHP_INI_ENTRY("timecop.global_value",      "42", PHP_INI_ALL, OnUpdateLong, global_value, zend_timecop_globals, timecop_globals)
    STD_PHP_INI_ENTRY("timecop.global_string", "foobar", PHP_INI_ALL, OnUpdateString, global_string, zend_timecop_globals, timecop_globals)
PHP_INI_END()
*/
/* }}} */

/* {{{ php_timecop_init_globals
 */
/* Uncomment this function if you have INI entries
static void php_timecop_init_globals(zend_timecop_globals *timecop_globals)
{
	timecop_globals->global_value = 0;
	timecop_globals->global_string = NULL;
}
*/
/* }}} */

/* {{{ PHP_MINIT_FUNCTION
 */
PHP_MINIT_FUNCTION(timecop)
{
	/* If you have INI entries, uncomment these lines 
	REGISTER_INI_ENTRIES();
	*/
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MSHUTDOWN_FUNCTION
 */
PHP_MSHUTDOWN_FUNCTION(timecop)
{
	/* uncomment this line if you have INI entries
	UNREGISTER_INI_ENTRIES();
	*/
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MINFO_FUNCTION
 */
PHP_MINFO_FUNCTION(timecop)
{
	php_info_print_table_start();
	php_info_print_table_header(2, "timecop", "enabled");
	php_info_print_table_end();

	/* Remove comments if you have entries in php.ini
	DISPLAY_INI_ENTRIES();
	*/
}
/* }}} */


/* {{{ proto int timecop_time(void)
   Return timestamp for $_SERVER[request_time] */
PHP_FUNCTION(timecop_time)
{
	zval **array, **request_time_long;
	long ret;
	if (zend_hash_find(&EG(symbol_table), "_SERVER", sizeof("_SERVER"), (void **) &array) == SUCCESS &&
		Z_TYPE_PP(array) == IS_ARRAY &&
		zend_hash_find(Z_ARRVAL_PP(array), "REQUEST_TIME", sizeof("REQUEST_TIME"), (void **) &request_time_long)
		== SUCCESS
		) {
		ret = Z_LVAL_PP(request_time_long);
	} else {
		ret = time(NULL);
	}
	RETURN_LONG(ret);
}
/* }}} */
/* The previous line is meant for vim and emacs, so it can correctly fold and 
   unfold functions in source code. See the corresponding marks just before 
   function definition, where the functions purpose is also documented. Please 
   follow this convention for the convenience of others editing your code.
*/


/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
