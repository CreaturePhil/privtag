#include <sam.h>
#include "src/include/clock.h"
#include "src/include/timer.h"
#include "src/include/ledcircle.h"
#include "src/include/i2c.h"
#include "src/include/bma250.h"
#include "src/include/rtc.h"
#include "src/include/print.h"
#include "src/include/privtag.h"

volatile int t;

void privtag_init()
{

/*GCLK->CTRL.reg = GCLK_CTRL_SWRST;*/

  /*while ( (GCLK->CTRL.reg & GCLK_CTRL_SWRST) && (GCLK->STATUS.reg & GCLK_STATUS_SYNCBUSY) )*/
  /*{*/
    /*[> Wait for reset to complete <]*/
/*}*/

  config32kOSC();

/*GCLK->GENDIV.reg = GCLK_GENDIV_ID( 1 ); // Generic Clock Generator 1*/

  /*while ( GCLK->STATUS.reg & GCLK_STATUS_SYNCBUSY )*/
  /*{*/
    /*[> Wait for synchronization <]*/
  /*}*/

  /*[> Write Generic Clock Generator 1 configuration <]*/
  /*GCLK->GENCTRL.reg = GCLK_GENCTRL_ID( 1 ) | // Generic Clock Generator 1*/
                      /*GCLK_GENCTRL_SRC_OSC32K | // Selected source is Internal 32KHz Oscillator*/
/*//                      GCLK_GENCTRL_OE | // Output clock to a pin for tests*/
                      /*GCLK_GENCTRL_GENEN;*/




  /*while ( GCLK->STATUS.reg & GCLK_STATUS_SYNCBUSY )*/
  /*{*/
    /*[> Wait for synchronization <]*/
  /*}*/

   /*SYSCTRL->DFLLCTRL.bit.ENABLE = 0;*/
     /*while ( (SYSCTRL->PCLKSR.reg & SYSCTRL_PCLKSR_DFLLRDY) == 0 )*/
  /*{*/
    /*[> Wait for synchronization <]*/
/*}*/

  clock_setup(GCLK_CLKCTRL_GEN_GCLK1_Val, GCLK_CLKCTRL_ID_TCC2_TC3_Val);
  PM->APBCMASK.reg |= PM_APBCMASK_TC3;
  timer_init(TC3, TC3_IRQn, TC_CTRLA_PRESCALER_DIV1024, compute_cc_value(50));

  PM->APBCMASK.reg |= PM_APBCMASK_SERCOM3;
  clock_setup(GCLK_CLKCTRL_GEN_GCLK1_Val, GCLK_CLKCTRL_ID_SERCOM3_CORE_Val);
  i2c_init();
  bma250_init();

  PM->APBAMASK.reg |= PM_APBAMASK_RTC;
  clock_setup(GCLK_CLKCTRL_GEN_GCLK1_Val, GCLK_CLKCTRL_ID_RTC_Val);
  rtc_init(0xA, 100); // 3 seconds
}

void privtag_app()
{
#if 1
  int16_t x, y, z;
  bma250_read_xyz(&x, &y, &z);

  bool hasMove = movement_detected(x,y,z);
  prev_x = x; prev_y = y; prev_z = z;

  if (hasMove) {
    seconds = 0;
    ledcircle_select(0);
    turnOnLEDs = false;
  }
  else if (seconds >= TIME_NOT_MOVE) {
    turnOnLEDs = true;
  }

  if (turnOnLEDs) {
    // TODO(phil): change name of macro
    DELAY_SECOND(2) {
      SHOW_PRIV_TAG();
    }

    ledcircle_select(0);

    DELAY_SECOND(2) {
      display_time_led(seconds);
    }

    ledcircle_select(0);
  }
  #endif

  if (t) {

  ledcircle_select(8);
  }
  else {

  ledcircle_select(0);
  }

  // TODO(phil): sleep
  sleep();

  /*printf("x:%d y:%d z:%d movement_detected:%d seconds:%d\n", x, y, z, hasMove, seconds);*/
}

/*
 * Interrupt handlers
 */

void TC3_Handler(void) {
/*t = !t;*/
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
/*t = !t;*/
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
  return 16;
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
