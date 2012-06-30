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

ZEND_DECLARE_MODULE_GLOBALS(timecop)

/* declare the class handlers */
/*
static zend_object_handlers timecop_datetime_object_handlers;
*/

/* decalre the class entry */
/*
static zend_class_entry *timecop_datetime_ce;
*/

/* {{{ timecop_overload_def mb_ovld_func[] */
static const struct timecop_overload_def timecop_ovld_func[] = {
        {"time", "timecop_time", "timecop_orig_time"},
        {"date", "timecop_date", "timecop_orig_date"},
        {"gmdate", "timecop_gmdate", "timecop_orig_gmdate"},
        {"strtotime", "timecop_strtotime", "timecop_orig_strtotime"},
        {"strftime", "timecop_strftime", "timecop_orig_strftime"},
        {"gmstrftime", "timecop_gmstrftime", "timecop_orig_gmstrftime"},
        {NULL, NULL, NULL}
};
/* }}} */

/* {{{ timecop_overload_def mb_ovld_class[] */
static const struct timecop_overload_def timecop_ovld_class[] = {
        {"datetime", "timecopdatetime", "timecoporigdatetime"},
        {NULL, NULL, NULL}
};
/* }}} */

/* {{{ timecop_functions[] */
const zend_function_entry timecop_functions[] = {
	PHP_FE(timecop_freeze, NULL)
	PHP_FE(timecop_travel, NULL)
	PHP_FE(timecop_return, NULL)
	PHP_FE(timecop_time, NULL)
	PHP_FE(timecop_date, NULL)
	PHP_FE(timecop_gmdate, NULL)
	PHP_FE(timecop_strtotime, NULL)
	PHP_FE(timecop_strftime, NULL)
	PHP_FE(timecop_gmstrftime, NULL)
	PHP_FE_END
};
/* }}} */

/* declare method parameters, */
static ZEND_BEGIN_ARG_INFO(arginfo_timecop_datetime_create, 0)
	ZEND_ARG_INFO(0, time)
	ZEND_ARG_INFO(0, object)
ZEND_END_ARG_INFO();

/* each method can have its own parameters and visibility */
static zend_function_entry timecop_datetime_class_functions[] = {
	PHP_ME(TimecopDateTime, __construct, arginfo_timecop_datetime_create,
		   ZEND_ACC_CTOR | ZEND_ACC_PUBLIC)
	{NULL, NULL, NULL}
};

static int init_timecop_datetime();
static int timecop_func_overload();
static int timecop_class_overload();
static int timecop_func_overload_clear();
static int timecop_class_overload_clear();

static long _timecop_current_timestamp();
static void call_callable_with_optional_timestamp(INTERNAL_FUNCTION_PARAMETERS, zval* callable, int num_required_func_args);
static void _timecop_call_function(INTERNAL_FUNCTION_PARAMETERS, char* orig_func_name, char* saved_func_name, int num_required_func_args);
static void _timecop_call_constructor(INTERNAL_FUNCTION_PARAMETERS, zval **object_pp, zend_class_entry *obj_ce);

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
	int ret;
	zend_class_entry *parent_ce;

	init_timecop_datetime();

	if (TIMECOP_G(func_overload)) {
		if (SUCCESS != timecop_func_overload() ||
			SUCCESS != timecop_class_overload()) {
			return FAILURE;
		}
	}

	return SUCCESS;
}
/* }}} */

/* {{{ PHP_RSHUTDOWN_FUNCTION(timecop) */
PHP_RSHUTDOWN_FUNCTION(timecop)
{
	if (TIMECOP_G(func_overload)) {
		timecop_func_overload_clear();
		timecop_class_overload_clear();
	}

	TIMECOP_G(timecap_mode) = TIMECAP_MODE_NORMAL;

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

	DISPLAY_INI_ENTRIES();
}
/* }}} */

static int init_timecop_datetime()
{
	zend_class_entry **pce;
	zend_class_entry ce;
	zend_class_entry *self_ce, *parent_ce;

	if (zend_hash_find(CG(class_table), "datetime", sizeof("datetime"), (void **) &pce) == FAILURE) {
		return SUCCESS; /* DateTime must be initialized before */
	}
	parent_ce = *pce;

	INIT_CLASS_ENTRY(ce, "TimecopDateTime", timecop_datetime_class_functions);
	self_ce = zend_register_internal_class_ex(&ce, parent_ce, NULL TSRMLS_CC);
	self_ce->create_object = parent_ce->create_object;

	TIMECOP_G(ce_DateTime) = parent_ce;
	TIMECOP_G(ce_TimecopDateTime) = self_ce;

	return SUCCESS;
}

