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

#include "timecop.h"

ZEND_DECLARE_MODULE_GLOBALS(timecop)

static void timecop_globals_ctor(zend_timecop_globals *globals TSRMLS_DC) {
	/* Initialize your global struct */
	globals->func_override = 1;
	globals->sync_request_time = 1;
#if PHP_MAJOR_VERSION >= 7
	ZVAL_NULL(&globals->orig_request_time);
#else
    globals->orig_request_time = NULL;
#endif
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
#if PHP_VERSION_ID >= 70200
    ZEND_ARG_OBJ_INFO(0, object, DateTimeZone, 1)
#else
	ZEND_ARG_INFO(0, object)
#endif
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

#if defined(COMPILE_DL_TIMECOP)
#  if defined(ZTS) && PHP_MAJOR_VERSION >= 7
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
#if defined(COMPILE_DL_TIMECOP) && defined(ZTS) && PHP_MAJOR_VERSION >= 7
	ZEND_TSRMLS_CACHE_UPDATE();
#endif

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

#if PHP_MAJOR_VERSION >= 7
	if (Z_TYPE(TIMECOP_G(orig_request_time)) == IS_NULL) {
#else
	if (TIMECOP_G(orig_request_time)) {
#endif
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
	zend_class_entry ce, *tmp, *date_ce, *timezone_ce, *immutable_ce = NULL, *interface_ce = NULL;

#if PHP_MAJOR_VERSION >= 7
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
#else
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
	tmp = register_internal_class_ex(&ce, date_ce);
	tmp->create_object = date_ce->create_object;

	TIMECOP_G(ce_DateTime) = date_ce;
	TIMECOP_G(ce_TimecopDateTime) = tmp;

	INIT_CLASS_ENTRY(ce, "TimecopOrigDateTime", timecop_funcs_orig_date);
	tmp = register_internal_class_ex(&ce, date_ce);
	tmp->create_object = date_ce->create_object;

#if PHP_VERSION_ID >= 50500
	/* replace DateTimeImmutable */
	INIT_CLASS_ENTRY(ce, "TimecopDateTimeImmutable", timecop_funcs_immutable);
	tmp = register_internal_class_ex(&ce, immutable_ce);
	tmp->create_object = immutable_ce->create_object;

	TIMECOP_G(ce_DateTimeImmutable) = immutable_ce;
	TIMECOP_G(ce_TimecopDateTimeImmutable) = tmp;

	INIT_CLASS_ENTRY(ce, "TimecopOrigDateTimeImmutable", timecop_funcs_orig_immutable);
	tmp = register_internal_class_ex(&ce, immutable_ce);
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
#if PHP_MAJOR_VERSION >= 7
		zf_orig = zend_hash_str_find_ptr(EG(function_table), p->orig_func, strlen(p->orig_func));
		if (zf_orig == NULL) {
			/* Do nothing. Because some functions are introduced by optional extensions. */
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
#else
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
#endif

		TIMECOP_ASSERT(zf_orig->type == ZEND_INTERNAL_FUNCTION);
		TIMECOP_ASSERT(zf_ovrd->type == ZEND_INTERNAL_FUNCTION);

#if PHP_MAJOR_VERSION >= 7
		zend_hash_str_add_mem(EG(function_table), p->save_func, strlen(p->save_func),
							  zf_orig, sizeof(zend_internal_function));
#else
		zend_hash_add(EG(function_table), p->save_func, strlen(p->save_func)+1,
					  zf_orig, sizeof(zend_function), NULL);
#endif
		function_add_ref(zf_orig);

#if PHP_MAJOR_VERSION >= 7
		zend_hash_str_update_mem(EG(function_table), p->orig_func, strlen(p->orig_func),
								 zf_ovrd, sizeof(zend_internal_function));
#else
		zend_hash_update(EG(function_table), p->orig_func, strlen(p->orig_func)+1,
						 zf_ovrd, sizeof(zend_function), NULL);
#endif
		function_add_ref(zf_ovrd);

		p++;
	}

	return SUCCESS;
}

static int timecop_class_override(TSRMLS_D)
{
	const struct timecop_override_class_entry *p;
	zend_class_entry *ce_orig, *ce_ovrd;
	zend_function *zf_orig, *zf_ovrd, *zf_save, *zf_new;

#if PHP_MAJOR_VERSION < 7
	zend_class_entry **pce_ovrd, **pce_orig;
#endif

	p = &(timecop_override_class_table[0]);
	while (p->orig_class != NULL) {
#if PHP_MAJOR_VERSION >= 7
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
#else
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
#endif

		TIMECOP_ASSERT(zf_orig->type == ZEND_INTERNAL_FUNCTION);
		TIMECOP_ASSERT(ce_orig->type & ZEND_INTERNAL_CLASS);

#if PHP_MAJOR_VERSION >= 7
		zend_hash_str_add_mem(&ce_orig->function_table,
							  p->save_method, strlen(p->save_method),
							  zf_orig, sizeof(zend_internal_function));
#else
		zend_hash_add(&ce_orig->function_table, p->save_method, strlen(p->save_method)+1,
					  zf_orig, sizeof(zend_function), NULL);
#endif
		function_add_ref(zf_orig);

		TIMECOP_ASSERT(zf_ovrd->type == ZEND_INTERNAL_FUNCTION);
		TIMECOP_ASSERT(ce_ovrd->type & ZEND_INTERNAL_CLASS);

#if PHP_MAJOR_VERSION >= 7
		zf_new = zend_hash_str_update_mem(&ce_orig->function_table,
										  p->orig_method, strlen(p->orig_method),
										  zf_ovrd, sizeof(zend_internal_function));
		function_add_ref(zf_ovrd);
		TIMECOP_ASSERT(zf_new != NULL);
		TIMECOP_ASSERT(zf_new != zf_orig);
#else
		zend_hash_update(&ce_orig->function_table, p->orig_method, strlen(p->orig_method)+1,
						 zf_ovrd, sizeof(zend_function), (void**)&zf_new);
		function_add_ref(zf_new);
#endif
		// BUG HERE?

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
#if PHP_MAJOR_VERSION >= 7
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
#else
		if (zend_hash_find(EG(function_table), p->save_func, strlen(p->save_func)+1,
						   (void **)&zf_orig) != SUCCESS) {
			p++;
			continue;
		}

		zend_hash_update(EG(function_table), p->orig_func, strlen(p->orig_func)+1,
						 zf_orig, sizeof(zend_function), NULL);
		function_add_ref(zf_orig);

		zend_hash_del(EG(function_table), p->save_func, strlen(p->save_func)+1);
#endif
		p++;
	}
	return SUCCESS;
}


static int timecop_class_override_clear(TSRMLS_D)
{
	const struct timecop_override_class_entry *p;
	zend_class_entry *ce_orig;
	zend_function *zf_orig;
#if PHP_MAJOR_VERSION < 7
	zend_class_entry **pce_orig;
#endif

	p = &(timecop_override_class_table[0]);
	while (p->orig_class != NULL) {
#if PHP_MAJOR_VERSION >= 7
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
#else
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
#endif
		function_add_ref(zf_orig);

#if PHP_MAJOR_VERSION >= 7
		zend_hash_str_del(&ce_orig->function_table, p->save_method, strlen(p->save_method));
#else
		zend_hash_del(&ce_orig->function_table, p->save_method, strlen(p->save_method)+1);
#endif

		if (strcmp(p->orig_method, "__construct") == 0) {
			ce_orig->constructor = zf_orig;
		}
		p++;
	}
	return SUCCESS;
}

#if PHP_MAJOR_VERSION >= 7
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
#else
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
#endif

#if PHP_MAJOR_VERSION >= 7
static int get_formatted_mock_time(zval *time, zval *timezone_obj, zval *retval_time, zval *retval_timezone)
#else
static int get_formatted_mock_time(zval *time, zval *timezone_obj, zval **retval_time, zval **retval_timezone TSRMLS_DC)
#endif
{
	zval str_now, now_timestamp, format_str;
	tc_timeval now;
	zend_long fixed_usec;
	char buf[64];

#if PHP_MAJOR_VERSION >= 7
	zval fixed_sec, orig_zonename, dt;
#else
	zval *fixed_sec, *orig_sec, *orig_zonename, *dt;
#endif

	if (TIMECOP_G(timecop_mode) == TIMECOP_MODE_REALTIME) {
#if PHP_MAJOR_VERSION >= 7
		ZVAL_FALSE(retval_time);
		ZVAL_NULL(retval_timezone);
#else
		MAKE_STD_ZVAL(*retval_time);
		ZVAL_FALSE(*retval_time);
		MAKE_STD_ZVAL(*retval_timezone);
		ZVAL_NULL(*retval_timezone);
#endif
		return -1;
	}

#if PHP_MAJOR_VERSION >= 7
	if (time == NULL || Z_TYPE_P(time) == IS_NULL ||
		Z_TYPE_P(time) == IS_FALSE ||
		(Z_TYPE_P(time) == IS_STRING && Z_STRLEN_P(time) == 0)) {
		ZVAL_STRING(&str_now, "now");
		time = &str_now;
	}
#else
	if (time == NULL || Z_TYPE_P(time) == IS_NULL ||
		(Z_TYPE_P(time) == IS_BOOL && !Z_BVAL_P(time)) ||
		(Z_TYPE_P(time) == IS_STRING && Z_STRLEN_P(time) == 0)) {
		INIT_ZVAL(str_now);
		ZVAL_STRING(&str_now, "now", 0);
		time = &str_now;
	}
#endif

	get_mock_timeval(&now, NULL TSRMLS_CC);

	if (timezone_obj && Z_TYPE_P(timezone_obj) == IS_OBJECT) {
#if PHP_MAJOR_VERSION >= 7
		zval zonename;
		call_php_method_with_0_params(timezone_obj, Z_OBJCE_P(timezone_obj), "getname", &zonename);
		call_php_function_with_0_params("date_default_timezone_get", &orig_zonename);
		call_php_function_with_1_params("date_default_timezone_set", NULL, &zonename);
		zval_ptr_dtor(&zonename);
#else
		zval *zonename;
		call_php_method_with_0_params(&timezone_obj, Z_OBJCE_PP(&timezone_obj), "getname", &zonename);
		if (zonename) {
			call_php_function_with_0_params("date_default_timezone_get", &orig_zonename);
			if (orig_zonename) {
				call_php_function_with_1_params("date_default_timezone_set", NULL, zonename);
			}
			zval_ptr_dtor(&zonename);
		}
#endif
	}

#if PHP_MAJOR_VERSION < 7
	INIT_ZVAL(now_timestamp);
#endif
	ZVAL_LONG(&now_timestamp, now.sec);
	call_php_function_with_2_params(ORIG_FUNC_NAME("strtotime"), &fixed_sec, time, &now_timestamp);

	if (timezone_obj && Z_TYPE_P(timezone_obj) == IS_OBJECT) {
#if PHP_MAJOR_VERSION >= 7
		call_php_function_with_1_params("date_default_timezone_set", NULL, &orig_zonename);
#else
		call_php_function_with_1_params("date_default_timezone_set", NULL, orig_zonename);
#endif
		zval_ptr_dtor(&orig_zonename);
	}

#if PHP_MAJOR_VERSION >= 7
	if (Z_TYPE(fixed_sec) == IS_FALSE) {
		ZVAL_FALSE(retval_time);
		ZVAL_NULL(retval_timezone);
		return -1;
	}
#else
	if (Z_TYPE_P(fixed_sec) == IS_BOOL && !Z_BVAL_P(fixed_sec)) {
		/* $fixed_sec === false */
		MAKE_STD_ZVAL(*retval_time);
		ZVAL_FALSE(*retval_time);
		MAKE_STD_ZVAL(*retval_timezone);
		ZVAL_NULL(*retval_timezone);
		return -1;
	}
#endif

	fixed_usec = get_mock_fraction(time, timezone_obj TSRMLS_CC);

	if (fixed_usec == -1) {
		fixed_usec = now.usec;
	}

	call_php_function_with_2_params(ORIG_FUNC_NAME("date_create"), &dt, time, timezone_obj);

#if PHP_MAJOR_VERSION >= 7
	if (Z_TYPE(dt) == IS_FALSE) {
		ZVAL_FALSE(retval_time);
		ZVAL_NULL(retval_timezone);
		return -1;
	}
#else
	if (Z_TYPE_P(dt) == IS_BOOL && !Z_BVAL_P(dt)) {
		MAKE_STD_ZVAL(*retval_time);
		ZVAL_FALSE(*retval_time);
		MAKE_STD_ZVAL(*retval_timezone);
		ZVAL_NULL(*retval_timezone);
		return -1;
	}
#endif

	sprintf(buf, "Y-m-d H:i:s.%06ld", fixed_usec);
#if PHP_MAJOR_VERSION >= 7
	ZVAL_STRING(&format_str, buf);
	call_php_method_with_1_params(&dt, TIMECOP_G(ce_DateTime), "settimestamp", NULL, &fixed_sec);
#else
	INIT_ZVAL(format_str);
	ZVAL_STRING(&format_str, buf, 0);
	call_php_function_with_1_params("date_timestamp_get", &orig_sec, dt);
	if (Z_TYPE_P(orig_sec) == IS_LONG &&
		Z_TYPE_P(fixed_sec) == IS_LONG &&
		Z_LVAL_P(orig_sec) != Z_LVAL_P(fixed_sec)) {
		call_php_function_with_2_params("date_timestamp_set", NULL, dt, fixed_sec);
	}
#endif
	call_php_method_with_0_params(&dt, TIMECOP_G(ce_DateTime), "gettimezone", retval_timezone);
	call_php_method_with_1_params(&dt, TIMECOP_G(ce_DateTime), "format", retval_time, &format_str);
	zval_ptr_dtor(&dt);
#if PHP_MAJOR_VERSION >= 7
	zval_ptr_dtor(&fixed_sec);
	zval_ptr_dtor(&format_str);

	if (time == &str_now) {
		zval_ptr_dtor(&str_now);
	}
#else
	if (fixed_sec) {
		zval_ptr_dtor(&fixed_sec);
	}
#endif

	return 0;
}

static long get_mock_fraction(zval *time, zval *timezone_obj TSRMLS_DC)
{
#if PHP_MAJOR_VERSION >= 7
	zval dt1, dt2, usec1, usec2, sleep_usec;
#else
	zval *dt1, *dt2, *usec1, *usec2;
#endif
	zval u_str;
	zend_long fixed_usec;

	call_php_function_with_2_params(ORIG_FUNC_NAME("date_create"), &dt1, time, timezone_obj);
#if PHP_MAJOR_VERSION >= 7
	if (Z_TYPE(dt1) == IS_FALSE) {
		return -1;
	}

	ZVAL_LONG(&sleep_usec, 1);
	call_php_function_with_1_params("usleep", NULL, &sleep_usec);
#else
	if (Z_TYPE_P(dt1) == IS_BOOL && !Z_BVAL_P(dt1)) {
		return -1;
	}
#endif

	call_php_function_with_2_params(ORIG_FUNC_NAME("date_create"), &dt2, time, timezone_obj);
#if PHP_MAJOR_VERSION >= 7
	if (Z_TYPE(dt2) == IS_FALSE) {
#else
	if (Z_TYPE_P(dt2) == IS_BOOL && !Z_BVAL_P(dt2)) {
#endif
		zval_ptr_dtor(&dt1);
		return -1;
	}
#if PHP_MAJOR_VERSION >= 7
	ZVAL_STRING(&u_str, "u");
#else
	INIT_ZVAL(u_str);
	ZVAL_STRING(&u_str, "u", 0);
#endif
	call_php_method_with_1_params(&dt1, TIMECOP_G(ce_DateTime), "format", &usec1, &u_str);
	call_php_method_with_1_params(&dt2, TIMECOP_G(ce_DateTime), "format", &usec2, &u_str);
#if PHP_MAJOR_VERSION >= 7
	convert_to_long(&usec1);
	convert_to_long(&usec2);
#else
	convert_to_long(usec1);
	convert_to_long(usec2);
#endif

#if PHP_MAJOR_VERSION >= 7
	if (Z_LVAL(usec1) == Z_LVAL(usec2)) {
		fixed_usec = Z_LVAL(usec1);
	} else {
		fixed_usec = -1;
	}
	zval_ptr_dtor(&u_str);
#else
	if (Z_LVAL_P(usec1) == Z_LVAL_P(usec2)) {
		fixed_usec = Z_LVAL_P(usec1);
	} else {
		fixed_usec = -1;
	}
#endif
	zval_ptr_dtor(&dt1);
	zval_ptr_dtor(&dt2);

#if PHP_MAJOR_VERSION < 7
	zval_ptr_dtor(&usec1);
	zval_ptr_dtor(&usec2);
#endif

	return fixed_usec;
}

#if PHP_MAJOR_VERSION >= 7
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

#else

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

#endif

/* {{{ proto int timecop_freeze(long timestamp)
   Time travel to specified timestamp and freeze */
PHP_FUNCTION(timecop_freeze)
{
	zval *dt;
	tc_timeval freezed_tv;
	zend_long timestamp;

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
	zend_long timestamp;

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
	zend_long scale;
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
		zend_long scale = TIMECOP_G(scaling_factor);

		if (NULL == now) {
			get_current_time(&delta TSRMLS_CC);
		} else {
			delta = *now;
		}
		tc_timeval_sub(&delta, &delta, &origin);
		tc_timeval_mul(&delta, &delta, scale);
		tc_timeval_add(fixed, &origin, &TIMECOP_G(travel_offset));
		tc_timeval_add(fixed, fixed, &delta);
	} else {
		if (NULL == now) {
			get_current_time(fixed TSRMLS_CC);
		} else {
			*fixed = *now;
		}
	}
	return 0;
}

static zend_long mock_timestamp(TSRMLS_D)
{
	tc_timeval tv;
	get_mock_timeval(&tv, NULL TSRMLS_CC);
	return tv.sec;
}

static int get_timeval_from_datetime(tc_timeval *tp, zval *dt TSRMLS_DC)
{
#if PHP_MAJOR_VERSION >= 7
	zval sec, usec;
#else
	zval *sec, *usec;
#endif
	zval u_str;

#if PHP_MAJOR_VERSION >= 7
	call_php_method_with_0_params(dt, Z_OBJCE_P(dt), "gettimestamp", &sec);
	ZVAL_STRING(&u_str, "u");
	call_php_method_with_1_params(dt, Z_OBJCE_P(dt), "format", &usec, &u_str);
	zval_ptr_dtor(&u_str);
	convert_to_long(&usec);

	tp->sec = Z_LVAL(sec);
	tp->usec = Z_LVAL(usec);
#else
	call_php_function_with_1_params("date_timestamp_get", &sec, dt);
	INIT_ZVAL(u_str);
	ZVAL_STRING(&u_str, "u", 0);
	call_php_method_with_1_params(&dt, Z_OBJCE_P(dt), "format", &usec, &u_str);
	convert_to_long(usec);

	tp->sec = Z_LVAL_P(sec);
	tp->usec = Z_LVAL_P(usec);

	zval_ptr_dtor(&sec);
	zval_ptr_dtor(&usec);
#endif

	return 0;
}

static int get_current_time(tc_timeval *now TSRMLS_DC)
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

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|b", &get_as_float) == FAILURE) {
		RETURN_FALSE;
	}

	if (get_mock_timeval(&fixed, NULL TSRMLS_CC)) {
		RETURN_FALSE;
	}

	if (get_as_float) {
		RETURN_DOUBLE((double)(fixed.sec + fixed.usec / MICRO_IN_SEC));
	}
	if (mode) {
#if PHP_MAJOR_VERSION >= 7
		zval zv_offset, zv_dst;
#else
		zval *zv_offset, *zv_dst;
#endif
		zval format, timestamp;
		zend_long offset = 0, is_dst = 0;

#if PHP_MAJOR_VERSION < 7
		INIT_ZVAL(timestamp);
#endif
		ZVAL_LONG(&timestamp, fixed.sec);

		/* offset */
#if PHP_MAJOR_VERSION >= 7
		ZVAL_STRING(&format, "Z");
#else
		INIT_ZVAL(format);
		ZVAL_STRING(&format, "Z", 0);
#endif
		call_php_function_with_2_params(ORIG_FUNC_NAME("date"), &zv_offset, &format, &timestamp);
#if PHP_MAJOR_VERSION >= 7
		convert_to_long(&zv_offset);
		offset = Z_LVAL(zv_offset);
		zval_ptr_dtor(&format);
#else
		convert_to_long(zv_offset);
		offset = Z_LVAL_P(zv_offset);
#endif
		zval_ptr_dtor(&zv_offset);

		/* is_dst */
#if PHP_MAJOR_VERSION >= 7
		ZVAL_STRING(&format, "I");
#else
		ZVAL_STRING(&format, "I", 0);
#endif
		call_php_function_with_2_params(ORIG_FUNC_NAME("date"), &zv_dst, &format, &timestamp);
#if PHP_MAJOR_VERSION >= 7
		convert_to_long(&zv_dst);
		is_dst = Z_LVAL(zv_dst);
		zval_ptr_dtor(&format);
#else
		convert_to_long(zv_dst);
		is_dst = Z_LVAL_P(zv_dst);
#endif
		zval_ptr_dtor(&zv_dst);

		array_init(return_value);
		add_assoc_long(return_value, "sec", fixed.sec);
		add_assoc_long(return_value, "usec", fixed.usec);
		add_assoc_long(return_value, "minuteswest", -offset / SEC_IN_MIN);
		add_assoc_long(return_value, "dsttime", is_dst);
	} else {
		char ret[100];
		snprintf(ret, 100, "%.8F %ld", fixed.usec / MICRO_IN_SEC, fixed.sec);
#if PHP_MAJOR_VERSION >= 7
		RETURN_STRING(ret);
#else
		RETURN_STRING(ret, 1);
#endif
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

#if PHP_MAJOR_VERSION >= 7
	call_php_method_with_2_params(getThis(), real_ce, ORIG_FUNC_NAME("__construct"), NULL, arg1, arg2);
#else
	call_php_method_with_2_params(&getThis(), real_ce, ORIG_FUNC_NAME("__construct"), NULL, arg1, arg2);
#endif
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
	char *orig_time_str = NULL;
	const char *real_func;
	zend_class_entry *real_ce;

#if PHP_MAJOR_VERSION >= 7
	zval fixed_time, fixed_timezone, *arg1, *arg2;
	size_t orig_time_len = 0;
#else
	zval *fixed_time, *fixed_timezone, *dt, *arg1, *arg2;
	int orig_time_len = 0;
#endif

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|sO!", &orig_time_str, &orig_time_len, &orig_timezone, TIMECOP_G(ce_DateTimeZone)) == FAILURE) {
		RETURN_FALSE;
	}

#if PHP_MAJOR_VERSION < 7
	INIT_ZVAL(orig_time);
#endif

	if (orig_time_str == NULL) {
		ZVAL_NULL(&orig_time);
	} else {
#if PHP_MAJOR_VERSION >= 7
		ZVAL_STRINGL(&orig_time, orig_time_str, orig_time_len);
#else
		ZVAL_STRINGL(&orig_time, orig_time_str, orig_time_len, 0);
#endif
	}
	if (immutable) {
		real_func = ORIG_FUNC_NAME("date_create_immutable");
		real_ce = TIMECOP_G(ce_DateTimeImmutable);
	} else {
		real_func = ORIG_FUNC_NAME("date_create");
		real_ce = TIMECOP_G(ce_DateTime);
	}

	if (get_formatted_mock_time(&orig_time, orig_timezone, &fixed_time, &fixed_timezone TSRMLS_CC) == 0) {
#if PHP_MAJOR_VERSION >= 7
		arg1 = &fixed_time;
		arg2 = &fixed_timezone;
#else
		arg1 = fixed_time;
		arg2 = fixed_timezone;
#endif
	} else {
		arg1 = &orig_time;
		arg2 = orig_timezone;
	}
#if PHP_MAJOR_VERSION >= 7
	if (obj == NULL) {
		call_php_function_with_2_params(real_func, return_value, arg1, arg2);
	} else {
		call_php_method_with_2_params(obj, real_ce, ORIG_FUNC_NAME("__construct"), NULL, arg1, arg2);
	}

	zval_ptr_dtor(&orig_time);
#else
	if (obj == NULL) {
		call_php_function_with_2_params(real_func, &dt, arg1, arg2);
	} else {
		call_php_method_with_2_params(&obj, real_ce, ORIG_FUNC_NAME("__construct"), NULL, arg1, arg2);
	}
#endif
	
	zval_ptr_dtor(&fixed_time);
	zval_ptr_dtor(&fixed_timezone);

#if PHP_MAJOR_VERSION < 7
	if (obj == NULL) {
		RETURN_ZVAL(dt, 1, 1);
	}
#endif
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
	zval orig_format, orig_time, fixed_format, now_timestamp, tmp;
	char *orig_format_str, *orig_time_str;
	tc_timeval now;
	char buf[64];
	const char *real_func;

#if PHP_MAJOR_VERSION >= 7
	zval fixed_time, new_format, new_time, dt;
	size_t orig_format_len, orig_time_len;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ss|O!", &orig_format_str, &orig_format_len, &orig_time_str, &orig_time_len, &orig_timezone, TIMECOP_G(ce_DateTimeZone)) == FAILURE) {
		RETURN_FALSE;
	}

	ZVAL_STRINGL(&orig_format, orig_format_str, orig_format_len);
	ZVAL_STRINGL(&orig_time, orig_time_str, orig_time_len);
