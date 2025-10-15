#include <hal/systime.h>
#include <hal/platform.h>
#include <hal/interrupts.h>
#include <IA32/rtc.h>
#include <IA32/pit.h>
#include <IA32/init_defs.h>
#include <IA32/memdefs.h>
#include <IA32/sysinit.h>
#include <IA32/apic.h>
#include <IA32/utils.h>
#include <IA32/sysinit.h>

extern void* isr_table[INT_NUM];
extern SysConf_Info_t SysConf_Info;

static SysTickFun_t* SysTickFun  = NULL;
static sys_time_t SysTickCounter = 0;
static uint32_t SysTickIncrement = 1;
static uint32_t TickAdjustReload = 0;
static uint32_t TickAdjustCurrent = 0;

static bool HWTimerEnabled = false;
static uint32_t APIC_Ticks_us = 0;

#define SYS_TICK_CORRECTION(SYS_TICK_FREQ_HZ)\
    (PIT_CLOCK%SYS_TICK_FREQ_HZ == 0) ? 0 :\
    (((PIT_CLOCK/SYS_TICK_FREQ_HZ) * SYS_TICK_FREQ_HZ)/\
    (PIT_CLOCK%SYS_TICK_FREQ_HZ))

static void SysTick(void);
static uint32_t get_apic_timer_clock(void);

static void SysTick(void) {
  TickAdjustCurrent--;
  if(TickAdjustCurrent == 0) {
    /* do not increment syscounter to compensate the extra ms */
    TickAdjustCurrent = TickAdjustReload;
  } else {
    SysTickCounter += SysTickIncrement;
  }
  if(SysTickFun != NULL) {
    SysTickFun(SysTickCounter);
  }
}

time_t* GetSysTimestamp(time_t* timestamp) {
  if(RTC_GetTimestamp(timestamp)) {
    //TODO: gestione errore
  }
  return timestamp;
}

void SetSysTimerFrequency(uint32_t SysTickFreqHZ) {
  TickAdjustReload = SYS_TICK_CORRECTION(SysTickFreqHZ);
  TickAdjustCurrent = TickAdjustReload;

  SysTickIncrement = (1000/SysTickFreqHZ);

  /* Inititalize PIT timer Counter */
  PIT_set_mode(PIT_INT_TERM_COUNT, PIT_DATA_CH0);
  PIT_set_count(PIT_CLOCK/SysTickFreqHZ, PIT_DATA_CH0);
}

void InitSysTime(void) {
  SysTickFun = NULL;
  isr_table[ISA_IRQ0_PIT + HW_INT_START] = (void*)SysTick;
  EnableIRQ(0, ISA_IRQ0_PIT + HW_INT_START);
}

void InitMicroTimer(void) {
  if((SysConf_Info.LAPIC_tim_freqHz = get_apic_timer_clock()) != 0){
    HWTimerEnabled = true;
    APIC_Ticks_us = SysConf_Info.LAPIC_tim_freqHz / 1000000;
  }
}

/* Set sys tick callback */
void SetSysTickFun(SysTickFun_t* SysTickFunIn) {
  DisableInterrupts();
  SysTickFun = SysTickFunIn;
  EnableInterrupts();
}

sys_time_t* GetSysTime_ms(sys_time_t* SysTime_ms)
{
  ReadTimeCounter ((uint64_t*)&SysTickCounter, (uint64_t*)SysTime_ms);
  _pause();
  return SysTime_ms;
}

void Wait_ms(sys_time_t time_ms)
{
  sys_time_t timeStart = 0;
  sys_time_t timeNow = 0;
  GetSysTime_ms(&timeStart);
  while ((*(GetSysTime_ms(&timeNow)) - timeStart) < time_ms);
}

void Wait_us(sys_time_t time_us) {
  if(HWTimerEnabled) {
    LAPIC_TIM_SetClockDivisor(LAPIC_TIM_DIV_1);
    LAPIC_TIM_SetMode(LAPIC_TIM_MODE_ONESHOT);
    LAPIC_TIM_SetInitCount(APIC_Ticks_us * time_us);
    while(LAPIC_TIM_GetCurrentCount() > 0) {
      _pause();
    }
    LAPIC_TIM_StopTimer();
  } else {
    sys_time_t us_to_ms = ((time_us % 1000) == 0) ? 1 : time_us/1000;
    Wait_ms(us_to_ms);
  }
}

void ReadTimeCounter (uint64_t* t_input, uint64_t* t_out) {
  volatile uint64_t firstRead = 0;
  volatile uint64_t secondRead = 0;

  *t_out = 0;

  /* 
    It's ok to use the following approach because
    the increments of the high part are usually
    very far apart.
  */
  do {
    firstRead = *((volatile uint64_t*)t_input);
    secondRead = *((volatile uint64_t*)t_input);
    _pause();
  } while (((firstRead >> 32) != (secondRead >> 32)));

  *t_out = secondRead;
}

static uint32_t get_apic_timer_clock(void) {
  #define TICKS_PER_MS      (32)
  uint32_t TimerClock = 0;
  uint64_t CurrentTime_ms = 0;
  if(SysConf_Info.ACPI_Init && SysConf_Info.LAPIC_Ptr != NULL) {
    LAPIC_TIM_SetClockDivisor(LAPIC_TIM_DIV_1);
    LAPIC_TIM_SetMode(LAPIC_TIM_MODE_ONESHOT);
    LAPIC_TIM_SetInitCount(UINT32_MAX);
    CurrentTime_ms = SysTickCounter;
    while((SysTickCounter - CurrentTime_ms) < TICKS_PER_MS) {
      _pause();
    }
    TimerClock = ((UINT32_MAX - LAPIC_TIM_GetCurrentCount()) * 1000)\
                 / TICKS_PER_MS;
  }
  return TimerClock;
}