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

#ifdef ZFS
#include "TSRM.h"
#endif

ZEND_DECLARE_MODULE_GLOBALS(timecop)

static void timecop_globals_ctor(zend_timecop_globals *globals TSRMLS_DC) {
	/* Initialize your global struct */
	globals->func_override = 1;
	globals->sync_request_time = 1;
	globals->orig_request_time = NULL;
	globals->timecop_mode = TIMECOP_MODE_NORMAL;
	globals->freezed_timestamp = 0;
	globals->travel_offset = 0;
	globals->ce_DateTime = NULL;
	globals->ce_TimecopDateTime = NULL;
}

/* declare the class handlers */
/*
static zend_object_handlers timecop_datetime_object_handlers;
*/

/* decalre the class entry */
/*
static zend_class_entry *timecop_datetime_ce;
*/

/* {{{ timecop_override_def mb_ovld_func[] */
static const struct timecop_override_def timecop_ovld_func[] = {
	{"time", "timecop_time", "timecop_orig_time"},
	{"mktime", "timecop_mktime", "timecop_orig_mktime"},
	{"gmmktime", "timecop_gmmktime", "timecop_orig_gmmktime"},
	{"date", "timecop_date", "timecop_orig_date"},
	{"gmdate", "timecop_gmdate", "timecop_orig_gmdate"},
	{"idate", "timecop_idate", "timecop_orig_idate"},
	{"getdate", "timecop_getdate", "timecop_orig_getdate"},
	{"localtime", "timecop_localtime", "timecop_orig_localtime"},
	{"strtotime", "timecop_strtotime", "timecop_orig_strtotime"},
	{"strftime", "timecop_strftime", "timecop_orig_strftime"},
	{"gmstrftime", "timecop_gmstrftime", "timecop_orig_gmstrftime"},
	{"unixtojd", "timecop_unixtojd", "timecop_orig_unixtojd"},
	{NULL, NULL, NULL}
};
/* }}} */

/* {{{ timecop_override_def mb_ovld_class[] */
static const struct timecop_override_def timecop_ovld_class[] = {
	{"datetime", "timecopdatetime", "timecoporigdatetime"},
	{NULL, NULL, NULL}
};
/* }}} */

ZEND_BEGIN_ARG_INFO_EX(arginfo_timecop_freeze, 0, 0, 1)
	ZEND_ARG_INFO(0, timestamp)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_timecop_travel, 0, 0, 1)
	ZEND_ARG_INFO(0, timestamp)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_timecop_return, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_timecop_time, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_timecop_mktime, 0, 0, 0)
	ZEND_ARG_INFO(0, hour)
	ZEND_ARG_INFO(0, min)
	ZEND_ARG_INFO(0, sec)
	ZEND_ARG_INFO(0, mon)
	ZEND_ARG_INFO(0, day)
	ZEND_ARG_INFO(0, year)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_timecop_gmmktime, 0, 0, 0)
	ZEND_ARG_INFO(0, hour)
	ZEND_ARG_INFO(0, min)
	ZEND_ARG_INFO(0, sec)
	ZEND_ARG_INFO(0, mon)
	ZEND_ARG_INFO(0, day)
	ZEND_ARG_INFO(0, year)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_timecop_date, 0, 0, 1)
	ZEND_ARG_INFO(0, format)
	ZEND_ARG_INFO(0, timestamp)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_timecop_gmdate, 0, 0, 1)
	ZEND_ARG_INFO(0, format)
	ZEND_ARG_INFO(0, timestamp)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_timecop_idate, 0, 0, 1)
	ZEND_ARG_INFO(0, format)
	ZEND_ARG_INFO(0, timestamp)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_timecop_getdate, 0, 0, 0)
	ZEND_ARG_INFO(0, timestamp)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_timecop_localtime, 0, 0, 0)
	ZEND_ARG_INFO(0, timestamp)
	ZEND_ARG_INFO(0, associative_array)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_timecop_strtotime, 0, 0, 1)
	ZEND_ARG_INFO(0, time)
	ZEND_ARG_INFO(0, now)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_timecop_strftime, 0, 0, 1)
	ZEND_ARG_INFO(0, format)
	ZEND_ARG_INFO(0, timestamp)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_timecop_gmstrftime, 0, 0, 1)
	ZEND_ARG_INFO(0, format)
	ZEND_ARG_INFO(0, timestamp)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_timecop_unixtojd, 0, 0, 0)
	ZEND_ARG_INFO(0, timestamp)
