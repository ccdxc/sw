
/*
 * Copyright (c) 2019, Pensando Systems Inc.
 */

#include "dtls.h"
#include "iomem.h"
#include "defs.h"
#include "cap_ssi.h"

static inline uint32_t
ssi_readreg(int reg)
{
    return readreg(SSI_BASE + reg);
}

static inline void
ssi_writereg(int reg, uint32_t val)
{
    writereg(SSI_BASE + reg, val);
}

// Initialize the SPI controller for a transaction
static void
ssi_init(int scpol, int scph, int tmod, int rxlen)
{
    // Disable master and output-enable
    ssi_writereg(SSI_SSIENR, 0);
    ssi_writereg(SSI_SER, 0);

    // Set SPI format, TX-only mode, 8-bit data frame
    // The target SPI frequency is 12.5MHz.  A /32 divider will work for
    // both the standard core(833MHz/2) and also high (1.1GHz/2) speeds.
    ssi_writereg(SSI_CTRLR0, (tmod << 8) | (scpol << 7) | (scph << 6) | 0x7);
    ssi_writereg(SSI_BAUDR, 32);

    // Set len Rx data bytes
    if (tmod >= 2) {
	ssi_writereg(SSI_CTRLR1, rxlen - 1);
    }
    // Enable
    ssi_writereg(SSI_SSIENR, 1);
}

static void
ssi_enable_cs(int cs)
{
    ssi_writereg(SSI_SER, 1 << cs);
}

static void
ssi_wait_done(void)
{
    while (ssi_readreg(SSI_SR) & SSI_SR_BUSY) {
        ; /* spin */
    }
}

uint8_t
cpld_read(int reg)
{
    ssi_init(0, 0, 3, 2);
    ssi_writereg(SSI_DR, 0x0b);
    ssi_writereg(SSI_DR, reg);
    ssi_enable_cs(0);
    ssi_wait_done();
    ssi_writereg(SSI_SSIENR, 0);
    return CAP_GPIO_CPLD_DATA(gpio_read_bits());
}
