#ifndef _LEDCIRCLE_H_
#define _LEDCIRCLE_H_

#define IO5 PORT_PA15
#define IO6 PORT_PA20
#define IO7 PORT_PA21
#define IO8 PORT_PA06
#define IO9 PORT_PA07

// For brightness
#define PIN_IO5 31
#define PIN_IO6 29
#define PIN_IO7 27
#define PIN_IO8 25
#define PIN_IO9 23

void ledcircle_select(uint8_t led);

/*
 * Set these port as output
 * First argument is drive the port high
 * Second argument is drive the port low
 */
#define HIGH_LOW(a,b)                  \
	PORT->Group[0].OUTCLR.reg = a | b; \
    PORT->Group[0].DIRCLR.reg = a | b; \
    PORT->Group[0].DIRSET.reg = a | b; \
    PORT->Group[0].OUTSET.reg = a;     \
    PORT->Group[0].OUTCLR.reg = b;     \

#endif