static int timecop_func_overload()
{
	zend_function *func, *orig;
	const struct timecop_overload_def *p;

	p = &(timecop_ovld_func[0]);
	while (p->orig_name != NULL) {
		if (zend_hash_find(EG(function_table), p->save_name, strlen(p->save_name)+1,
						   (void **)&orig) == SUCCESS) {
			continue;
		}
		if (zend_hash_find(EG(function_table), p->ovld_name, strlen(p->ovld_name)+1 ,
						   (void **)&func) != SUCCESS) {
			php_error_docref("https://github.com/hnw/php-timecop" TSRMLS_CC, E_WARNING, "timecop couldn't find function %s.", p->ovld_name);
		}
		if (zend_hash_find(EG(function_table), p->orig_name, strlen(p->orig_name)+1,
						   (void **)&orig) != SUCCESS) {
			php_error_docref("https://github.com/hnw/php-timecop" TSRMLS_CC, E_WARNING, "timecop couldn't find function %s.", p->orig_name);
				return FAILURE;
		}
		zend_hash_add(EG(function_table), p->save_name, strlen(p->save_name)+1,
					  orig, sizeof(zend_function), NULL);
		if (zend_hash_update(EG(function_table), p->orig_name, strlen(p->orig_name)+1,
							 func, sizeof(zend_function), NULL) == FAILURE) {
			php_error_docref("https://github.com/hnw/php-timecop" TSRMLS_CC, E_WARNING, "timecop couldn't replace function %s.", p->orig_name);
			return FAILURE;
		}
		p++;
	}
	return SUCCESS;
}

static int timecop_class_overload()
{
	zend_class_entry **pce_ovld, **pce_orig, *ce_ovld, *ce_orig;
	const struct timecop_overload_def *p;

	p = &(timecop_ovld_class[0]);
	while (p->orig_name != NULL) {
		if (zend_hash_find(EG(class_table), p->save_name, strlen(p->save_name)+1,
						   (void **)&pce_orig) == SUCCESS) {
			continue;
		}
		if (zend_hash_find(EG(class_table), p->ovld_name, strlen(p->ovld_name)+1 ,
						   (void **)&pce_ovld) != SUCCESS) {
			php_error_docref("https://github.com/hnw/php-timecop" TSRMLS_CC, E_WARNING, "timecop couldn't find function %s.", p->ovld_name);
		}
		if (zend_hash_find(EG(class_table), p->orig_name, strlen(p->orig_name)+1,
						   (void **)&pce_orig) != SUCCESS) {
			php_error_docref("https://github.com/hnw/php-timecop" TSRMLS_CC, E_WARNING, "timecop couldn't find function %s.", p->orig_name);
				return FAILURE;
		}
		ce_ovld = *pce_ovld;
		ce_orig = *pce_orig;

		ce_orig->refcount++;
		zend_hash_add(EG(class_table), p->save_name, strlen(p->save_name)+1,
					  &ce_orig, sizeof(zend_class_entry *), NULL);


		ce_ovld->refcount++;
		if (zend_hash_update(EG(class_table), p->orig_name, strlen(p->orig_name)+1,
							 &ce_ovld, sizeof(zend_class_entry *), NULL) == FAILURE) {
			php_error_docref("https://github.com/hnw/php-timecop" TSRMLS_CC, E_WARNING, "timecop couldn't replace function %s.", p->orig_name);

			ce_ovld->refcount--;
			return FAILURE;
		}
		p++;
	}
	return SUCCESS;
}

/*  clear overloaded function. */
static int timecop_func_overload_clear()
{
	const struct timecop_overload_def *p;
	zend_function *func, *orig;

	p = &(timecop_ovld_func[0]);
	while (p->orig_name != NULL) {
		if (zend_hash_find(EG(function_table), p->save_name,
						   strlen(p->save_name)+1, (void **)&orig) == SUCCESS) {
			zend_hash_update(EG(function_table), p->orig_name, strlen(p->orig_name)+1,
							 orig, sizeof(zend_function), NULL);
			zend_hash_del(EG(function_table), p->save_name, strlen(p->save_name)+1);
		}
		p++;
	}
	return SUCCESS;
}

static int timecop_class_overload_clear()
{
	const struct timecop_overload_def *p;
	zend_class_entry **pce_ovld, **pce_orig, *ce_ovld, *ce_orig;
	zend_function *func, *orig;

	p = &(timecop_ovld_class[0]);
	while (p->orig_name != NULL) {
		if (zend_hash_find(EG(class_table), p->save_name,
						   strlen(p->save_name)+1, (void **)&pce_orig) == SUCCESS) {
			ce_orig = *pce_orig;
			zend_hash_update(EG(class_table), p->orig_name, strlen(p->orig_name)+1,
							 &ce_orig, sizeof(zend_class_entry *), NULL);
			zend_hash_del(EG(class_table), p->save_name, strlen(p->save_name)+1);
		}
		p++;
	}
	return SUCCESS;
}

