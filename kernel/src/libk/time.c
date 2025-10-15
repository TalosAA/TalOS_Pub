#include <libk/time.h>
#include <libk/stdio.h>

static const char wday_name[][4] = {
  "Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun"
};
static const char mon_name[][4] = {
  "Jan", "Feb", "Mar", "Apr", "May", "Jun",
  "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
};

char *asctime_r(const struct tm *restrict timeptr, char *restrict buf)
{
  sprintf(buf, "%.3s %.3s%3d %.2d:%.2d:%.2d %d\n",
    wday_name[timeptr->tm_wday - 1],
    mon_name[timeptr->tm_mon - 1],
    timeptr->tm_mday, timeptr->tm_hour,
    timeptr->tm_min, timeptr->tm_sec,
    1900 + timeptr->tm_year);
  return buf;
}

static uint8_t days_per_month[2][12] = { \
  {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31}, \
  {31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31}  \
};

static uint16_t GetDayOfYear(uint16_t y, uint8_t m, uint8_t d)
{
  bool is_leap = 0;
  uint16_t ydays = 0;
  uint8_t i = 0;

  if(y % 400 == 0){
    is_leap = 1;
  } else {
    if((y % 4 == 0) && !(y % 100 == 0)){
      is_leap = 1;
    }
  }

  for(i = 0; i < (m - 1); i++){
    ydays += days_per_month[is_leap][i];
  }

  ydays += d;

  return ydays;
}

static uint8_t GetDayOfWeek(uint16_t y, uint8_t m, uint8_t d)
{
  uint32_t h;
  uint32_t j;
  uint32_t k;

  // January and February are counted as months 13 and 14 of the previous year
  if (m <= 2)
  {
    m += 12;
    y -= 1;
  }

  // J is the century
  j = y / 100;
  // K the year of the century
  k = y % 100;

  // Compute H using Zeller's congruence
  h = d + (26 * (m + 1) / 10) + k + (k / 4) + (5 * j) + (j / 4);

  // Return the day of the week
  return ((h + 5) % 7) + 1;
}

struct tm * ts_to_date(const time_t timestamp, struct tm * timeptr){
  uint32_t a;
  uint32_t b;
  uint32_t c;
  uint32_t d;
  uint32_t e;
  uint32_t f;
  time_t times = timestamp;

  // Negative Unix time values are not supported
  if (timestamp < 1)
    return NULL;

  // Retrieve hours, minutes and seconds
  timeptr->tm_sec = times % 60;
  times /= 60;
  timeptr->tm_min = times % 60;
  times /= 60;
  timeptr->tm_hour = times % 24;
  times /= 24;

  // Convert Unix time to date
  a = (uint32_t)((4 * times + 102032) / 146097 + 15);
  b = (uint32_t)(times + 2442113 + a - (a / 4));
  c = (20 * b - 2442) / 7305;
  d = b - 365 * c - (c / 4);
  e = d * 1000 / 30601;
  f = d - e * 30 - e * 601 / 1000;

  // January and February are counted as months 13 and 14 of the previous year
  if (e <= 13)
  {
    c -= 4716;
    e -= 1;
  }
  else
  {
    c -= 4715;
    e -= 13;
  }

  // Retrieve year, month and day
  timeptr->tm_year = c - 1900;
  timeptr->tm_mon = e;
  timeptr->tm_mday = f;

  // Calculate day of week
  timeptr->tm_wday = GetDayOfWeek(c, e, f);
  timeptr->tm_yday = GetDayOfYear(c, e, f);

  return timeptr;
}