ZEND_END_ARG_INFO()

/* {{{ timecop_functions[] */
const zend_function_entry timecop_functions[] = {
	PHP_FE(timecop_freeze, arginfo_timecop_freeze)
	PHP_FE(timecop_travel, arginfo_timecop_travel)
	PHP_FE(timecop_return, arginfo_timecop_return)
	PHP_FE(timecop_time, arginfo_timecop_time)
	PHP_FE(timecop_mktime, arginfo_timecop_mktime)
	PHP_FE(timecop_gmmktime, arginfo_timecop_gmmktime)
	PHP_FE(timecop_date, arginfo_timecop_date)
	PHP_FE(timecop_gmdate, arginfo_timecop_gmdate)
	PHP_FE(timecop_idate, arginfo_timecop_idate)
	PHP_FE(timecop_getdate, arginfo_timecop_getdate)
	PHP_FE(timecop_localtime, arginfo_timecop_localtime)
	PHP_FE(timecop_strtotime, arginfo_timecop_strtotime)
	PHP_FE(timecop_strftime, arginfo_timecop_strftime)
	PHP_FE(timecop_gmstrftime, arginfo_timecop_gmstrftime)
	PHP_FE(timecop_unixtojd, arginfo_timecop_unixtojd)
	{NULL, NULL, NULL}
};
/* }}} */

/* declare method parameters, */
ZEND_BEGIN_ARG_INFO_EX(arginfo_timecop_datetime_create, 0, 0, 0)
	ZEND_ARG_INFO(0, time)
	ZEND_ARG_INFO(0, object)
ZEND_END_ARG_INFO()

/* each method can have its own parameters and visibility */
static zend_function_entry timecop_datetime_class_functions[] = {
	PHP_ME(TimecopDateTime, __construct, arginfo_timecop_datetime_create,
		   ZEND_ACC_CTOR | ZEND_ACC_PUBLIC)
	{NULL, NULL, NULL}
};

static void timecop_globals_ctor(zend_timecop_globals *globals TSRMLS_DC);

static int register_timecop_classes(TSRMLS_D);
static int timecop_func_override(TSRMLS_D);
static int timecop_class_override(TSRMLS_D);
static int timecop_func_override_clear(TSRMLS_D);
static int timecop_class_override_clear(TSRMLS_D);

static int update_request_time(long unixtime TSRMLS_DC);
static int restore_request_time(TSRMLS_D);

static int timecop_zend_fcall_info_init(zval *callable, zend_fcall_info *fci, zend_fcall_info_cache *fcc TSRMLS_DC);
static int init_fcall_info(zval *callable, zend_fcall_info * fci, zend_fcall_info_cache * fcc, int num_args TSRMLS_DC);
static int init_timecop_date_fcall_info(zval *callable, zend_fcall_info * fci, zend_fcall_info_cache * fcc TSRMLS_DC);
static void dtor_datefunc_info(zend_fcall_info *fci);
static zval *timecop_date_fcall(const char *format, zend_fcall_info * fci, zend_fcall_info_cache * fcc TSRMLS_DC);
static zval ***alloc_fcall_params(int num_args);
static void dtor_fcall_params(zval ***params, int num_args);
static void copy_fcall_params(zval ***src, zval ***dst, int num_args);
static int fill_mktime_params(zval ***params, zval *date_callable, int from TSRMLS_DC);

static long timecop_current_timestamp(TSRMLS_D);
static zval *timecop_current_date(char *format TSRMLS_DC);
PHPAPI void php_timecop_mktime(INTERNAL_FUNCTION_PARAMETERS, zval *mktime_callable, zval *date_callable);

static void call_callable_with_optional_timestamp(INTERNAL_FUNCTION_PARAMETERS, zval* callable, int num_required_func_args);
static void _timecop_call_function(INTERNAL_FUNCTION_PARAMETERS, char* orig_func_name, char* saved_func_name, int num_required_func_args);
static void call_constructor(zval **object_pp, zend_class_entry *ce, zval ***params, int param_count TSRMLS_DC);


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
    STD_PHP_INI_ENTRY("timecop.func_override", "1",
    PHP_INI_SYSTEM, OnUpdateLong, func_override, zend_timecop_globals, timecop_globals)
    STD_PHP_INI_ENTRY("timecop.sync_request_time", "1",
    PHP_INI_SYSTEM, OnUpdateLong, sync_request_time, zend_timecop_globals, timecop_globals)
