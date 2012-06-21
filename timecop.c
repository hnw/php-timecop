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
        {"date", "timecop_date", "timecop_orig_date"},
        {"gmdate", "timecop_gmdate", "timecop_orig_gmdate"},
        {"strtotime", "timecop_strtotime", "timecop_orig_strtotime"},
        {NULL, NULL, NULL}
};
/* }}} */

/* {{{ timecop_functions[] */
const zend_function_entry timecop_functions[] = {
	PHP_FE(timecop_time, NULL)
	PHP_FE(timecop_date, NULL)
	PHP_FE(timecop_gmdate, NULL)
	PHP_FE(timecop_strtotime, NULL)
	PHP_FE_END
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
	if (TIMECOP_G(func_overload)) {
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

static long _timecop_current_timestamp()
{
	zval **array, **request_time_long;
	long current_timestamp;

	if (zend_hash_find(&EG(symbol_table), "_SERVER", sizeof("_SERVER"), (void **) &array) == SUCCESS &&
		Z_TYPE_PP(array) == IS_ARRAY &&
		zend_hash_find(Z_ARRVAL_PP(array), "REQUEST_TIME", sizeof("REQUEST_TIME"), (void **) &request_time_long)
		== SUCCESS
		) {
		current_timestamp = Z_LVAL_PP(request_time_long);
	} else {
		current_timestamp = time(NULL);
	}

	return current_timestamp;
}

static void call_callable_with_optional_timestamp(INTERNAL_FUNCTION_PARAMETERS, zval* callable, int num_required_func_args)
{
	zval *retval_ptr = NULL;
	zend_fcall_info fci;
	zend_fcall_info_cache fci_cache;
	char *is_callable_error = NULL;

	/* fci_cache = empty_fcall_info_cache; */
	zend_fcall_info_init(callable, 0, &fci, &fci_cache, NULL, &is_callable_error TSRMLS_CC);
	fci.retval_ptr_ptr = &retval_ptr;
	fci.no_separation = 0;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "*", &fci.params, &fci.param_count) == FAILURE) {
		return;
	}

	if (ZEND_NUM_ARGS() == num_required_func_args) {
		/* append optional timestamp argument */
		zval *tmp;
		zval ***old_params;
		int i;
		ALLOC_INIT_ZVAL(tmp);
		ZVAL_LONG(tmp, _timecop_current_timestamp());
		old_params = fci.params;
		fci.param_count = num_required_func_args + 1;
		fci.params = (zval ***)safe_emalloc(fci.param_count, sizeof(zval **), 0);
		for (i = 0; i < fci.param_count - 1; i++) {
			fci.params[i] = old_params[i];
		}
		fci.params[fci.param_count - 1] = (zval**)emalloc(sizeof(zval*));
		*fci.params[fci.param_count - 1] = tmp;
	}

	if (zend_call_function(&fci, &fci_cache TSRMLS_CC) == SUCCESS && fci.retval_ptr_ptr && *fci.retval_ptr_ptr) {
		COPY_PZVAL_TO_ZVAL(*return_value, *fci.retval_ptr_ptr);
	}
	if (fci.params) {
		efree(fci.params);
	}
}

static void _timecop_call_function(INTERNAL_FUNCTION_PARAMETERS, char* orig_func_name, char* saved_func_name, int num_required_func_args)
{
	zval callable;
	if (TIMECOP_G(func_overload)){
		ZVAL_STRING(&callable, saved_func_name, 1);
	} else {
		ZVAL_STRING(&callable, orig_func_name, 1);
	}
	call_callable_with_optional_timestamp(INTERNAL_FUNCTION_PARAM_PASSTHRU, &callable, num_required_func_args);
}

/* {{{ proto int timecop_time(void)
   Return virtual timestamp */
PHP_FUNCTION(timecop_time)
{
	RETURN_LONG(_timecop_current_timestamp());
}
/* }}} */

/* {{{ proto string date(string format [, long timestamp])
   Format a local date/time */
PHP_FUNCTION(timecop_date)
{
	_timecop_call_function(INTERNAL_FUNCTION_PARAM_PASSTHRU, "date", "timecop_orig_date", 1);
}
/* }}} */

/* {{{ proto string gmdate(string format [, long timestamp])
   Format a GMT date/time */
PHP_FUNCTION(timecop_gmdate)
{
	_timecop_call_function(INTERNAL_FUNCTION_PARAM_PASSTHRU, "gmdate", "timecop_orig_gmdate", 1);
}
/* }}} */

/* {{{ proto int timecop_strtotime(string time [, int now ])
   Convert string representation of date and time to a timestamp */
PHP_FUNCTION(timecop_strtotime)
{
	_timecop_call_function(INTERNAL_FUNCTION_PARAM_PASSTHRU, "strtotime", "timecop_orig_strtotime", 1);
}
/* }}} */

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
