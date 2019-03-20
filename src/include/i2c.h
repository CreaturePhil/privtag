#ifndef _I2C_H_
#define _I2C_H_

void i2c_init();
uint8_t i2c_transaction(uint8_t address, uint8_t dir, uint8_t* data, uint8_t len);

// 5-6 SCL cycle time-out (50-60μs)
#define CSE190_SERCOM_I2CM_CTRLA_INACTOUT_55US 0x1
// 10-11 SCL cycle time-out (100-110μs)
#define CSE190_SERCOM_I2CM_CTRLA_INACTOUT_105US 0x2
// 20-21 SCL cycle time-out (200-210μs)
#define CSE190_SERCOM_I2CM_CTRLA_INACTOUT_205US 0x3

#define F_GCLK 48000000  // 48 MHz
#define F_SCL  400000    // 100 KHz

#define BUS_STATE_IDLE  0b01
#define BUS_STATE_OWNER 0b10
#define BUS_STATE_BUSY  0b11

#define DIR_READ 1
#define DIR_WRITE 0

#define PA22_SDA 22
#define PA23_SDC 23

#define PERIPHERAL_FUNCTION_C 0x2

#define ADDR_BMA250 0x19

#define STOP_CONDITION 3

#define ACK 0
#define NACK 1

#define HOLD_75NS 0x1

#endif

