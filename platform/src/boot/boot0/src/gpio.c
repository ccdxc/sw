
/*
 * Copyright (c) 2019, Pensando Systems Inc.
 */

#include "dtls.h"
#include "iomem.h"
#include "cap_gpio.h"
#include "dtls.h"

static inline uint32_t
gpio_readreg(int reg)
{
    return readreg(GPIO_BASE + reg);
}

uint32_t
gpio_read_bits(void)
{
    uint32_t a_bits = gpio_readreg(GPIO_EXT_PORTA);
    uint32_t b_bits = gpio_readreg(GPIO_EXT_PORTB);
    return ((b_bits & 0xff) << 8) | (a_bits & 0xff);
}

int
gpio_bit(int pin)
{
    return (gpio_read_bits() >> pin) & 0x1;
}

void
gpio_init(void)
{
    /* no op */
}
