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

#ifndef PHP_TIMECOP_H
#define PHP_TIMECOP_H

#define PHP_TIMECOP_VERSION "1.2.10"

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"

#ifdef PHP_WIN32
#	define PHP_TIMECOP_API __declspec(dllexport)
#elif defined(__GNUC__) && __GNUC__ >= 4
#	define PHP_TIMECOP_API __attribute__ ((visibility("default")))
#else
#	define PHP_TIMECOP_API
#endif

#ifdef ZTS
#include "TSRM.h"
#endif

#ifndef PHP_WIN32
#include <time.h>
#else
#include "win32/time.h"
#endif

#include "Zend/zend_interfaces.h"
#include "tc_timeval.h"


extern zend_module_entry timecop_module_entry;
#define phpext_timecop_ptr &timecop_module_entry

PHP_MINIT_FUNCTION(timecop);
PHP_MSHUTDOWN_FUNCTION(timecop);
PHP_RINIT_FUNCTION(timecop);
PHP_RSHUTDOWN_FUNCTION(timecop);
PHP_MINFO_FUNCTION(timecop);

PHP_FUNCTION(timecop_freeze);
PHP_FUNCTION(timecop_travel);
PHP_FUNCTION(timecop_scale);
PHP_FUNCTION(timecop_return);
PHP_FUNCTION(timecop_time);
PHP_FUNCTION(timecop_mktime);
PHP_FUNCTION(timecop_gmmktime);
PHP_FUNCTION(timecop_date);
PHP_FUNCTION(timecop_gmdate);
PHP_FUNCTION(timecop_idate);
PHP_FUNCTION(timecop_getdate);
PHP_FUNCTION(timecop_localtime);
PHP_FUNCTION(timecop_strtotime);
PHP_FUNCTION(timecop_strftime);
PHP_FUNCTION(timecop_gmstrftime);
#ifdef HAVE_GETTIMEOFDAY
PHP_FUNCTION(timecop_microtime);
PHP_FUNCTION(timecop_gettimeofday);
#endif
PHP_FUNCTION(timecop_unixtojd);
PHP_FUNCTION(timecop_date_create);
PHP_FUNCTION(timecop_date_create_from_format);
#if PHP_VERSION_ID >= 50500
PHP_FUNCTION(timecop_date_create_immutable);
PHP_FUNCTION(timecop_date_create_immutable_from_format);
#endif
#if !defined(PHP_VERSION_ID) || PHP_VERSION_ID < 50300
PHP_FUNCTION(date_timestamp_set);
PHP_FUNCTION(date_timestamp_get);
#endif

PHP_METHOD(TimecopDateTime, __construct);
PHP_METHOD(TimecopOrigDateTime, __construct);

#if PHP_VERSION_ID >= 50500
PHP_METHOD(TimecopDateTimeImmutable, __construct);
PHP_METHOD(TimecopOrigDateTimeImmutable, __construct);
#endif

PHP_METHOD(Timecop, freeze);
PHP_METHOD(Timecop, travel);

typedef enum timecop_mode_t {
	TIMECOP_MODE_REALTIME,
	TIMECOP_MODE_FREEZE,
	TIMECOP_MODE_TRAVEL
} timecop_mode_t;

ZEND_BEGIN_MODULE_GLOBALS(timecop)
	long func_override;
	long sync_request_time;
#if PHP_MAJOR_VERSION >= 7
	zval orig_request_time;
#else
	zval *orig_request_time;
#endif
	timecop_mode_t timecop_mode;
	tc_timeval freezed_time;
	tc_timeval travel_origin;
	tc_timeval travel_offset;
	zend_long scaling_factor;
	zend_class_entry *ce_DateTimeZone;
	zend_class_entry *ce_DateTimeInterface;
	zend_class_entry *ce_DateTime;
	zend_class_entry *ce_TimecopDateTime;
	zend_class_entry *ce_DateTimeImmutable;
	zend_class_entry *ce_TimecopDateTimeImmutable;
ZEND_END_MODULE_GLOBALS(timecop)

