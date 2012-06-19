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

ZEND_DECLARE_MODULE_GLOBALS(timecop)

/* True global resources - no need for thread safety here */
static int le_timecop;

/* {{{ timecop_overload_def mb_ovld[] */
static const struct timecop_overload_def timecop_ovld[] = {
        {"time", "timecop_time", "timecop_orig_time"},
        {NULL, NULL, NULL}
};
/* }}} */

/* {{{ timecop_functions[] */
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
	PHP_RINIT(timecop),
	PHP_RSHUTDOWN(timecop),
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
PHP_INI_BEGIN()
    STD_PHP_INI_ENTRY("timecop.func_overload", "0",
    PHP_INI_SYSTEM, OnUpdateLong, func_overload, zend_timecop_globals, timecop_globals)
PHP_INI_END()
/* }}} */

/* {{{ PHP_MINIT_FUNCTION
 */
PHP_MINIT_FUNCTION(timecop)
{
	REGISTER_INI_ENTRIES();
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MSHUTDOWN_FUNCTION
 */
PHP_MSHUTDOWN_FUNCTION(timecop)
{
	UNREGISTER_INI_ENTRIES();
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_RINIT_FUNCTION(timecop) */
PHP_RINIT_FUNCTION(timecop)
{
	zend_function *func, *orig;
	const struct timecop_overload_def *p;
	if (TIMECOP_G(func_overload)){
		p = &(timecop_ovld[0]);
		while (p->orig_func != NULL) {
			if (zend_hash_find(EG(function_table), p->save_func,
							   strlen(p->save_func)+1, (void **)&orig) != SUCCESS) {
				zend_hash_find(EG(function_table), p->ovld_func, strlen(p->ovld_func)+1 ,
							   (void **)&func);
				if (zend_hash_find(EG(function_table), p->orig_func, strlen(p->orig_func)+
								   1, (void **)&orig) != SUCCESS) {
					php_error_docref("https://github.com/hnw/php-timecop" TSRMLS_CC, E_WARNING, "timecop couldn't find function %s.", p->orig_func);
					return FAILURE;
				} else {
					zend_hash_add(EG(function_table), p->save_func, strlen(p->save_func)+1, orig, sizeof(zend_function), NULL);
					if (zend_hash_update(EG(function_table), p->orig_func, strlen(p->orig_func)+1, func, sizeof(zend_function),
										 NULL) == FAILURE) {
						php_error_docref("https://github.com/hnw/php-timecop" TSRMLS_CC, E_WARNING, "timecop couldn't replace function %s.", p->orig_func);
						return FAILURE;
					}
				}
			}
			p++;
		}
	}
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_RSHUTDOWN_FUNCTION(timecop) */
PHP_RSHUTDOWN_FUNCTION(timecop)
{
	zend_function *func, *orig;
	const struct timecop_overload_def *p;

	/*  clear overloaded function. */
	if (TIMECOP_G(func_overload)){
		p = &(timecop_ovld[0]);
		while (p->orig_func != NULL) {
			if (zend_hash_find(EG(function_table), p->save_func,
							   strlen(p->save_func)+1, (void **)&orig) == SUCCESS) {
				zend_hash_update(EG(function_table), p->orig_func, strlen(p->orig_func)+1,
								 orig, sizeof(zend_function), NULL);
				zend_hash_del(EG(function_table), p->save_func, strlen(p->save_func)+1);
			}
			p++;
		}
	}
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