PHP_INI_END()
/* }}} */

/* {{{ PHP_MINIT_FUNCTION
 */
PHP_MINIT_FUNCTION(timecop)
{
	ZEND_INIT_MODULE_GLOBALS(timecop, timecop_globals_ctor, NULL);
	REGISTER_INI_ENTRIES();
	register_timecop_classes(TSRMLS_C);
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

	if (TIMECOP_G(func_override)) {
		if (SUCCESS != timecop_func_override(TSRMLS_C) ||
			SUCCESS != timecop_class_override(TSRMLS_C)) {
			return FAILURE;
		}
	}

	return SUCCESS;
}
/* }}} */

/* {{{ PHP_RSHUTDOWN_FUNCTION(timecop) */
PHP_RSHUTDOWN_FUNCTION(timecop)
{
	if (TIMECOP_G(func_override)) {
		timecop_func_override_clear(TSRMLS_C);
		timecop_class_override_clear(TSRMLS_C);
	}

	if (TIMECOP_G(orig_request_time)) {
		restore_request_time(TSRMLS_C);
	}

	TIMECOP_G(timecop_mode) = TIMECOP_MODE_NORMAL;

	return SUCCESS;
}
/* }}} */


/* {{{ PHP_MINFO_FUNCTION
 */
PHP_MINFO_FUNCTION(timecop)
{
	php_info_print_table_start();
	php_info_print_table_header(2, "timecop", "enabled");
	php_info_print_table_row(2, "Version", PHP_TIMECOP_VERSION);
	php_info_print_table_end();

	DISPLAY_INI_ENTRIES();
}
/* }}} */

static int register_timecop_classes(TSRMLS_D)
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

static int timecop_func_override(TSRMLS_D)
{
	zend_function *func, *orig;
	const struct timecop_override_def *p;

	p = &(timecop_ovld_func[0]);
	while (p->orig_name != NULL) {
		if (zend_hash_find(EG(function_table), p->save_name, strlen(p->save_name)+1,
						   (void **)&orig) == SUCCESS) {
			php_error_docref("https://github.com/hnw/php-timecop" TSRMLS_CC, E_WARNING,
							 "timecop couldn't create function %s because already exists.", p->save_name);
		} else if(zend_hash_find(EG(function_table), p->ovld_name, strlen(p->ovld_name)+1,
								 (void **)&func) != SUCCESS) {
			php_error_docref("https://github.com/hnw/php-timecop" TSRMLS_CC, E_WARNING,
							 "timecop couldn't find function %s.", p->ovld_name);
		} else if(zend_hash_find(EG(function_table), p->orig_name, strlen(p->orig_name)+1,
								 (void **)&orig) != SUCCESS) {
			// Do nothing. Because some functions are introduced by optional extensions.
		} else {
			zend_hash_add(EG(function_table), p->save_name, strlen(p->save_name)+1,
						  orig, sizeof(zend_function), NULL);
			if (zend_hash_update(EG(function_table), p->orig_name, strlen(p->orig_name)+1,
								 func, sizeof(zend_function), NULL) == FAILURE) {
				php_error_docref("https://github.com/hnw/php-timecop" TSRMLS_CC, E_WARNING, "timecop couldn't replace function %s.", p->orig_name);
				return FAILURE;
			}
		}
		p++;
	}
	return SUCCESS;
}

