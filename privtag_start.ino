#include <sam.h>

#include "src/include/privtag.h"

extern "C" void __libc_init_array(void);

int main(void) {
__libc_init_array(); // needed for rtc
  privtag_init();

  while (1) {
    privtag_app();
  }

  return 0;
}
