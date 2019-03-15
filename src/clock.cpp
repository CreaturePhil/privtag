#include <sam.h>
#include "include/clock.h"

void clock_setup(uint8_t gen, uint8_t id) {
  GCLK->CLKCTRL.bit.GEN = gen;
  GCLK->CLKCTRL.bit.ID = id;
  GCLK->CLKCTRL.bit.CLKEN = 1;
  while (GCLK->STATUS.bit.SYNCBUSY);
}

/* Configure the 32768Hz Oscillator */
void config32kOSC()
{
  SYSCTRL->XOSC32K.reg = SYSCTRL_XOSC32K_ONDEMAND |
                         SYSCTRL_XOSC32K_RUNSTDBY |
                         SYSCTRL_XOSC32K_EN32K |
                         SYSCTRL_XOSC32K_XTALEN |
                         SYSCTRL_XOSC32K_STARTUP(6) |
                         SYSCTRL_XOSC32K_ENABLE;
}
