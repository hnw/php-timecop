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

#ifndef TC_TIMEVAL_H
#define TC_TIMEVAL_H

#ifndef USEC_PER_SEC
#  define USEC_PER_SEC 1000000
#endif

typedef struct _tc_timeval {
#if PHP_MAJOR_VERSION >= 7
	zend_long sec;
	zend_long usec;
#else
	long sec;
	long usec;
#endif
} tc_timeval;


int tc_timeval_add(tc_timeval *ret, const tc_timeval *arg1, const tc_timeval *arg2);
int tc_timeval_sub(tc_timeval *ret, const tc_timeval *arg1, const tc_timeval *arg2);
#if PHP_MAJOR_VERSION >= 7
int tc_timeval_mul(tc_timeval *ret, const tc_timeval *arg1, const zend_long arg2);
#else
int tc_timeval_mul(tc_timeval *ret, const tc_timeval *arg1, const long arg2);
#endif

#endif	/* TC_TIMEVAL_H */

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
