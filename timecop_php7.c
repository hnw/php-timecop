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

#include "php_timecop.h"

#ifdef ZFS
#include "TSRM.h"
#endif

ZEND_DECLARE_MODULE_GLOBALS(timecop)

static void timecop_globals_ctor(zend_timecop_globals *globals) {
	/* Initialize your global struct */
	globals->func_override = 1;
	globals->sync_request_time = 1;
	ZVAL_NULL(&globals->orig_request_time);
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
	TIMECOP_OFE("unixtojd"),
	TIMECOP_OFE("date_create"),
	TIMECOP_OFE("date_create_from_format"),
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

ZEND_BEGIN_ARG_INFO_EX(arginfo_timecop_unixtojd, 0, 0, 0)
	ZEND_ARG_INFO(0, timestamp)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_timecop_date_create, 0, 0, 0)
	ZEND_ARG_INFO(0, time)
	ZEND_ARG_INFO(0, object)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_timecop_date_create_from_format, 0, 0, 2)
	ZEND_ARG_INFO(0, format)
	ZEND_ARG_INFO(0, time)
	ZEND_ARG_INFO(0, object)
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
	PHP_FE(timecop_date_create, arginfo_timecop_date_create)
	PHP_FE(timecop_date_create_from_format, arginfo_timecop_date_create_from_format)
	{NULL, NULL, NULL}
};
/* }}} */

/* declare method parameters, */

