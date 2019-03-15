#include <sam.h>
#include "../include/i2c.h"
#include "../include/bma250.h"

/*
 * \brief Configure and enable the BMA250
 */
void bma250_init() {
  // Configure pins
  PORT->Group[0].PINCFG[PA22_SDA].bit.PMUXEN = 1;
  PORT->Group[0].PINCFG[PA23_SDC].bit.PMUXEN = 1;
  PORT->Group[0].PMUX[PA22_SDA/2].bit.PMUXO = PERIPHERAL_FUNCTION_C;
  PORT->Group[0].PMUX[PA22_SDA/2].bit.PMUXE = PERIPHERAL_FUNCTION_C;
}

// Get acceleration data
static void read_accd_reg(int16_t *a, uint8_t lsb, uint8_t msb) {
  uint8_t data[1];

  data[0] = lsb;
  i2c_transaction(ADDR_BMA250, DIR_WRITE, data, 1);
  i2c_transaction(ADDR_BMA250, DIR_READ, data, 1);

  *a = data[0] >> 6;

  data[0] = msb;
  i2c_transaction(ADDR_BMA250, DIR_WRITE, data, 1);
  i2c_transaction(ADDR_BMA250, DIR_READ, data, 1);

  *a |= data[0] << 2;
}

/*
 * Read the current X,Y, and Z acceleration data from the accelerometer. This
 * data will reflect the orientation and movement of the hardware.
 */
void bma250_read_xyz(int16_t* x, int16_t* y, int16_t* z) {
  read_accd_reg(x, REG_ACCD_X_LSB, REG_ACCD_X_MSB);
  read_accd_reg(y, REG_ACCD_Y_LSB, REG_ACCD_Y_MSB);
  read_accd_reg(z, REG_ACCD_Z_LSB, REG_ACCD_Z_MSB);
}
