// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
#include "stdio.h"
#include "stdint.h"
#include "string.h"
#include "zlib.h"

int main()
{
    unsigned char buf[8];
    unsigned int crc = 0;

    memset(buf, 0xFF, sizeof(buf));
    for (auto c = 0; c < 1024 * 1024; c++) {
        crc = crc32(0L, Z_NULL, 0);
        for (auto i = 0; i < 8; i++) {
            crc = crc32(crc, buf + i, 1);
        }
    }

    printf("%#x\n", crc);
    return 0;
}