#if ZEND_DEBUG
#  define TIMECOP_ASSERT(c) assert(c)
#else
#  define TIMECOP_ASSERT(c)
#endif /* ZEND_DEBUG */

#define SAVE_FUNC_PREFIX "timecop_orig_"
#define OVRD_FUNC_PREFIX "timecop_"

#define OVRD_CLASS_PREFIX "timecop"

#define ORIG_FUNC_NAME(fname) \
	(TIMECOP_G(func_override) ? (SAVE_FUNC_PREFIX fname) : fname)

#define TIMECOP_OFE(fname) {fname, OVRD_FUNC_PREFIX fname, SAVE_FUNC_PREFIX fname}
#define TIMECOP_OCE(cname, mname) \
	{cname, mname, OVRD_CLASS_PREFIX cname, SAVE_FUNC_PREFIX mname}

/*
 * Trick for guarding the multi-referenced internal function from function destructor on PHP 7.2.0+
 * See: https://github.com/hnw/php-timecop/issues/29#issuecomment-332171527
 */
#define GUARD_FUNCTION_ARG_INFO_BEGIN(zend_func) { \
    zend_arg_info *orig_arg_info; \
    zend_function *zf = zend_func; \
    if (zf->type == ZEND_INTERNAL_FUNCTION) { \
        orig_arg_info = zf->common.arg_info; \
        zf->common.arg_info = NULL; \
    }

#define GUARD_FUNCTION_ARG_INFO_END() \
    if (zf->type == ZEND_INTERNAL_FUNCTION) { \
        zf->common.arg_info = orig_arg_info; \
    } \
}

struct timecop_override_func_entry {
	char *orig_func;
	char *ovrd_func;
	char *save_func;
};

struct timecop_override_class_entry {
	char *orig_class;
	char *orig_method;
	char *ovrd_class;
	char *save_method;
};

static void timecop_globals_ctor(zend_timecop_globals *globals TSRMLS_DC);

static int register_timecop_classes(TSRMLS_D);
static int timecop_func_override(TSRMLS_D);
static int timecop_class_override(TSRMLS_D);
static int timecop_func_override_clear(TSRMLS_D);
static int timecop_class_override_clear(TSRMLS_D);

static int update_request_time(zend_long unixtime TSRMLS_DC);
static int restore_request_time(TSRMLS_D);

#if PHP_MAJOR_VERSION >= 7
static int fill_mktime_params(zval *fill_params, const char *date_function_name, int from);
static int get_formatted_mock_time(zval *time, zval *timezone_obj, zval *retval_time, zval *retval_timezone);
#else
static int fill_mktime_params(zval ***params, const char *date_function_name, int from TSRMLS_DC);
static int get_formatted_mock_time(zval *time, zval *timezone_obj, zval **retval_time, zval **retval_timezone TSRMLS_DC);
#endif

static long get_mock_fraction(zval *time, zval *timezone_obj TSRMLS_DC);

static void _timecop_call_function(INTERNAL_FUNCTION_PARAMETERS, const char *function_name, int index_to_fill_timestamp);
static void _timecop_call_mktime(INTERNAL_FUNCTION_PARAMETERS, const char *mktime_function_name, const char *date_function_name);

static int get_mock_timeval(tc_timeval *fixed, const tc_timeval *now TSRMLS_DC);

static inline zend_long mock_timestamp();

static int get_timeval_from_datetime(tc_timeval *tp, zval *dt TSRMLS_DC);
static int get_current_time(tc_timeval *now TSRMLS_DC);

static void _timecop_orig_datetime_constructor(INTERNAL_FUNCTION_PARAMETERS, int immutable);
static inline void _timecop_date_create(INTERNAL_FUNCTION_PARAMETERS, int immutable);
static inline void _timecop_datetime_constructor(INTERNAL_FUNCTION_PARAMETERS, int immutable);
static void _timecop_datetime_constructor_ex(INTERNAL_FUNCTION_PARAMETERS, zval *obj, int immutable);

