// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
#include <stdio.h>
#include <stdlib.h>
#include <intrin.h>
#include "types.h"
 
int main()
{
    unsigned char buf[8];
    unsigned int crcin = 0;
    unsigned int crcout = 0;

    memset(buf, 0xFF, sizeof(buf));
    for (auto c = 0; c < 1024 * 1024; c++) {
        for (auto i = 0; i < 8; i++) {
            crc = _mm_crc32_u8(crc, buf[i]);
        }
    }

    printf("%#x\n", crcout);
    return 0;
}

