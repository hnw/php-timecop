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
	globals->timecop_mode = TIMECOP_MODE_REALTIME;
	globals->freezed_time.sec = 0;
	globals->freezed_time.usec = 0;
	globals->travel_origin.sec = 0;
	globals->travel_origin.usec = 0;
	globals->travel_offset.sec = 0;
	globals->travel_offset.usec = 0;
	globals->scaling_factor = 1;
	globals->ce_DateTimeZone = NULL;
	globals->ce_DateTimeInterface = NULL;
	globals->ce_DateTime = NULL;
	globals->ce_TimecopDateTime = NULL;
	globals->ce_DateTimeImmutable = NULL;
	globals->ce_TimecopDateTimeImmutable = NULL;
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
#if PHP_VERSION_ID >= 50500
	TIMECOP_OFE("date_create_immutable"),
	TIMECOP_OFE("date_create_immutable_from_format"),
#endif
	{NULL, NULL, NULL}
};

static const struct timecop_override_class_entry timecop_override_class_table[] = {
	TIMECOP_OCE("datetime", "__construct"),
#if PHP_VERSION_ID >= 50300
	TIMECOP_OCE("datetime", "createfromformat"),
#endif
#if PHP_VERSION_ID >= 50500
	TIMECOP_OCE("datetimeimmutable", "__construct"),
	TIMECOP_OCE("datetimeimmutable", "createfromformat"),
#endif
	{NULL, NULL, NULL, NULL}
};

ZEND_BEGIN_ARG_INFO_EX(arginfo_timecop_freeze, 0, 0, 1)
	ZEND_ARG_INFO(0, timestamp)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_timecop_travel, 0, 0, 1)
	ZEND_ARG_INFO(0, timestamp)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_timecop_scale, 0, 0, 1)
	ZEND_ARG_INFO(0, scale)
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
ZEND_BEGIN_ARG_INFO_EX(arginfo_date_timestamp_set, 0, 0, 2)
        ZEND_ARG_INFO(0, object)
        ZEND_ARG_INFO(0, unixtimestamp)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_date_timestamp_get, 0, 0, 1)
        ZEND_ARG_INFO(0, object)
ZEND_END_ARG_INFO()
#endif

/* {{{ timecop_functions[] */
const zend_function_entry timecop_functions[] = {
	PHP_FE(timecop_freeze, arginfo_timecop_freeze)
	PHP_FE(timecop_travel, arginfo_timecop_travel)
	PHP_FE(timecop_scale, arginfo_timecop_scale)
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
#if PHP_VERSION_ID >= 50500
	PHP_FE(timecop_date_create_immutable, arginfo_timecop_date_create)
	PHP_FE(timecop_date_create_immutable_from_format, arginfo_timecop_date_create_from_format)
#endif
#if !defined(PHP_VERSION_ID) || PHP_VERSION_ID < 50300
	PHP_FE(date_timestamp_set, arginfo_date_timestamp_set)
	PHP_FE(date_timestamp_get, arginfo_date_timestamp_get)
#endif
	{NULL, NULL, NULL}
};
/* }}} */

/* declare method parameters, */

