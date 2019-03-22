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
#include "src/include/alert_level_characteristic.h"

#define BLE_DEBUG true

volatile int t;
extern "C" bool g_phone_detected;
extern "C" bool findMeMode;
extern "C" bool g_phone_paired;
bool g_phone_paired = false;
bool g_phone_detected = false; // global import...
bool findMeMode = false;

// Initializes the Peripherals for our Sensor
void privtag_init()
{
  // Reduced Clock speed Enabled
  #if SLOWER_CLOCK_ENABLE
  //  clock_GCLK_reset(); // reset the clock and enable 8MHz can work...
    clock_XOSC32K_enable();
   // clock_GCLK_OSC8M_enable();
    // clock_DFLLCTRL_disable();
  #endif

  // Deep Sleep mode state activated
  #if SLEEP_MODE_ENABLE
    PM->APBAMASK.reg |= PM_APBAMASK_RTC;
    clock_setup(GCLK_CLKCTRL_GEN_GCLK2_Val, GCLK_CLKCTRL_ID_RTC_Val);
    rtc_init(0xA, 8);
  #endif

  // Setup of Bluetooth
  ble_setup();

  // Clock Setup
  clock_setup(GCLK_CLKCTRL_GEN_GCLK0_Val, GCLK_CLKCTRL_ID_TCC2_TC3_Val);
  /*PM->APBCMASK.reg |= PM_APBCMASK_TC3;*/
  timer_init(TC3, TC3_IRQn, TC_CTRLA_PRESCALER_DIV1024, compute_cc_value(1000));

  /*PM->APBCMASK.reg |= PM_APBCMASK_SERCOM3;*/
  clock_setup(GCLK_CLKCTRL_GEN_GCLK0_Val, GCLK_CLKCTRL_ID_SERCOM3_CORE_Val);
  
  // Acceleromter setup
  i2c_init();
  bma250_init();

}

void privtag_app()
{
  ble_loop();
  int16_t x, y, z;
  bma250_read_xyz(&x, &y, &z); // the bma read is causing a bug in sercom?? somehow we aren't broadcasting unless usb is connected    
  bool hasMove = movement_detected(x,y,z);
  prev_x = x; prev_y = y; prev_z = z;

  if (hasMove) {
    seconds = 0;
    turnOnBeacon = false;
  }
  else
  {
    if (seconds >= TIME_NOT_MOVE) {
      turnOnBeacon = true; // we arm it 
    }
  }
  if (g_phone_detected && g_phone_paired){
    if (findMeMode) {
      if (hasMove) {
        // Debugging via USB
        if (PRINT_DEBUG_ENABLE) {
          printf("Burglar moving your item!\n");
          printf("%d\n", linkDisconnect());
          printf("Alarm\n");
        } else {
          linkDisconnect();
        }
        linkReset();
        DELAY_SECOND(1);
      } else {
        // don't push that alarm again please
      }
    } 
  }
// Debugging!
if (PRINT_DEBUG_ENABLE) {
  if (g_phone_detected) {
    printf("Phone connected!\n"); 
  } else {
    printf("Phone not connected!\n");
  }

  if (g_phone_paired) {
    printf("Phone paired!\n");
  } else {
    printf("Phone not paired!\n");
  }

  if (findMeMode) {
    printf("Findme Mode on\n");
  } else {
    printf("Findme Mode off\n");
  }

  if (turnOnBeacon) {
    printf("Armed!\n");
  } else {
    printf("Not Armed\n");
  }
  printf("x:%d y:%d z:%d movement_detected:%d seconds:%d\n", x, y, z, hasMove, seconds);
}
  // #if SLEEP_MODE_ENABLE
  // sleep();
  // #endif
  
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