static int timecop_class_override(TSRMLS_D)
{
	zend_class_entry **pce_ovld, **pce_orig, *ce_ovld, *ce_orig;
	const struct timecop_override_def *p;

	p = &(timecop_ovld_class[0]);
	while (p->orig_name != NULL) {
		if (zend_hash_find(EG(class_table), p->save_name, strlen(p->save_name)+1,
						   (void **)&pce_orig) == SUCCESS) {
			php_error_docref("https://github.com/hnw/php-timecop" TSRMLS_CC, E_WARNING,
							 "timecop couldn't create class %s because already exists.", p->save_name);
		} else if (zend_hash_find(EG(class_table), p->ovld_name, strlen(p->ovld_name)+1 ,
								  (void **)&pce_ovld) != SUCCESS) {
			php_error_docref("https://github.com/hnw/php-timecop" TSRMLS_CC, E_WARNING,
							 "timecop couldn't find class %s.", p->ovld_name);
		} else if (zend_hash_find(EG(class_table), p->orig_name, strlen(p->orig_name)+1,
								  (void **)&pce_orig) != SUCCESS) {
			php_error_docref("https://github.com/hnw/php-timecop" TSRMLS_CC, E_WARNING,
							 "timecop couldn't find function %s.", p->orig_name);
		} else {
			ce_ovld = *pce_ovld;
			ce_orig = *pce_orig;
			ce_orig->refcount++;
			zend_hash_add(EG(class_table), p->save_name, strlen(p->save_name)+1,
						  &ce_orig, sizeof(zend_class_entry *), NULL);
			ce_ovld->refcount++;
			if (zend_hash_update(EG(class_table), p->orig_name, strlen(p->orig_name)+1,
								 &ce_ovld, sizeof(zend_class_entry *), NULL) == FAILURE) {
				ce_ovld->refcount--;
				return FAILURE;
			}
		}
		p++;
	}
	return SUCCESS;
}

