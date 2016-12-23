/*
The MIT License

Copyright (c) 2012-2016 Yoshio HANAWA

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
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

static const struct timecop_override_func_entry timecop_override_func_table[] = {
	TIMECOP_OFE("time"),
	TIMECOP_OFE("mktime"),
	TIMECOP_OFE("gmmktime"),
	TIMECOP_OFE("date"),
	TIMECOP_OFE("gmdate"),
	TIMECOP_OFE("idate"),
	TIMECOP_OFE("getdate"),
	TIMECOP_OFE("localtime"),
	TIMECOP_OFE("strtotime"),
	TIMECOP_OFE("strftime"),
	TIMECOP_OFE("gmstrftime"),
#ifdef HAVE_GETTIMEOFDAY
	TIMECOP_OFE("microtime"),
	TIMECOP_OFE("gettimeofday"),
#endif
	TIMECOP_OFE("unixtojd"),
	TIMECOP_OFE("date_create"),
#if PHP_VERSION_ID >= 50300
	TIMECOP_OFE("date_create_from_format"),
#endif
	{NULL, NULL, NULL}
};

static const struct timecop_override_class_entry timecop_override_class_table[] = {
	TIMECOP_OCE("datetime", "__construct"),
	{NULL, NULL, NULL, NULL}
};

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

#ifdef HAVE_GETTIMEOFDAY
ZEND_BEGIN_ARG_INFO_EX(arginfo_timecop_microtime, 0, 0, 0)
	ZEND_ARG_INFO(0, get_as_float)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_timecop_gettimeofday, 0, 0, 0)
	ZEND_ARG_INFO(0, get_as_float)
ZEND_END_ARG_INFO()
#endif

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
#ifdef HAVE_GETTIMEOFDAY
	PHP_FE(timecop_microtime, arginfo_timecop_microtime)
	PHP_FE(timecop_gettimeofday, arginfo_timecop_gettimeofday)
#endif
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

static zend_function_entry timecop_orig_datetime_class_functions[] = {
	PHP_ME(TimecopOrigDateTime, __construct, arginfo_timecop_date_create,
		   ZEND_ACC_CTOR | ZEND_ACC_PUBLIC)
	{NULL, NULL, NULL}
};

#define MKTIME_NUM_ARGS 6

#define TIMECOP_CALL_FUNCTION(func_name, index_to_fill_timestamp) \
	_timecop_call_function(INTERNAL_FUNCTION_PARAM_PASSTHRU, ORIG_FUNC_NAME(func_name), index_to_fill_timestamp);

#define TIMECOP_CALL_MKTIME(mktime_func_name, date_func_name) \
	_timecop_call_mktime(INTERNAL_FUNCTION_PARAM_PASSTHRU, ORIG_FUNC_NAME(mktime_func_name), ORIG_FUNC_NAME(date_func_name));

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

static void _timecop_call_function(INTERNAL_FUNCTION_PARAMETERS, const char *function_name, int index_to_fill_timestamp);
static void _timecop_call_mktime(INTERNAL_FUNCTION_PARAMETERS, const char *mktime_function_name, const char *date_function_name);

static inline void timecop_call_original_constructor(zval **obj, zend_class_entry *ce, zval ***params, int param_count TSRMLS_DC);
static inline void timecop_call_constructor(zval **obj, zend_class_entry *ce, zval ***params, int param_count TSRMLS_DC);
static void timecop_call_constructor_ex(zval **obj, zend_class_entry *ce, zval ***params, int param_count, int call_original TSRMLS_DC);

static void simple_call_function(const char *function_name, zval **retval_ptr_ptr, zend_uint param_count, zval **params[] TSRMLS_DC);
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

	INIT_CLASS_ENTRY(ce, "TimecopOrigDateTime", timecop_orig_datetime_class_functions);
	self_ce = zend_register_internal_class_ex(&ce, parent_ce, NULL TSRMLS_CC);
	self_ce->create_object = parent_ce->create_object;

	return SUCCESS;
}

static int timecop_func_override(TSRMLS_D)
{
	const struct timecop_override_func_entry *p;
	zend_function *zf_orig, *zf_ovrd, *zf_save;

	p = &(timecop_override_func_table[0]);
	while (p->orig_func != NULL) {
		if (zend_hash_find(EG(function_table), p->orig_func, strlen(p->orig_func)+1,
						   (void **)&zf_orig) != SUCCESS) {
			// Do nothing. Because some functions are introduced by optional extensions.
			p++;
			continue;
		}
		if (zend_hash_find(EG(function_table), p->ovrd_func, strlen(p->ovrd_func)+1,
						   (void **)&zf_ovrd) != SUCCESS) {
			php_error_docref("https://github.com/hnw/php-timecop" TSRMLS_CC, E_WARNING,
							 "timecop couldn't find function %s.", p->ovrd_func);
			p++;
			continue;
		}
		if (zend_hash_find(EG(function_table), p->save_func, strlen(p->save_func)+1,
						   (void **)&zf_save) == SUCCESS) {
			php_error_docref("https://github.com/hnw/php-timecop" TSRMLS_CC, E_WARNING,
							 "timecop couldn't create function %s because already exists.",
							 p->save_func);
			p++;
			continue;
		}

		TIMECOP_ASSERT(zf_orig->type == ZEND_INTERNAL_FUNCTION);
		TIMECOP_ASSERT(zf_ovrd->type == ZEND_INTERNAL_FUNCTION);

		zend_hash_add(EG(function_table), p->save_func, strlen(p->save_func)+1,
					  zf_orig, sizeof(zend_function), NULL);
		function_add_ref(zf_orig);

		zend_hash_update(EG(function_table), p->orig_func, strlen(p->orig_func)+1,
						 zf_ovrd, sizeof(zend_function), NULL);
		function_add_ref(zf_ovrd);

		p++;
	}
	return SUCCESS;
}

static int timecop_class_override(TSRMLS_D)
{
	const struct timecop_override_class_entry *p;
	zend_class_entry **pce_ovrd, **pce_orig, *ce_ovrd, *ce_orig;
	zend_function *zf_orig, *zf_ovrd, *zf_save, *zf_new;

	p = &(timecop_override_class_table[0]);
	while (p->orig_class != NULL) {
		if (zend_hash_find(EG(class_table), p->ovrd_class, strlen(p->ovrd_class)+1 ,
						   (void **)&pce_ovrd) != SUCCESS) {
			php_error_docref("https://github.com/hnw/php-timecop" TSRMLS_CC, E_WARNING,
							 "timecop couldn't find class %s.", p->ovrd_class);
			p++;
			continue;
		}
		if (zend_hash_find(EG(class_table), p->orig_class, strlen(p->orig_class)+1,
						   (void **)&pce_orig) != SUCCESS) {
			php_error_docref("https://github.com/hnw/php-timecop" TSRMLS_CC, E_WARNING,
							 "timecop couldn't find function %s.", p->orig_class);
			p++;
			continue;
		}

		ce_ovrd = *pce_ovrd;
		ce_orig = *pce_orig;

		if (zend_hash_find(&ce_orig->function_table, p->orig_method, strlen(p->orig_method)+1,
						   (void **)&zf_orig) != SUCCESS) {
			php_error_docref("https://github.com/hnw/php-timecop" TSRMLS_CC, E_WARNING,
							 "timecop couldn't find method %s::%s.",
							 p->orig_class, p->orig_method);
			p++;
			continue;
		}
		if (zend_hash_find(&ce_ovrd->function_table, p->orig_method, strlen(p->orig_method)+1,
						   (void **)&zf_ovrd) != SUCCESS) {
			php_error_docref("https://github.com/hnw/php-timecop" TSRMLS_CC, E_WARNING,
							 "timecop couldn't find method %s::%s.",
							 p->ovrd_class, p->orig_method);
			p++;
			continue;
		}
		if (zend_hash_find(&ce_orig->function_table, p->save_method, strlen(p->save_method)+1,
						   (void **)&zf_save) == SUCCESS) {
			php_error_docref("https://github.com/hnw/php-timecop" TSRMLS_CC, E_WARNING,
							 "timecop couldn't create method %s::%s because already exists.",
							 p->orig_class, p->save_method);
			p++;
			continue;
		}

		TIMECOP_ASSERT(zf_orig->type == ZEND_INTERNAL_FUNCTION);
		TIMECOP_ASSERT(ce_orig->type & ZEND_INTERNAL_CLASS);

		zend_hash_add(&ce_orig->function_table, p->save_method, strlen(p->save_method)+1,
					  zf_orig, sizeof(zend_function), NULL);
		function_add_ref(zf_orig);

		TIMECOP_ASSERT(zf_ovrd->type == ZEND_INTERNAL_FUNCTION);
		TIMECOP_ASSERT(ce_ovrd->type & ZEND_INTERNAL_CLASS);

		zend_hash_update(&ce_orig->function_table, p->orig_method, strlen(p->orig_method)+1,
						 zf_ovrd, sizeof(zend_function), (void**)&zf_new);
		function_add_ref(zf_new);

		//TIMECOP_ASSERT(zf_new != NULL);
		//TIMECOP_ASSERT(zf_new != zf_orig);

		if (strcmp(p->orig_method, "__construct") == 0) {
			ce_orig->constructor = zf_new;
		}
		p++;
	}
	return SUCCESS;
}

/*  clear overrideed function. */
static int timecop_func_override_clear(TSRMLS_D)
{
	const struct timecop_override_func_entry *p;
	zend_function *zf_orig;

	p = &(timecop_override_func_table[0]);
	while (p->orig_func != NULL) {
		if (zend_hash_find(EG(function_table), p->save_func, strlen(p->save_func)+1,
						   (void **)&zf_orig) != SUCCESS) {
			p++;
			continue;
		}

		zend_hash_update(EG(function_table), p->orig_func, strlen(p->orig_func)+1,
						 zf_orig, sizeof(zend_function), NULL);
		function_add_ref(zf_orig);

		zend_hash_del(EG(function_table), p->save_func, strlen(p->save_func)+1);

		p++;
	}
	return SUCCESS;
}

