#ifndef DEFINE_H
#define DEFINE_H
#define ENABLE_GRAY 0

#define ENABLE_THREADS 0

#define restrict 
#define ENABLE_SMALL 0
#ifndef INT64_C
#define INT64_C __int64
#endif

#ifdef WIN32
#define snprintf sprintf
#include "stdint_win32.h"
#else
#include <stdint.h>
#include <stdlib.h>
#ifndef INT_MAX
#define INT_MAX INT32_MAX
#endif
#ifndef INT_MIN
#define INT_MIN INT32_MIN
#endif
#endif


enum
{
	 ENOMEM = 0,
	EINVAL   = 1,
	START_ENCODE  = 2,
    IN_ENCODE     = 3,
	END_ENCODE    = 4
};

#define ENABLE_H264_DECODER 1
#endif