#if PHP_MAJOR_VERSION >= 7
static inline zval* _call_php_method(zval *object_pp, zend_class_entry *obj_ce, const char *method_name, zval *retval_ptr, zval* arg1, zval* arg2);
static inline void _call_php_function(const char *function_name, zval *retval_ptr, zval* arg1, zval* arg2);
static void _call_php_function_with_3_params(const char *function_name, zval *retval_ptr, zval *arg1, zval *arg2, zval *arg3);
static inline void _call_php_function_with_params(const char *function_name, zval *retval_ptr, uint32_t param_count, zval params[]);
#else
static inline zval* _call_php_function(const char *method_name, zval **retval_ptr_ptr, zval* arg1, zval* arg2 TSRMLS_DC);
static inline zval* _call_php_method(zval **object_pp, zend_class_entry *obj_ce, const char *method_name, zval **retval_ptr_ptr, zval* arg1, zval* arg2 TSRMLS_DC);
static void _call_php_function_with_3_params(const char *function_name, zval **retval_ptr_ptr, zval *arg1, zval *arg2, zval *arg3 TSRMLS_DC);
static inline void _call_php_function_with_params(const char *function_name, zval **retval_ptr_ptr, zend_uint param_count, zval **params[] TSRMLS_DC);
#endif

#if PHP_MAJOR_VERSION >= 7
#define register_internal_class_ex(class_entry, parent_ce) \
	zend_register_internal_class_ex(class_entry, parent_ce)
#else
#define register_internal_class_ex(class_entry, parent_ce) \
	zend_register_internal_class_ex(class_entry, parent_ce, NULL TSRMLS_CC)
#endif

#define call_php_method_with_0_params(obj, ce, method_name, retval) \
	_call_php_method(obj, ce, method_name, retval, NULL, NULL TSRMLS_CC)

#define call_php_method_with_1_params(obj, ce, method_name, retval, arg1)	\
	_call_php_method(obj, ce, method_name, retval, arg1, NULL TSRMLS_CC)

#define call_php_method_with_2_params(obj, ce, method_name, retval, arg1, arg2) \
	_call_php_method(obj, ce, method_name, retval, arg1, arg2 TSRMLS_CC)

#define call_php_function_with_0_params(function_name, retval) \
	_call_php_function(function_name, retval, NULL, NULL TSRMLS_CC)

#define call_php_function_with_1_params(function_name, retval, arg1) \
	_call_php_function(function_name, retval, arg1, NULL TSRMLS_CC)

#define call_php_function_with_2_params(function_name, retval, arg1, arg2) \
	_call_php_function(function_name, retval, arg1, arg2 TSRMLS_CC)

#define call_php_function_with_3_params(function_name, retval, arg1, arg2, arg3) \
	_call_php_function_with_3_params(function_name, retval, arg1, arg2, arg3 TSRMLS_CC)

#define call_php_function_with_params(function_name, retval, param_count, params) \
	_call_php_function_with_params(function_name, retval, param_count, params TSRMLS_CC)

/* In every utility function you add that needs to use variables 
   in php_timecop_globals, call TSRMLS_FETCH(); after declaring other 
   variables used by that function, or better yet, pass in TSRMLS_CC
   after the last function argument and declare your utility function
   with TSRMLS_DC after the last declared argument.  Always refer to
   the globals in your function as TIMECOP_G(variable).  You are 
   encouraged to rename these macros something shorter, see
   examples in any other php module directory.
*/

#if PHP_MAJOR_VERSION >= 7
#  define TIMECOP_G(v) ZEND_MODULE_GLOBALS_ACCESSOR(timecop, v)
#  if defined(ZTS) && defined(COMPILE_DL_TIMECOP)
     ZEND_TSRMLS_CACHE_EXTERN();
#  endif
#else
#  ifdef ZTS
#    define TIMECOP_G(v) TSRMG(timecop_globals_id, zend_timecop_globals *, v)
#  else
#    define TIMECOP_G(v) (timecop_globals.v)
#  endif
#endif

#endif	/* PHP_TIMECOP_H */

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