/*  clear overrideed function. */
static int timecop_func_override_clear(TSRMLS_D)
{
	const struct timecop_override_def *p;
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

static int timecop_class_override_clear(TSRMLS_D)
{
	const struct timecop_override_def *p;
	zend_class_entry **pce_ovld, **pce_orig, *ce_ovld, *ce_orig;
	zend_function *func, *orig;

	p = &(timecop_ovld_class[0]);
	while (p->orig_name != NULL) {
		if (zend_hash_find(EG(class_table), p->save_name, strlen(p->save_name)+1,
						   (void **)&pce_orig) == SUCCESS) {
			ce_orig = *pce_orig;
			ce_orig->refcount++;
			zend_hash_update(EG(class_table), p->orig_name, strlen(p->orig_name)+1,
							 &ce_orig, sizeof(zend_class_entry *), NULL);
			zend_hash_del(EG(class_table), p->save_name, strlen(p->save_name)+1);
		}
		p++;
	}
	return SUCCESS;
}

static int update_request_time(long unixtime TSRMLS_DC)
{
	zval **server_vars;
	zval **request_time;
	zval *tmp;

	if (zend_hash_find(&EG(symbol_table), "_SERVER", sizeof("_SERVER"), (void **) &server_vars) == SUCCESS &&
		Z_TYPE_PP(server_vars) == IS_ARRAY &&
		zend_hash_find(Z_ARRVAL_PP(server_vars), "REQUEST_TIME", sizeof("REQUEST_TIME"), (void **) &request_time) == SUCCESS) {
		if (TIMECOP_G(orig_request_time) == NULL) {
			MAKE_STD_ZVAL(TIMECOP_G(orig_request_time));
			ZVAL_ZVAL(TIMECOP_G(orig_request_time), *request_time, 1, 0);
		}
		MAKE_STD_ZVAL(tmp);
		ZVAL_LONG(tmp, unixtime);
		add_assoc_zval(*server_vars, "REQUEST_TIME", tmp);
		zval_dtor(tmp);
	}

	return SUCCESS;
}

static int restore_request_time(TSRMLS_D)
{
	zval **server_vars;
	zval **request_time;
	zval *orig_request_time;
	orig_request_time = TIMECOP_G(orig_request_time);

	if (orig_request_time &&
		zend_hash_find(&EG(symbol_table), "_SERVER", sizeof("_SERVER"), (void **) &server_vars) == SUCCESS &&
		Z_TYPE_PP(server_vars) == IS_ARRAY &&
		zend_hash_find(Z_ARRVAL_PP(server_vars), "REQUEST_TIME", sizeof("REQUEST_TIME"), (void **) &request_time) == SUCCESS) {
		add_assoc_zval(*server_vars, "REQUEST_TIME", orig_request_time);
		zval_dtor(orig_request_time);
		TIMECOP_G(orig_request_time) = NULL;
	}
	return SUCCESS;
}

static int timecop_zend_fcall_info_init(zval *callable, zend_fcall_info *fci, zend_fcall_info_cache *fcc TSRMLS_DC)
{
	int init_result;
	char *is_callable_error = NULL;

#if !defined(PHP_VERSION_ID) || PHP_VERSION_ID < 50300
	init_result = zend_fcall_info_init(callable, fci, fcc TSRMLS_CC);
#else
	init_result = zend_fcall_info_init(callable, 0, fci, fcc, NULL, &is_callable_error TSRMLS_CC);
#endif
	if (init_result == FAILURE) {
		if (is_callable_error) {
			zend_error(E_ERROR, "INTERNAL ERROR: to be a valid callback, %s", is_callable_error);
		}
	}
	if (is_callable_error) {
		efree(is_callable_error);
	}
	return init_result;
}

static void call_callable_with_optional_timestamp(INTERNAL_FUNCTION_PARAMETERS, zval* callable, int num_required_func_args)
{
	zval *retval_ptr = NULL;
	zend_fcall_info fci;
	zend_fcall_info_cache fci_cache;
	int fill_timestamp = 0;

	if (timecop_zend_fcall_info_init(callable, &fci, &fci_cache TSRMLS_CC) == FAILURE) {
		return;
	}
	fci.retval_ptr_ptr = &retval_ptr;
	fci.no_separation = 0;

	fci.param_count = ZEND_NUM_ARGS();
	fci.params = (zval ***) safe_emalloc(fci.param_count, sizeof(zval **), 0);
	if (zend_get_parameters_array_ex(fci.param_count, fci.params) == FAILURE) {
		efree(fci.params);
		return;
	}

	fill_timestamp = (fci.param_count == num_required_func_args) ? 1 : 0;

	if (fill_timestamp) {
		/* append optional timestamp argument */
		zval ***orig_params;

		orig_params = fci.params;
		fci.param_count = num_required_func_args + 1;
		fci.params = alloc_fcall_params(fci.param_count);
		copy_fcall_params(orig_params, fci.params, num_required_func_args);
		if (orig_params) {
			efree(orig_params);
		}
		ZVAL_LONG(*fci.params[fci.param_count - 1], timecop_current_timestamp(TSRMLS_C));
	}

	if (zend_call_function(&fci, &fci_cache TSRMLS_CC) == SUCCESS && fci.retval_ptr_ptr && *fci.retval_ptr_ptr) {
		COPY_PZVAL_TO_ZVAL(*return_value, *fci.retval_ptr_ptr);
	}

	if (fci.params) {
		if (fill_timestamp) {
			dtor_fcall_params(fci.params, fci.param_count);
		} else {
			efree(fci.params);
		}
	}
}

static void _timecop_call_function(INTERNAL_FUNCTION_PARAMETERS, char* orig_func_name, char* saved_func_name, int num_required_func_args)
{
	zval callable;
	if (TIMECOP_G(func_override)){
		ZVAL_STRING(&callable, saved_func_name, 1);
	} else {
		ZVAL_STRING(&callable, orig_func_name, 1);
	}
	call_callable_with_optional_timestamp(INTERNAL_FUNCTION_PARAM_PASSTHRU, &callable, num_required_func_args);
	zval_dtor(&callable);
}

static void call_constructor(zval **object_pp, zend_class_entry *ce, zval ***params, int param_count TSRMLS_DC)
{
	char* method_name = "__constructor";

	if (param_count == 0) {
		zend_call_method_with_0_params(object_pp, ce, &ce->constructor, method_name, NULL);
	} else if (param_count == 1) {
		zend_call_method_with_1_params(object_pp, ce, &ce->constructor, method_name, NULL, *params[0]);
	} else if (param_count == 2) {
		zend_call_method_with_2_params(object_pp, ce, &ce->constructor, method_name, NULL, *params[0], *params[1]);
	} else {
		zend_error(E_ERROR, "INTERNAL ERROR: too many parameters for constructor.");
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
	TIMECOP_G(timecop_mode) = TIMECOP_MODE_FREEZE;
	TIMECOP_G(freezed_timestamp) = timestamp;

	if (TIMECOP_G(sync_request_time)){
		update_request_time(timestamp TSRMLS_CC);
	}

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
	TIMECOP_G(timecop_mode) = TIMECOP_MODE_TRAVEL;
	TIMECOP_G(travel_offset) = timestamp - time(NULL);

	if (TIMECOP_G(sync_request_time)){
		update_request_time(timestamp TSRMLS_CC);
	}

	RETURN_TRUE;
}
/* }}} */

/* {{{ proto int timecop_return(void)
   Return to Time travel to specified timestamp */
PHP_FUNCTION(timecop_return)
{
	TIMECOP_G(timecop_mode) = TIMECOP_MODE_NORMAL;

	if (TIMECOP_G(sync_request_time)){
		restore_request_time(TSRMLS_C);
	}

	RETURN_TRUE;
}
/* }}} */

/* {{{ proto int timecop_time(void)
   Return virtual timestamp */
PHP_FUNCTION(timecop_time)
{
	RETURN_LONG(timecop_current_timestamp(TSRMLS_C));
}
/* }}} */

static int init_fcall_info(zval *callable, zend_fcall_info * fci, zend_fcall_info_cache * fcc, int num_args TSRMLS_DC)
{
	zval **args;
	int i;

	if (timecop_zend_fcall_info_init(callable, fci, fcc TSRMLS_CC) == FAILURE) {
		return 0;
	}
	fci->param_count = num_args;
	fci->params = safe_emalloc(num_args, sizeof(zval**), 0);
	args = safe_emalloc(num_args, sizeof(zval *), 0);
	for (i = 0; i < num_args; i++) {
		MAKE_STD_ZVAL(args[i]);
		fci->params[i] = &args[i];
	}
	fci->no_separation = 0;
	return 1;
}

static int init_timecop_date_fcall_info(zval *callable, zend_fcall_info * fci, zend_fcall_info_cache * fcc TSRMLS_DC)
{
	int ret;
	ret = init_fcall_info(callable, fci, fcc, 2 TSRMLS_CC);
	if (ret) {
		ZVAL_LONG(*fci->params[1], timecop_current_timestamp(TSRMLS_C));
	}
	return ret;
}

static void dtor_datefunc_info(zend_fcall_info *fci)
{
	if (fci->params) {
		int i;
		for (i = 0; i < fci->param_count; i++) {
			zval_ptr_dtor(fci->params[i]);
		}
		efree(*fci->params);
		efree(fci->params);
	}
}

static zval *timecop_date_fcall(const char *format, zend_fcall_info * fci, zend_fcall_info_cache * fcc TSRMLS_DC)
{
	zval *zvalue;
	MAKE_STD_ZVAL(zvalue);
	ZVAL_STRING(*(fci->params[0]), format, 0);
	if (zend_call_function(fci, fcc TSRMLS_CC) == SUCCESS &&
		fci->retval_ptr_ptr && *fci->retval_ptr_ptr) {
		COPY_PZVAL_TO_ZVAL(*zvalue, *fci->retval_ptr_ptr);
	}
	ZVAL_NULL(*(fci->params[0]));
	return zvalue;
}

static zval ***alloc_fcall_params(int num_args)
{
	int i;
	zval ***params, **args;
	params = safe_emalloc(num_args, sizeof(zval**), 0);
	args = safe_emalloc(num_args, sizeof(zval *), 0);
	for (i = 0; i < num_args; i++) {
		MAKE_STD_ZVAL(args[i]);
		params[i] = &args[i];
	}
	return params;
}

static void dtor_fcall_params(zval ***params, int num_args)
{
	int i;
	for (i = 0; i < num_args; i++) {
		zval_ptr_dtor(params[i]);
	}
	efree(*params);
	efree(params);
}

static void copy_fcall_params(zval ***src, zval ***dst, int num_args)
{
	int i;
	for (i = 0; i < num_args; i++) {
		ZVAL_ZVAL(*dst[i], *src[i], 1, 0)
	}
}

static int fill_mktime_params(zval ***params, zval *date_callable, int from TSRMLS_DC)
{
	zval *zp, *date_retval_ptr = NULL;
	zend_fcall_info date_fci;
	zend_fcall_info_cache date_fci_cache;
	char *formats[] = {"H", "i", "s", "n", "j", "Y"};
	int i, max_params = 6;

	if (!init_timecop_date_fcall_info(date_callable, &date_fci, &date_fci_cache TSRMLS_CC)) {
		return from;
	}
	date_fci.retval_ptr_ptr = &date_retval_ptr;

	for (i = from; i < max_params; i++) {
		zp = timecop_date_fcall(formats[i], &date_fci, &date_fci_cache TSRMLS_CC);
		ZVAL_ZVAL(*params[i], zp, 1, 1);
	}
	dtor_datefunc_info(&date_fci);

	return max_params;
}

static long timecop_current_timestamp(TSRMLS_D)
{
	zval **array, **request_time_long;
	long current_timestamp;

	switch (TIMECOP_G(timecop_mode)) {
	case TIMECOP_MODE_FREEZE:
		current_timestamp = TIMECOP_G(freezed_timestamp);
		break;
	case TIMECOP_MODE_TRAVEL:
		current_timestamp = time(NULL) + TIMECOP_G(travel_offset);
		break;
	default:
		current_timestamp = time(NULL);
		break;
	}

	return current_timestamp;
}

static zval *timecop_current_date(char *format TSRMLS_DC)
{
	zval *zp, *date_retval_ptr = NULL;
	zend_fcall_info date_fci;
	zend_fcall_info_cache date_fci_cache;
	zval date_callable;

	if (TIMECOP_G(func_override)){
		ZVAL_STRING(&date_callable, "timecop_orig_date", 1);
	} else {
		ZVAL_STRING(&date_callable, "date", 1);
	}
	if (!init_timecop_date_fcall_info(&date_callable, &date_fci, &date_fci_cache TSRMLS_CC)) {
		return NULL;
	}
	date_fci.retval_ptr_ptr = &date_retval_ptr;
	zp = timecop_date_fcall(format, &date_fci, &date_fci_cache TSRMLS_CC);

	dtor_datefunc_info(&date_fci);

	zval_dtor(&date_callable);
	return zp;
}

/* {{{ php_timecop_mktime - timecop_(gm)mktime helper */
PHPAPI void php_timecop_mktime(INTERNAL_FUNCTION_PARAMETERS, zval *mktime_callable, zval *date_callable)
{
	zval *retval_ptr = NULL;
	zend_fcall_info fci;
	zend_fcall_info_cache fci_cache;
	int argc = ZEND_NUM_ARGS();
	zval ***args = NULL;

	if (timecop_zend_fcall_info_init(mktime_callable, &fci, &fci_cache TSRMLS_CC) == FAILURE) {
		return;
	}
	fci.retval_ptr_ptr = &retval_ptr;
	fci.no_separation = 0;

	args = (zval ***) safe_emalloc(argc, sizeof(zval **), 0);
	if (zend_get_parameters_array_ex(argc, args) == FAILURE) {
		efree(args);
		return;
	}

	if (ZEND_NUM_ARGS() >= 6) {
		fci.params = args;
		fci.param_count = argc;
	} else {
		fci.params = alloc_fcall_params(6);
		copy_fcall_params(args, fci.params, argc);
		if (args) {
			efree(args);
		}
		fci.param_count = fill_mktime_params(fci.params, date_callable, argc TSRMLS_CC);
	}

	if (ZEND_NUM_ARGS() == 0) {
		php_error_docref(NULL TSRMLS_CC, E_STRICT, "You should be using the time() function instead");
	}

	if (zend_call_function(&fci, &fci_cache TSRMLS_CC) == SUCCESS && fci.retval_ptr_ptr && *fci.retval_ptr_ptr) {
		COPY_PZVAL_TO_ZVAL(*return_value, *fci.retval_ptr_ptr);
	}

	if (fci.params) {
		if (ZEND_NUM_ARGS() >= 6) {
			efree(fci.params);
		} else {
			dtor_fcall_params(fci.params, 6);
		}
	}
}

/* {{{ proto int timecop_mktime([int hour [, int min [, int sec [, int mon [, int day [, int year]]]]]])
   Get UNIX timestamp for a date */
PHP_FUNCTION(timecop_mktime)
{
	zval mktime_callable, date_callable;
	if (TIMECOP_G(func_override)){
		ZVAL_STRING(&mktime_callable, "timecop_orig_mktime", 1);
		ZVAL_STRING(&date_callable, "timecop_orig_date", 1);
	} else {
		ZVAL_STRING(&mktime_callable, "mktime", 1);
		ZVAL_STRING(&date_callable, "date", 1);
	}
	php_timecop_mktime(INTERNAL_FUNCTION_PARAM_PASSTHRU, &mktime_callable, &date_callable);

	zval_dtor(&mktime_callable);
	zval_dtor(&date_callable);
}
/* }}} */

/* {{{ proto int timecop_gmmktime([int hour [, int min [, int sec [, int mon [, int day [, int year]]]]]])
   Get UNIX timestamp for a GMT date */
PHP_FUNCTION(timecop_gmmktime)
{
	zval mktime_callable, date_callable;
	if (TIMECOP_G(func_override)){
		ZVAL_STRING(&mktime_callable, "timecop_orig_gmmktime", 1);
		ZVAL_STRING(&date_callable, "timecop_orig_gmdate", 1);
	} else {
		ZVAL_STRING(&mktime_callable, "gmmktime", 1);
		ZVAL_STRING(&date_callable, "gmdate", 1);
	}
	php_timecop_mktime(INTERNAL_FUNCTION_PARAM_PASSTHRU, &mktime_callable, &date_callable);

	zval_dtor(&mktime_callable);
	zval_dtor(&date_callable);
}
/* }}} */

/* {{{ proto string timecop_date(string format [, long timestamp])
   Format a local date/time */
PHP_FUNCTION(timecop_date)
{
	_timecop_call_function(INTERNAL_FUNCTION_PARAM_PASSTHRU, "date", "timecop_orig_date", 1);
}
/* }}} */

/* {{{ proto string timecop_gmdate(string format [, long timestamp])
   Format a GMT date/time */
PHP_FUNCTION(timecop_gmdate)
{
	_timecop_call_function(INTERNAL_FUNCTION_PARAM_PASSTHRU, "gmdate", "timecop_orig_gmdate", 1);
}
/* }}} */

/* {{{ proto int timecop_idate(string format [, int timestamp])
   Format a local time/date as integer */
PHP_FUNCTION(timecop_idate)
{
	_timecop_call_function(INTERNAL_FUNCTION_PARAM_PASSTHRU, "idate", "timecop_orig_idate", 1);
}
/* }}} */

/* {{{ proto array timecop_getdate([int timestamp])
   Get date/time information */
PHP_FUNCTION(timecop_getdate)
{
	_timecop_call_function(INTERNAL_FUNCTION_PARAM_PASSTHRU, "getdate", "timecop_orig_getdate", 0);
}
/* }}} */

/* {{{ proto array timecop_localtime([int timestamp [, bool associative_array]])
   Returns the results of the C system call localtime as an associative array if
 the associative_array argument is set to 1 other wise it is a regular array */
PHP_FUNCTION(timecop_localtime)
{
	_timecop_call_function(INTERNAL_FUNCTION_PARAM_PASSTHRU, "localtime", "timecop_orig_localtime", 0);
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

/* {{{ proto int timecop_unixtojd([int timestamp])
   Convert UNIX timestamp to Julian Day */
PHP_FUNCTION(timecop_unixtojd)
{
	_timecop_call_function(INTERNAL_FUNCTION_PARAM_PASSTHRU, "unixtojd", "timecop_orig_unixtojd", 0);
}
/* }}} */

/* {{{ proto TimecopDateTime::__construct([string time[, DateTimeZone object]])
   Creates new TimecopDateTime object
*/
PHP_METHOD(TimecopDateTime, __construct)
{
	int param_count, orig_param_count;
	zval ***params;
	zval *obj = getThis();
	zend_class_entry *datetime_ce;

	datetime_ce = TIMECOP_G(ce_DateTime);

	param_count = orig_param_count = ZEND_NUM_ARGS();

	params = (zval ***) safe_emalloc(param_count, sizeof(zval **), 0);
	if (zend_get_parameters_array_ex(param_count, params) == FAILURE) {
		if (params) {
			efree(params);
		}
		return;
	}
	if (orig_param_count == 0) {
		int i;
		zval **zpp, *zp;

		param_count = 1;
		if (params) {
			efree(params);
		}
		params = (zval ***)safe_emalloc(param_count, sizeof(zval **), 0);
		zpp = (zval **)safe_emalloc(param_count, sizeof(zval *), 0);
		for (i = 0; i < param_count; i++) {
			MAKE_STD_ZVAL(zpp[i]);
			params[i] = &zpp[i];
		}
		zp = timecop_current_date("Y-m-d H:i:s" TSRMLS_CC);
		ZVAL_ZVAL(*params[0], zp, 1, 1);
	}

	/* call DateTime::__constuctor() */
	call_constructor(&obj, datetime_ce, params, param_count TSRMLS_CC);

	if (orig_param_count == 0) {
		int i;
		for (i = 0; i < param_count; i++) {
			zval_ptr_dtor(params[i]);
		}
		if (params && params[0]) {
			efree(params[0]);
		}
	}
	if (params) {
		efree(params);
	}
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