/* each method can have its own parameters and visibility */
static zend_function_entry timecop_funcs_timecop[] = {
	PHP_ME_MAPPING(freeze, timecop_freeze, arginfo_timecop_travel, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME_MAPPING(travel, timecop_travel, arginfo_timecop_travel, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME_MAPPING(scale,  timecop_scale,  arginfo_timecop_scale,  ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME_MAPPING(return, timecop_return, arginfo_timecop_return, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	{NULL, NULL, NULL}
};

static zend_function_entry timecop_funcs_date[] = {
	PHP_ME(TimecopDateTime, __construct, arginfo_timecop_date_create,
		   ZEND_ACC_CTOR | ZEND_ACC_PUBLIC)
#if PHP_VERSION_ID >= 50300
   PHP_ME_MAPPING(createFromFormat, timecop_date_create_from_format, arginfo_timecop_date_create_from_format,
		   ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
#endif
	{NULL, NULL, NULL}
};

static zend_function_entry timecop_funcs_orig_date[] = {
	PHP_ME(TimecopOrigDateTime, __construct, arginfo_timecop_date_create,
		   ZEND_ACC_CTOR | ZEND_ACC_PUBLIC)
	{NULL, NULL, NULL}
};

#if PHP_VERSION_ID >= 50500
static zend_function_entry timecop_funcs_immutable[] = {
	PHP_ME(TimecopDateTimeImmutable, __construct, arginfo_timecop_date_create,
		   ZEND_ACC_CTOR | ZEND_ACC_PUBLIC)
	PHP_ME_MAPPING(createFromFormat, timecop_date_create_immutable_from_format, arginfo_timecop_date_create_from_format,
				   ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	{NULL, NULL, NULL}
};

static zend_function_entry timecop_funcs_orig_immutable[] = {
	PHP_ME(TimecopOrigDateTimeImmutable, __construct, arginfo_timecop_date_create,
		   ZEND_ACC_CTOR | ZEND_ACC_PUBLIC)
	{NULL, NULL, NULL}
};
#endif

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

static int fill_mktime_params(zval ***params, const char *date_function_name, int from TSRMLS_DC);
static int get_formatted_mock_time(zval *time, zval *timezone_obj, zval **retval_time, zval **retval_timezone TSRMLS_DC);
static long get_mock_fraction(zval *time, zval *timezone_obj TSRMLS_DC);

static void _timecop_call_function(INTERNAL_FUNCTION_PARAMETERS, const char *function_name, int index_to_fill_timestamp);
static void _timecop_call_mktime(INTERNAL_FUNCTION_PARAMETERS, const char *mktime_function_name, const char *date_function_name);

static int get_mock_timeval(tc_timeval *fixed, const tc_timeval *now TSRMLS_DC);
static inline long mock_timestamp();

static int get_timeval_from_datetime(tc_timeval *tp, zval *dt TSRMLS_DC);
static int get_current_time(tc_timeval *now TSRMLS_DC);

static void _timecop_orig_datetime_constructor(INTERNAL_FUNCTION_PARAMETERS, int immutable);
static inline void _timecop_date_create(INTERNAL_FUNCTION_PARAMETERS, int immutable);
static inline void _timecop_datetime_constructor(INTERNAL_FUNCTION_PARAMETERS, int immutable);
static void _timecop_datetime_constructor_ex(INTERNAL_FUNCTION_PARAMETERS, zval *obj, int immutable);

static inline zval* _call_php_method_with_0_params(zval **object_pp, zend_class_entry *obj_ce, const char *method_name, zval **retval_ptr_ptr TSRMLS_DC);
static inline zval* _call_php_method_with_1_params(zval **object_pp, zend_class_entry *obj_ce, const char *method_name, zval **retval_ptr_ptr, zval *arg1 TSRMLS_DC);
static inline zval* _call_php_method_with_2_params(zval **object_pp, zend_class_entry *obj_ce, const char *method_name, zval **retval_ptr_ptr, zval *arg1, zval *arg2 TSRMLS_DC);
static inline zval* _call_php_method(zval **object_pp, zend_class_entry *obj_ce, const char *method_name, zval **retval_ptr_ptr, int param_count, zval* arg1, zval* arg2 TSRMLS_DC);
static inline void _call_php_function_with_0_params(const char *function_name, zval **retval_ptr_ptr TSRMLS_DC);
static inline void _call_php_function_with_1_params(const char *function_name, zval **retval_ptr_ptr, zval *arg1 TSRMLS_DC);
static inline void _call_php_function_with_2_params(const char *function_name, zval **retval_ptr_ptr, zval *arg1, zval *arg2 TSRMLS_DC);
static void _call_php_function_with_3_params(const char *function_name, zval **retval_ptr_ptr, zval *arg1, zval *arg2, zval *arg3 TSRMLS_DC);
static inline void _call_php_function_with_params(const char *function_name, zval **retval_ptr_ptr, zend_uint param_count, zval **params[] TSRMLS_DC);

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

	TIMECOP_G(timecop_mode) = TIMECOP_MODE_REALTIME;
	TIMECOP_G(scaling_factor) = 1;

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
	zend_class_entry *tmp, *date_ce, *timezone_ce, *immutable_ce = NULL, *interface_ce = NULL;

	if (zend_hash_find(CG(class_table), "datetime", sizeof("datetime"), (void **) &pce) == FAILURE) {
		/* DateTime must be initialized before */
		php_error_docref("https://github.com/hnw/php-timecop" TSRMLS_CC, E_WARNING,
						 "timecop couldn't find class %s.", "DateTime");
		return SUCCESS;
	}
	date_ce = *pce;

	if (zend_hash_find(CG(class_table), "datetimezone", sizeof("datetimezone"), (void **) &pce) == FAILURE) {
		/* DateTimeImmutable must be initialized before */
		php_error_docref("https://github.com/hnw/php-timecop" TSRMLS_CC, E_WARNING,
						 "timecop couldn't find class %s.", "DateTimeZone");
		return SUCCESS;
	}
	timezone_ce = *pce;

#if PHP_VERSION_ID >= 50500
	if (zend_hash_find(CG(class_table), "datetimeimmutable", sizeof("datetimeimmutable"), (void **) &pce) == FAILURE) {
		/* DateTimeImmutable must be initialized before */
		php_error_docref("https://github.com/hnw/php-timecop" TSRMLS_CC, E_WARNING,
						 "timecop couldn't find class %s.", "DateTimeImmutable");
		return SUCCESS;
	}
	immutable_ce = *pce;

	if (zend_hash_find(CG(class_table), "datetimeinterface", sizeof("datetimeinterface"), (void **) &pce) == FAILURE) {
		/* DateTimeInterface must be initialized before */
		php_error_docref("https://github.com/hnw/php-timecop" TSRMLS_CC, E_WARNING,
						 "timecop couldn't find interface %s.", "DateTimeInterface");
		return SUCCESS;
	}
	interface_ce = *pce;
#endif

	INIT_CLASS_ENTRY(ce, "Timecop", timecop_funcs_timecop);
	zend_register_internal_class(&ce TSRMLS_CC);

	TIMECOP_G(ce_DateTimeZone) = timezone_ce;
	if (interface_ce) {
		TIMECOP_G(ce_DateTimeInterface) = interface_ce;
	} else {
		TIMECOP_G(ce_DateTimeInterface) = date_ce;
	}

	/* replace DateTime */
	INIT_CLASS_ENTRY(ce, "TimecopDateTime", timecop_funcs_date);
	tmp = zend_register_internal_class_ex(&ce, date_ce, NULL TSRMLS_CC);
	tmp->create_object = date_ce->create_object;

	TIMECOP_G(ce_DateTime) = date_ce;
	TIMECOP_G(ce_TimecopDateTime) = tmp;

	INIT_CLASS_ENTRY(ce, "TimecopOrigDateTime", timecop_funcs_orig_date);
	tmp = zend_register_internal_class_ex(&ce, date_ce, NULL TSRMLS_CC);
	tmp->create_object = date_ce->create_object;

#if PHP_VERSION_ID >= 50500
	/* replace DateTimeImmutable */
	INIT_CLASS_ENTRY(ce, "TimecopDateTimeImmutable", timecop_funcs_immutable);
	tmp = zend_register_internal_class_ex(&ce, immutable_ce, NULL TSRMLS_CC);
	tmp->create_object = immutable_ce->create_object;

	TIMECOP_G(ce_DateTimeImmutable) = immutable_ce;
	TIMECOP_G(ce_TimecopDateTimeImmutable) = tmp;

	INIT_CLASS_ENTRY(ce, "TimecopOrigDateTimeImmutable", timecop_funcs_orig_immutable);
	tmp = zend_register_internal_class_ex(&ce, immutable_ce, NULL TSRMLS_CC);
	tmp->create_object = immutable_ce->create_object;
#endif

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
			/* Do nothing. Because some functions are introduced by optional extensions. */
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

/*  clear function overriding. */
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

static int fill_mktime_params(zval ***params, const char *date_function_name, int from TSRMLS_DC)
{
	int i;
	char *formats[MKTIME_NUM_ARGS] = {"H", "i", "s", "n", "j", "Y"};
	zval format, timestamp, *retval_ptr;

	INIT_ZVAL(timestamp);
	ZVAL_LONG(&timestamp, mock_timestamp(TSRMLS_C));
	INIT_ZVAL(format);

	for (i = from; i < MKTIME_NUM_ARGS; i++) {
		ZVAL_STRING(&format, formats[i], 0);

		call_php_function_with_2_params(date_function_name, &retval_ptr, &format, &timestamp);
		if (retval_ptr) {
			ZVAL_ZVAL(*params[i], retval_ptr, 1, 1);
		}
	}

	return MKTIME_NUM_ARGS;
}

/*
 * get_formatted_mock_time() : return formatted mock time like "2000-12-30 01:02:03.456000"
 *
 * pseudo code:
 *
 * function get_formatted_mock_time($time, $timezone_obj) {
 *     if ($time === null || $time === false || $time === "") {
 *         $time = "now";
 *     }
 *     $now = get_mock_timeval();
 *     if ($timezone_obj) {
 *         // save default timezone
 *         $zonename = $timezone_obj->getName()
 *         $orig_zonename = date_default_timezone_get();
 *         date_default_timezone_set($zonename);
 *     }
 *     $fixed_sec = strtotime($time, $now->sec);
 *     if ($timezone_obj && $orig_zonename) {
 *         // restore default timezone
 *         date_default_timezone_set($orig_zonename);
 *     }
 *     if ($fixed_sec === FALSE) {
 *         return false;
 *     }
 *     $fixed_usec = get_mock_fraction($time, $timezone_obj);
 *     if ($fixed_usec === -1) {
 *         $fixed_usec = $now->usec;
 *     }
 *     $dt = date_create($time, $timezone_obj);
 *     $dt->setTimestamp($fixed_sec);
 *     $format = sprintf("Y-m-d H:i:s.%06d", $fixed_usec);
 *     $formatted_time = $dt->format($format);
 *     return $formatted_time;
 * }
 */
static int get_formatted_mock_time(zval *time, zval *timezone_obj, zval **retval_time, zval **retval_timezone TSRMLS_DC)
{
	zval *fixed_sec, *orig_zonename;
	zval str_now, now_timestamp;
	tc_timeval now;
	long fixed_usec;

	if (TIMECOP_G(timecop_mode) == TIMECOP_MODE_REALTIME) {
		MAKE_STD_ZVAL(*retval_time);
		ZVAL_FALSE(*retval_time);
		MAKE_STD_ZVAL(*retval_timezone);
		ZVAL_NULL(*retval_timezone);
		return -1;
	}

	if (time == NULL || Z_TYPE_P(time) == IS_NULL ||
		(Z_TYPE_P(time) == IS_BOOL && !Z_BVAL_P(time)) ||
		(Z_TYPE_P(time) == IS_STRING && Z_STRLEN_P(time) == 0)) {
		INIT_ZVAL(str_now);
		ZVAL_STRING(&str_now, "now", 0);
		time = &str_now;
	}

	get_mock_timeval(&now, NULL TSRMLS_CC);

	if (timezone_obj && Z_TYPE_P(timezone_obj) == IS_OBJECT) {
		zval *zonename;
		call_php_method_with_0_params(&timezone_obj, Z_OBJCE_PP(&timezone_obj), "getname", &zonename);
		if (zonename) {
			call_php_function_with_0_params("date_default_timezone_get", &orig_zonename);
			if (orig_zonename) {
				call_php_function_with_1_params("date_default_timezone_set", NULL, zonename);
			}
			zval_ptr_dtor(&zonename);
		}
	}

	INIT_ZVAL(now_timestamp);
	ZVAL_LONG(&now_timestamp, now.sec);
	call_php_function_with_2_params(ORIG_FUNC_NAME("strtotime"), &fixed_sec, time, &now_timestamp);

	if (timezone_obj && Z_TYPE_P(timezone_obj) == IS_OBJECT) {
		call_php_function_with_1_params("date_default_timezone_set", NULL, orig_zonename);
		zval_ptr_dtor(&orig_zonename);
	}

	if (Z_TYPE_P(fixed_sec) == IS_BOOL && !Z_BVAL_P(fixed_sec)) {
		/* $fixed_sec === false */
		MAKE_STD_ZVAL(*retval_time);
		ZVAL_FALSE(*retval_time);
		MAKE_STD_ZVAL(*retval_timezone);
		ZVAL_NULL(*retval_timezone);
		return -1;
	}

	fixed_usec = get_mock_fraction(time, timezone_obj TSRMLS_CC);
	if (fixed_usec == -1) {
		fixed_usec = now.usec;
	}

	{
		zval *dt;
		zval format_str;
		char buf[64];

		call_php_function_with_2_params(ORIG_FUNC_NAME("date_create"), &dt, time, timezone_obj);
		if (Z_TYPE_P(dt) == IS_BOOL && !Z_BVAL_P(dt)) {
			MAKE_STD_ZVAL(*retval_time);
			ZVAL_FALSE(*retval_time);
			MAKE_STD_ZVAL(*retval_timezone);
			ZVAL_NULL(*retval_timezone);
			return -1;
		}

		sprintf(buf, "Y-m-d H:i:s.%06ld", fixed_usec);
		INIT_ZVAL(format_str);
		ZVAL_STRING(&format_str, buf, 0);
		call_php_function_with_2_params("date_timestamp_set", NULL, dt, fixed_sec);
		call_php_method_with_0_params(&dt, TIMECOP_G(ce_DateTime), "gettimezone", retval_timezone);
		call_php_method_with_1_params(&dt, TIMECOP_G(ce_DateTime), "format", retval_time, &format_str);

		zval_ptr_dtor(&dt);
	}

	if (fixed_sec) {
		zval_ptr_dtor(&fixed_sec);
	}

	return 0;
}

/*
 * get_mock_fraction()
 *
 * pseudo code:
 *
 * function get_mock_fraction($time, $timezone_obj) {
 *     $dt1 = date_create($time, $timezone_obj);
 *     $dt2 = date_create($time, $timezone_obj);
 *     $usec1 = $dt1->format("u");
 *     $usec2 = $dt2->format("u");
 *     if ($usec1 === $usec2) {
 *         $fixed_usec = $usec1;
 *     } else {
 *         $fixed_usec = -1;
 *     }
 *     return $fixed_usec;
 * }
 */
static long get_mock_fraction(zval *time, zval *timezone_obj TSRMLS_DC)
{
	zval *dt1, *dt2, *usec1, *usec2;
	long fixed_usec;
	zval u_str;

	call_php_function_with_2_params(ORIG_FUNC_NAME("date_create"), &dt1, time, timezone_obj);
	if (Z_TYPE_P(dt1) == IS_BOOL && !Z_BVAL_P(dt1)) {
		return -1;
	}
	call_php_function_with_2_params(ORIG_FUNC_NAME("date_create"), &dt2, time, timezone_obj);
	if (Z_TYPE_P(dt2) == IS_BOOL && !Z_BVAL_P(dt2)) {
		zval_ptr_dtor(&dt1);
		return -1;
	}
	INIT_ZVAL(u_str);
	ZVAL_STRING(&u_str, "u", 0);
	call_php_method_with_1_params(&dt1, TIMECOP_G(ce_DateTime), "format", &usec1, &u_str);
	call_php_method_with_1_params(&dt2, TIMECOP_G(ce_DateTime), "format", &usec2, &u_str);
	convert_to_long(usec1);
	convert_to_long(usec2);

	if (Z_LVAL_P(usec1) == Z_LVAL_P(usec2)) {
		fixed_usec = Z_LVAL_P(usec1);
	} else {
		fixed_usec = -1;
	}
	zval_ptr_dtor(&dt1);
	zval_ptr_dtor(&dt2);
	zval_ptr_dtor(&usec1);
	zval_ptr_dtor(&usec2);

	return fixed_usec;
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
		ZVAL_LONG(&filled_timestamp, mock_timestamp(TSRMLS_C));
		zp = &filled_timestamp;

		params[argc] = &zp;
		argc++;
	}

	call_php_function_with_params(function_name, &retval_ptr, argc, params);

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

	call_php_function_with_params(mktime_function_name, &retval_ptr, params_size, params);

	for (i = argc; i < MKTIME_NUM_ARGS; i++) {
		zval_ptr_dtor(&filled_value[i]);
	}
	efree(params);

	if (retval_ptr) {
		RETVAL_ZVAL(retval_ptr, 1, 1);
	}
}
/* }}} */

/* {{{ proto int timecop_freeze(long timestamp)
   Time travel to specified timestamp and freeze */
PHP_FUNCTION(timecop_freeze)
{
	zval *dt;
	long timestamp;
	tc_timeval freezed_tv;

	if (zend_parse_parameters_ex(ZEND_PARSE_PARAMS_QUIET, ZEND_NUM_ARGS() TSRMLS_CC, "O", &dt, TIMECOP_G(ce_DateTimeInterface)) != FAILURE) {
		get_timeval_from_datetime(&freezed_tv, dt TSRMLS_CC);
	} else if (zend_parse_parameters_ex(ZEND_PARSE_PARAMS_QUIET, ZEND_NUM_ARGS() TSRMLS_CC, "l", &timestamp) != FAILURE) {
		freezed_tv.sec = timestamp;
		freezed_tv.usec = 0;
	} else {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "This function accepts either (DateTimeInterface) OR (int) as arguments.");
		RETURN_FALSE;
	}

	TIMECOP_G(timecop_mode) = TIMECOP_MODE_FREEZE;
	TIMECOP_G(freezed_time) = freezed_tv;

	if (TIMECOP_G(sync_request_time)){
		update_request_time(freezed_tv.sec TSRMLS_CC);
	}

	RETURN_TRUE;
}
/* }}} */

/* {{{ proto int timecop_travel(long timestamp)
   Time travel to specified timestamp */
PHP_FUNCTION(timecop_travel)
{
	zval *dt;
	long timestamp;
	tc_timeval now, mock_tv;

	if (zend_parse_parameters_ex(ZEND_PARSE_PARAMS_QUIET, ZEND_NUM_ARGS() TSRMLS_CC, "O", &dt, TIMECOP_G(ce_DateTimeInterface)) != FAILURE) {
		get_timeval_from_datetime(&mock_tv, dt TSRMLS_CC);
	} else if (zend_parse_parameters_ex(ZEND_PARSE_PARAMS_QUIET, ZEND_NUM_ARGS() TSRMLS_CC, "l", &timestamp) != FAILURE) {
		mock_tv.sec = timestamp;
		mock_tv.usec = 0;
	} else {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "This function accepts either (DateTimeInterface) OR (int) as arguments.");
		RETURN_FALSE;
	}

	TIMECOP_G(timecop_mode) = TIMECOP_MODE_TRAVEL;
	get_current_time(&now TSRMLS_CC);
	tc_timeval_sub(&TIMECOP_G(travel_offset), &mock_tv, &now);
	TIMECOP_G(travel_origin) = now;

	if (TIMECOP_G(sync_request_time)){
		update_request_time(mock_tv.sec TSRMLS_CC);
	}

	RETURN_TRUE;
}
/* }}} */

/* {{{ proto int timecop_scale(long scale)
   Time travel to specified timestamp */
PHP_FUNCTION(timecop_scale)
{
	long scale;
	tc_timeval now, mock_time;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &scale) == FAILURE) {
		RETURN_FALSE;
	}
	if (scale < 0) {
		RETURN_FALSE;
	}
	get_current_time(&now TSRMLS_CC);
	get_mock_timeval(&mock_time, &now TSRMLS_CC);
	TIMECOP_G(timecop_mode) = TIMECOP_MODE_TRAVEL;
	TIMECOP_G(travel_origin) = now;
	tc_timeval_sub(&TIMECOP_G(travel_offset), &mock_time, &now);
	TIMECOP_G(scaling_factor) = scale;

	if (TIMECOP_G(sync_request_time)){
		update_request_time(mock_time.sec TSRMLS_CC);
	}

	RETURN_TRUE;
}
/* }}} */

/* {{{ proto int timecop_return(void)
   Return to Time travel to specified timestamp */
PHP_FUNCTION(timecop_return)
{
	TIMECOP_G(timecop_mode) = TIMECOP_MODE_REALTIME;

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
	RETURN_LONG(mock_timestamp(TSRMLS_C));
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

/*
 * get_mock_timeval(fixed, now)
 *
 *
 *               delta
 *   |<----------------------->|
 *     travel_offset                  delta * scaling_factor
 *   |<------------->|<------------------------------------------------->|
 * ==@===============@=========@=========================================@==
 *   ^                         ^                                         ^
 *   |                         |                                         |
 *   travel_origin             orig_time                     traveled_time
 *
 *
 * delta = orig_time - travel_origin
 * traveled_time = travel_origin + travel_offset + delta * scaling_factor
 */
static int get_mock_timeval(tc_timeval *fixed, const tc_timeval *now TSRMLS_DC)
{
	if (TIMECOP_G(timecop_mode) == TIMECOP_MODE_FREEZE) {
		*fixed = TIMECOP_G(freezed_time);
	} else if (TIMECOP_G(timecop_mode) == TIMECOP_MODE_TRAVEL) {
		tc_timeval delta, origin = TIMECOP_G(travel_origin);
		long scale = TIMECOP_G(scaling_factor);
		if (now == NULL) {
			get_current_time(&delta TSRMLS_CC);
		} else {
			delta = *now;
		}
		tc_timeval_sub(&delta, &delta, &origin);
		tc_timeval_mul(&delta, &delta, scale);
		tc_timeval_add(fixed, &origin, &TIMECOP_G(travel_offset));
		tc_timeval_add(fixed, fixed, &delta);
	} else {
		if (now == NULL) {
			get_current_time(fixed TSRMLS_CC);
		} else {
			*fixed = *now;
		}
	}
	return 0;
}

static inline long mock_timestamp(TSRMLS_D)
{
	tc_timeval tv;
	get_mock_timeval(&tv, NULL TSRMLS_CC);
	return tv.sec;
}

static int get_timeval_from_datetime(tc_timeval *tp, zval *dt TSRMLS_DC)
{
	zval *sec, *usec;
	zval u_str;

	call_php_function_with_1_params("date_timestamp_get", &sec, dt);
	INIT_ZVAL(u_str);
	ZVAL_STRING(&u_str, "u", 0);
	call_php_method_with_1_params(&dt, Z_OBJCE_P(dt), "format", &usec, &u_str);
	convert_to_long(usec);

	tp->sec = Z_LVAL_P(sec);
	tp->usec = Z_LVAL_P(usec);

	zval_ptr_dtor(&sec);
	zval_ptr_dtor(&usec);

	return 0;
}

static int get_current_time(tc_timeval *now TSRMLS_DC)
{
	int ret = 0;
#if HAVE_GETTIMEOFDAY
	struct timeval tv;
	ret = gettimeofday(&tv, NULL);
	if (ret == 0) {
		now->sec  = (long)tv.tv_sec;
		now->usec = (long)tv.tv_usec;
	}
#else
	time_t ts = time(NULL);
	if (ts == -1) {
		ret = -1;
	} else {
		now->sec  = ts;
		now->usec = 0;
	}
#endif
	return ret;
}

#ifdef HAVE_GETTIMEOFDAY

#define MICRO_IN_SEC 1000000.00
#define SEC_IN_MIN 60

static void _timecop_gettimeofday(INTERNAL_FUNCTION_PARAMETERS, int mode)
{
	zend_bool get_as_float = 0;
	tc_timeval fixed;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|b", &get_as_float) == FAILURE) {
		return;
	}

	get_mock_timeval(&fixed, NULL TSRMLS_CC);

	if (get_as_float) {
		RETURN_DOUBLE((double)(fixed.sec + fixed.usec / MICRO_IN_SEC));
	}
	if (mode) {
		zval *zv_offset, *zv_dst, format, timestamp;
		long offset = 0, is_dst = 0;

		INIT_ZVAL(timestamp);
		ZVAL_LONG(&timestamp, fixed.sec);

		/* offset */
		INIT_ZVAL(format);
		ZVAL_STRING(&format, "Z", 0);
		call_php_function_with_2_params(ORIG_FUNC_NAME("date"), &zv_offset, &format, &timestamp);
		convert_to_long(zv_offset);
		offset = Z_LVAL_P(zv_offset);
		zval_ptr_dtor(&zv_offset);

		/* is_dst */
		ZVAL_STRING(&format, "I", 0);
		call_php_function_with_2_params(ORIG_FUNC_NAME("date"), &zv_dst, &format, &timestamp);
		convert_to_long(zv_dst);
		is_dst = Z_LVAL_P(zv_dst);
		zval_ptr_dtor(&zv_dst);

		array_init(return_value);
		add_assoc_long(return_value, "sec", fixed.sec);
		add_assoc_long(return_value, "usec", fixed.usec);
		add_assoc_long(return_value, "minuteswest", -offset / SEC_IN_MIN);
		add_assoc_long(return_value, "dsttime", is_dst);
	} else {
		char ret[100];
		snprintf(ret, 100, "%.8F %ld", fixed.usec / MICRO_IN_SEC, fixed.sec);
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

static void _timecop_orig_datetime_constructor(INTERNAL_FUNCTION_PARAMETERS, int immutable)
{
	zval *arg1 = NULL, *arg2 = NULL;
	zend_class_entry *real_ce;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|zz", &arg1, &arg2) == FAILURE) {
		RETURN_FALSE;
	}

	if (immutable) {
		real_ce = TIMECOP_G(ce_DateTimeImmutable);
	} else {
		real_ce = TIMECOP_G(ce_DateTime);
	}

	call_php_method_with_2_params(&getThis(), real_ce, ORIG_FUNC_NAME("__construct"), NULL, arg1, arg2);
}

static inline void _timecop_datetime_constructor(INTERNAL_FUNCTION_PARAMETERS, int immutable)
{
	_timecop_datetime_constructor_ex(INTERNAL_FUNCTION_PARAM_PASSTHRU, getThis(), immutable);
}

static inline void _timecop_date_create(INTERNAL_FUNCTION_PARAMETERS, int immutable)
{
	_timecop_datetime_constructor_ex(INTERNAL_FUNCTION_PARAM_PASSTHRU, NULL, immutable);
}

static void _timecop_datetime_constructor_ex(INTERNAL_FUNCTION_PARAMETERS, zval *obj, int immutable)
{
	zval orig_time, *orig_timezone = NULL;
	zval *fixed_time, *fixed_timezone, *dt, *arg1, *arg2;
	char *orig_time_str = NULL;
	int orig_time_len = 0;
	const char *real_func;
	zend_class_entry *real_ce;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|sO!", &orig_time_str, &orig_time_len, &orig_timezone, TIMECOP_G(ce_DateTimeZone)) == FAILURE) {
		RETURN_FALSE;
	}

	INIT_ZVAL(orig_time);
	if (orig_time_str == NULL) {
		ZVAL_NULL(&orig_time);
	} else {
		ZVAL_STRINGL(&orig_time, orig_time_str, orig_time_len, 0);
	}
	if (immutable) {
		real_func = ORIG_FUNC_NAME("date_create_immutable");
		real_ce = TIMECOP_G(ce_DateTimeImmutable);
	} else {
		real_func = ORIG_FUNC_NAME("date_create");
		real_ce = TIMECOP_G(ce_DateTime);
	}

	if (get_formatted_mock_time(&orig_time, orig_timezone, &fixed_time, &fixed_timezone TSRMLS_CC) == 0) {
		arg1 = fixed_time;
		arg2 = fixed_timezone;
	} else {
		arg1 = &orig_time;
		arg2 = orig_timezone;
	}
	if (obj == NULL) {
		call_php_function_with_2_params(real_func, &dt, arg1, arg2);
	} else {
		call_php_method_with_2_params(&obj, real_ce, ORIG_FUNC_NAME("__construct"), NULL, arg1, arg2);
	}

	zval_ptr_dtor(&fixed_time);
	zval_ptr_dtor(&fixed_timezone);

	if (obj == NULL) {
		RETURN_ZVAL(dt, 1, 1);
	}
}

/* {{{ proto DateTime timecop_date_create([string time[, DateTimeZone object]])
   Returns new DateTime object initialized with traveled time */
PHP_FUNCTION(timecop_date_create)
{
	_timecop_date_create(INTERNAL_FUNCTION_PARAM_PASSTHRU, 0);
}
/* }}} */

#if PHP_VERSION_ID >= 50300
static void _timecop_date_create_from_format(INTERNAL_FUNCTION_PARAMETERS, int immutable)
{
	zval *orig_timezone = NULL;
	zval orig_format, orig_time, fixed_format, *fixed_time, *new_format, *new_time;
	zval *dt, *new_dt, now_timestamp, tmp;
	char *orig_format_str, *orig_time_str;
	int orig_format_len, orig_time_len;
	tc_timeval now;
	char buf[64];
	const char *real_func;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ss|O!", &orig_format_str, &orig_format_len, &orig_time_str, &orig_time_len, &orig_timezone, TIMECOP_G(ce_DateTimeZone)) == FAILURE) {
		RETURN_FALSE;
	}

	INIT_ZVAL(orig_format);
	ZVAL_STRINGL(&orig_format, orig_format_str, orig_format_len, 0);
	INIT_ZVAL(orig_time);
	ZVAL_STRINGL(&orig_time, orig_time_str, orig_time_len, 0);

	call_php_function_with_3_params(ORIG_FUNC_NAME("date_create_from_format"), &dt, &orig_format, &orig_time, orig_timezone);
	if (Z_TYPE_P(dt) == IS_BOOL && !Z_BVAL_P(dt)) {
		RETURN_FALSE;
	}

	if (memchr(orig_format_str, '!', orig_format_len) ||
		memchr(orig_format_str, '|', orig_format_len)) {
		RETURN_ZVAL(dt, 1, 1);
	}

	get_mock_timeval(&now, NULL TSRMLS_CC);

	INIT_ZVAL(now_timestamp);
	ZVAL_LONG(&now_timestamp, now.sec);
	call_php_method_with_1_params(&dt, TIMECOP_G(ce_DateTime), "settimestamp", NULL, &now_timestamp);
	sprintf(buf, "Y-m-d H:i:s.%06ld ", now.usec);
	INIT_ZVAL(tmp);
	ZVAL_STRINGL(&tmp, buf, strlen(buf), 0);
	call_php_method_with_1_params(&dt, TIMECOP_G(ce_DateTime), "format", &fixed_time, &tmp);

	INIT_ZVAL(fixed_format);
	if (memchr(orig_format_str, 'g', orig_format_len) ||
		memchr(orig_format_str, 'h', orig_format_len) ||
		memchr(orig_format_str, 'G', orig_format_len) ||
		memchr(orig_format_str, 'H', orig_format_len) ||
		memchr(orig_format_str, 'i', orig_format_len) ||
		memchr(orig_format_str, 's', orig_format_len)) {
		ZVAL_STRING(&fixed_format, "Y-m-d ??:??:??.??????", 0);
	} else if (memchr(orig_format_str, 'Y', orig_format_len) ||
			   memchr(orig_format_str, 'y', orig_format_len) ||
			   memchr(orig_format_str, 'F', orig_format_len) ||
			   memchr(orig_format_str, 'M', orig_format_len) ||
			   memchr(orig_format_str, 'm', orig_format_len) ||
			   memchr(orig_format_str, 'n', orig_format_len) ||
			   memchr(orig_format_str, 'd', orig_format_len) ||
			   memchr(orig_format_str, 'j', orig_format_len) ||
			   memchr(orig_format_str, 'D', orig_format_len) ||
			   memchr(orig_format_str, 'l', orig_format_len) ||
			   memchr(orig_format_str, 'U', orig_format_len)) {
		ZVAL_STRING(&fixed_format, "Y-m-d H:i:s.??????", 0);
	} else {
		ZVAL_STRING(&fixed_format, "Y-m-d H:i:s.??????", 0);
	}

	ZVAL_STRING(&tmp, "%s %s", 0);
	call_php_function_with_3_params("sprintf", &new_format, &tmp, &fixed_format, &orig_format);
	call_php_function_with_3_params("sprintf", &new_time, &tmp, fixed_time, &orig_time);

	if (immutable) {
		real_func = ORIG_FUNC_NAME("date_create_immutable_from_format");
	} else {
		real_func = ORIG_FUNC_NAME("date_create_from_format");
	}
	call_php_function_with_3_params(real_func, &new_dt, new_format, new_time, orig_timezone);

	zval_ptr_dtor(&dt);
	zval_ptr_dtor(&fixed_time);
	zval_ptr_dtor(&new_format);
	zval_ptr_dtor(&new_time);
	RETURN_ZVAL(new_dt, 1, 1);
}

