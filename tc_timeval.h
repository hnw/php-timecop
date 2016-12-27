#ifndef TC_TIMEVAL_H
#define TC_TIMEVAL_H

#ifndef USEC_PER_SEC
#  define USEC_PER_SEC 1000000
#endif

#if PHP_MAJOR_VERSION >= 7
typedef zend_long tc_timeval_long;
#else
typedef long tc_timeval_long;
#endif

typedef struct _tc_timeval {
	tc_timeval_long sec;
	tc_timeval_long usec;
} tc_timeval;

int tc_timeval_add(tc_timeval *ret, const tc_timeval *arg1, const tc_timeval *arg2);
int tc_timeval_sub(tc_timeval *ret, const tc_timeval *arg1, const tc_timeval *arg2);
int tc_timeval_mul(tc_timeval *ret, const tc_timeval *arg1, const long arg2);

#endif	/* TC_TIMEVAL_H */

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
