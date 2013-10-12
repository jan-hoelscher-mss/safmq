#ifndef _TIMEZONE_HACK_
#define _TIMEZONE_HACK_

#ifdef USE_FREEBSD_TIMEZONE

	#include <time.h>
	// declare the function static so it can exist in multiple modules, not great but it's small
	static long FreeBSDTimezone() {
		time_t	now;
		now = time(NULL);
		return localtime(&now)->tm_gmtoff;
	}

	// Replace the _timezone with the function	
	#ifdef _timezone
	#undef _timezone
	#endif
	#define _timezone FreeBSDTimezone()

#endif

#endif