/* {{{ proto DateTime timecop_date_create_from_format(string format, string time[, DateTimeZone object])
   Returns new DateTime object initialized with traveled time */
PHP_FUNCTION(timecop_date_create_from_format)
{
	_timecop_date_create_from_format(INTERNAL_FUNCTION_PARAM_PASSTHRU, 0);
}
/* }}} */
#endif

#if PHP_VERSION_ID >= 50500
/* {{{ proto DateTimeImmutable timecop_date_create_immutable([string time[, DateTimeZone object]])
   Returns new DateTimeImmutable object initialized with traveled time */
PHP_FUNCTION(timecop_date_create_immutable)
{
	_timecop_date_create(INTERNAL_FUNCTION_PARAM_PASSTHRU, 1);
}
/* }}} */

/* {{{ proto DateTimeImmutable timecop_date_create_immutable_from_format(string format, string time[, DateTimeZone object])
   Returns new DateTimeImmutable object initialized with traveled time */
PHP_FUNCTION(timecop_date_create_immutable_from_format)
{
	_timecop_date_create_from_format(INTERNAL_FUNCTION_PARAM_PASSTHRU, 1);
}
/* }}} */
#endif

/* {{{ proto TimecopDateTime::__construct([string time[, DateTimeZone object]])
   Creates new TimecopDateTime object */
