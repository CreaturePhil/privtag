#include <sam.h>

#include "src/include/privtag.h"

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
