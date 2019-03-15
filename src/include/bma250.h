#ifndef BMA250_H
#define BMA250_H

void bma250_init();

void bma250_read_xyz(int16_t* x, int16_t* y, int16_t* z);

#define PA22_SDA 22
#define PA23_SDC 23

#define PERIPHERAL_FUNCTION_C 0x2

#define REG_BGW_CHIPID 0x00

#define REG_ACCD_X_LSB 0x02
#define REG_ACCD_X_MSB 0x03

#define REG_ACCD_Y_LSB 0x04
#define REG_ACCD_Y_MSB 0x05

#define REG_ACCD_Z_LSB 0x06
#define REG_ACCD_Z_MSB 0x07

#endif