PHP_METHOD(TimecopDateTime, __construct)
{
	_timecop_datetime_constructor(INTERNAL_FUNCTION_PARAM_PASSTHRU, 0);
}
/* }}} */

/* {{{ proto TimecopOrigDateTime::__construct([string time[, DateTimeZone object]])
   Creates new TimecopOrigDateTime object */
PHP_METHOD(TimecopOrigDateTime, __construct)
{
	_timecop_orig_datetime_constructor(INTERNAL_FUNCTION_PARAM_PASSTHRU, 0);
}
/* }}} */

#if PHP_VERSION_ID >= 50500
/* {{{ proto TimecopDateTimeImmutable::__construct([string time[, DateTimeZone object]])
   Creates new TimecopDateTimeImmutable object */
PHP_METHOD(TimecopDateTimeImmutable, __construct)
{
	_timecop_datetime_constructor(INTERNAL_FUNCTION_PARAM_PASSTHRU, 1);
}
/* }}} */

/* {{{ proto TimecopOrigDateTimeImmutable::__construct([string time[, DateTimeZone object]])
   Creates new TimecopOrigDateTimeImmutable object */
PHP_METHOD(TimecopOrigDateTimeImmutable, __construct)
{
	_timecop_orig_datetime_constructor(INTERNAL_FUNCTION_PARAM_PASSTHRU, 1);
}
/* }}} */
#endif