static int timecop_class_override_clear(TSRMLS_D)
{
	const struct timecop_override_class_entry *p;
	zend_class_entry **pce_orig, *ce_orig;
	zend_function *zf_orig, *zf_ovrd, *zf_save;

	p = &(timecop_override_class_table[0]);
	while (p->orig_class != NULL) {
		if (zend_hash_find(EG(class_table), p->orig_class, strlen(p->orig_class)+1,
						   (void **)&pce_orig) != SUCCESS) {
			php_error_docref("https://github.com/hnw/php-timecop" TSRMLS_CC, E_WARNING,
							 "timecop couldn't find class %s.", p->orig_class);
			p++;
			continue;
		}
		ce_orig = *pce_orig;

		if (zend_hash_find(&ce_orig->function_table, p->save_method, strlen(p->save_method)+1,
						   (void **)&zf_orig) != SUCCESS) {
			php_error_docref("https://github.com/hnw/php-timecop" TSRMLS_CC, E_WARNING,
							 "timecop couldn't find method %s::%s.",
							 p->orig_class, p->save_method);
			p++;
			continue;
		}

		zend_hash_update(&ce_orig->function_table, p->orig_method, strlen(p->orig_method)+1,
						 zf_orig, sizeof(zend_function), NULL);
		function_add_ref(zf_orig);

		zend_hash_del(&ce_orig->function_table, p->save_method, strlen(p->save_method)+1);

		if (strcmp(p->orig_method, "__construct") == 0) {
			ce_orig->constructor = zf_orig;
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
		TIMECOP_G(orig_request_time) = NULL;
	}
	return SUCCESS;
}

static long timecop_current_timestamp(TSRMLS_D)
{
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
	int i;
	char *formats[MKTIME_NUM_ARGS] = {"H", "i", "s", "n", "j", "Y"};
	zval time, format, *retval_ptr;
	zval *zps[2] = {&format, &time};
	zval **date_params[2] = {&zps[0], &zps[1]};

	INIT_ZVAL(time);
	ZVAL_LONG(&time, timecop_current_timestamp(TSRMLS_C));

	for (i = from; i < MKTIME_NUM_ARGS; i++) {
		INIT_ZVAL(format);
		ZVAL_STRING(&format, formats[i], 0);

		simple_call_function(date_function_name, &retval_ptr, 2, date_params TSRMLS_CC);
		if (retval_ptr) {
			ZVAL_ZVAL(*params[i], retval_ptr, 1, 1);
		}
	}

	return MKTIME_NUM_ARGS;
}

static int fix_datetime_timestamp(zval **datetime_obj, zval *time, zval *timezone_obj TSRMLS_DC)
{
	zval *orig_timestamp, *fixed_timestamp;
	zval now;
	zval *orig_zonename;

	if (TIMECOP_G(timecop_mode) == TIMECOP_MODE_NORMAL) {
		return 0;
	}

	/*
	 * if ($time === null || $time === false || $time === "") {
	 *     $time = "now";
	 * }
	 */
	if (time == NULL || Z_TYPE_P(time) == IS_NULL ||
		(Z_TYPE_P(time) == IS_BOOL && !Z_BVAL_P(time)) ||
		(Z_TYPE_P(time) == IS_STRING && Z_STRLEN_P(time) == 0)) {
		INIT_ZVAL(now);
		ZVAL_STRING(&now, "now", 0);
		time = &now;
	}

	/*
	 * $orig_timestamp = $datetime_obj->getTimeStamp();
	 */
	zend_call_method_with_0_params(datetime_obj, Z_OBJCE_PP(datetime_obj), NULL, "gettimestamp", &orig_timestamp);

	/*
	 * if ($timezone_obj) {
	 *     $zonename = $timezone_obj->getName()
	 *     if ($zonename) {
	 *         $orig_zonename = date_default_timezone_get();
	 *         if ($orig_zonename) {
	 *             date_default_timezone_set($zonename);
	 *         }
	 *     }
	 * }
	 */
	if (timezone_obj) {
		zval *zonename;
		zend_call_method_with_0_params(&timezone_obj, Z_OBJCE_PP(&timezone_obj), NULL, "getname", &zonename);
		if (zonename) {
			zend_call_method_with_0_params(NULL, NULL, NULL, "date_default_timezone_get", &orig_zonename);
			if (orig_zonename) {
				zend_call_method_with_1_params(NULL, NULL, NULL, "date_default_timezone_set", NULL, zonename);
			}
			zval_ptr_dtor(&zonename);
		}
	}

	/*
	 * $fixed_timestamp = timecop_strtotime($time);
	 */
	zend_call_method_with_1_params(NULL, NULL, NULL, "timecop_strtotime", &fixed_timestamp, time);

	/*
	 * if ($timezone_obj && $orig_zonename) {
	 *     date_default_timezone_set($orig_zonename);
	 * }
	 */
	if (timezone_obj && orig_zonename) {
		zend_call_method_with_1_params(NULL, NULL, NULL, "date_default_timezone_set", NULL, orig_zonename);
		zval_ptr_dtor(&orig_zonename);
	}

	/*
	 * if ($fixed_timestamp === false) {
	 *     // warn
	 * } elseif ($orig_timestamp != $fixed_timestamp) {
	 *     $datetime_obj->setTimeStamp($fixed_timestamp)
	 * }
	 */
	if (Z_TYPE_P(fixed_timestamp) == IS_BOOL && !Z_BVAL_P(fixed_timestamp)) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Failed to parse time string '%s': giving up time traveling", Z_STRVAL_P(time));
	} else if (Z_TYPE_P(orig_timestamp) == IS_LONG &&
			   Z_TYPE_P(fixed_timestamp) == IS_LONG &&
			   Z_LVAL_P(orig_timestamp) != Z_LVAL_P(fixed_timestamp)) {
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

static void _timecop_call_function(INTERNAL_FUNCTION_PARAMETERS, const char *function_name, int index_to_fill_timestamp)
{
	int params_size;
	zval ***params, *zp, filled_timestamp, *retval_ptr;
	zend_uint argc;

	params_size = MAX(ZEND_NUM_ARGS(), index_to_fill_timestamp+1);
	params = (zval ***)safe_emalloc(sizeof(zval **), params_size, 0);

	if (zend_get_parameters_array_ex(ZEND_NUM_ARGS(), params) == FAILURE) {
		efree(params);
		return;
	}
	argc = ZEND_NUM_ARGS();

	if (ZEND_NUM_ARGS() == index_to_fill_timestamp) {
		INIT_ZVAL(filled_timestamp);
		ZVAL_LONG(&filled_timestamp, timecop_current_timestamp(TSRMLS_C));
		zp = &filled_timestamp;

		params[argc] = &zp;
		argc++;
	}

	simple_call_function(function_name, &retval_ptr, argc, params TSRMLS_CC);

	efree(params);

	if (retval_ptr) {
		RETVAL_ZVAL(retval_ptr, 1, 1);
	}
}

/* {{{ _timecop_call_mktime - timecop_(gm)mktime helper */
static void _timecop_call_mktime(INTERNAL_FUNCTION_PARAMETERS, const char *mktime_function_name, const char *date_function_name)
{
	int params_size;
	zval ***params, *filled_value[MKTIME_NUM_ARGS], *retval_ptr;
	zend_uint argc;
	int i;

	params_size = MAX(ZEND_NUM_ARGS(), MKTIME_NUM_ARGS);
	params = (zval ***)safe_emalloc(sizeof(zval **), params_size, 0);

	if (zend_get_parameters_array_ex(ZEND_NUM_ARGS(), params) == FAILURE) {
		efree(params);
		return;
	}
	argc = ZEND_NUM_ARGS();

	for (i = argc; i < MKTIME_NUM_ARGS; i++) {
		ALLOC_INIT_ZVAL(filled_value[i]);
		params[i] = &filled_value[i];
	}
	fill_mktime_params(params, date_function_name, argc TSRMLS_CC);

	if (ZEND_NUM_ARGS() == 0) {
		php_error_docref(NULL TSRMLS_CC, E_STRICT, "You should be using the time() function instead");
	}

	simple_call_function(mktime_function_name, &retval_ptr, params_size, params TSRMLS_CC);

	for (i = argc; i < MKTIME_NUM_ARGS; i++) {
		zval_ptr_dtor(&filled_value[i]);
	}
	efree(params);

	if (retval_ptr) {
		RETVAL_ZVAL(retval_ptr, 1, 1);
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

/* {{{ proto int timecop_mktime([int hour [, int min [, int sec [, int mon [, int day [, int year]]]]]])
   Get UNIX timestamp for a date */
PHP_FUNCTION(timecop_mktime)
{
	TIMECOP_CALL_MKTIME("mktime", "date");
}
/* }}} */

/* {{{ proto int timecop_gmmktime([int hour [, int min [, int sec [, int mon [, int day [, int year]]]]]])
   Get UNIX timestamp for a GMT date */
PHP_FUNCTION(timecop_gmmktime)
{
	TIMECOP_CALL_MKTIME("gmmktime", "gmdate");
}
/* }}} */

/* {{{ proto string timecop_date(string format [, long timestamp])
   Format a local date/time */
PHP_FUNCTION(timecop_date)
{
	TIMECOP_CALL_FUNCTION("date", 1);
}
/* }}} */

/* {{{ proto string timecop_gmdate(string format [, long timestamp])
   Format a GMT date/time */
PHP_FUNCTION(timecop_gmdate)
{
	TIMECOP_CALL_FUNCTION("gmdate", 1);
}
/* }}} */

/* {{{ proto int timecop_idate(string format [, int timestamp])
   Format a local time/date as integer */
PHP_FUNCTION(timecop_idate)
{
	TIMECOP_CALL_FUNCTION("idate", 1);
}
/* }}} */

/* {{{ proto array timecop_getdate([int timestamp])
   Get date/time information */
PHP_FUNCTION(timecop_getdate)
{
	TIMECOP_CALL_FUNCTION("getdate", 0);
}
/* }}} */

/* {{{ proto array timecop_localtime([int timestamp [, bool associative_array]])
   Returns the results of the C system call localtime as an associative array if
 the associative_array argument is set to 1 other wise it is a regular array */
PHP_FUNCTION(timecop_localtime)
{
	TIMECOP_CALL_FUNCTION("localtime", 0);
}
/* }}} */

/* {{{ proto int timecop_strtotime(string time [, int now ])
   Convert string representation of date and time to a timestamp */
PHP_FUNCTION(timecop_strtotime)
{
	TIMECOP_CALL_FUNCTION("strtotime", 1);
}
/* }}} */

/* {{{ proto string timecop_strftime(string format [, int timestamp])
   Format a local time/date according to locale settings */
PHP_FUNCTION(timecop_strftime)
{
	TIMECOP_CALL_FUNCTION("strftime", 1);
}
/* }}} */

/* {{{ proto string timecop_gmstrftime(string format [, int timestamp])
   Format a GMT/UCT time/date according to locale settings */
PHP_FUNCTION(timecop_gmstrftime)
{
	TIMECOP_CALL_FUNCTION("gmstrftime", 1);
}
/* }}} */

#ifdef HAVE_GETTIMEOFDAY

#define MICRO_IN_SEC 1000000.00
#define SEC_IN_MIN 60

static void _timecop_gettimeofday(INTERNAL_FUNCTION_PARAMETERS, int mode)
{
	zend_bool get_as_float = 0;
	long fixed_sec = 0, fixed_usec = 0;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|b", &get_as_float) == FAILURE) {
		return;
	}

	switch (TIMECOP_G(timecop_mode)) {
	case TIMECOP_MODE_FREEZE:
		fixed_sec = TIMECOP_G(freezed_timestamp);
		fixed_usec = 0;
		break;
	case TIMECOP_MODE_TRAVEL:
		{
			zval *timeofday, **zv_sec, **zv_usec;
			timecop_call_orig_method_with_0_params(NULL, NULL, NULL, "gettimeofday", &timeofday);
			if (zend_hash_find(Z_ARRVAL_P(timeofday), "sec", sizeof("sec"), (void **)&zv_sec) != FAILURE &&
				zv_sec) {
				fixed_sec = Z_LVAL_PP(zv_sec) + TIMECOP_G(travel_offset);
			}
			if (zend_hash_find(Z_ARRVAL_P(timeofday), "usec", sizeof("usec"), (void **)&zv_usec) != FAILURE &&
				zv_usec) {
				fixed_usec = Z_LVAL_PP(zv_usec);
			}
			zval_ptr_dtor(&timeofday);
		}
		break;
	default:
		{
			const char *func_name;
			size_t func_name_len;
			zval *arg1 = NULL, *retval_ptr, tmp;
			int param_count = 0;

			if (mode) {
				func_name = ORIG_FUNC_NAME("gettimeofday");
				func_name_len = ORIG_FUNC_NAME_SIZEOF("gettimeofday")-1;
			} else {
				func_name = ORIG_FUNC_NAME("microtime");
				func_name_len = ORIG_FUNC_NAME_SIZEOF("microtime")-1;
			}
			if (get_as_float) {
				INIT_ZVAL(tmp);
				ZVAL_TRUE(&tmp);
				arg1 = &tmp;
				param_count = 1;
			}
			zend_call_method(NULL, NULL, NULL, func_name, func_name_len, &retval_ptr, param_count, arg1, NULL TSRMLS_CC);
			if (retval_ptr) {
				RETURN_ZVAL(retval_ptr, 1, 1);
			}

			return;
		}
	}

	if (get_as_float) {
		RETURN_DOUBLE((double)(fixed_sec + fixed_usec / MICRO_IN_SEC));
	}
	if (mode) {
		zval *zv_offset, *zv_dst, format, timestamp;
		long offset = 0, is_dst = 0;

		INIT_ZVAL(timestamp);
		ZVAL_LONG(&timestamp, fixed_sec);

		/* offset */
		INIT_ZVAL(format);
		ZVAL_STRING(&format, "Z", 0);
		timecop_call_orig_method_with_2_params(NULL, NULL, NULL, "date", &zv_offset, &format, &timestamp);
		convert_to_long(zv_offset);
		offset = Z_LVAL_P(zv_offset);
		zval_ptr_dtor(&zv_offset);

		/* is_dst */
		ZVAL_STRING(&format, "I", 0);
		timecop_call_orig_method_with_2_params(NULL, NULL, NULL, "date", &zv_dst, &format, &timestamp);
		convert_to_long(zv_dst);
		is_dst = Z_LVAL_P(zv_dst);
		zval_ptr_dtor(&zv_dst);

		array_init(return_value);
		add_assoc_long(return_value, "sec", fixed_sec);
		add_assoc_long(return_value, "usec", fixed_usec);
		add_assoc_long(return_value, "minuteswest", -offset / SEC_IN_MIN);
		add_assoc_long(return_value, "dsttime", is_dst);
	} else {
		char ret[100];
		snprintf(ret, 100, "%.8F %ld", fixed_usec / MICRO_IN_SEC, fixed_sec);
		RETURN_STRING(ret, 1);
	}
}

/* {{{ proto mixed microtime([bool get_as_float])
   Returns either a string or a float containing the current time in seconds and microseconds */
PHP_FUNCTION(timecop_microtime)
{
	_timecop_gettimeofday(INTERNAL_FUNCTION_PARAM_PASSTHRU, 0);
}
/* }}} */

/* {{{ proto array gettimeofday([bool get_as_float])
   Returns the current time as array */
PHP_FUNCTION(timecop_gettimeofday)
{
	_timecop_gettimeofday(INTERNAL_FUNCTION_PARAM_PASSTHRU, 1);
}
/* }}} */
#endif

/* {{{ proto int timecop_unixtojd([int timestamp])
   Convert UNIX timestamp to Julian Day */
PHP_FUNCTION(timecop_unixtojd)
{
	TIMECOP_CALL_FUNCTION("unixtojd", 0);
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

	php_timecop_date_instantiate(TIMECOP_G(ce_DateTime), return_value TSRMLS_CC);

	/* call TimecopDateTime::__construct() */
	timecop_call_constructor(&return_value, TIMECOP_G(ce_TimecopDateTime), params, ZEND_NUM_ARGS() TSRMLS_CC);

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

	params = (zval ***) safe_emalloc(ZEND_NUM_ARGS(), sizeof(zval **), 0);

	if (zend_get_parameters_array_ex(ZEND_NUM_ARGS(), params) == FAILURE) {
		efree(params);
		RETURN_FALSE;
	}

	/* call original DateTime::__construct() */
	timecop_call_original_constructor(&obj, TIMECOP_G(ce_DateTime), params, ZEND_NUM_ARGS() TSRMLS_CC);

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

/* {{{ proto TimecopOrigDateTime::__construct([string time[, DateTimeZone object]])
   Creates new TimecopOrigDateTime object
*/
PHP_METHOD(TimecopOrigDateTime, __construct)
{
	zval ***params;
	zval *obj = getThis();

	params = (zval ***) safe_emalloc(ZEND_NUM_ARGS(), sizeof(zval **), 0);

	if (zend_get_parameters_array_ex(ZEND_NUM_ARGS(), params) == FAILURE) {
		efree(params);
		RETURN_FALSE;
	}

	/* call original DateTime::__construct() */
	timecop_call_original_constructor(&obj, TIMECOP_G(ce_DateTime), params, ZEND_NUM_ARGS() TSRMLS_CC);

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

static inline void timecop_call_original_constructor(zval **obj, zend_class_entry *ce, zval ***params, int param_count TSRMLS_DC) {
	timecop_call_constructor_ex(obj, ce, params, param_count, 1 TSRMLS_CC);
}
static inline void timecop_call_constructor(zval **obj, zend_class_entry *ce, zval ***params, int param_count TSRMLS_DC) {
	timecop_call_constructor_ex(obj, ce, params, param_count, 0 TSRMLS_CC);
}

static void timecop_call_constructor_ex(zval **obj, zend_class_entry *ce, zval ***params, int param_count, int call_original TSRMLS_DC)
{
	zval *arg1 = NULL, *arg2 = NULL;
	const char *func_name;
	size_t func_name_len;

	if (param_count > 2) {
		zend_error(E_ERROR, "INTERNAL ERROR: too many parameters for constructor.");
		return;
	}

	if (param_count == 1) {
		arg1 = *params[0];
	} else if (param_count == 2) {
		arg1 = *params[0];
		arg2 = *params[1];
	}

	if (call_original) {
		func_name = ORIG_FUNC_NAME("__construct");
		func_name_len = ORIG_FUNC_NAME_SIZEOF("__construct")-1;
	} else {
		func_name = "__construct";
		func_name_len = sizeof("__construct")-1;
	}

	zend_call_method(obj, ce, NULL, func_name, func_name_len, NULL, param_count, arg1, arg2 TSRMLS_CC);
}

static void simple_call_function(const char *function_name, zval **retval_ptr_ptr, zend_uint param_count, zval **params[] TSRMLS_DC)
{
	zval callable;

	INIT_ZVAL(callable);
	ZVAL_STRING(&callable, function_name, 0);

	call_user_function_ex(EG(function_table), NULL, &callable, retval_ptr_ptr, param_count, params, 1, NULL TSRMLS_CC);
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
