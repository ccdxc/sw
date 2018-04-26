// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#include <iostream>
#define __STDC_FORMAT_MACROS
#include <inttypes.h>
#include <string.h>
#include "sdk/pal.hpp"
#include "sdk/types.hpp"

void tcam_mem_dump(uint64_t base_addr,
                   uint32_t num_entries,
                   uint32_t valid_entry_size_bytes,
                   uint32_t total_entry_size_bytes)
{
    uint32_t i               = 0;
    uint32_t entry           = 0;
    uint32_t data            = 0x0;
    uint32_t read_size_bytes = 4;

    for (entry = 0; entry < num_entries; ++entry) {
        uint64_t addr = base_addr;

        for (i = 0; i < valid_entry_size_bytes; i += read_size_bytes) {
            data = 0x0;

            sdk::lib::pal_mem_read(addr, (uint8_t *)&data, read_size_bytes);

            printf("%s 0x%08lx 0x%08x\n",
                   __FUNCTION__, addr, data);
            fflush(stdout);

            addr += read_size_bytes;
        }

        base_addr = base_addr + total_entry_size_bytes;
    }
}

void egress_parser_tcam_dump()
{
    tcam_mem_dump(0x3404000, 256, 20, 0x20);
}

void
debug_hbm_read_write (uint64_t start_addr, uint32_t num_bytes, bool read)
{
    uint32_t count = (num_bytes+7) >> 3;
    uint64_t addr;
    uint64_t data = 0xabababababababab;

    printf("------------------ READ/WRITE HBM START -----------\n");
    for (uint32_t i = 0; i < count; i++) {
        addr = (start_addr + (i<<3)) & 0xffffffff8; 
        if (read) {
            sdk::lib::pal_mem_read(addr, (uint8_t *)&data, sizeof(data));
            if (data != 0xabababababababab) {
                printf("index 0x%x, data %" PRIx64 , i, data);
                printf("\n");
                fflush(stdout);
            }
        } else {
            sdk::lib::pal_mem_write(addr, (uint8_t *)&data, sizeof(data));
        }
    }
    printf("------------------ READ/WRITE HBM END -----------\n");
    fflush(stdout);
}

int main (int argc, char **argv)
{
    sdk::lib::pal_init(sdk::types::platform_type_t::PLATFORM_TYPE_HW);
    uint64_t addr = 0x0;
    uint64_t data = 0x0;

    if (argc < 2) {
        printf("Usage: %s <hbm_read/hbm_write/tcam_read>\n", argv[0]);
        return 0;
    }

    if (!strcmp (argv[1], "hbm_read")) {
        printf("HBM READ\n");
        debug_hbm_read_write(0x0C0000000, 0x200*8, true);

    } else if (!strcmp (argv[1], "hbm_write")) {
        printf("HBM WRITE\n");
        debug_hbm_read_write(0x0C0000000, 0x200*8, false);

    } else if (!strcmp (argv[1], "hbm_test")) {
        printf("HBM TEST\n");
        sdk::lib::pal_mem_read(0x0C0000000, (uint8_t *)&data, sizeof(data));
        printf ("Read data: 0x%lx\n", data);

    } else if (!strcmp (argv[1], "tcam_read")) {
        printf("TCAM DUMP\n");
        egress_parser_tcam_dump();

    } else if (!strcmp (argv[1], "doorbell_ring")) {
        if (argc != 4) {
            printf ("Usage: %s doorbell_ring <addr> <data>\n", argv[0]);
            return 0;
        }

        sscanf(argv[2], "%" SCNx64, &addr);
        sscanf(argv[3], "%" SCNx64, &data);

        printf ("Addr: 0x%lx, data: 0x%lx\n", addr, data);

        sdk::lib::pal_ring_doorbell(addr, data);

    } else {
        printf("Usage: %s <hbm_read/hbm_write/tcam_read>\n", argv[0]);
    }

    return 0;
}
