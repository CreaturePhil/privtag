#include <sam.h>
#include "include/clock.h"

void clock_setup(uint8_t gen, uint8_t id) {
  GCLK->CLKCTRL.bit.GEN = gen;
  GCLK->CLKCTRL.bit.ID = id;
  GCLK->CLKCTRL.bit.CLKEN = 1;
  while (GCLK->STATUS.bit.SYNCBUSY);
}

void clock_XOSC32K_enable()
{
  SYSCTRL->XOSC32K.reg = SYSCTRL_XOSC32K_ONDEMAND |
                         SYSCTRL_XOSC32K_RUNSTDBY |
                         SYSCTRL_XOSC32K_EN32K |
                         SYSCTRL_XOSC32K_XTALEN |
                         SYSCTRL_XOSC32K_STARTUP(6) |
                         SYSCTRL_XOSC32K_ENABLE;

  GCLK->GENDIV.reg = GCLK_GENDIV_ID(2);
  while (GCLK->STATUS.reg & GCLK_STATUS_SYNCBUSY);

  GCLK->GENCTRL.reg = GCLK_GENCTRL_GENEN |
                      GCLK_GENCTRL_SRC_XOSC32K |
                      GCLK_GENCTRL_ID(2) |
                      GCLK_GENCTRL_DIVSEL;
  while (GCLK->STATUS.reg & GCLK_STATUS_SYNCBUSY);

  while (GCLK->STATUS.bit.SYNCBUSY);
}

void clock_GCLK_reset()
{
  GCLK->CTRL.reg = GCLK_CTRL_SWRST;

  while ((GCLK->CTRL.reg & GCLK_CTRL_SWRST) && (GCLK->STATUS.reg & GCLK_STATUS_SYNCBUSY));
}

void clock_GCLK_OSC8M_enable()
{
  SYSCTRL->OSC8M.bit.PRESC = SYSCTRL_OSC8M_PRESC_1_Val;
  //SYSCTRL->OSC8M.bit.ONDEMAND = 0;
  //SYSCTRL->OSC8M.bit.RUNSTDBY = 0;

  GCLK->GENDIV.reg = GCLK_GENDIV_ID(0) | GCLK_GENDIV_DIV(3);
  while ( GCLK->STATUS.reg & GCLK_STATUS_SYNCBUSY );

  GCLK->GENCTRL.reg = GCLK_GENCTRL_ID(0) |
                      GCLK_GENCTRL_SRC_OSC8M |
                      GCLK_GENCTRL_DIVSEL|
                      GCLK_GENCTRL_GENEN;

  while ( GCLK->STATUS.reg & GCLK_STATUS_SYNCBUSY );
}

void clock_DFLLCTRL_disable()
{
    SYSCTRL->DFLLCTRL.bit.ENABLE = 0;
   while ((SYSCTRL->PCLKSR.reg & SYSCTRL_PCLKSR_DFLLRDY) == 0);
}
