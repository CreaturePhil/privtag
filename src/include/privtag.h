#ifndef _PRIVTAG_H_
#define _PRIVTAG_H_

#define SLEEP_MODE_ENABLE 0
#define SLOWER_CLOCK_ENABLE 0
#define PRINT_DEBUG_ENABLE 1

#define THRESHOLD 500
#define TIME_NOT_MOVE 10

// for timer cc formula
#define CPU_HZ 48000000
#define PRESCALER_DIV 1024
#define ONE_mHZ 1000

#define ABS(a) ((a) < 0 ? (-(a)) : (a))
#define IS_DIFF(cur,prev) ((cur)!=(prev) && ABS((cur)-(prev)) > THRESHOLD)
#define MIN(a,b) ((a) < (b) ? (a) : (b))
#define SHOW_PRIV_TAG() for (int i = 1; i <= 16; i++) ledcircle_select(i);
#define DELAY_SECOND(s) timerCount = (s); while (timerCount != 0)

static uint16_t volatile seconds;
static uint16_t volatile timerCount;

static int16_t prev_x, prev_y, prev_z;
static bool turnOnBeacon;

void privtag_init();

void privtag_app();

bool movement_detected(int16_t x, int16_t y, int16_t z);

void decimalToBinary(uint16_t n, uint16_t *onLEDs);

uint32_t compute_cc_value(uint32_t period_ms);

void display_time_led(uint16_t seconds);

void sleep();

#endif
