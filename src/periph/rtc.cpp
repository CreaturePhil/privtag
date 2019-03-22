#include <sam.h>
#include "../include/rtc.h"
// #include "../include/ledcircle.h"

void rtc_init(uint8_t prescaler, uint32_t per)
{
  rtc_reset();

  RTC->MODE1.CTRL.bit.MODE = 1; // count 16
  RTC->MODE1.CTRL.bit.PRESCALER = prescaler;
  RTC->MODE1.PER.bit.PER = per;
  RTC->MODE1.INTENSET.bit.OVF = 1; // enable overflow interrupt
  RTC->MODE1.CTRL.bit.ENABLE = 1;
  while (RTC->MODE1.STATUS.bit.SYNCBUSY) {
      //ledcircle_select(1);
  }
 // ledcircle_select(0);

  NVIC_EnableIRQ(RTC_IRQn);
  NVIC_SetPriority(RTC_IRQn, 0x00);
}

void rtc_reset()
{
  RTC->MODE1.CTRL.reg &= ~RTC_MODE1_CTRL_ENABLE;
  while (RTC->MODE1.STATUS.bit.SYNCBUSY);
  RTC->MODE1.CTRL.reg |= RTC_MODE1_CTRL_SWRST;// software reset
  while (RTC->MODE1.STATUS.bit.SYNCBUSY);
}
