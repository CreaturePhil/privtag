#ifndef _TIMER_H_
#define _TIMER_H_

void timer_init(Tc *timer, IRQn_Type irq, uint32_t prescaler, uint32_t ccValue);
void timer_reset(Tc *timer);

#endif
