#include "time_v1_interface.h"

/* 
 * Time stuff - results are time_v1.time, ie. nanoseconds.
 */

#define SECONDS(_s)         (((time_v1::time)(_s))  * 1000000000UL)
#define TENTHS(_ts)         (((time_v1::time)(_ts)) * 100000000UL)
#define HUNDREDTHS(_hs)     (((time_v1::time)(_hs)) * 10000000UL)
#define MILLISECS(_ms)      (((time_v1::time)(_ms)) * 1000000UL)
#define MICROSECS(_us)      (((time_v1::time)(_us)) * 1000UL)

#define NOW()               (PVS(time)->now())

#define TIME_MAX            ((time_v1::time) 0x7fffffffffffffffLL)
#define FOREVER             TIME_MAX

/* IN_FUTURE is a predicate to determine whether a time value is in
   the future. It will improve performance for situations where a
   timeout value is quite likely to be either 0 (don't block) or
   FOREVER (block with no timeout), since in both these situations it
   avoids invoking NOW(). It will marginally worsen performance when
   the value being checked is guaranteed to be a real time value,
   since in that case it performs a couple of useless 64-bit tests, so
   don't use it in such situations ... */

#define IN_FUTURE(_until)                           \
  ((_until) && (((_until) == FOREVER) || ((_until) > NOW())))
