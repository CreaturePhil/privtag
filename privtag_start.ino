#include <sam.h>

#include "src/include/privtag.h"
#include "src/include/ble.h"

extern "C" void __libc_init_array(void);

void usb_ctrl(int en)
{
  if (en)
  {
    init();
    __libc_init_array();
    USBDevice.init();
    USBDevice.attach();
  }
  else
  {
    __libc_init_array(); // needed for rtc

    // Set Systick to 1ms interval, common to all Cortex-M variants
    if ( SysTick_Config( SystemCoreClock / 1000 ) )
    {
      // Capture error
      while ( 1 ) ;
    }

// SPI NOT ENABLED?
    // Clock PORT for Digital I/O
    //	PM->APBBMASK.reg |= PM_APBBMASK_PORT ;
    //
    //  // Clock EIC for I/O interrupts
    //	PM->APBAMASK.reg |= PM_APBAMASK_EIC ;

    // Clock SERCOM for Serial
    // Sercom 1 is USB, 3 is for i2c...
    PM->APBCMASK.reg |= PM_APBCMASK_SERCOM1 | PM_APBCMASK_SERCOM3;

    // Clock TC/TCC for Pulse and Analog
    PM->APBCMASK.reg |= PM_APBCMASK_TCC0 | PM_APBCMASK_TC3;
     USBDevice.init();
     USBDevice.attach();
  }
}

int main(void) {
  usb_ctrl(0);

  privtag_init();

  while (1) {
    privtag_app();
  }

  return 0;
}
