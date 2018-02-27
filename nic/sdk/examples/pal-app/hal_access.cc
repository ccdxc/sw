// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#include <iostream>
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

int main (int argc, char **arv)
{
    sdk::lib::pal_init(sdk::types::platform_type_t::PLATFORM_TYPE_HW);

    egress_parser_tcam_dump();

    return 0;
}
