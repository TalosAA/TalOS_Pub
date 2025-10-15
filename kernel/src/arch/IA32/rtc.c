#include <libk/string.h>
#include <IA32/utils.h>
#include <IA32/rtc.h>
#include <IA32/io.h>

/* CMOS Registers:
 *   Register  Contents            Range
 *   0x00      Seconds             0–59
 *   0x02      Minutes             0–59
 *   0x04      Hours               0–23 in 24-hour mode, 
 *                                 1–12 in 12-hour mode, highest bit set if pm
 *   0x06      Weekday             1–7, Sunday = 1
 *   0x07      Day of Month        1–31
 *   0x08      Month               1–12
 *   0x09      Year                0–99
 *   0x32      Century (maybe)     19–20?
 *   0x0A      Status Register A
 *   0x0B      Status Register B
 */

/* RTC PORTS */
#define RTC_CTRL_PORT       0x70
#define RTC_DATA_PORT       0x71

/* RTC REGISTERS */
#define RTC_REG_STATUS_A    0x0A
#define RTC_REG_STATUS_B    0x0B
#define RTC_REG_SECONDS     0x00
#define RTC_REG_MINUTES     0x02
#define RTC_REG_HOURS       0x04
#define RTC_REG_WEEK_DAY    0x06
#define RTC_REG_DAY         0x07
#define RTC_REG_MONTH       0x08
#define RTC_REG_YEAR        0x09
#define RTC_REG_CENTURY     0x32

#define RTC_SET_24H         0x02

/* CMOS Date type */
typedef struct {
  uint8_t sec;
  uint8_t min;
  uint8_t hours;
  uint8_t day;
  uint8_t weekd;
  uint8_t month;
  uint8_t year;
  uint8_t century;
} date_t;

static time_t RTC_DateToTimestamp(const date_t *date);

static uint8_t readCMOS_reg(uint8_t reg){
  uint8_t data = 0;
  outb (reg, RTC_CTRL_PORT); 
  data = inb(RTC_DATA_PORT);
  return data;
}

static void writeCMOS_reg(uint8_t reg, uint8_t value){
  outb (reg, RTC_CTRL_PORT); 
  outb (value, RTC_DATA_PORT); 
}


static void RTC_read_date_time(date_t * returnDate){

  /* Read Seconds */
  returnDate->sec = BCD2DEC(readCMOS_reg(RTC_REG_SECONDS));
  
  /* Read Minutes */
  returnDate->min = BCD2DEC(readCMOS_reg(RTC_REG_MINUTES));
  
  /* Read Hours */
  returnDate->hours = BCD2DEC(readCMOS_reg(RTC_REG_HOURS) & 0x7F);
  
  /* Read Weakday */
  returnDate->weekd = BCD2DEC(readCMOS_reg(RTC_REG_WEEK_DAY));
  
  /* Read Day of Month */
  returnDate->day = BCD2DEC(readCMOS_reg(RTC_REG_DAY));
  
  /* Read Month */
  returnDate->month = BCD2DEC(readCMOS_reg(RTC_REG_MONTH));
  
  /* Read Year */
  returnDate->year = BCD2DEC(readCMOS_reg(RTC_REG_YEAR));

  /* Read Year */
  returnDate->century = BCD2DEC(readCMOS_reg(RTC_REG_CENTURY));

}

void RTC_Init(void){

  register uint8_t reg_data;

  reg_data = readCMOS_reg(RTC_REG_STATUS_B) | RTC_SET_24H;
  writeCMOS_reg(RTC_REG_STATUS_B, reg_data);

}

bool RTC_GetTimestamp(time_t* timestamp) {

  uint8_t up_in_progress = 0;
  date_t first_read;
  date_t second_read;
  uint8_t read_ok = false;
  uint8_t tries = 3;

  *timestamp = 0;

  while(!read_ok && tries > 0){

    memset((void*)&first_read, 0, sizeof(date_t));
    memset((void*)&second_read, 1, sizeof(date_t));
    
    /* wait end of update */
    do{
        up_in_progress = readCMOS_reg(0x0A) & (1 << 7);
    }while(up_in_progress == 0);

    RTC_read_date_time(&first_read);

    /* wait end of update */
    do{
        up_in_progress = readCMOS_reg(0x0A) & (1 << 7);
    }while(up_in_progress == 0);

    RTC_read_date_time(&second_read);

    if(memcmp(&first_read, &second_read, sizeof(date_t)) == 0)
      read_ok = true;
    else
      tries--;
  }

  if(read_ok) {
    *timestamp = RTC_DateToTimestamp(&second_read);
  }
  
  return read_ok;
}

static time_t RTC_DateToTimestamp(const date_t *date)
{
  uint16_t y;
  uint8_t m;
  uint8_t d;
  uint32_t t;

  // Year
  y = date->year + date->century * 100;
  // Month of year
  m = date->month;
  // Day of month
  d = date->day;

  // January and February are counted as months 13 and 14 of the previous year
  if (m <= 2)
  {
    m += 12;
    y -= 1;
  }

  // Convert years to days
  t = (365 * y) + (y / 4) - (y / 100) + (y / 400);
  // Convert months to days
  t += (30 * m) + (3 * (m + 1) / 5) + d;
  // Unix time starts on January 1st, 1970
  t -= 719561;
  // Convert days to seconds
  t *= 86400;
  // Add hours, minutes and seconds
  t += (3600 * date->hours) + (60 * date->min) + date->sec;

  // Return Unix time
  return t;
}