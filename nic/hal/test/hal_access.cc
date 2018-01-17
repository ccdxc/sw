// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#include <iostream>
#include "sdk/pal.hpp"

void capri_egress_parser_tcam_dump()
{
    int      i             = 0;
    int      entry         = 0;
    uint64_t base_addr     = 0x3404000;
    uint32_t size_in_bytes = 4;
    uint32_t data          = 0x0;

    for (entry = 0; entry < 255; ++entry) {
        uint64_t addr = base_addr;

        for (i = 0; i < 20; i+=4) {
            data = 0x0;

            sdk::lib::pal_mem_read(addr, (uint8_t *)&data, size_in_bytes);

            printf("%s 0x%08lx 0x%08x\n",
                   __FUNCTION__, addr, data);
            fflush(stdout);

            addr += 4;
        }

        base_addr = base_addr + 0x20;
    }
}

int main (int argc, char **arv)
{
    sdk::lib::pal_init(false);

    capri_egress_parser_tcam_dump();

    return 0;
}
