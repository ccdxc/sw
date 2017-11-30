/*
 * Copyright (c) 2018, Pensando Systems Inc.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include <inttypes.h>
#include <sys/types.h>

#include "pal.h"

#include "cap_top_csr_defines.h"
#include "cap_ms_c_hdr.h"

int
main(int argc, char *argv[])
{
    u_int32_t version;
    u_int8_t buf[1024];
    u_int32_t bufsz = sizeof(bufsz);

    //
    // Register access example.
    //

    version = pal_reg_rd32(CAP_ADDR_BASE_MS_MS_OFFSET +
                           CAP_MS_CSR_STA_VER_BYTE_OFFSET);

    printf("Type:    %d\n", version & 0xf);
    printf("Build:   %d\n", (version >> 16) & 0xffff);
    printf("Version: %d\n", (version >> 4) & 0xfff);

    //
    // Memory access example.
    //
    pal_mem_rd(0xa0000000, buf, bufsz);
    buf[0] = 0;
    pal_mem_wr(0xa0000000, buf, bufsz);

    exit(0);
}
