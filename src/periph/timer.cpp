#include <sam.h>
#include "../include/timer.h"

/*
 * \brief setup timer using 16 bit count and match frequency
 * require clock is setup for timer
 */
void timer_init(Tc *timer, IRQn_Type irq, uint32_t prescaler, uint32_t ccValue) {
  timer_reset(timer);

  timer->COUNT16.CTRLA.reg |= TC_CTRLA_MODE_COUNT16;
  timer->COUNT16.CTRLA.reg |= TC_CTRLA_WAVEGEN_MFRQ;
  timer->COUNT16.CTRLA.reg |= prescaler;
  while (timer->COUNT16.STATUS.bit.SYNCBUSY);

  timer->COUNT16.CC[0].reg = ccValue;
  while (timer->COUNT16.STATUS.bit.SYNCBUSY);

  // Enable the timer interrupt request
  timer->COUNT16.INTENSET.bit.MC0 = 1;
  while (timer->COUNT16.STATUS.bit.SYNCBUSY);

  NVIC_EnableIRQ(irq);
  NVIC_SetPriority(irq, 0x00);

  timer->COUNT16.CTRLA.reg |= TC_CTRLA_ENABLE;
  while (timer->COUNT16.STATUS.bit.SYNCBUSY);
}

void timer_reset(Tc *timer) {
  // Disable timer
  timer->COUNT16.CTRLA.reg &= ~TC_CTRLA_ENABLE;
  while (timer->COUNT16.STATUS.bit.SYNCBUSY);

  // Software reset timer
  timer->COUNT16.CTRLA.reg = TC_CTRLA_SWRST;
  while (timer->COUNT16.STATUS.bit.SYNCBUSY);
  while (timer->COUNT16.CTRLA.bit.SWRST);
}
