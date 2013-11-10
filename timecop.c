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
#include "ext/date/php_date.h"

#if !defined(PHP_VERSION_ID) || PHP_VERSION_ID < 50300
#include "ext/date/lib/timelib.h"
#endif

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
	{"date_create", "timecop_date_create", "timecop_orig_date_create"},
#if PHP_VERSION_ID >= 50300
	{"date_create_from_format", "timecop_date_create_from_format", "timecop_orig_date_create_from_format"},
#endif
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

ZEND_BEGIN_ARG_INFO_EX(arginfo_timecop_date_create, 0, 0, 0)
	ZEND_ARG_INFO(0, time)
	ZEND_ARG_INFO(0, object)
ZEND_END_ARG_INFO()

#if PHP_VERSION_ID >= 50300
ZEND_BEGIN_ARG_INFO_EX(arginfo_timecop_date_create_from_format, 0, 0, 2)
	ZEND_ARG_INFO(0, format)
	ZEND_ARG_INFO(0, time)
	ZEND_ARG_INFO(0, object)
ZEND_END_ARG_INFO()
#endif

#if !defined(PHP_VERSION_ID) || PHP_VERSION_ID < 50300
ZEND_BEGIN_ARG_INFO_EX(arginfo_timecop_date_method_timestamp_set, 0, 0, 1)
        ZEND_ARG_INFO(0, unixtimestamp)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_timecop_date_method_timestamp_get, 0)
ZEND_END_ARG_INFO()
#endif

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
	PHP_FE(timecop_date_create, arginfo_timecop_date_create)
#if PHP_VERSION_ID >= 50300
	PHP_FE(timecop_date_create_from_format, arginfo_timecop_date_create_from_format)
#endif
	{NULL, NULL, NULL}
};
/* }}} */

/* declare method parameters, */

