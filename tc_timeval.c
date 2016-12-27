#include "tc_timeval.h"

int tc_timeval_add(tc_timeval *ret, const tc_timeval *arg1, const tc_timeval *arg2)
{
	tc_timeval_long sec, usec;
	usec = arg1->usec + arg2->usec;
	sec  = arg1->sec + arg2->sec;
	if (usec < 0) {
		sec -= ((-usec) / USEC_PER_SEC + 1);
		usec = USEC_PER_SEC - ((-usec) % USEC_PER_SEC);
		if (usec == USEC_PER_SEC) {
			sec++;
			usec = 0;
		}
	} else if (usec >= USEC_PER_SEC) {
		sec += usec / USEC_PER_SEC;
		usec = usec % USEC_PER_SEC;
	}
	ret->sec  = sec;
	ret->usec = usec;

	return 0;
}
int tc_timeval_sub(tc_timeval *ret, const tc_timeval *arg1, const tc_timeval *arg2)
{
	tc_timeval_long sec, usec;
	usec = arg1->usec - arg2->usec;
	sec  = arg1->sec - arg2->sec;
	if (usec < 0) {
		sec -= ((-usec) / USEC_PER_SEC + 1);
		usec = USEC_PER_SEC - ((-usec) % USEC_PER_SEC);
		if (usec == USEC_PER_SEC) {
			sec++;
			usec = 0;
		}
	} else if (usec >= USEC_PER_SEC) {
		sec += usec / USEC_PER_SEC;
		usec = usec % USEC_PER_SEC;
	}
	ret->sec  = sec;
	ret->usec = usec;

	return 0;
}
int tc_timeval_mul(tc_timeval *ret, const tc_timeval *arg1, const tc_timeval_long arg2)
{
	tc_timeval_long sec, usec;
	usec = arg1->usec * arg2;
	sec  = arg1->sec * arg2;
	if (usec < 0) {
		sec -= ((-usec) / USEC_PER_SEC + 1);
		usec = USEC_PER_SEC - ((-usec) % USEC_PER_SEC);
		if (usec == USEC_PER_SEC) {
			sec++;
			usec = 0;
		}
	} else if (usec >= USEC_PER_SEC) {
		sec += usec / USEC_PER_SEC;
		usec = usec % USEC_PER_SEC;
	}
	ret->sec  = sec;
	ret->usec = usec;

	return 0;
}

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
