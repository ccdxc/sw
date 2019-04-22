// {C} Copyright 2018 Pensando Systems Inc. All rights reserved

#include "lib/utils/crc_fast.hpp"
#include "lib/utils/crc_fast.cc"
#include "string.h"

int main()
{
    unsigned char buf[64];
    unsigned int crc = 0;
    sdk::utils::crcFast *crcgen;

    memset(buf, 0xFF, sizeof(buf));
    crcgen = sdk::utils::crcFast::factory(0);
    for (int i = 0; i < 1024*1024; i++) {
        crc = crcgen->compute_crc(buf, sizeof(buf), 0);
    }
    printf("%#x\n", crc);
    return 0;
}

