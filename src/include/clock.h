#ifndef _CLOCK_H_
#define _CLOCK_H_

void clock_setup(uint8_t gen, uint8_t id);
void clock_XOSC32K_enable();
void clock_OSC8M_enable();
void clock_GCLK_OSC8M_enable();
void clock_DFLLCTRL_disable();
void clock_GCLK_reset();

#endif
