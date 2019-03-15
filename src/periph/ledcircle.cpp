#include <sam.h>
#include "../include/ledcircle.h"

/*
 * \brief Turn on the selected LED. Use 0 to turn off all LEDs

 * \param led - Charlieplexed LEDs D1-16
 */
void ledcircle_select(uint8_t led) {
  // Set Low output -> Output declar -> High imped
  PORT->Group[0].OUTCLR.reg = IO5 | IO6 | IO7 | IO8 | IO9;
  PORT->Group[0].DIRSET.reg = IO5 | IO6 | IO7 | IO8 | IO9;
  PORT->Group[0].DIRCLR.reg = IO5 | IO6 | IO7 | IO8 | IO9;

  switch (led)
  {
    case 1:
      {
        HIGH_LOW(IO5, IO6);
      } break;

    case 2:
      {
        HIGH_LOW(IO6, IO5);
      } break;

    case 3:
      {
        HIGH_LOW(IO5, IO7);
      } break;

    case 4:
      {
        HIGH_LOW(IO7, IO5);
      } break;

    case 5:
      {
        HIGH_LOW(IO6, IO7);
      } break;

    case 6:
      {
        HIGH_LOW(IO7, IO6);
      } break;

    case 7:
      {
        HIGH_LOW(IO6, IO8);
      } break;

    case 8:
      {
        HIGH_LOW(IO8, IO6);
      } break;

    case 9:
      {
        HIGH_LOW(IO5, IO8);
      } break;

    case 10:
      {
        HIGH_LOW(IO8, IO5);
      } break;

    case 11:
      {
        HIGH_LOW(IO8, IO7);
      } break;

    case 12:
      {
        HIGH_LOW(IO7, IO8);
      } break;

    case 13:
      {
        HIGH_LOW(IO9, IO7);
      } break;

    case 14:
      {
        HIGH_LOW(IO7, IO9);
      } break;

    case 15:
      {
        HIGH_LOW(IO9, IO8);
      } break;

    case 16:
      {
        HIGH_LOW(IO8, IO9);
      } break;

    default:
      {
      } break;
  }

  // Bright lights
  for (int i = 0; i < 1; i++) {
    PORT->Group[0].PINCFG[PIN_IO5].bit.DRVSTR = i;
    PORT->Group[0].PINCFG[PIN_IO6].bit.DRVSTR = i;
    PORT->Group[0].PINCFG[PIN_IO7].bit.DRVSTR = i;
    PORT->Group[0].PINCFG[PIN_IO8].bit.DRVSTR = i;
    PORT->Group[0].PINCFG[PIN_IO9].bit.DRVSTR = i;
  }
}
