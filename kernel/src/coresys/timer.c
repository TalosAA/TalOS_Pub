#include <libk/kermsg.h>
#include <hal/platform.h>
#include <coresys/kernel.h>
#include <coresys/timer.h>
#include <coresys/spinlock.h>

extern os_data_t OSData;

static void SysTickFun(uint64_t counter);

void InitKernelTimers(void)
{
  uint16_t i;
  i = 0;

  SetSysTickFun(SysTickFun);

  kermsg_info("SysTick period ");
  printf("%d ms\n", SYS_TICK_INCREMENT_MS);

  /*
   * Time Slice Reload Value = OS_TIME_SLICE
   */
  OSData.TimeSliceTimer.CountDown = OS_TIME_SLICE;

  for (i = 0; i < OS_TIMERS_N; i++)
  {
    OSData.TimerBlocks[i].CountDown = 0;
  }

  /* Initialize System Timer */
  OSData.SysTime_ms = 0;
}

static void SysTickFun(uint64_t counter)
{
  register uint16_t TimeBlockCount;

  OSData.SysTime_ms = counter;

  if ((OSData.SysTime_ms % 1000) == 0)
  {
    OSData.timestamp++;
  }

  if(SYS_TICK_INCREMENT_MS >= OSData.TimeSliceTimer.CountDown) {
    /* Load Reload Value */
    OSData.TimeSliceTimer.CountDown = OS_TIME_SLICE;
    // TODO: context switch
  } else {
    OSData.TimeSliceTimer.CountDown -= SYS_TICK_INCREMENT_MS;
  }

  for (TimeBlockCount = 0; TimeBlockCount < OS_TIMERS_N; TimeBlockCount++)
  {
    /* If the timer is started and not expired */
    if (OSData.TimerBlocks[TimeBlockCount].CountDown > 0 &&
        OSData.TimerBlocks[TimeBlockCount].Started == TRUE)
    {
      if(SYS_TICK_INCREMENT_MS >= OSData.TimerBlocks[TimeBlockCount].CountDown) {
        OSData.TimerBlocks[TimeBlockCount].CountDown = 0;
        // TODO: DEADLINE HANDLING
      } else {
        OSData.TimerBlocks[TimeBlockCount].CountDown -= SYS_TICK_INCREMENT_MS;
      }
    }
  }
}

time_t GetTimestamp(void)
{
  time_t timestamp = 0;
  ReadTimeCounter ((uint64_t*)&OSData.timestamp, (uint64_t*)&timestamp);
  return timestamp;
}

bool SetTimer(uint16_t index, sys_time_t countDown)
{
  /* If it is not startd or it is started but expired */
  if(OSData.TimerBlocks[index].Started == FALSE ||
    OSData.TimerBlocks[index].CountDown == 0) {
    /* Here I'm sure that the SysTick is not modifying the countDown */
    OSData.TimerBlocks[index].CountDown = countDown;
    OSData.TimerBlocks[index].Started = FALSE;
    return TRUE;
  }
  return FALSE;
}

bool StartTimer(uint16_t index)
{
  /* StartTimer can be executed only after a SetTimer */
  if(OSData.TimerBlocks[index].Started == FALSE &&
    OSData.TimerBlocks[index].CountDown > 0) {
    OSData.TimerBlocks[index].Started = TRUE;
    return TRUE;
  }
  return FALSE;
}

bool SetTimerStart(uint16_t index, sys_time_t countDown)
{
  if(SetTimer(index, countDown)) {
    return StartTimer (index);
  }
  return FALSE;
}

bool TimerExpired(uint16_t index)
{
  SpinDelay();
  if (OSData.TimerBlocks[index].CountDown == 0) {
    return TRUE;
  }
  return FALSE;
}

uint32_t GetTimerCount(uint16_t index) {
  SpinDelay();
  return OSData.TimerBlocks[index].CountDown;
}