#else
	zval *fixed_time, *new_format, *new_time, *dt, *new_dt;
	int orig_format_len, orig_time_len;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ss|O!", &orig_format_str, &orig_format_len, &orig_time_str, &orig_time_len, &orig_timezone, TIMECOP_G(ce_DateTimeZone)) == FAILURE) {
		RETURN_FALSE;
	}

	INIT_ZVAL(orig_format);
	ZVAL_STRINGL(&orig_format, orig_format_str, orig_format_len, 0);
	INIT_ZVAL(orig_time);
	ZVAL_STRINGL(&orig_time, orig_time_str, orig_time_len, 0);
#endif
	if (immutable) {
		real_func = ORIG_FUNC_NAME("date_create_immutable_from_format");
	} else {
		real_func = ORIG_FUNC_NAME("date_create_from_format");
	}

	call_php_function_with_3_params(real_func, &dt, &orig_format, &orig_time, orig_timezone);

#if PHP_MAJOR_VERSION >= 7
	if (Z_TYPE(dt) == IS_FALSE) {
		RETURN_FALSE;
	}
#else
	if (Z_TYPE_P(dt) == IS_BOOL && !Z_BVAL_P(dt)) {
		RETURN_FALSE;
	}
#endif

	if (memchr(orig_format_str, '!', orig_format_len) ||
		memchr(orig_format_str, '|', orig_format_len)) {
#if PHP_MAJOR_VERSION >= 7
		zval_ptr_dtor(&orig_format);
		zval_ptr_dtor(&orig_time);
		RETURN_ZVAL(&dt, 1, 1);
#else
		RETURN_ZVAL(dt, 1, 1);
#endif
	}

	get_mock_timeval(&now, NULL TSRMLS_CC);