/* each method can have its own parameters and visibility */
static zend_function_entry timecop_datetime_class_functions[] = {
	PHP_ME(TimecopDateTime, __construct, arginfo_timecop_date_create,
		   ZEND_ACC_CTOR | ZEND_ACC_PUBLIC)
#if PHP_VERSION_ID >= 50300
   PHP_ME_MAPPING(createFromFormat, timecop_date_create_from_format, arginfo_timecop_date_create_from_format,
		   ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
#endif
#if !defined(PHP_VERSION_ID) || PHP_VERSION_ID < 50300
	PHP_ME(TimecopDateTime, getTimestamp,
				   arginfo_timecop_date_method_timestamp_get, 0)
	PHP_ME(TimecopDateTime, setTimestamp,
				   arginfo_timecop_date_method_timestamp_set, 0)
#endif
	{NULL, NULL, NULL}
};

#define MKTIME_NUM_ARGS 6

#define ORIG_FUNC_NAME(funcname) \
	TIMECOP_G(func_override) ? "timecop_orig_" funcname : funcname

#define TIMECOP_CALL_FUNCTION(func_name, index_to_fill_timestamp) \
	{\
		zval *retval;\
		_timecop_call_function(INTERNAL_FUNCTION_PARAM_PASSTHRU, func_name, &retval, index_to_fill_timestamp);\
		RETURN_ZVAL(retval, 0, 1);\
	}

#define TIMECOP_CALL_MKTIME(mktime_func_name, date_func_name) \
	{\
		zval *retval;\
		_timecop_call_mktime(INTERNAL_FUNCTION_PARAM_PASSTHRU, mktime_func_name, date_func_name, &retval);\
		RETURN_ZVAL(retval, 0, 1);\
	}

#if !defined(PHP_VERSION_ID) || PHP_VERSION_ID < 50300
#define DATE_CHECK_INITIALIZED(member, class_name) \
        if (!(member)) { \
                php_error_docref(NULL TSRMLS_CC, E_WARNING, "The " #class_name " object has not been correctly initialized by its constructor"); \
                RETURN_FALSE; \
        }

typedef struct _php_date_obj php_date_obj;

struct _php_date_obj {
        zend_object   std;
        timelib_time *time;
};
#endif

static void timecop_globals_ctor(zend_timecop_globals *globals TSRMLS_DC);

static int register_timecop_classes(TSRMLS_D);
static int timecop_func_override(TSRMLS_D);
static int timecop_class_override(TSRMLS_D);
static int timecop_func_override_clear(TSRMLS_D);
static int timecop_class_override_clear(TSRMLS_D);

static int update_request_time(long unixtime TSRMLS_DC);
static int restore_request_time(TSRMLS_D);

static long timecop_current_timestamp(TSRMLS_D);

static int fill_mktime_params(zval ***params, const char *date_function_name, int from TSRMLS_DC);
static int fix_datetime_timestamp(zval **datetime_obj, zval *time, zval *timezone_obj TSRMLS_DC);

static void _timecop_call_function(INTERNAL_FUNCTION_PARAMETERS, const char *function_name, zval **retval_ptr_ptr, int index_to_fill_timestamp);
static void _timecop_call_mktime(INTERNAL_FUNCTION_PARAMETERS, const char *mktime_function_name, const char *date_function_name, zval **retval_ptr_ptr);

static void call_constructor(zval **object_pp, zend_class_entry *ce, zval ***params, int param_count TSRMLS_DC);
static void simple_call_function(const char *function_name, zval **retval_ptr_ptr, zend_uint param_count, zval **params[] TSRMLS_DC);
static zval **alloc_zval_ptr_ptr();
static void zval_ptr_ptr_dtor(zval **zval_ptr_ptr);
static zval *php_timecop_date_instantiate(zend_class_entry *pce, zval *object TSRMLS_DC);

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
	PHP_TIMECOP_VERSION,
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

			if (zend_hash_add(EG(class_table), p->save_name, strlen(p->save_name)+1,
							  &ce_orig, sizeof(zend_class_entry *), NULL) == FAILURE) {
				return FAILURE;
			}
			ce_orig->refcount++;

			if (zend_hash_update(EG(class_table), p->orig_name, strlen(p->orig_name)+1,
								 &ce_ovld, sizeof(zend_class_entry *), NULL) == FAILURE) {
				return FAILURE;
			}
			ce_ovld->refcount++;
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

			if (zend_hash_update(EG(class_table), p->orig_name, strlen(p->orig_name)+1,
								 &ce_orig, sizeof(zend_class_entry *), NULL) == FAILURE) {
				return FAILURE;
			}
			ce_orig->refcount++;

			if (zend_hash_del(EG(class_table), p->save_name, strlen(p->save_name)+1) == FAILURE) {
				return FAILURE;
			}
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

static int fill_mktime_params(zval ***params, const char *date_function_name, int from TSRMLS_DC)
{
	zval time, format;
	char *formats[MKTIME_NUM_ARGS] = {"H", "i", "s", "n", "j", "Y"};
	int i;
	zval **date_params[2], *tmp[2];

	date_params[0] = &tmp[0];
	date_params[1] = &tmp[1];
	tmp[0] = &format;
	tmp[1] = &time;

	INIT_ZVAL(time);
	ZVAL_LONG(&time, timecop_current_timestamp(TSRMLS_C));

	for (i = from; i < MKTIME_NUM_ARGS; i++) {
		INIT_ZVAL(format);
		ZVAL_STRING(&format, formats[i], 0);

		simple_call_function(date_function_name, params[i], 2, date_params TSRMLS_CC);
	}

	return MKTIME_NUM_ARGS;
}

static int fix_datetime_timestamp(zval **datetime_obj, zval *time, zval *timezone_obj TSRMLS_DC)
{
	zval *orig_timestamp, *fixed_timestamp;
	zval now;
	zval *orig_zonename;

	INIT_ZVAL(now);
	ZVAL_STRING(&now, "now", 0);

	if (time == NULL) {
		time = &now;
	} else {
		zval *len;
		zend_call_method_with_1_params(NULL, NULL, NULL, "strlen", &len, time);
		if (len) {
			if (Z_LVAL_P(len) == 0) {
				time = &now;
			}
			zval_ptr_dtor(&len);
		}
	}

	//zend_call_method_with_1_params(NULL, NULL, NULL, "var_dump", NULL, &now);

	zend_call_method_with_0_params(datetime_obj, Z_OBJCE_PP(datetime_obj), NULL, "gettimestamp", &orig_timestamp);
	if (timezone_obj) {
		zval *zonename;
		zend_call_method_with_0_params(&timezone_obj, Z_OBJCE_PP(&timezone_obj), NULL, "getname", &zonename);
		zend_call_method_with_0_params(NULL, NULL, NULL, "date_default_timezone_get", &orig_zonename);
		zend_call_method_with_1_params(NULL, NULL, NULL, "date_default_timezone_set", NULL, zonename);
		zval_ptr_dtor(&zonename);
	}
	zend_call_method_with_1_params(NULL, NULL, NULL, "timecop_strtotime", &fixed_timestamp, time);
	if (timezone_obj) {
		zend_call_method_with_1_params(NULL, NULL, NULL, "date_default_timezone_set", NULL, orig_zonename);
		zval_ptr_dtor(&orig_zonename);
	}

	if (Z_TYPE_P(fixed_timestamp) == IS_BOOL && Z_BVAL_P(fixed_timestamp) == 0) {
		// timecop_strtotime($time) === false
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Failed to parse time string '%s': giving up time traveling", Z_STRVAL_P(time));
	} else if (Z_LVAL_P(orig_timestamp) != Z_LVAL_P(fixed_timestamp)) {
		zend_call_method_with_1_params(datetime_obj, Z_OBJCE_PP(datetime_obj), NULL, "settimestamp", NULL, fixed_timestamp);
	}

	if (orig_timestamp) {
		zval_ptr_dtor(&orig_timestamp);
	}
	if (fixed_timestamp) {
		zval_ptr_dtor(&fixed_timestamp);
	}

	return 0;
}

static void _timecop_call_function(INTERNAL_FUNCTION_PARAMETERS, const char *function_name, zval **retval_ptr_ptr, int index_to_fill_timestamp)
{
	zval ***params;
	zend_uint param_count;
	int params_size;

	params_size = MAX(ZEND_NUM_ARGS(), index_to_fill_timestamp+1);
	params = (zval ***)safe_emalloc(sizeof(zval **), params_size, 0);

	if (zend_get_parameters_array_ex(ZEND_NUM_ARGS(), params) == FAILURE) {
		efree(params);
		return;
	}
	param_count = ZEND_NUM_ARGS();

	if (ZEND_NUM_ARGS() == index_to_fill_timestamp) {
		zval *zp, **zpp;
		int last_index = param_count;

		ALLOC_INIT_ZVAL(zp);
		ZVAL_LONG(zp, timecop_current_timestamp(TSRMLS_C));

		params[last_index] = zpp = alloc_zval_ptr_ptr();
		*zpp = zp;

		param_count++;
	}

	simple_call_function(function_name, retval_ptr_ptr, param_count, params TSRMLS_CC);

	if (ZEND_NUM_ARGS() == index_to_fill_timestamp) {
		int last_index = param_count-1;
		zval_ptr_ptr_dtor(params[last_index]);
	}
	efree(params);
}

/* {{{ _timecop_call_mktime - timecop_(gm)mktime helper */
static void _timecop_call_mktime(INTERNAL_FUNCTION_PARAMETERS, const char *mktime_function_name, const char *date_function_name, zval **retval_ptr_ptr)
{
	int params_size;
	zval ***params;
	zend_uint param_count;
	zval *retval;
	int i;

	params_size = MAX(ZEND_NUM_ARGS(), MKTIME_NUM_ARGS);
	params = (zval ***)safe_emalloc(sizeof(zval **), params_size, 0);

	if (zend_get_parameters_array_ex(ZEND_NUM_ARGS(), params) == FAILURE) {
		efree(params);
		return;
	}
	param_count = ZEND_NUM_ARGS();

	if (ZEND_NUM_ARGS() < MKTIME_NUM_ARGS) {
		for (i = ZEND_NUM_ARGS(); i < MKTIME_NUM_ARGS; i++) {
			params[i] = alloc_zval_ptr_ptr();
		}
		fill_mktime_params(params, date_function_name, ZEND_NUM_ARGS() TSRMLS_CC);
		param_count = MKTIME_NUM_ARGS;
	}

	if (ZEND_NUM_ARGS() == 0) {
		php_error_docref(NULL TSRMLS_CC, E_STRICT, "You should be using the time() function instead");
	}

	simple_call_function(mktime_function_name, retval_ptr_ptr, param_count, params TSRMLS_CC);

	if (ZEND_NUM_ARGS() < MKTIME_NUM_ARGS) {
		for (i = ZEND_NUM_ARGS(); i < MKTIME_NUM_ARGS; i++) {
			zval_ptr_ptr_dtor(params[i]);
		}
	}
	efree(params);
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

/* {{{ proto int timecop_mktime([int hour [, int min [, int sec [, int mon [, int day [, int year]]]]]])
   Get UNIX timestamp for a date */
PHP_FUNCTION(timecop_mktime)
{
	TIMECOP_CALL_MKTIME(ORIG_FUNC_NAME("mktime"), ORIG_FUNC_NAME("date"));
}
/* }}} */

/* {{{ proto int timecop_gmmktime([int hour [, int min [, int sec [, int mon [, int day [, int year]]]]]])
   Get UNIX timestamp for a GMT date */
PHP_FUNCTION(timecop_gmmktime)
{
	TIMECOP_CALL_MKTIME(ORIG_FUNC_NAME("gmmktime"), ORIG_FUNC_NAME("gmdate"));
}
/* }}} */

/* {{{ proto string timecop_date(string format [, long timestamp])
   Format a local date/time */
PHP_FUNCTION(timecop_date)
{
	TIMECOP_CALL_FUNCTION(ORIG_FUNC_NAME("date"), 1);
}
/* }}} */

/* {{{ proto string timecop_gmdate(string format [, long timestamp])
   Format a GMT date/time */
PHP_FUNCTION(timecop_gmdate)
{
	TIMECOP_CALL_FUNCTION(ORIG_FUNC_NAME("gmdate"), 1);
}
/* }}} */

/* {{{ proto int timecop_idate(string format [, int timestamp])
   Format a local time/date as integer */
PHP_FUNCTION(timecop_idate)
{
	TIMECOP_CALL_FUNCTION(ORIG_FUNC_NAME("idate"), 1);
}
/* }}} */

/* {{{ proto array timecop_getdate([int timestamp])
   Get date/time information */
PHP_FUNCTION(timecop_getdate)
{
	TIMECOP_CALL_FUNCTION(ORIG_FUNC_NAME("getdate"), 0);
}
/* }}} */

/* {{{ proto array timecop_localtime([int timestamp [, bool associative_array]])
   Returns the results of the C system call localtime as an associative array if
 the associative_array argument is set to 1 other wise it is a regular array */
PHP_FUNCTION(timecop_localtime)
{
	TIMECOP_CALL_FUNCTION(ORIG_FUNC_NAME("localtime"), 0);
}
/* }}} */

/* {{{ proto int timecop_strtotime(string time [, int now ])
   Convert string representation of date and time to a timestamp */
PHP_FUNCTION(timecop_strtotime)
{
	TIMECOP_CALL_FUNCTION(ORIG_FUNC_NAME("strtotime"), 1);
}
/* }}} */

/* {{{ proto string timecop_strftime(string format [, int timestamp])
   Format a local time/date according to locale settings */
PHP_FUNCTION(timecop_strftime)
{
	TIMECOP_CALL_FUNCTION(ORIG_FUNC_NAME("strftime"), 1);
}
/* }}} */

/* {{{ proto string timecop_gmstrftime(string format [, int timestamp])
   Format a GMT/UCT time/date according to locale settings */
PHP_FUNCTION(timecop_gmstrftime)
{
	TIMECOP_CALL_FUNCTION(ORIG_FUNC_NAME("gmstrftime"), 1);
}
/* }}} */

/* {{{ proto int timecop_unixtojd([int timestamp])
   Convert UNIX timestamp to Julian Day */
PHP_FUNCTION(timecop_unixtojd)
{
	TIMECOP_CALL_FUNCTION(ORIG_FUNC_NAME("unixtojd"), 0);
}
/* }}} */

/* {{{ proto TimecopDateTime timecop_date_create([string time[, DateTimeZone object]])
   Returns new TimecopDateTime object
*/
PHP_FUNCTION(timecop_date_create)
{
	zval ***params, *datetime_obj, *time = NULL;

	params = (zval ***) safe_emalloc(ZEND_NUM_ARGS(), sizeof(zval **), 0);

	if (zend_get_parameters_array_ex(ZEND_NUM_ARGS(), params) == FAILURE) {
		efree(params);
		RETURN_FALSE;
	}

	php_timecop_date_instantiate(TIMECOP_G(ce_TimecopDateTime), return_value TSRMLS_CC);

	/* call TimecopDateTime::__constuctor() */
	call_constructor(&return_value, TIMECOP_G(ce_TimecopDateTime), params, ZEND_NUM_ARGS() TSRMLS_CC);

	efree(params);
}
/* }}} */

#if PHP_VERSION_ID >= 50300
/* {{{ proto TimecopDateTime timecop_date_create_from_format(string format, string time[, DateTimeZone object])
 Returns new TimecopDateTime object
 */
PHP_FUNCTION(timecop_date_create_from_format)
{
	zval *timezone_object = NULL;
	char *time_str = NULL, *format_str = NULL;
	int  time_str_len = 0, format_str_len = 0;

#if PHP_VERSION_ID <= 50303
	php_error_docref(NULL TSRMLS_CC, E_ERROR, "Currently this method is unsupported on PHP 5.3.0-5.3.3. Please upgrade PHP to 5.3.4+.");
#else
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ss|O", &format_str, &format_str_len, &time_str, &time_str_len, &timezone_object, php_date_get_timezone_ce()) == FAILURE) {
		RETURN_FALSE;
	}

	php_timecop_date_instantiate(TIMECOP_G(ce_TimecopDateTime), return_value TSRMLS_CC);
	if (!php_date_initialize(zend_object_store_get_object(return_value TSRMLS_CC), time_str, time_str_len, format_str, timezone_object, 0 TSRMLS_CC)) {
		RETURN_FALSE;
	}
#endif
}
/* }}} */
#endif

/* {{{ proto TimecopDateTime::__construct([string time[, DateTimeZone object]])
   Creates new TimecopDateTime object
*/
PHP_METHOD(TimecopDateTime, __construct)
{
	zval ***params;
	zval *obj = getThis();
	zend_class_entry *datetime_ce;

	params = (zval ***) safe_emalloc(ZEND_NUM_ARGS(), sizeof(zval **), 0);

	if (zend_get_parameters_array_ex(ZEND_NUM_ARGS(), params) == FAILURE) {
		efree(params);
		RETURN_FALSE;
	}

	/* call DateTime::__constuctor() */
	call_constructor(&obj, TIMECOP_G(ce_DateTime), params, ZEND_NUM_ARGS() TSRMLS_CC);

	if (!EG(exception)) {
		zval *time = NULL, *timezone_obj = NULL;
		if (ZEND_NUM_ARGS() >= 1) {
			time = *params[0];
		}
		if (ZEND_NUM_ARGS() >= 2) {
			timezone_obj = *params[1];
		}
		fix_datetime_timestamp(&obj, time, timezone_obj TSRMLS_CC);
	}

	efree(params);
}

#if !defined(PHP_VERSION_ID) || PHP_VERSION_ID < 50300

/* {{{ proto long TimecopDateTime::getTimestamp()
   Gets the Unix timestamp.
*/
PHP_METHOD(TimecopDateTime, getTimestamp)
{
        zval         *object;
        php_date_obj *dateobj;
        long          timestamp;
        int           error;

        if (zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "O", &object, TIMECOP_G(ce_TimecopDateTime)) == FAILURE) {
                RETURN_FALSE;
        }
        dateobj = (php_date_obj *) zend_object_store_get_object(object TSRMLS_CC);
        DATE_CHECK_INITIALIZED(dateobj->time, DateTime);
        timelib_update_ts(dateobj->time, NULL);

        timestamp = timelib_date_to_int(dateobj->time, &error);
        if (error) {
                RETURN_FALSE;
        } else {
                RETVAL_LONG(timestamp);
        }
}
/* }}} */


/* {{{ proto DateTime TimecopDateTime::getTimestamp(long unixTimestamp)
   Sets the date and time based on an Unix timestamp.
*/
PHP_METHOD(TimecopDateTime, setTimestamp)
{
        zval         *object;
        php_date_obj *dateobj;
        long          timestamp;

        if (zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "Ol", &object, TIMECOP_G(ce_TimecopDateTime), &timestamp) == FAILURE) {
                RETURN_FALSE;
        }
        dateobj = (php_date_obj *) zend_object_store_get_object(object TSRMLS_CC);
        DATE_CHECK_INITIALIZED(dateobj->time, DateTime);
        timelib_unixtime2local(dateobj->time, (timelib_sll)timestamp);
        timelib_update_ts(dateobj->time, NULL);

        RETURN_ZVAL(object, 1, 0);
}
/* }}} */
#endif