static long _timecop_current_timestamp()
{
	zval **array, **request_time_long;
	long current_timestamp;

	switch (TIMECOP_G(timecap_mode)) {
	case TIMECAP_MODE_FREEZE:
		current_timestamp = TIMECOP_G(freezed_timestamp);
		break;
	case TIMECAP_MODE_TRAVEL:
		current_timestamp = time(NULL) + TIMECOP_G(travel_offset);
		break;
	default:
		current_timestamp = time(NULL);
		break;
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

static void _timecop_call_constructor(INTERNAL_FUNCTION_PARAMETERS, zval **object_pp, zend_class_entry *obj_ce)
{
	zend_function **fn_proxy = &obj_ce->constructor;
	char* method_name = "__constructor";
	zend_uint param_count;
	zval ***params;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "*", &params, &param_count) == FAILURE) {
		return;
	}
	if (ZEND_NUM_ARGS() == 0) {
		zend_call_method_with_0_params(object_pp, obj_ce, &obj_ce->constructor, method_name, NULL);
	} else if (ZEND_NUM_ARGS() == 1) {
		zend_call_method_with_1_params(object_pp, obj_ce, &obj_ce->constructor, method_name, NULL, *params[0]);
	} else if (ZEND_NUM_ARGS() == 2) {
		zend_call_method_with_2_params(object_pp, obj_ce, &obj_ce->constructor, method_name, NULL, *params[0], *params[1]);
	} else {
		zend_error(E_ERROR, "INTERNAL ERROR: too many parameters for method call.");
	}

}


/* {{{ proto int timecop_freeze(long timestamp)
   Time travel to specified timestamp and freeze */
PHP_FUNCTION(timecop_freeze)
{
	long timestamp;
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &timestamp) == FAILURE) {
		RETURN_FALSE;
	}
	TIMECOP_G(timecap_mode) = TIMECAP_MODE_FREEZE;
	TIMECOP_G(freezed_timestamp) = timestamp;
	RETURN_TRUE;
}
/* }}} */

/* {{{ proto int timecop_travel(long timestamp)
   Time travel to specified timestamp */
PHP_FUNCTION(timecop_travel)
{
	long timestamp;
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &timestamp) == FAILURE) {
		RETURN_FALSE;
	}
	TIMECOP_G(timecap_mode) = TIMECAP_MODE_TRAVEL;
	TIMECOP_G(travel_offset) = timestamp - time(NULL);
	RETURN_TRUE;
}
/* }}} */

/* {{{ proto int timecop_return(void)
   Return to Time travel to specified timestamp */
PHP_FUNCTION(timecop_return)
{
	TIMECOP_G(timecap_mode) = TIMECAP_MODE_NORMAL;
	RETURN_TRUE;
}
/* }}} */

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

/* {{{ proto string timecop_strftime(string format [, int timestamp])
   Format a local time/date according to locale settings */
PHP_FUNCTION(timecop_strftime)
{
	_timecop_call_function(INTERNAL_FUNCTION_PARAM_PASSTHRU, "strftime", "timecop_orig_strftime", 1);
}
/* }}} */

/* {{{ proto string timecop_gmstrftime(string format [, int timestamp])
   Format a GMT/UCT time/date according to locale settings */
PHP_FUNCTION(timecop_gmstrftime)
{
	_timecop_call_function(INTERNAL_FUNCTION_PARAM_PASSTHRU, "gmstrftime", "timecop_orig_gmstrftime", 1);
}
/* }}} */

/* {{{ proto TimecopDateTime::__construct([string time[, DateTimeZone object]])
   Creates new TimecopDateTime object
*/
PHP_METHOD(TimecopDateTime, __construct)
{
	zval *tmp;
	zend_class_entry *parent_ce;
	zval *obj = getThis();

	/* call DateTime::__constuctor() */
	parent_ce = TIMECOP_G(ce_DateTime);
	_timecop_call_constructor(INTERNAL_FUNCTION_PARAM_PASSTHRU, &obj, parent_ce);

	/* call DateTime::setTimestamp() */
	ALLOC_INIT_ZVAL(tmp);
	ZVAL_LONG(tmp, _timecop_current_timestamp());
	zend_call_method_with_1_params(&obj, parent_ce, NULL, "settimestamp", NULL, tmp);
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