#if !defined(PHP_VERSION_ID) || PHP_VERSION_ID < 50300
/* {{{ proto DateTime date_timestamp_set(DateTime object, long unixTimestamp)
   Sets the date and time based on an Unix timestamp.
*/
PHP_FUNCTION(date_timestamp_set)
{
	zval         *object;
	php_date_obj *dateobj;
	long          timestamp;

	if (zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "Ol", &object, TIMECOP_G(ce_DateTime), &timestamp) == FAILURE) {
		RETURN_FALSE;
	}
	dateobj = (php_date_obj *) zend_object_store_get_object(object TSRMLS_CC);
	DATE_CHECK_INITIALIZED(dateobj->time, DateTime);
	timelib_unixtime2local(dateobj->time, (timelib_sll)timestamp);
	timelib_update_ts(dateobj->time, NULL);

	RETURN_ZVAL(object, 1, 0);
}
/* }}} */

/* {{{ proto long date_timestamp_get(DateTime object)
   Gets the Unix timestamp.
*/
PHP_FUNCTION(date_timestamp_get)
{
	zval         *object;
	php_date_obj *dateobj;
	long          timestamp;
	int           error;

	if (zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "O", &object, TIMECOP_G(ce_DateTime)) == FAILURE) {
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
#endif

static inline zval* _call_php_method_with_0_params(zval **object_pp, zend_class_entry *obj_ce, const char *method_name, zval **retval_ptr_ptr TSRMLS_DC)
{
	return _call_php_method(object_pp, obj_ce, method_name, retval_ptr_ptr, 0, NULL, NULL TSRMLS_CC);
}

static inline zval* _call_php_method_with_1_params(zval **object_pp, zend_class_entry *obj_ce, const char *method_name, zval **retval_ptr_ptr, zval *arg1 TSRMLS_DC)
{
	int nparams = 1;
	if (arg1 == NULL) {
		nparams = 0;
	}
	return _call_php_method(object_pp, obj_ce, method_name, retval_ptr_ptr, nparams, arg1, NULL TSRMLS_CC);
}

static inline zval* _call_php_method_with_2_params(zval **object_pp, zend_class_entry *obj_ce, const char *method_name, zval **retval_ptr_ptr, zval *arg1, zval *arg2 TSRMLS_DC)
{
	int nparams = 2;
	if (arg1 == NULL) {
		nparams = 0;
	} else if (arg2 == NULL) {
		nparams = 1;
	}
	return _call_php_method(object_pp, obj_ce, method_name, retval_ptr_ptr, nparams, arg1, arg2 TSRMLS_CC);
}

static inline zval* _call_php_method(zval **object_pp, zend_class_entry *obj_ce, const char *method_name, zval **retval_ptr_ptr, int param_count, zval* arg1, zval* arg2 TSRMLS_DC)
{
	return zend_call_method(object_pp, obj_ce, NULL, method_name, strlen(method_name), retval_ptr_ptr, param_count, arg1, arg2 TSRMLS_CC);
}

static inline void _call_php_function_with_0_params(const char *function_name, zval **retval_ptr_ptr TSRMLS_DC)
{
	_call_php_method_with_0_params(NULL, NULL, function_name, retval_ptr_ptr TSRMLS_CC);
}

static inline void _call_php_function_with_1_params(const char *function_name, zval **retval_ptr_ptr, zval *arg1 TSRMLS_DC)
{
	_call_php_method_with_1_params(NULL, NULL, function_name, retval_ptr_ptr, arg1 TSRMLS_CC);
}
static inline void _call_php_function_with_2_params(const char *function_name, zval **retval_ptr_ptr, zval *arg1, zval *arg2 TSRMLS_DC)
{
	_call_php_method_with_2_params(NULL, NULL, function_name, retval_ptr_ptr, arg1, arg2 TSRMLS_CC);
}
static void _call_php_function_with_3_params(const char *function_name, zval **retval_ptr_ptr, zval *arg1, zval *arg2, zval *arg3 TSRMLS_DC)
{
	if (arg3 == NULL) {
		_call_php_function_with_2_params(function_name, retval_ptr_ptr, arg1, arg2 TSRMLS_CC);
	} else {
		zval *zps[3] = {arg1, arg2, arg3};
		zval **params[3] = {&zps[0], &zps[1], &zps[2]};
		_call_php_function_with_params(function_name, retval_ptr_ptr, 3, params TSRMLS_CC);
	}
}

static inline void _call_php_function_with_params(const char *function_name, zval **retval_ptr_ptr, zend_uint param_count, zval **params[] TSRMLS_DC)
{
	zval callable;

	INIT_ZVAL(callable);
	ZVAL_STRING(&callable, function_name, 0);

	call_user_function_ex(EG(function_table), NULL, &callable, retval_ptr_ptr, param_count, params, 1, NULL TSRMLS_CC);
}

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