static void call_constructor(zval **object_pp, zend_class_entry *ce, zval ***params, int param_count TSRMLS_DC)
{
	if (param_count > 2) {
		zend_error(E_ERROR, "INTERNAL ERROR: too many parameters for constructor.");
		return;
	}

	if (param_count == 0) {
		zend_call_method_with_0_params(object_pp, ce, &ce->constructor, "__construct", NULL);
	} else if (param_count == 1) {
		zend_call_method_with_1_params(object_pp, ce, &ce->constructor, "__construct", NULL, *params[0]);
	} else if (param_count == 2) {
		zend_call_method_with_2_params(object_pp, ce, &ce->constructor, "__construct", NULL, *params[0], *params[1]);
	}
}

static void simple_call_function(const char *function_name, zval **retval_ptr_ptr, zend_uint param_count, zval **params[] TSRMLS_DC)
{
	zval callable;

	INIT_ZVAL(callable);
	ZVAL_STRING(&callable, function_name, 0);

	call_user_function_ex(EG(function_table), NULL, &callable, retval_ptr_ptr, param_count, params, 1, NULL TSRMLS_CC);
}

static zval **alloc_zval_ptr_ptr()
{
	zval **zpp;
	zpp = (zval **) emalloc(sizeof(zval *));
	return zpp;
}

static void zval_ptr_ptr_dtor(zval **zval_ptr_ptr)
{
	zval_ptr_dtor(zval_ptr_ptr);
	efree(zval_ptr_ptr);
}

/* Advanced Interface */
static zval *php_timecop_date_instantiate(zend_class_entry *pce, zval *object TSRMLS_DC)
{
        Z_TYPE_P(object) = IS_OBJECT;
        object_init_ex(object, pce);
#if !defined(PHP_VERSION_ID) || PHP_VERSION_ID < 50300
        object->refcount = 1;
        object->is_ref = 0;
#else
        Z_SET_REFCOUNT_P(object, 1);
        Z_UNSET_ISREF_P(object);
#endif
        return object;
}

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
