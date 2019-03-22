#include <sam.h>
#include "../../samd21/include/samd21g18a.h" // for vincent's computer since he can't find samd21 code
#include "../include/i2c.h"

/*
 * \brief Configure and enable the SERCOM I2C peripheral to communicate in
 * master mode, at a standard BAUD RATE.
 */
void i2c_init() {
  // Disable and reset SERCOM
  SERCOM3->I2CM.CTRLA.bit.SWRST = 1;
  while (SERCOM3->I2CM.CTRLA.bit.SWRST || SERCOM3->I2CM.SYNCBUSY.bit.SWRST);

  // Select Master Mode
  SERCOM3->I2CM.CTRLA.bit.MODE = SERCOM_I2CM_CTRLA_MODE_I2C_MASTER_Val;

  // Derived from F_SCL = F_GCLK / (10 + 2*BAUD + F_GCLK*T_RISE)
  SERCOM3->I2CM.BAUD.bit.BAUD = (F_GCLK - 10*F_SCL) / (2*F_SCL);

  // Disable Smart Mode
  SERCOM3->I2CM.CTRLB.bit.SMEN = 0;

  // These bits define the SDA hold time with respect to the negative edge of SCL.
  SERCOM3->I2CM.CTRLA.bit.SDAHOLD = HOLD_75NS;

  // Enable SERCOM
  SERCOM3->I2CM.CTRLA.bit.ENABLE = 1;
  while (SERCOM3->I2CM.SYNCBUSY.bit.ENABLE);

  // Set bus state to idle
  SERCOM3->I2CM.STATUS.bit.BUSSTATE = BUS_STATE_IDLE;
  while (SERCOM3->I2CM.SYNCBUSY.bit.SYSOP);
}

/*
 * \brief - Perform a transaction to Write/Read bytes to/from the I2C
 * peripheral to a slave with the address specified as a parameter. If the dir
 * parameter is 0 it is writing to a slave,  if it is 1 it is reading from the
 * slave. This is a blocking function, in that it should return only when the
 * entire transaction has completed.
 *
 * \param address - slave address
 * \param dir - read or write bit
 * \param data - register
 * \param len - num registers
 */
uint8_t i2c_transaction(uint8_t address, uint8_t dir, uint8_t* data, uint8_t len) {
  while (SERCOM3->I2CM.STATUS.bit.BUSSTATE == BUS_STATE_BUSY);

  SERCOM3->I2CM.ADDR.bit.ADDR = (address << 1) | dir;

  for (;;) {
    // Case 1: Arbitration lost or bus error during address packet transmission
    if (SERCOM3->I2CM.INTFLAG.bit.MB && SERCOM3->I2CM.STATUS.bit.ARBLOST) {
      SERCOM3->I2CM.STATUS.bit.ARBLOST = 1;
      SERCOM3->I2CM.STATUS.bit.BUSERR = 1;
      SERCOM3->I2CM.CTRLB.bit.CMD = STOP_CONDITION;
      while (SERCOM3->I2CM.SYNCBUSY.bit.SYSOP);
      //printf("ERROR(Case 1): Arbitration lost or bus error during address packet transmission\n");
      return 0;
    }
    // Case 2: Address packet transmit complete – No ACK received
    else if (SERCOM3->I2CM.INTFLAG.bit.MB && SERCOM3->I2CM.STATUS.bit.RXNACK) {
      //printf("ERROR(Case 2): Address packet transmit complete – No ACK received\n");
      SERCOM3->I2CM.CTRLB.bit.CMD = STOP_CONDITION;
      while (SERCOM3->I2CM.SYNCBUSY.bit.SYSOP);
      return 0;
    }
   else {
      // Case 3: Address packet transmit complete – Write packet, Master on Bus set
      if (dir == DIR_WRITE) {
        SERCOM3->I2CM.DATA.bit.DATA = data[len-1];
        if (SERCOM3->I2CM.INTFLAG.bit.MB && !SERCOM3->I2CM.STATUS.bit.RXNACK) {
          SERCOM3->I2CM.CTRLB.bit.CMD = STOP_CONDITION;
          while (SERCOM3->I2CM.SYNCBUSY.bit.SYSOP);

          return 0;
        }
      }
      // Case 4: Address packet transmit complete – Read packet, Slave on Bus set
      else if (dir == DIR_READ) {
        if (SERCOM3->I2CM.INTFLAG.bit.SB && !SERCOM3->I2CM.STATUS.bit.RXNACK) {
          data[0] = SERCOM3->I2CM.DATA.bit.DATA;
          SERCOM3->I2CM.CTRLB.bit.ACKACT = NACK;
          SERCOM3->I2CM.CTRLB.bit.CMD = STOP_CONDITION;
          while (SERCOM3->I2CM.SYNCBUSY.bit.SYSOP);

          return data[0];
        }
      }
    }
  }

  return 0;
}

