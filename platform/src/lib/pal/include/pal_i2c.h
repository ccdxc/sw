/*
 * Copyright (c) 2018, Pensando Systems Inc.
 */

#ifndef __PAL_I2C_H__
#define __PAL_I2C_H__

#define QSFP_PORT_1 1
#define QSFP_PORT_2 2

#define QSFP_1_I2C_BUS 1
#define QSFP_1_SLAVE_ADDRESS 0x50

#define QSFP_2_I2C_BUS 0
#define QSFP_2_SLAVE_ADDRESS 0x50

#define QSFP_DOM_1_I2C_BUS 1
#define QSFP_DOM_1_SLAVE_ADDRESS 0x51

#define QSFP_DOM_2_I2C_BUS 0
#define QSFP_DOM_2_SLAVE_ADDRESS 0x51

#define I2C_BUS 2
#define FRU_SLAVE_ADDRESS 0x50
#define FRU_SIZE 256

/*
 * I2C read/write API's
 */
int pal_i2c_read(uint8_t *buffer, uint32_t size, uint32_t nretry,
                 uint32_t bus, uint32_t slaveaddr);

/*
 * I2C read/write API's
 */
int pal_fru_read(uint8_t *buffer, uint32_t size, uint32_t nretry);
int pal_qsfp_read(uint8_t *buffer, uint32_t size,
                  uint32_t nretry, uint32_t port);
int pal_qsfp_dom_read(uint8_t *buffer, uint32_t size,
                      uint32_t nretry, uint32_t port);

#endif /* __PAL_FRU_IMPL_H__ */