#if PHP_MAJOR_VERSION < 7
	INIT_ZVAL(now_timestamp);
#endif
	ZVAL_LONG(&now_timestamp, now.sec);

	if (immutable) {
		call_php_method_with_1_params(&dt, TIMECOP_G(ce_DateTimeImmutable), "settimestamp", &dt, &now_timestamp);
	} else {
		call_php_method_with_1_params(&dt, TIMECOP_G(ce_DateTime), "settimestamp", &dt, &now_timestamp);
	}

	sprintf(buf, "Y-m-d H:i:s.%06ld ", now.usec);

#if PHP_MAJOR_VERSION >= 7
	ZVAL_STRING(&tmp, buf);
#else
	INIT_ZVAL(tmp);
	ZVAL_STRINGL(&tmp, buf, strlen(buf), 0);
#endif

	if (immutable) {
		call_php_method_with_1_params(&dt, TIMECOP_G(ce_DateTimeImmutable), "format", &fixed_time, &tmp);
	} else {
		call_php_method_with_1_params(&dt, TIMECOP_G(ce_DateTime), "format", &fixed_time, &tmp);
	}

#if PHP_MAJOR_VERSION >= 7
	zval_ptr_dtor(&tmp);
#else
	INIT_ZVAL(fixed_format);
#endif

	if (memchr(orig_format_str, 'g', orig_format_len) ||
		memchr(orig_format_str, 'h', orig_format_len) ||
		memchr(orig_format_str, 'G', orig_format_len) ||
		memchr(orig_format_str, 'H', orig_format_len) ||
		memchr(orig_format_str, 'i', orig_format_len) ||
		memchr(orig_format_str, 's', orig_format_len)) {
#if PHP_MAJOR_VERSION >= 7
		ZVAL_STRING(&fixed_format, "Y-m-d ??:??:??.??????");
#else
		ZVAL_STRING(&fixed_format, "Y-m-d ??:??:??.??????", 0);
#endif
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
#if PHP_MAJOR_VERSION >= 7
		ZVAL_STRING(&fixed_format, "Y-m-d H:i:s.??????");
#else
		ZVAL_STRING(&fixed_format, "Y-m-d H:i:s.??????", 0);
#endif
	} else {
#if PHP_VERSION_ID >= 70100
		ZVAL_STRING(&fixed_format, "Y-m-d H:i:s.u");
#elif PHP_MAJOR_VERSION >= 7
		ZVAL_STRING(&fixed_format, "Y-m-d H:i:s.??????");
#else
		ZVAL_STRING(&fixed_format, "Y-m-d H:i:s.??????", 0);
#endif
	}

