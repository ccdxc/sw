
/*
 * Copyright (c) 2019, Pensando Systems Inc.
 */

#include "dtls.h"
#include "iomem.h"
#include "defs.h"
#include "cap_qspi.h"

/*
 * QSPI Initialization (HAPS direct boot from flash)
 * -------------------------------------------------
 * When booting u-boot directly from QSPI on HAPS, the QSPI operates in
 * single SPI mode with a /32 clock divider.  This is very slow, and it takes
 * a couple of minutes for u-boot to start.
 *
 * This file is compiled into a standalone .bin file and included in
 * qspi_sram.S.  Eary in startup the code is copied into SSRAM and we jump
 * to it from flash so that it may reconfigure the QSPI to operate in a
 * faster read mode.
 * On HAPS, reads are configured as 1-4-4.
 * On ASIC, reads are configured as 1-2-2 (only 2 data pins on Naples)
 * The clock divider is set to /4.
 */

#define OP_WRITE_SR                 0x01
#define OP_READ_SR                  0x05
#define OP_WRITE_ENABLE             0x06
#define OP_READ_ID                  0x9f
#define OP_MX_READ_CONFIG_REGISTER  0x15

#define SR_WIP                      0x01

static inline uint32_t
qspi_readreg(int reg)
{
    return readreg(QSPI_BASE + reg);
}

static inline void
qspi_writereg(int reg, uint32_t val)
{
    writereg(QSPI_BASE + reg, val);
}

static void
qspi_wait_idle(void)
{
    // Wait for controller idle
    while ((qspi_readreg(QSPI_CONFIG) & QSPI_CONFIG_IDLE) == 0) {
        /* spin */
    }
}

static void
qspi_disable(void)
{
    qspi_wait_idle();
    qspi_writereg(QSPI_CONFIG, qspi_readreg(QSPI_CONFIG) & ~QSPI_CONFIG_ENABLE);
}

static void
qspi_enable(void)
{
    qspi_writereg(QSPI_CONFIG, qspi_readreg(QSPI_CONFIG) | QSPI_CONFIG_ENABLE);
}

static uint32_t
qspi_stig_op(int op, int n_read, int n_write, uint32_t wrdat)
{
    uint32_t res = 0;
    uint32_t val = QSPI_STIG_CMD_OP(op);

    if (n_read > 0) {
        val |= QSPI_STIG_CMD_READ(n_read);
    }
    if (n_write > 0) {
        val |= QSPI_STIG_CMD_WRITE(n_write);
        qspi_writereg(QSPI_STIG_WRDAT_LO, wrdat);
    }
    qspi_writereg(QSPI_STIG_CMD, val);
    qspi_writereg(QSPI_STIG_CMD, val | QSPI_STIG_CMD_START);
    (void)qspi_readreg(QSPI_STIG_CMD);
    while (qspi_readreg(QSPI_STIG_CMD) & QSPI_STIG_CMD_BUSY) {
        ; /* wait */
    }
    if (n_read > 0) {
        res = qspi_readreg(QSPI_STIG_RDDAT_LO);
        res &= (1 << (8 * n_read)) - 1;
    }
    qspi_wait_idle();
    return res;
}

static int
qspi_is_mx(void)
{
    uint32_t id = qspi_stig_op(OP_READ_ID, 3, 0, 0);
    return ((id & 0xff) == 0xc2);
}

static void
qspi_wait_write_complete(void)
{
    uint32_t sr;
    do {
        sr = qspi_stig_op(OP_READ_SR, 1, 0, 0);
    } while (sr & SR_WIP);
}

static void
qspi_mx_set_cr_dc(uint32_t dc)
{
    uint32_t cr;
    qspi_enable();
    cr = qspi_stig_op(OP_MX_READ_CONFIG_REGISTER, 1, 0, 0);
    cr = (cr & 0x3f) | (dc << 6);
    qspi_stig_op(OP_WRITE_ENABLE, 0, 0, 0);
    qspi_stig_op(OP_WRITE_SR, 0, 2, cr << 8);
    qspi_wait_write_complete();
    qspi_disable();
}

static void
qspi_mx_set_122_dummy(int ndummy)
{
    uint32_t dc;
    switch (ndummy) {
    case 4:  dc = 0; break;
    case 6:  dc = 1; break;
    case 8:  dc = 2; break;
    case 10: dc = 3; break;
    default: break;
    }
    qspi_mx_set_cr_dc(dc);
}

static void
qspi_mx_set_144_dummy(int ndummy)
{
    uint32_t dc;
    switch (ndummy) {
    case 6:  dc = 0; break;
    case 4:  dc = 1; break;
    case 8:  dc = 2; break;
    case 10: dc = 3; break;
    default: break;
    }
    qspi_mx_set_cr_dc(dc);
}

static void
qspi_set_read_144_mode(int is_mx)
{
    if (is_mx) {
        qspi_mx_set_144_dummy(10);
    }
    // read command 0xec - 4-BYTE QUAD I/O FAST READ, 10 dummy clocks
    qspi_writereg(QSPI_READ_IR,
            QSPI_READ_IR_DUMMY(10) |
            QSPI_READ_IR_INST_TYPE(QSPI_IO_1BIT) |
            QSPI_READ_IR_DATA_TYPE(QSPI_IO_4BIT) |
            QSPI_READ_IR_ADDR_TYPE(QSPI_IO_4BIT) |
            QSPI_READ_IR_OPCODE(0xec));
}

static void
qspi_set_read_122_mode(int is_mx)
{
    if (is_mx) {
        qspi_mx_set_122_dummy(8);
    }
    // read command 0xbc - 4-BYTE DUAL I/O FAST READ, 8 dummy clocks
    qspi_writereg(QSPI_READ_IR,
            QSPI_READ_IR_DUMMY(8) |
            QSPI_READ_IR_INST_TYPE(QSPI_IO_1BIT) |
            QSPI_READ_IR_ADDR_TYPE(QSPI_IO_2BIT) |
            QSPI_READ_IR_DATA_TYPE(QSPI_IO_2BIT) |
            QSPI_READ_IR_OPCODE(0xbc));
}

static void
qspi_set_div(int n)
{
    uint32_t val = qspi_readreg(QSPI_CONFIG);
    
    val &= ~QSPI_CONFIG_BAUDDIV_MASK;
    val |= QSPI_CONFIG_BAUDDIV(n);
    qspi_writereg(QSPI_CONFIG, val);
}

static void
qspi_set_read_delay(uint32_t n)
{
    uint32_t val = qspi_readreg(QSPI_READ_CAPTURE);
    
    val &= ~QSPI_READ_CAPTURE_READDELAY_MASK;
    val |= QSPI_READ_CAPTURE_READDELAY(n);
    qspi_writereg(QSPI_READ_CAPTURE, val);
}

void
qspi_init(void)
{
    int is_mx = qspi_is_mx();

    qspi_disable();
    if (get_chip_type() == CHIP_TYPE_HAPS) {
        qspi_set_div(4);
        qspi_set_read_144_mode(is_mx);
    } else {
        qspi_set_div(QSPI_CLK_FREQ_ASIC / board_qspi_frequency());
        qspi_set_read_122_mode(is_mx);
    }
    qspi_set_read_delay(board_qspi_read_delay());
    qspi_enable();
    asm volatile("dsb sy" ::: "memory");
    asm volatile("isb" ::: "memory");
}
