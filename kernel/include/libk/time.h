#ifndef _TIME_H_
#define _TIME_H_

#include <libk/stdint.h>
#include <libk/stddef.h>

#define ASCTIME_BUFF_LEN            (26)

typedef int64_t         clock_t;          /* Hz */
typedef int64_t         time_t;           /* s  */
typedef uint64_t        sys_time_t;       /* ms */ 

struct tm {
    int tm_sec;    /* Seconds (0-60) */
    int tm_min;    /* Minutes (0-59) */
    int tm_hour;   /* Hours (0-23) */
    int tm_mday;   /* Day of the month (1-31) */
    int tm_mon;    /* Month (0-11) */
    int tm_year;   /* Year - 1900 */
    int tm_wday;   /* Day of the week (0-6, Sunday = 0) */
    int tm_yday;   /* Day in the year (0-365, 1 Jan = 0) */
    int tm_isdst;  /* Daylight saving time */
};

char *asctime_r(const struct tm *restrict tm, char *restrict buf);
struct tm * ts_to_date(const time_t timestamp, struct tm * timeptr);

#endif