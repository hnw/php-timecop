/*
MIT License

Copyright (c) 2012-2017 Yoshio HANAWA

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
 */

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

static void timecop_globals_ctor(zend_timecop_globals *globals) {
	/* Initialize your global struct */
	globals->func_override = 1;
	globals->sync_request_time = 1;
	ZVAL_NULL(&globals->orig_request_time);
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
	TIMECOP_OFE("date_create_from_format"),
	TIMECOP_OFE("date_create_immutable"),
	TIMECOP_OFE("date_create_immutable_from_format"),
	{NULL, NULL, NULL}
};

static const struct timecop_override_class_entry timecop_override_class_table[] = {
	TIMECOP_OCE("datetime", "__construct"),
	TIMECOP_OCE("datetime", "createfromformat"),
	TIMECOP_OCE("datetimeimmutable", "__construct"),
	TIMECOP_OCE("datetimeimmutable", "createfromformat"),
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

ZEND_BEGIN_ARG_INFO_EX(arginfo_timecop_date_create_from_format, 0, 0, 2)
	ZEND_ARG_INFO(0, format)
	ZEND_ARG_INFO(0, time)
#if PHP_VERSION_ID >= 70200
	ZEND_ARG_OBJ_INFO(0, object, DateTimeZone, 1)
#else
	ZEND_ARG_INFO(0, object)
#endif
ZEND_END_ARG_INFO()

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
	PHP_FE(timecop_date_create_from_format, arginfo_timecop_date_create_from_format)
	PHP_FE(timecop_date_create_immutable, arginfo_timecop_date_create)
	PHP_FE(timecop_date_create_immutable_from_format, arginfo_timecop_date_create_from_format)
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
	PHP_ME_MAPPING(createFromFormat, timecop_date_create_from_format, arginfo_timecop_date_create_from_format,
				   ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	{NULL, NULL, NULL}
};

static zend_function_entry timecop_funcs_orig_date[] = {
	PHP_ME(TimecopOrigDateTime, __construct, arginfo_timecop_date_create,
		   ZEND_ACC_CTOR | ZEND_ACC_PUBLIC)
	{NULL, NULL, NULL}
};

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

static int update_request_time(zend_long unixtime);
static int restore_request_time();

static int fill_mktime_params(zval *fill_params, const char *date_function_name, int from);
static int get_formatted_mock_time(zval *time, zval *timezone_obj, zval *retval_time, zval *retval_timezone);
static long get_mock_fraction(zval *time, zval *timezone_obj);

static void _timecop_call_function(INTERNAL_FUNCTION_PARAMETERS, const char *function_name, int index_to_fill_timestamp);
static void _timecop_call_mktime(INTERNAL_FUNCTION_PARAMETERS, const char *mktime_function_name, const char *date_function_name);

static int get_mock_timeval(tc_timeval *fixed, const tc_timeval *now);
static inline zend_long mock_timestamp();

static int get_timeval_from_datetime(tc_timeval *tp, zval *dt);
static int get_current_time(tc_timeval *now);

static void _timecop_orig_datetime_constructor(INTERNAL_FUNCTION_PARAMETERS, int immutable);
static inline void _timecop_date_create(INTERNAL_FUNCTION_PARAMETERS, int immutable);
static inline void _timecop_datetime_constructor(INTERNAL_FUNCTION_PARAMETERS, int immutable);
static void _timecop_datetime_constructor_ex(INTERNAL_FUNCTION_PARAMETERS, zval *obj, int immutable);

static inline zval* _call_php_method_with_0_params(zval *object_pp, zend_class_entry *obj_ce, const char *method_name, zval *retval_ptr);
static inline zval* _call_php_method_with_1_params(zval *object_pp, zend_class_entry *obj_ce, const char *method_name, zval *retval_ptr, zval *arg1);
static inline zval* _call_php_method_with_2_params(zval *object_pp, zend_class_entry *obj_ce, const char *method_name, zval *retval_ptr, zval *arg1, zval *arg2);
static inline zval* _call_php_method(zval *object_pp, zend_class_entry *obj_ce, const char *method_name, zval *retval_ptr, int param_count, zval* arg1, zval* arg2);
static inline void _call_php_function_with_0_params(const char *function_name, zval *retval_ptr);
static inline void _call_php_function_with_1_params(const char *function_name, zval *retval_ptr, zval *arg1);
static inline void _call_php_function_with_2_params(const char *function_name, zval *retval_ptr, zval *arg1, zval *arg2);
static void _call_php_function_with_3_params(const char *function_name, zval *retval_ptr, zval *arg1, zval *arg2, zval *arg3);
static inline void _call_php_function_with_params(const char *function_name, zval *retval_ptr, uint32_t param_count, zval params[]);

static const zend_module_dep timecop_module_deps[] = {
	ZEND_MOD_REQUIRED("Date")
    ZEND_MOD_END
};

/* {{{ timecop_module_entry
 */
zend_module_entry timecop_module_entry = {
    STANDARD_MODULE_HEADER_EX,
    NULL,
    timecop_module_deps,
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

	if (TIMECOP_G(func_override)) {
		if (SUCCESS != timecop_func_override() ||
			SUCCESS != timecop_class_override()) {
			return FAILURE;
		}
	}

	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MSHUTDOWN_FUNCTION
 */
PHP_MSHUTDOWN_FUNCTION(timecop)
{
	UNREGISTER_INI_ENTRIES();
	
	if (TIMECOP_G(func_override)) {
		timecop_func_override_clear();
		timecop_class_override_clear();
	}
	
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_RINIT_FUNCTION(timecop) */
PHP_RINIT_FUNCTION(timecop)
{
#if defined(COMPILE_DL_TIMECOP) && defined(ZTS)
	ZEND_TSRMLS_CACHE_UPDATE();
#endif

	return SUCCESS;
}
/* }}} */

/* {{{ PHP_RSHUTDOWN_FUNCTION(timecop) */
PHP_RSHUTDOWN_FUNCTION(timecop)
{
	if (Z_TYPE(TIMECOP_G(orig_request_time)) == IS_NULL) {
		restore_request_time();
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

static int register_timecop_classes()
{
	zend_class_entry ce;
	zend_class_entry *tmp, *date_ce, *timezone_ce, *immutable_ce, *interface_ce;

	date_ce = zend_hash_str_find_ptr(CG(class_table), "datetime", sizeof("datetime")-1);
	if (date_ce == NULL) {
		/* DateTime must be initialized before */
		php_error_docref("https://github.com/hnw/php-timecop", E_WARNING,
						 "timecop couldn't find class %s.", "DateTime");
		return SUCCESS;
	}

	timezone_ce = zend_hash_str_find_ptr(CG(class_table), "datetimezone", sizeof("datetimezone")-1);
	if (timezone_ce == NULL) {
		/* DateTime must be initialized before */
		php_error_docref("https://github.com/hnw/php-timecop", E_WARNING,
						 "timecop couldn't find class %s.", "DateTimeZone");
		return SUCCESS;
	}

	immutable_ce = zend_hash_str_find_ptr(CG(class_table), "datetimeimmutable", sizeof("datetimeimmutable")-1);
	if (immutable_ce == NULL) {
		/* DateTimeImmutable must be initialized before */
		php_error_docref("https://github.com/hnw/php-timecop", E_WARNING,
						 "timecop couldn't find class %s.", "DateTimeImmutable");
		return SUCCESS;
	}

	interface_ce = zend_hash_str_find_ptr(CG(class_table), "datetimeinterface", sizeof("datetimeinterface")-1);
	if (interface_ce == NULL) {
		/* DateTimeInterface must be initialized before */
		php_error_docref("https://github.com/hnw/php-timecop", E_WARNING,
						 "timecop couldn't find interface %s.", "DateTimeInterface");
		return SUCCESS;
	}

	INIT_CLASS_ENTRY(ce, "Timecop", timecop_funcs_timecop);
	zend_register_internal_class(&ce);

	TIMECOP_G(ce_DateTimeZone) = timezone_ce;
	TIMECOP_G(ce_DateTimeInterface) = interface_ce;

	/* replace DateTime */
	INIT_CLASS_ENTRY(ce, "TimecopDateTime", timecop_funcs_date);
	tmp = zend_register_internal_class_ex(&ce, date_ce);
	tmp->create_object = date_ce->create_object;

	TIMECOP_G(ce_DateTime) = date_ce;
	TIMECOP_G(ce_TimecopDateTime) = tmp;

	INIT_CLASS_ENTRY(ce, "TimecopOrigDateTime", timecop_funcs_orig_date);
	tmp = zend_register_internal_class_ex(&ce, date_ce);
	tmp->create_object = date_ce->create_object;

	/* replace DateTimeImmutable */
	INIT_CLASS_ENTRY(ce, "TimecopDateTimeImmutable", timecop_funcs_immutable);
	tmp = zend_register_internal_class_ex(&ce, immutable_ce);
	tmp->create_object = immutable_ce->create_object;

	TIMECOP_G(ce_DateTimeImmutable) = immutable_ce;
	TIMECOP_G(ce_TimecopDateTimeImmutable) = tmp;

	INIT_CLASS_ENTRY(ce, "TimecopOrigDateTimeImmutable", timecop_funcs_orig_immutable);
	tmp = zend_register_internal_class_ex(&ce, immutable_ce);
	tmp->create_object = immutable_ce->create_object;

	return SUCCESS;
}

static int timecop_func_override()
{
	const struct timecop_override_func_entry *p;
	zend_function *zf_orig, *zf_ovrd, *zf_save;

	p = &(timecop_override_func_table[0]);
	while (p->orig_func != NULL) {
		zf_orig = zend_hash_str_find_ptr(CG(function_table), p->orig_func, strlen(p->orig_func));
		if (zf_orig == NULL) {
			/* Do nothing. Because some functions are introduced by optional extensions. */
			p++;
			continue;
		}

		zf_ovrd = zend_hash_str_find_ptr(CG(function_table), p->ovrd_func, strlen(p->ovrd_func));
		if (zf_ovrd == NULL) {
			php_error_docref("https://github.com/hnw/php-timecop", E_WARNING,
							 "timecop couldn't find function %s.", p->ovrd_func);
			p++;
			continue;
		}

		zf_save = zend_hash_str_find_ptr(CG(function_table), p->save_func, strlen(p->save_func));
		if (zf_save != NULL) {
			php_error_docref("https://github.com/hnw/php-timecop", E_WARNING,
							 "timecop couldn't create function %s because already exists.",
							 p->save_func);
			p++;
			continue;
		}

		TIMECOP_ASSERT(zf_orig->type == ZEND_INTERNAL_FUNCTION);
		TIMECOP_ASSERT(zf_ovrd->type == ZEND_INTERNAL_FUNCTION);

		zend_hash_str_add_mem(CG(function_table), p->save_func, strlen(p->save_func),
							  zf_orig, sizeof(zend_internal_function));
		function_add_ref(zf_orig);

		GUARD_FUNCTION_ARG_INFO_BEGIN(zf_orig);
		zend_hash_str_update_mem(CG(function_table), p->orig_func, strlen(p->orig_func),
								 zf_ovrd, sizeof(zend_internal_function));
		GUARD_FUNCTION_ARG_INFO_END();
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
		ce_orig = zend_hash_str_find_ptr(CG(class_table), p->orig_class, strlen(p->orig_class));
		if (ce_orig == NULL) {
			php_error_docref("https://github.com/hnw/php-timecop", E_WARNING,
							 "timecop couldn't find class %s.", p->orig_class);
			p++;
			continue;
		}

		ce_ovrd = zend_hash_str_find_ptr(CG(class_table), p->ovrd_class, strlen(p->ovrd_class));
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

/*  clear function overriding. */
static int timecop_func_override_clear()
{
	const struct timecop_override_func_entry *p;
	zend_function *zf_orig, *zf_ovld;

	p = &(timecop_override_func_table[0]);
	while (p->orig_func != NULL) {
		zf_orig = zend_hash_str_find_ptr(CG(function_table),
										 p->save_func, strlen(p->save_func));
		zf_ovld = zend_hash_str_find_ptr(CG(function_table),
										 p->orig_func, strlen(p->orig_func));
		if (zf_orig == NULL || zf_ovld == NULL) {
			p++;
			continue;
		}

		GUARD_FUNCTION_ARG_INFO_BEGIN(zf_ovld);
		zend_hash_str_update_mem(CG(function_table), p->orig_func, strlen(p->orig_func),
								 zf_orig, sizeof(zend_internal_function));
		GUARD_FUNCTION_ARG_INFO_END();
		function_add_ref(zf_orig);

		GUARD_FUNCTION_ARG_INFO_BEGIN(zf_orig);
		zend_hash_str_del(CG(function_table), p->save_func, strlen(p->save_func));
		GUARD_FUNCTION_ARG_INFO_END();

		p++;
	}
	return SUCCESS;
}

static int timecop_class_override_clear()
{
	const struct timecop_override_class_entry *p;
	zend_class_entry *ce_orig;
	zend_function *zf_orig;

	p = &(timecop_override_class_table[0]);
	while (p->orig_class != NULL) {
		ce_orig = zend_hash_str_find_ptr(CG(class_table),
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

static int update_request_time(zend_long unixtime)
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
	zval *server_vars;

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

static int fill_mktime_params(zval *fill_params, const char *date_function_name, int from)
{
	char *formats[MKTIME_NUM_ARGS] = {"H", "i", "s", "n", "j", "Y"};
	zval format, timestamp;
	int i;

	ZVAL_LONG(&timestamp, mock_timestamp());

	for (i = from; i < MKTIME_NUM_ARGS; i++) {
		ZVAL_STRING(&format, formats[i]);
		call_php_function_with_2_params(date_function_name, &fill_params[i], &format, &timestamp);
		zval_ptr_dtor(&format);
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
static int get_formatted_mock_time(zval *time, zval *timezone_obj, zval *retval_time, zval *retval_timezone)
{
	zval fixed_sec, orig_zonename;
	zval now_timestamp, str_now;
	tc_timeval now;
	zend_long fixed_usec;

	if (TIMECOP_G(timecop_mode) == TIMECOP_MODE_REALTIME) {
		ZVAL_FALSE(retval_time);
		ZVAL_NULL(retval_timezone);
		return -1;
	}

	if (time == NULL || Z_TYPE_P(time) == IS_NULL ||
		Z_TYPE_P(time) == IS_FALSE ||
		(Z_TYPE_P(time) == IS_STRING && Z_STRLEN_P(time) == 0)) {
		ZVAL_STRING(&str_now, "now");
		time = &str_now;
	}

	get_mock_timeval(&now, NULL);

	if (timezone_obj && Z_TYPE_P(timezone_obj) == IS_OBJECT) {
		zval zonename;
		call_php_method_with_0_params(timezone_obj, Z_OBJCE_P(timezone_obj), "getname", &zonename);
		call_php_function_with_0_params("date_default_timezone_get", &orig_zonename);
		call_php_function_with_1_params("date_default_timezone_set", NULL, &zonename);
		zval_ptr_dtor(&zonename);
	}

	ZVAL_LONG(&now_timestamp, now.sec);
	call_php_function_with_2_params(ORIG_FUNC_NAME("strtotime"), &fixed_sec, time, &now_timestamp);

	if (timezone_obj && Z_TYPE_P(timezone_obj) == IS_OBJECT) {
		call_php_function_with_1_params("date_default_timezone_set", NULL, &orig_zonename);
		zval_ptr_dtor(&orig_zonename);
	}

	if (Z_TYPE(fixed_sec) == IS_FALSE) {
		ZVAL_FALSE(retval_time);
		ZVAL_NULL(retval_timezone);
		return -1;
	}

	fixed_usec = get_mock_fraction(time, timezone_obj);
	if (fixed_usec == -1) {
		fixed_usec = now.usec;
	}

	{
		zval dt;
		zval format_str;

		char buf[64];

		call_php_function_with_2_params(ORIG_FUNC_NAME("date_create"), &dt, time, timezone_obj);
		if (Z_TYPE(dt) == IS_FALSE) {
			ZVAL_FALSE(retval_time);
			ZVAL_NULL(retval_timezone);
			return -1;
		}
		sprintf(buf, "Y-m-d H:i:s.%06ld", fixed_usec);
		ZVAL_STRING(&format_str, buf);
		call_php_method_with_1_params(&dt, TIMECOP_G(ce_DateTime), "settimestamp", NULL, &fixed_sec);
		call_php_method_with_0_params(&dt, TIMECOP_G(ce_DateTime), "gettimezone", retval_timezone);
		call_php_method_with_1_params(&dt, TIMECOP_G(ce_DateTime), "format", retval_time, &format_str);
		zval_ptr_dtor(&fixed_sec);
		zval_ptr_dtor(&format_str);
		zval_ptr_dtor(&dt);
	}

	if (time == &str_now) {
		zval_ptr_dtor(&str_now);
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
 *     usleep(1);
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
	zval dt1, dt2, usec1, usec2;
	zend_long fixed_usec;
	zval u_str, sleep_usec;

	call_php_function_with_2_params(ORIG_FUNC_NAME("date_create"), &dt1, time, timezone_obj);
	if (Z_TYPE(dt1) == IS_FALSE) {
		return -1;
	}

	ZVAL_LONG(&sleep_usec, 1);
	call_php_function_with_1_params("usleep", NULL, &sleep_usec);

	call_php_function_with_2_params(ORIG_FUNC_NAME("date_create"), &dt2, time, timezone_obj);
	if (Z_TYPE(dt2) == IS_FALSE) {
		zval_ptr_dtor(&dt1);
		return -1;
	}
	ZVAL_STRING(&u_str, "u");
	call_php_method_with_1_params(&dt1, TIMECOP_G(ce_DateTime), "format", &usec1, &u_str);
	call_php_method_with_1_params(&dt2, TIMECOP_G(ce_DateTime), "format", &usec2, &u_str);
	convert_to_long(&usec1);
	convert_to_long(&usec2);

	if (Z_LVAL(usec1) == Z_LVAL(usec2)) {
		fixed_usec = Z_LVAL(usec1);
	} else {
		fixed_usec = -1;
	}
	zval_ptr_dtor(&dt1);
	zval_ptr_dtor(&dt2);
	zval_ptr_dtor(&u_str);

	return fixed_usec;
}


static void _timecop_call_function(INTERNAL_FUNCTION_PARAMETERS, const char *function_name, int index_to_fill_timestamp)
{
	zval *params;
	uint32_t param_count;

	param_count = MAX(ZEND_NUM_ARGS(), index_to_fill_timestamp+1);
	params = (zval *)safe_emalloc(param_count, sizeof(zval), 0);

	if (zend_get_parameters_array_ex(ZEND_NUM_ARGS(), params) == FAILURE) {
		efree(params);
		return;
	}

	param_count = ZEND_NUM_ARGS();
	if (param_count == index_to_fill_timestamp) {
		ZVAL_LONG(&params[param_count], mock_timestamp());
		param_count++;
	}

	call_php_function_with_params(function_name, return_value, param_count, params);

	efree(params);
}

/* {{{ _timecop_call_mktime - timecop_(gm)mktime helper */
static void _timecop_call_mktime(INTERNAL_FUNCTION_PARAMETERS, const char *mktime_function_name, const char *date_function_name)
{
	zval *params;
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
		php_error_docref(NULL, E_DEPRECATED, "You should be using the time() function instead");
	}

	call_php_function_with_params(mktime_function_name, return_value, param_count, params);

	for (i = ZEND_NUM_ARGS(); i < MKTIME_NUM_ARGS; i++) {
		zval_ptr_dtor(&params[i]);
	}
	efree(params);
}
/* }}} */

/* {{{ proto int timecop_freeze(long timestamp)
   Time travel to specified timestamp and freeze */
PHP_FUNCTION(timecop_freeze)
{
	zval *dt;
	zend_long timestamp;
	tc_timeval freezed_tv;

	if (zend_parse_parameters_ex(ZEND_PARSE_PARAMS_QUIET, ZEND_NUM_ARGS(), "O", &dt, TIMECOP_G(ce_DateTimeInterface)) != FAILURE) {
		get_timeval_from_datetime(&freezed_tv, dt);
	} else if (zend_parse_parameters_ex(ZEND_PARSE_PARAMS_QUIET, ZEND_NUM_ARGS(), "l", &timestamp) != FAILURE) {
		freezed_tv.sec = timestamp;
		freezed_tv.usec = 0;
	} else {
		php_error_docref(NULL, E_WARNING, "This function accepts either (DateTimeInterface) OR (int) as arguments.");
		RETURN_FALSE;
	}

	TIMECOP_G(timecop_mode) = TIMECOP_MODE_FREEZE;
	TIMECOP_G(freezed_time) = freezed_tv;

	if (TIMECOP_G(sync_request_time)){
		update_request_time(freezed_tv.sec);
	}

	RETURN_TRUE;
}
/* }}} */

/* {{{ proto int timecop_travel(long timestamp)
   Time travel to specified timestamp */
PHP_FUNCTION(timecop_travel)
{
	zval *dt;
	zend_long timestamp;
	tc_timeval now, mock_tv;

	if (zend_parse_parameters_ex(ZEND_PARSE_PARAMS_QUIET, ZEND_NUM_ARGS(), "O", &dt, TIMECOP_G(ce_DateTimeInterface)) != FAILURE) {
		get_timeval_from_datetime(&mock_tv, dt);
	} else if (zend_parse_parameters_ex(ZEND_PARSE_PARAMS_QUIET, ZEND_NUM_ARGS(), "l", &timestamp) != FAILURE) {
		mock_tv.sec = timestamp;
		mock_tv.usec = 0;
	} else {
		php_error_docref(NULL, E_WARNING, "This function accepts either (DateTimeInterface) OR (int) as arguments.");
		RETURN_FALSE;
	}

	TIMECOP_G(timecop_mode) = TIMECOP_MODE_TRAVEL;
	get_current_time(&now);
	tc_timeval_sub(&TIMECOP_G(travel_offset), &mock_tv, &now);
	TIMECOP_G(travel_origin) = now;

	if (TIMECOP_G(sync_request_time)){
		update_request_time(mock_tv.sec);
	}

	RETURN_TRUE;
}
/* }}} */

/* {{{ proto int timecop_scale(long scale)
   Accelerate time with specified scale */
PHP_FUNCTION(timecop_scale)
{
	zend_long scale;
	tc_timeval now, mock_time;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "l", &scale) == FAILURE) {
		RETURN_FALSE;
	}
	if (scale < 0) {
		RETURN_FALSE;
	}
	get_current_time(&now);
	get_mock_timeval(&mock_time, &now);
	TIMECOP_G(timecop_mode) = TIMECOP_MODE_TRAVEL;
	TIMECOP_G(travel_origin) = now;
	tc_timeval_sub(&TIMECOP_G(travel_offset), &mock_time, &now);
	TIMECOP_G(scaling_factor) = scale;

	if (TIMECOP_G(sync_request_time)){
		update_request_time(mock_time.sec);
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
		restore_request_time();
	}

	RETURN_TRUE;
}
/* }}} */

/* {{{ proto int timecop_time(void)
   Return virtual timestamp */
PHP_FUNCTION(timecop_time)
{
	RETURN_LONG(mock_timestamp());
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
static int get_mock_timeval(tc_timeval *fixed, const tc_timeval *now)
{
	if (TIMECOP_G(timecop_mode) == TIMECOP_MODE_FREEZE) {
		*fixed = TIMECOP_G(freezed_time);
	} else if (TIMECOP_G(timecop_mode) == TIMECOP_MODE_TRAVEL) {
		tc_timeval delta, origin = TIMECOP_G(travel_origin);
		zend_long scale = TIMECOP_G(scaling_factor);
		if (now == NULL) {
			get_current_time(&delta);
		} else {
			delta = *now;
		}
		tc_timeval_sub(&delta, &delta, &origin);
		tc_timeval_mul(&delta, &delta, scale);
		tc_timeval_add(fixed, &origin, &TIMECOP_G(travel_offset));
		tc_timeval_add(fixed, fixed, &delta);
	} else {
		if (now == NULL) {
			get_current_time(fixed);
		} else {
			*fixed = *now;
		}
	}
	return 0;
}

static zend_long mock_timestamp()
{
	tc_timeval tv;
	get_mock_timeval(&tv, NULL);
	return tv.sec;
}

static int get_timeval_from_datetime(tc_timeval *tp, zval *dt)
{
	zval sec, usec;
	zval u_str;

	call_php_method_with_0_params(dt, Z_OBJCE_P(dt), "gettimestamp", &sec);
	ZVAL_STRING(&u_str, "u");
	call_php_method_with_1_params(dt, Z_OBJCE_P(dt), "format", &usec, &u_str);
	zval_ptr_dtor(&u_str);
	convert_to_long(&usec);

	tp->sec = Z_LVAL(sec);
	tp->usec = Z_LVAL(usec);

	return 0;
}

static int get_current_time(tc_timeval *now)
{
	int ret = 0;
#if HAVE_GETTIMEOFDAY
	struct timeval tv;
	ret = gettimeofday(&tv, NULL);
	if (ret == 0) {
		now->sec  = (zend_long)tv.tv_sec;
		now->usec = (zend_long)tv.tv_usec;
	}
#else
	time_t ts = time(NULL);
	if (ts == -1) {
		ret = -1;
	} else {
		now->sec  = (zend_long)ts;
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

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "|b", &get_as_float) == FAILURE) {
		RETURN_FALSE;
	}

	if (get_mock_timeval(&fixed, NULL)) {
		RETURN_FALSE;
	}

	if (get_as_float) {
		RETURN_DOUBLE((double)(fixed.sec + fixed.usec / MICRO_IN_SEC));
	}
	if (mode) {
		zval zv_offset, zv_dst, format, timestamp;
		zend_long offset = 0, is_dst = 0;

		ZVAL_LONG(&timestamp, fixed.sec);

		/* offset */
		ZVAL_STRING(&format, "Z");
		call_php_function_with_2_params(ORIG_FUNC_NAME("date"), &zv_offset, &format, &timestamp);
		convert_to_long(&zv_offset);
		offset = Z_LVAL(zv_offset);
		zval_ptr_dtor(&zv_offset);
		zval_ptr_dtor(&format);

		/* is_dst */
		ZVAL_STRING(&format, "I");
		call_php_function_with_2_params(ORIG_FUNC_NAME("date"), &zv_dst, &format, &timestamp);
		convert_to_long(&zv_dst);
		is_dst = Z_LVAL(zv_dst);
		zval_ptr_dtor(&zv_dst);
		zval_ptr_dtor(&format);

		array_init(return_value);
		add_assoc_long(return_value, "sec", fixed.sec);
		add_assoc_long(return_value, "usec", fixed.usec);
		add_assoc_long(return_value, "minuteswest", -offset / SEC_IN_MIN);
		add_assoc_long(return_value, "dsttime", is_dst);
	} else {
		char ret[100];
		snprintf(ret, 100, "%.8F %ld", fixed.usec / MICRO_IN_SEC, fixed.sec);
		RETURN_STRING(ret);
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

	call_php_method_with_2_params(getThis(), real_ce, ORIG_FUNC_NAME("__construct"), NULL, arg1, arg2);
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
	zval fixed_time, fixed_timezone, *arg1, *arg2;
	char *orig_time_str = NULL;
	size_t orig_time_len = 0;
	const char *real_func;
	zend_class_entry *real_ce;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|sO!", &orig_time_str, &orig_time_len, &orig_timezone, TIMECOP_G(ce_DateTimeZone)) == FAILURE) {
		RETURN_FALSE;
	}

	if (orig_time_str == NULL) {
		ZVAL_NULL(&orig_time);
	} else {
		ZVAL_STRINGL(&orig_time, orig_time_str, orig_time_len);
	}
	if (immutable) {
		real_func = ORIG_FUNC_NAME("date_create_immutable");
		real_ce = TIMECOP_G(ce_DateTimeImmutable);
	} else {
		real_func = ORIG_FUNC_NAME("date_create");
		real_ce = TIMECOP_G(ce_DateTime);
	}

	if (get_formatted_mock_time(&orig_time, orig_timezone, &fixed_time, &fixed_timezone TSRMLS_CC) == 0) {
		arg1 = &fixed_time;
		arg2 = &fixed_timezone;
	} else {
		arg1 = &orig_time;
		arg2 = orig_timezone;
	}
	if (obj == NULL) {
		call_php_function_with_2_params(real_func, return_value, arg1, arg2);
	} else {
		call_php_method_with_2_params(obj, real_ce, ORIG_FUNC_NAME("__construct"), NULL, arg1, arg2);
	}
	zval_ptr_dtor(&orig_time);
	zval_ptr_dtor(&fixed_time);
	zval_ptr_dtor(&fixed_timezone);
}

/* {{{ proto DateTime timecop_date_create([string time[, DateTimeZone object]])
   Returns new DateTime object initialized with traveled time */
PHP_FUNCTION(timecop_date_create)
{
	_timecop_date_create(INTERNAL_FUNCTION_PARAM_PASSTHRU, 0);
}
/* }}} */

static void _timecop_date_create_from_format(INTERNAL_FUNCTION_PARAMETERS, int immutable)
{
	zval *orig_timezone = NULL;
	zval orig_format, orig_time, fixed_format, fixed_time, new_format, new_time;
	zval dt, now_timestamp, tmp;
	char *orig_format_str, *orig_time_str;
	size_t  orig_format_len, orig_time_len;
	tc_timeval now;
	char buf[64];
	const char *real_func;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "ss|O!", &orig_format_str, &orig_format_len, &orig_time_str, &orig_time_len, &orig_timezone, TIMECOP_G(ce_DateTimeZone)) == FAILURE) {
		RETURN_FALSE;
	}

	ZVAL_STRINGL(&orig_format, orig_format_str, orig_format_len);
	ZVAL_STRINGL(&orig_time, orig_time_str, orig_time_len);

	call_php_function_with_3_params(ORIG_FUNC_NAME("date_create_from_format"), &dt, &orig_format, &orig_time, orig_timezone);
	if (Z_TYPE(dt) == IS_FALSE) {
		RETURN_FALSE;
	}

	if (memchr(orig_format_str, '!', orig_format_len) ||
		memchr(orig_format_str, '|', orig_format_len)) {
		zval_ptr_dtor(&orig_format);
		zval_ptr_dtor(&orig_time);
		RETURN_ZVAL(&dt, 1, 1);
	}

	get_mock_timeval(&now, NULL);

	ZVAL_LONG(&now_timestamp, now.sec);
	call_php_method_with_1_params(&dt, TIMECOP_G(ce_DateTime), "settimestamp", NULL, &now_timestamp);
	sprintf(buf, "Y-m-d H:i:s.%06ld ", now.usec);
	ZVAL_STRING(&tmp, buf);
	call_php_method_with_1_params(&dt, TIMECOP_G(ce_DateTime), "format", &fixed_time, &tmp);
	zval_ptr_dtor(&tmp);

	if (memchr(orig_format_str, 'g', orig_format_len) ||
		memchr(orig_format_str, 'h', orig_format_len) ||
		memchr(orig_format_str, 'G', orig_format_len) ||
		memchr(orig_format_str, 'H', orig_format_len) ||
		memchr(orig_format_str, 'i', orig_format_len) ||
		memchr(orig_format_str, 's', orig_format_len)) {
		ZVAL_STRING(&fixed_format, "Y-m-d ??:??:??.??????");
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
		ZVAL_STRING(&fixed_format, "Y-m-d H:i:s.??????");
	} else {
#if PHP_VERSION_ID >= 70100
		ZVAL_STRING(&fixed_format, "Y-m-d H:i:s.u");
#else
		ZVAL_STRING(&fixed_format, "Y-m-d H:i:s.??????");
#endif
	}

	ZVAL_STRING(&tmp, "%s %s");
	call_php_function_with_3_params("sprintf", &new_format, &tmp, &fixed_format, &orig_format);
	call_php_function_with_3_params("sprintf", &new_time, &tmp, &fixed_time, &orig_time);
	zval_ptr_dtor(&tmp);

	if (immutable) {
		real_func = ORIG_FUNC_NAME("date_create_immutable_from_format");
	} else {
		real_func = ORIG_FUNC_NAME("date_create_from_format");
	}
	call_php_function_with_3_params(real_func, return_value, &new_format, &new_time, orig_timezone);

	zval_ptr_dtor(&dt);
	zval_ptr_dtor(&orig_format);
	zval_ptr_dtor(&orig_time);
	zval_ptr_dtor(&fixed_format);
	zval_ptr_dtor(&fixed_time);
	zval_ptr_dtor(&new_format);
	zval_ptr_dtor(&new_time);
}

/* {{{ proto DateTime timecop_date_create_from_format(string format, string time[, DateTimeZone object])
   Returns new DateTime object initialized with traveled time */
PHP_FUNCTION(timecop_date_create_from_format)
{
	_timecop_date_create_from_format(INTERNAL_FUNCTION_PARAM_PASSTHRU, 0);
}
/* }}} */

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

static inline zval* _call_php_method_with_0_params(zval *object_pp, zend_class_entry *obj_ce, const char *method_name, zval *retval_ptr)
{
	return _call_php_method(object_pp, obj_ce, method_name, retval_ptr, 0, NULL, NULL);
}

static inline zval* _call_php_method_with_1_params(zval *object_pp, zend_class_entry *obj_ce, const char *method_name, zval *retval_ptr, zval *arg1)
{
	int nparams = 1;
	if (arg1 == NULL) {
		nparams = 0;
	}
	return _call_php_method(object_pp, obj_ce, method_name, retval_ptr, nparams, arg1, NULL);
}

static inline zval* _call_php_method_with_2_params(zval *object_pp, zend_class_entry *obj_ce, const char *method_name, zval *retval_ptr, zval *arg1, zval *arg2)
{
	int nparams = 2;
	if (arg1 == NULL) {
		nparams = 0;
	} else if (arg2 == NULL) {
		nparams = 1;
	}
	return _call_php_method(object_pp, obj_ce, method_name, retval_ptr, nparams, arg1, arg2);
}

static inline zval* _call_php_method(zval *object_pp, zend_class_entry *obj_ce, const char *method_name, zval *retval_ptr, int param_count, zval* arg1, zval* arg2)
{
	return zend_call_method(object_pp, obj_ce, NULL, method_name, strlen(method_name), retval_ptr, param_count, arg1, arg2);
}

static inline void _call_php_function_with_0_params(const char *function_name, zval *retval_ptr)
{
	_call_php_method_with_0_params(NULL, NULL, function_name, retval_ptr);
}

static inline void _call_php_function_with_1_params(const char *function_name, zval *retval_ptr, zval *arg1)
{
	_call_php_method_with_1_params(NULL, NULL, function_name, retval_ptr, arg1);
}

static inline void _call_php_function_with_2_params(const char *function_name, zval *retval_ptr, zval *arg1, zval *arg2)
{
	_call_php_method_with_2_params(NULL, NULL, function_name, retval_ptr, arg1, arg2);
}

static inline void _call_php_function_with_3_params(const char *function_name, zval *retval_ptr, zval *arg1, zval *arg2, zval *arg3)
{
	if (arg3 == NULL) {
		_call_php_function_with_2_params(function_name, retval_ptr, arg1, arg2);
	} else {
		zval params[3];
		ZVAL_COPY(&params[0], arg1);
		ZVAL_COPY(&params[1], arg2);
		ZVAL_COPY(&params[2], arg3);
		call_php_function_with_params(function_name, retval_ptr, 3, params);
		zval_ptr_dtor(&params[0]);
		zval_ptr_dtor(&params[1]);
		zval_ptr_dtor(&params[2]);
	}
}

static inline void _call_php_function_with_params(const char *function_name, zval *retval_ptr, uint32_t param_count, zval *params)
{
	zval callable;
	ZVAL_STRING(&callable, function_name);

	call_user_function_ex(EG(function_table), NULL, &callable, retval_ptr, param_count, params, 1, NULL);

	zval_ptr_dtor(&callable);
}

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
