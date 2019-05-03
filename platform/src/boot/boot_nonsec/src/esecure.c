
/*
 * Copyright (c) 2019, Pensando Systems Inc.
 */

#include <stdint.h>
#include "dtls.h"
#include "iomem.h"
#include "esecure.h"

void
esec_mark_boot_success(void)
{
    uint32_t val;

    /*
     * Write Boot Success command.
     */
    writereg(ESEC_MB_REG_TX_HEADER, 16);
    writereg(ESEC_MB_REG_FIFO_DATA, ESEC_CMD_BOOT_SUCCESS);
    writereg(ESEC_MB_REG_FIFO_DATA, 0);
    writereg(ESEC_MB_REG_FIFO_DATA, 0);

    /*
     * Wait for response.
     */
    do {
        val = readreg(ESEC_MB_REG_RX_STATUS);
    } while (val & ESEC_MB_REG_RX_STATUS_RXEMPTY);

    /*
     * Pop Rx Message
     */
    (void)readreg(ESEC_MB_REG_RX_HEADER);
}