#if PHP_MAJOR_VERSION >= 7
	ZVAL_STRING(&tmp, "%s %s");
#else
	ZVAL_STRING(&tmp, "%s %s", 0);
#endif
	call_php_function_with_3_params("sprintf", &new_format, &tmp, &fixed_format, &orig_format);
#if PHP_MAJOR_VERSION >= 7
	call_php_function_with_3_params("sprintf", &new_time, &tmp, &fixed_time, &orig_time);
	zval_ptr_dtor(&tmp);
#else
	call_php_function_with_3_params("sprintf", &new_time, &tmp, fixed_time, &orig_time);
#endif
#if PHP_MAJOR_VERSION >= 7
	call_php_function_with_3_params(real_func, return_value, &new_format, &new_time, orig_timezone);
#else
	call_php_function_with_3_params(real_func, &new_dt, new_format, new_time, orig_timezone);
#endif

	zval_ptr_dtor(&dt);
	zval_ptr_dtor(&fixed_time);
	zval_ptr_dtor(&new_format);
	zval_ptr_dtor(&new_time);

#if PHP_MAJOR_VERSION >= 7
	zval_ptr_dtor(&orig_format);
	zval_ptr_dtor(&orig_time);
	zval_ptr_dtor(&fixed_format);
#else
	RETURN_ZVAL(new_dt, 1, 1);
