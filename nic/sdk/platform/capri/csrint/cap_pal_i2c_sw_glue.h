/*
 * Copyright (c) 2019, Pensando Systems Inc.
 */

#ifndef __CAP_PAL_I2C_SW_GLUE_H__
#define __CAP_PAL_I2C_SW_GLUE_H__

#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include "platform/pal/include/pal.h"

#define MAX_RETRY 5

#ifdef NOTYET
int
pal_smbus_read_block_data(unsigned int dev_num, unsigned char addr,
                      unsigned char command, unsigned char *values) {
    printf("Not implemented yet\n");
    return 0;
}

int
smbus_read_byte(int dev_num, unsigned char addr) {
    printf("Not implemented yet\n");
    return 0;
}
#endif //NOTYET

void
pal_smbus_write_byte(unsigned int dev_num, unsigned char addr,
                 unsigned char value)
{
    printf("Not implemented yet;;NOP\n");
    return;
}

void
pal_smbus_write_byte_data(unsigned int dev_num, unsigned char addr,
                      unsigned char cmd, unsigned char value)
{
    int retval;
    retval = smbus_write(&value, 1, cmd, MAX_RETRY, dev_num, addr);
    if(retval)
    {
        printf("Error occured while writing\n");
    }

    return;
}

void
pal_smbus_write_word(unsigned int dev_num, unsigned char addr,
                 unsigned char cmd, int value) {

    int retval;
    retval = smbus_write((uint8_t*)&value, 2, cmd, MAX_RETRY, dev_num, addr);
    if(retval)
    {
        printf("Error occured while writing\n");
    }

    return;
}

int
pal_smbus_read_word(unsigned int dev_num, unsigned char addr,
                unsigned char cmd)
{
    int retval;
    uint8_t value[2];
    retval = smbus_read(value, 2, cmd, MAX_RETRY, dev_num, addr);
    if(retval)
    {
        printf("Error occured while reading\n");
    }

    return 0xffff & (value[0] | value[1] << 8 );
}

int
pal_smbus_read_byte_data(unsigned int dev_num, unsigned char addr,
                     unsigned char cmd) {

    int retval;
    uint8_t value = 0;
    retval = smbus_read(&value, 1, cmd, MAX_RETRY, dev_num, addr);
    if(retval)
    {
        printf("Error occured while reading\n");
    }
    return 0x0ff & value;
}
#endif /* __CAP_PAL_I2C_SW_GLUE_H__ */
