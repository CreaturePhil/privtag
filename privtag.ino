#include <sam.h>
#include "src/include/clock.h"
#include "src/include/timer.h"
#include "src/include/ledcircle.h"
#include "src/include/i2c.h"
#include "src/include/bma250.h"
#include "src/include/rtc.h"
#include "src/include/print.h"
#include "src/include/lib_aci.h"
#include "src/include/ble.h"
#include "src/include/privtag.h"

#define BLE_DEBUG true

// FIXME(phil): remove this debug code
volatile int t;

void privtag_init()
{
  #if SLOWER_CLOCK_ENABLE
  clock_GCLK_reset();

  clock_XOSC32K_enable();

  clock_GCLK_OSC8M_enable();

  clock_DFLLCTRL_disable();
  #endif

  ble_setup();

  clock_setup(GCLK_CLKCTRL_GEN_GCLK0_Val, GCLK_CLKCTRL_ID_TCC2_TC3_Val);
  /*PM->APBCMASK.reg |= PM_APBCMASK_TC3;*/
  timer_init(TC3, TC3_IRQn, TC_CTRLA_PRESCALER_DIV1024, compute_cc_value(1000));

  /*PM->APBCMASK.reg |= PM_APBCMASK_SERCOM3;*/
  clock_setup(GCLK_CLKCTRL_GEN_GCLK0_Val, GCLK_CLKCTRL_ID_SERCOM3_CORE_Val);
  i2c_init();
  bma250_init();

  #if SLEEP_MODE_ENABLE
  PM->APBAMASK.reg |= PM_APBAMASK_RTC;
  clock_setup(GCLK_CLKCTRL_GEN_GCLK2_Val, GCLK_CLKCTRL_ID_RTC_Val);
  rtc_init(0xA, 8);
  #endif

  ble_setup();
}

void privtag_app()
{
    ble_loop();

#if 0
  if (t) {
  printf("WAKE!\n");
    lib_aci_wakeup();
    t = !t;
  } else {
  printf("SLEEP!\n");
    lib_aci_sleep();
    t = !t;
  }
  #endif

#if 1
  int16_t x, y, z;
  bma250_read_xyz(&x, &y, &z);

  bool hasMove = movement_detected(x,y,z);
  prev_x = x; prev_y = y; prev_z = z;
  if (hasMove) {
    seconds = 0;

    #if LED_ENABLE
    ledcircle_select(0);
    #else
    /*printf("SLEEP!\n");*/
    if (g_phone_detected)
    {
      lib_aci_sleep();
    }
    #endif

    turnOnBeacon = false;
  }
  else
  {
    if (seconds >= TIME_NOT_MOVE) {
      turnOnBeacon = true;
    }
  }

  if (turnOnBeacon) {
    #if LED_ENABLE
    DELAY_SECOND(1) {
      SHOW_PRIV_TAG();
    }

    ledcircle_select(0);

    DELAY_SECOND(1) {
      display_time_led(seconds);
    }

    ledcircle_select(0);
    #else
    /*printf("WAKE!\n");*/
    if (!g_phone_detected)
    {
      lib_aci_wakeup();
    }
    #endif
  }

  #if SLEEP_MODE_ENABLE
  sleep();
  #endif

  printf("x:%d y:%d z:%d movement_detected:%d seconds:%d\n", x, y, z, hasMove, seconds);
  #endif
}

/*
 * Interrupt handlers
 */

void TC3_Handler(void) {
  #if 0
  if (t <= 16) t = t+1;
  else t = 0;
  #endif
  seconds++;
  if (timerCount != 0) {
    timerCount--;
  }

  TC3->COUNT32.INTFLAG.bit.MC0 = 1;
}

// TODO(phil): probably remove
void USB_Handler(void) {
  isUSBConnected = 1;
  USBDevice.ISRHandler();
}

// note may need to move seconds count in rtc
void RTC_Handler(void)
{
  if (t <= 16) t = t+1;
  else t = 0;
  // FIXME(phil): remove this
  printf("rtc\n");
  RTC->MODE1.INTFLAG.bit.OVF = 1;
}

/*
 * Helper functions
 */

bool movement_detected(int16_t x, int16_t y, int16_t z) {
  if (IS_DIFF(x, prev_x) || IS_DIFF(y, prev_y) || IS_DIFF(z, prev_z)) {
    return true;
  } else {
    return false;
  }
}

void decimalToBinary(uint16_t n, uint16_t *onLEDs)
{
    uint8_t ledIndex = 0;
    for (uint8_t i = 0; i < 16; i++) {
        uint16_t k = n >> i;
        if (k & 1) {
          onLEDs[ledIndex++] = 1;
        } else {
          onLEDs[ledIndex++] = 0;
        }
    }
}

uint32_t compute_cc_value(uint32_t period_ms)
{
  uint16_t hertz = ONE_mHZ / period_ms;
  uint16_t ccValue = CPU_HZ / (PRESCALER_DIV * hertz) - 1;
  #if SLOWER_CLOCK_ENABLE
  return 16; // 16
  #else
  return ccValue;
  #endif
}

void display_time_led(uint16_t seconds)
{
  uint8_t minute = seconds/60;
  if (minute > 0) {
    uint16_t onLEDs[16];

    decimalToBinary(minute, onLEDs);

    for (int i = 0; i < 16; i++) {
      if (onLEDs[i]) {
        ledcircle_select(i+1);
      }
    }
  }
}

void sleep()
{
  SCB->SCR |= SCB_SCR_SLEEPDEEP_Msk;
  __DSB();
  __WFI();
}