#endif
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

#if PHP_MAJOR_VERSION >= 7
static inline zval* _call_php_method(zval *object_pp, zend_class_entry *obj_ce, const char *method_name, zval *retval_ptr, zval* arg1, zval* arg2)
{
	int nparams = 2;
	if (arg2 == NULL) {
		--nparams;
	}

	if (arg1 == NULL) {
		--nparams;
	}

	return zend_call_method(object_pp, obj_ce, NULL, method_name, strlen(method_name), retval_ptr, nparams, arg1, arg2);
}

static inline void _call_php_function(const char *function_name, zval *retval_ptr, zval* arg1, zval* arg2)
{
	call_php_method_with_2_params(NULL, NULL, function_name, retval_ptr, arg1, arg2);
}

static inline void _call_php_function_with_3_params(const char *function_name, zval *retval_ptr, zval *arg1, zval *arg2, zval *arg3)
{
	if (arg3 == NULL) {
		call_php_function_with_2_params(function_name, retval_ptr, arg1, arg2);
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
#else
static inline zval* _call_php_method(zval **object_pp, zend_class_entry *obj_ce, const char *method_name, zval **retval_ptr_ptr, zval* arg1, zval* arg2 TSRMLS_DC)
{
	int nparams = 2;
	if (arg1 == NULL) {
		nparams = 0;
	} else if (arg2 == NULL) {
		nparams = 1;
	}

	return zend_call_method(object_pp, obj_ce, NULL, method_name, strlen(method_name), retval_ptr_ptr, nparams, arg1, arg2 TSRMLS_CC);
}

static inline zval* _call_php_function(const char *function_name, zval **retval_ptr_ptr, zval* arg1, zval* arg2 TSRMLS_DC)
{
	_call_php_method(NULL, NULL, function_name, retval_ptr_ptr, arg1, arg2 TSRMLS_CC);
}

static void _call_php_function_with_3_params(const char *function_name, zval **retval_ptr_ptr, zval *arg1, zval *arg2, zval *arg3 TSRMLS_DC)
{
	if (arg3 == NULL) {
		call_php_function_with_2_params(function_name, retval_ptr_ptr, arg1, arg2);
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
#endif

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