/* each method can have its own parameters and visibility */
static zend_function_entry timecop_datetime_class_functions[] = {
	PHP_ME(TimecopDateTime, __construct, arginfo_timecop_date_create,
		   ZEND_ACC_CTOR | ZEND_ACC_PUBLIC)
	PHP_ME_MAPPING(createFromFormat, timecop_date_create_from_format, arginfo_timecop_date_create_from_format,
				   ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	{NULL, NULL, NULL}
};

#define MKTIME_NUM_ARGS 6

#define TIMECOP_CALL_FUNCTION(func_name, index_to_fill_timestamp) \
	_timecop_call_function(INTERNAL_FUNCTION_PARAM_PASSTHRU, ORIG_FUNC_NAME(func_name), index_to_fill_timestamp);

#define TIMECOP_CALL_MKTIME(mktime_func_name, date_func_name) \
	_timecop_call_mktime(INTERNAL_FUNCTION_PARAM_PASSTHRU, ORIG_FUNC_NAME(mktime_func_name), ORIG_FUNC_NAME(date_func_name));

static void timecop_globals_ctor(zend_timecop_globals *globals);

static int register_timecop_classes();
static int timecop_func_override();
static int timecop_class_override();
static int timecop_func_override_clear();
static int timecop_class_override_clear();

static int update_request_time(long unixtime);
static int restore_request_time();

static long timecop_current_timestamp();

static int fill_mktime_params(zval *fill_params, const char *date_function_name, int from);
static int fix_datetime_timestamp(zval *datetime_obj, zval *time, zval *timezone_obj);

static void _timecop_call_function(INTERNAL_FUNCTION_PARAMETERS, const char *function_name, int index_to_fill_timestamp);
static void _timecop_call_mktime(INTERNAL_FUNCTION_PARAMETERS, const char *mktime_function_name, const char *date_function_name);

static inline void timecop_call_original_constructor(zval *obj, zend_class_entry *ce, zval *params, int param_count);
static inline void timecop_call_constructor(zval *obj, zend_class_entry *ce, zval *params, int param_count);
static void timecop_call_constructor_ex(zval *obj, zend_class_entry *ce, zval *params, int param_count, int call_original);
static void simple_call_function(const char *function_name, zval *retval_ptr, uint32_t param_count, zval params[]);

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
#  ifdef ZTS
   ZEND_TSRMLS_CACHE_DEFINE();
#  endif
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
	register_timecop_classes();
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
#if defined(COMPILE_DL_TIMECOP) && defined(ZTS)
	ZEND_TSRMLS_CACHE_UPDATE();
#endif

	int ret;

	if (TIMECOP_G(func_override)) {
		if (SUCCESS != timecop_func_override() ||
			SUCCESS != timecop_class_override()) {
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
		timecop_func_override_clear();
		timecop_class_override_clear();
	}

	if (Z_TYPE(TIMECOP_G(orig_request_time)) == IS_NULL) {
		restore_request_time();
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

static int register_timecop_classes()
{
	zend_class_entry ce;
	zend_class_entry *self_ce, *parent_ce;

	parent_ce = zend_hash_str_find_ptr(CG(class_table), "datetime", sizeof("datetime")-1);
	if (parent_ce == NULL) {
		return SUCCESS; /* DateTime must be initialized before */
	}

	INIT_CLASS_ENTRY(ce, "TimecopDateTime", timecop_datetime_class_functions);
	self_ce = zend_register_internal_class_ex(&ce, parent_ce);
	self_ce->create_object = parent_ce->create_object;

	TIMECOP_G(ce_DateTime) = parent_ce;
	TIMECOP_G(ce_TimecopDateTime) = self_ce;

	INIT_CLASS_ENTRY(ce, "TimecopOrigDateTime", NULL);
	self_ce = zend_register_internal_class_ex(&ce, parent_ce);
	self_ce->create_object = parent_ce->create_object;

	return SUCCESS;
}

static int timecop_func_override()
{
	const struct timecop_override_func_entry *p;
	zend_function *zf_orig, *zf_ovrd, *zf_save;

	p = &(timecop_override_func_table[0]);
	while (p->orig_func != NULL) {
		zf_orig = zend_hash_str_find_ptr(EG(function_table), p->orig_func, strlen(p->orig_func));
		if (zf_orig == NULL) {
			// Do nothing. Because some functions are introduced by optional extensions.
			p++;
			continue;
		}

		zf_ovrd = zend_hash_str_find_ptr(EG(function_table), p->ovrd_func, strlen(p->ovrd_func));
		if (zf_ovrd == NULL) {
			php_error_docref("https://github.com/hnw/php-timecop", E_WARNING,
							 "timecop couldn't find function %s.", p->ovrd_func);
			p++;
			continue;
		}

		zf_save = zend_hash_str_find_ptr(EG(function_table), p->save_func, strlen(p->save_func));
		if (zf_save != NULL) {
			php_error_docref("https://github.com/hnw/php-timecop", E_WARNING,
							 "timecop couldn't create function %s because already exists.",
							 p->save_func);
			p++;
			continue;
		}

		TIMECOP_ASSERT(zf_orig->type == ZEND_INTERNAL_FUNCTION);
		TIMECOP_ASSERT(zf_ovrd->type == ZEND_INTERNAL_FUNCTION);

		zend_hash_str_add_mem(EG(function_table), p->save_func, strlen(p->save_func),
							  zf_orig, sizeof(zend_internal_function));
		function_add_ref(zf_orig);

		zend_hash_str_update_mem(EG(function_table), p->orig_func, strlen(p->orig_func),
								 zf_ovrd, sizeof(zend_internal_function));
		function_add_ref(zf_ovrd);

		p++;
	}
	return SUCCESS;
}

static int timecop_class_override()
{
	const struct timecop_override_class_entry *p;
	zend_class_entry *ce_orig, *ce_ovrd;
	zend_function *zf_orig, *zf_ovrd, *zf_save, *zf_new;

	p = &(timecop_override_class_table[0]);
	while (p->orig_class != NULL) {
		ce_orig = zend_hash_str_find_ptr(EG(class_table), p->orig_class, strlen(p->orig_class));
		if (ce_orig == NULL) {
			php_error_docref("https://github.com/hnw/php-timecop", E_WARNING,
							 "timecop couldn't find class %s.", p->orig_class);
			p++;
			continue;
		}

		ce_ovrd = zend_hash_str_find_ptr(EG(class_table), p->ovrd_class, strlen(p->ovrd_class));
		if (ce_ovrd == NULL) {
			php_error_docref("https://github.com/hnw/php-timecop", E_WARNING,
							 "timecop couldn't find class %s.", p->ovrd_class);
			p++;
			continue;
		}

		zf_orig = zend_hash_str_find_ptr(&ce_orig->function_table,
										 p->orig_method, strlen(p->orig_method));
		if (zf_orig == NULL) {
			php_error_docref("https://github.com/hnw/php-timecop", E_WARNING,
							 "timecop couldn't find method %s::%s.",
							 p->orig_class, p->orig_method);
			p++;
			continue;
		}

		zf_ovrd = zend_hash_str_find_ptr(&ce_ovrd->function_table,
										 p->orig_method, strlen(p->orig_method));
		if (zf_ovrd == NULL) {
			php_error_docref("https://github.com/hnw/php-timecop", E_WARNING,
							 "timecop couldn't find method %s::%s.",
							 p->ovrd_class, p->orig_method);
			p++;
			continue;
		}

		zf_save = zend_hash_str_find_ptr(&ce_orig->function_table,
										 p->save_method, strlen(p->save_method));
		if (zf_save != NULL) {
			php_error_docref("https://github.com/hnw/php-timecop", E_WARNING,
							 "timecop couldn't create method %s::%s because already exists.",
							 p->orig_class, p->save_method);
			p++;
			continue;
		}

		TIMECOP_ASSERT(zf_orig->type == ZEND_INTERNAL_FUNCTION);
		TIMECOP_ASSERT(ce_orig->type & ZEND_INTERNAL_CLASS);
		TIMECOP_ASSERT(zf_ovrd->type == ZEND_INTERNAL_FUNCTION);
		TIMECOP_ASSERT(ce_ovrd->type & ZEND_INTERNAL_CLASS);

		zend_hash_str_add_mem(&ce_orig->function_table,
							  p->save_method, strlen(p->save_method),
							  zf_orig, sizeof(zend_internal_function));
		function_add_ref(zf_orig);

		zf_new = zend_hash_str_update_mem(&ce_orig->function_table,
										  p->orig_method, strlen(p->orig_method),
										  zf_ovrd, sizeof(zend_internal_function));
		function_add_ref(zf_ovrd);

		TIMECOP_ASSERT(zf_new != NULL);
		TIMECOP_ASSERT(zf_new != zf_orig);

		if (strcmp(p->orig_method, "__construct") == 0) {
			ce_orig->constructor = zf_new;
		}
		p++;
	}
	return SUCCESS;
}

/*  clear overrideed function. */
static int timecop_func_override_clear()
{
	const struct timecop_override_func_entry *p;
	zend_function *zf_orig;

	p = &(timecop_override_func_table[0]);
	while (p->orig_func != NULL) {
		zf_orig = zend_hash_str_find_ptr(EG(function_table),
										 p->save_func, strlen(p->save_func));
		if (zf_orig == NULL) {
			p++;
			continue;
		}

		zend_hash_str_update_mem(EG(function_table), p->orig_func, strlen(p->orig_func),
								 zf_orig, sizeof(zend_internal_function));
		function_add_ref(zf_orig);

		zend_hash_str_del(EG(function_table), p->save_func, strlen(p->save_func));

		p++;
	}
	return SUCCESS;
}

static int timecop_class_override_clear()
{
	const struct timecop_override_class_entry *p;
	zend_class_entry *ce_orig;
	zend_function *zf_orig, *zf_ovrd, *zf_save;

	p = &(timecop_override_class_table[0]);
	while (p->orig_class != NULL) {
		ce_orig = zend_hash_str_find_ptr(EG(class_table),
										 p->orig_class, strlen(p->orig_class));
		if (ce_orig == NULL) {
			php_error_docref("https://github.com/hnw/php-timecop", E_WARNING,
							 "timecop couldn't find class %s.", p->orig_class);
			p++;
			continue;
		}

		zf_orig = zend_hash_str_find_ptr(&ce_orig->function_table,
										 p->save_method, strlen(p->save_method));
		if (zf_orig == NULL) {
			php_error_docref("https://github.com/hnw/php-timecop", E_WARNING,
							 "timecop couldn't find method %s::%s.",
							 p->orig_class, p->save_method);
			p++;
			continue;
		}

		zend_hash_str_update_mem(&ce_orig->function_table, p->orig_method, strlen(p->orig_method),
								 zf_orig, sizeof(zend_internal_function));
		function_add_ref(zf_orig);

		zend_hash_str_del(&ce_orig->function_table, p->save_method, strlen(p->save_method));

		if (strcmp(p->orig_method, "__construct") == 0) {
			ce_orig->constructor = zf_orig;
		}
		p++;
	}
	return SUCCESS;
}

static int update_request_time(long unixtime)
{
	zval *server_vars, *request_time, tmp;

	server_vars = zend_hash_str_find(&EG(symbol_table), "_SERVER", sizeof("_SERVER")-1);
	if (server_vars != NULL && Z_TYPE_P(server_vars) == IS_ARRAY) {
		request_time = zend_hash_str_find(Z_ARRVAL_P(server_vars), "REQUEST_TIME", sizeof("REQUEST_TIME")-1);
		if (request_time != NULL) {
			if (Z_TYPE(TIMECOP_G(orig_request_time)) == IS_NULL) {
				ZVAL_COPY_VALUE(&TIMECOP_G(orig_request_time), request_time);
			}
		}
		ZVAL_LONG(&tmp, unixtime);
		zend_hash_str_update(Z_ARRVAL_P(server_vars),
							 "REQUEST_TIME", sizeof("REQUEST_TIME")-1,
							 &tmp);
	}

	return SUCCESS;
}

static int restore_request_time()
{
	zval *server_vars, *request_time, orig_request_time;

	server_vars = zend_hash_str_find(&EG(symbol_table), "_SERVER", sizeof("_SERVER")-1);
	if (Z_TYPE(TIMECOP_G(orig_request_time)) != IS_NULL &&
		server_vars != NULL && Z_TYPE_P(server_vars) == IS_ARRAY) {
		zend_hash_str_update(Z_ARRVAL_P(server_vars),
							 "REQUEST_TIME", sizeof("REQUEST_TIME")-1,
							 &TIMECOP_G(orig_request_time));
		ZVAL_NULL(&TIMECOP_G(orig_request_time));
	}

	return SUCCESS;
}

static long timecop_current_timestamp()
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

static int fill_mktime_params(zval *fill_params, const char *date_function_name, int from)
{
	char *formats[MKTIME_NUM_ARGS] = {"H", "i", "s", "n", "j", "Y"};
	zval params[2];
	int i;

	ZVAL_LONG(&params[1], timecop_current_timestamp());

	for (i = from; i < MKTIME_NUM_ARGS; i++) {
		ZVAL_STRING(&params[0], formats[i]);
		simple_call_function(date_function_name, &fill_params[i], 2, params);
		zval_dtor(&params[0]);
	}

	return MKTIME_NUM_ARGS;
}

static int fix_datetime_timestamp(zval *datetime_obj, zval *time, zval *timezone_obj)
{
	zval orig_timestamp, fixed_timestamp;
	zval now;
	zval orig_zonename;

	/*
	 * if ($time === null || $time === false || $time === "") {
	 *     $time = "now";
	 * }
	 */
	if (time == NULL || Z_TYPE_P(time) == IS_NULL ||
		Z_TYPE_P(time) == IS_FALSE ||
		(Z_TYPE_P(time) == IS_STRING && Z_STRLEN_P(time) == 0)) {
		ZVAL_STRING(&now, "now");
		time = &now;
	}

	/*
	 * $orig_timestamp = $datetime_obj->getTimeStamp();
	 */
	zend_call_method_with_0_params(datetime_obj, Z_OBJCE_P(datetime_obj), NULL, "gettimestamp", &orig_timestamp);

	/*
	 * if ($timezone_obj) {
	 *     $zonename = $timezone_obj->getName()
	 *     $orig_zonename = date_default_timezone_get();
	 *     date_default_timezone_set($zonename);
	 * }
	 */
	if (timezone_obj) {
		zval zonename;
		zend_call_method_with_0_params(timezone_obj, Z_OBJCE_P(timezone_obj), NULL, "getname", &zonename);
		zend_call_method_with_0_params(NULL, NULL, NULL, "date_default_timezone_get", &orig_zonename);
		zend_call_method_with_1_params(NULL, NULL, NULL, "date_default_timezone_set", NULL, &zonename);
		zval_dtor(&zonename);
	}

	/*
	 * $fixed_timestamp = timecop_strtotime();
	 */
	zend_call_method_with_1_params(NULL, NULL, NULL, "timecop_strtotime", &fixed_timestamp, time);

	/*
	 * if ($timezone_obj && $orig_zonename) {
	 *     date_default_timezone_set($orig_zonename);
	 * }
	 */
	if (timezone_obj) {
		zend_call_method_with_1_params(NULL, NULL, NULL, "date_default_timezone_set", NULL, &orig_zonename);
	}

	/*
	 * if ($fixed_timestamp === false) {
	 *     // warn
	 * } elseif ($orig_timestamp != $fixed_timestamp) {
	 *     $datetime_obj->setTimeStamp($fixed_timestamp)
	 * }
	 */
	if (Z_TYPE(fixed_timestamp) == IS_FALSE) {
		php_error_docref(NULL, E_WARNING,
						 "Failed to parse time string '%s': giving up time traveling",
						 Z_STRVAL_P(time));
	} else if (Z_LVAL(orig_timestamp) != Z_LVAL(fixed_timestamp)) {
		// set time-travelled timestamp
		zend_call_method_with_1_params(datetime_obj, Z_OBJCE_P(datetime_obj), NULL, "settimestamp", NULL, &fixed_timestamp);
	}

	// restore original timezone
	/*
	 * if ($timezone_obj) {
	 *     date_default_timezone_set($orig_zonename);
	 * }
	 */
	if (timezone_obj) {
		zend_call_method_with_1_params(NULL, NULL, NULL, "date_default_timezone_set", NULL, &orig_zonename);
		zval_dtor(&orig_zonename);
	}
	zval_dtor(&now);
	return 0;
}

static void _timecop_call_function(INTERNAL_FUNCTION_PARAMETERS, const char *function_name, int index_to_fill_timestamp)
{
	zval *params, retval;
	uint32_t param_count;

	param_count = MAX(ZEND_NUM_ARGS(), index_to_fill_timestamp+1);
	params = (zval *)safe_emalloc(param_count, sizeof(zval), 0);

	if (zend_get_parameters_array_ex(ZEND_NUM_ARGS(), params) == FAILURE) {
		efree(params);
		return;
	}

	param_count = ZEND_NUM_ARGS();
	if (param_count == index_to_fill_timestamp) {
		ZVAL_LONG(&params[param_count], timecop_current_timestamp());
		param_count++;
	}

	simple_call_function(function_name, return_value, param_count, params);

	efree(params);
}

/* {{{ _timecop_call_mktime - timecop_(gm)mktime helper */
static void _timecop_call_mktime(INTERNAL_FUNCTION_PARAMETERS, const char *mktime_function_name, const char *date_function_name)
{
	zval *params, retval;
	uint32_t param_count;

	int i;

	param_count = MAX(ZEND_NUM_ARGS(), MKTIME_NUM_ARGS);
	params = (zval *)safe_emalloc(param_count, sizeof(zval), 0);

	if (zend_get_parameters_array_ex(ZEND_NUM_ARGS(), params) == FAILURE) {
		efree(params);
		zend_throw_error(NULL, "Cannot get arguments for calling");
		return;
	}

	param_count = ZEND_NUM_ARGS();
	if (param_count < MKTIME_NUM_ARGS) {
		fill_mktime_params(params, date_function_name, param_count);
		param_count = MKTIME_NUM_ARGS;
	}

	if (ZEND_NUM_ARGS() == 0) {
		php_error_docref(NULL, E_STRICT, "You should be using the time() function instead");
	}

	simple_call_function(mktime_function_name, return_value, param_count, params);

	for (i = ZEND_NUM_ARGS(); i < MKTIME_NUM_ARGS; i++) {
		zval_dtor(&params[i]);
	}
	efree(params);
}


/* {{{ proto int timecop_freeze(long timestamp)
   Time travel to specified timestamp and freeze */
PHP_FUNCTION(timecop_freeze)
{
	long timestamp;
	if (zend_parse_parameters(ZEND_NUM_ARGS(), "l", &timestamp) == FAILURE) {
		RETURN_FALSE;
	}

	TIMECOP_G(timecop_mode) = TIMECOP_MODE_FREEZE;
	TIMECOP_G(freezed_timestamp) = timestamp;

	if (TIMECOP_G(sync_request_time)){
		update_request_time(timestamp);
	}

	RETURN_TRUE;
}
/* }}} */

/* {{{ proto int timecop_travel(long timestamp)
   Time travel to specified timestamp */
PHP_FUNCTION(timecop_travel)
{
	long timestamp;
	if (zend_parse_parameters(ZEND_NUM_ARGS(), "l", &timestamp) == FAILURE) {
		RETURN_FALSE;
	}
	TIMECOP_G(timecop_mode) = TIMECOP_MODE_TRAVEL;
	TIMECOP_G(travel_offset) = timestamp - time(NULL);

	if (TIMECOP_G(sync_request_time)){
		update_request_time(timestamp);
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
		restore_request_time();
	}

	RETURN_TRUE;
}
/* }}} */

/* {{{ proto int timecop_time(void)
   Return virtual timestamp */
PHP_FUNCTION(timecop_time)
{
	RETURN_LONG(timecop_current_timestamp());
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
	zval *params, *datetime_obj, *time = NULL;

	params = (zval *) safe_emalloc(ZEND_NUM_ARGS(), sizeof(zval), 0);

	if (zend_get_parameters_array_ex(ZEND_NUM_ARGS(), params) == FAILURE) {
		efree(params);
		RETURN_FALSE;
	}

	object_init_ex(return_value, TIMECOP_G(ce_DateTime));

	/* call TimecopDateTime::__construct() */
	timecop_call_constructor(return_value, TIMECOP_G(ce_TimecopDateTime), params, ZEND_NUM_ARGS());

	efree(params);
}
/* }}} */

/* {{{ proto TimecopDateTime timecop_date_create_from_format(string format, string time[, DateTimeZone object])
 Returns new TimecopDateTime object
 */
PHP_FUNCTION(timecop_date_create_from_format)
{
	zval *timezone_object = NULL;
	char *time_str = NULL, *format_str = NULL;
	size_t time_str_len = 0, format_str_len = 0;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "ss|O", &format_str, &format_str_len, &time_str, &time_str_len, &timezone_object, php_date_get_timezone_ce()) == FAILURE) {
		RETURN_FALSE;
	}

	object_init_ex(return_value, TIMECOP_G(ce_TimecopDateTime));

	if (!php_date_initialize(Z_PHPDATE_P(return_value), time_str, time_str_len, format_str, timezone_object, 0)) {
		RETURN_FALSE;
	}
}
/* }}} */

/* {{{ proto TimecopDateTime::__construct([string time[, DateTimeZone object]])
   Creates new TimecopDateTime object
*/
PHP_METHOD(TimecopDateTime, __construct)
{
	zval *params;
	zval *obj = getThis();
	zend_class_entry *datetime_ce;

	params = (zval *)safe_emalloc(ZEND_NUM_ARGS(), sizeof(zval), 0);

	if (zend_get_parameters_array_ex(ZEND_NUM_ARGS(), params) == FAILURE) {
		efree(params);
		zend_throw_error(NULL, "Cannot get arguments for TimecopDateTime::__construct");
		RETURN_FALSE;
	}

	/* call original DateTime::__construct() */
	timecop_call_original_constructor(obj, TIMECOP_G(ce_DateTime), params, ZEND_NUM_ARGS());

	if (!EG(exception)) {
		zval *time = NULL, *timezone_obj = NULL;
		if (ZEND_NUM_ARGS() >= 1) {
			time = &params[0];
		}
		if (ZEND_NUM_ARGS() >= 2) {
			timezone_obj = &params[1];
		}
		fix_datetime_timestamp(obj, time, timezone_obj);
	}

	efree(params);
}

static inline void timecop_call_original_constructor(zval *obj, zend_class_entry *ce, zval *params, int param_count) {
	timecop_call_constructor_ex(obj, ce, params, param_count, 1);
}
static inline void timecop_call_constructor(zval *obj, zend_class_entry *ce, zval *params, int param_count) {
	timecop_call_constructor_ex(obj, ce, params, param_count, 0);
}

static void timecop_call_constructor_ex(zval *obj, zend_class_entry *ce, zval *params, int param_count, int call_original) {
	zval *arg1 = NULL, *arg2 = NULL;
	char *func_name;
	size_t func_name_len;

	if (param_count > 2) {
		zend_error(E_ERROR, "INTERNAL ERROR: too many parameters for constructor.");
		return;
	}

	if (param_count == 1) {
		arg1 = &params[0];
	} else if (param_count == 2) {
		arg1 = &params[0];
		arg2 = &params[1];
	}

	if (call_original) {
		func_name = ORIG_FUNC_NAME("__construct");
		func_name_len = ORIG_FUNC_NAME_SIZEOF("__construct")-1;
	} else {
		func_name = "__construct";
		func_name_len = sizeof("__construct")-1;
	}

	zend_call_method(obj, ce, NULL, func_name, func_name_len, NULL, param_count, arg1, arg2);
}

static void simple_call_function(const char *function_name, zval *retval_ptr, uint32_t param_count, zval *params)
{
	zval callable;
	ZVAL_STRING(&callable, function_name);

	call_user_function_ex(EG(function_table), NULL, &callable, retval_ptr, param_count, params, 1, NULL);

	zval_dtor(&callable);
}

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
