// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
#include "stdio.h"
#include "stdint.h"
#include "string.h"

#define CRC32X(crc, value) __asm__("crc32x %w[c], %w[c], %x[v]":[c]"+r"(crc):[v]"r"(value))
#define CRC32W(crc, value) __asm__("crc32w %w[c], %w[c], %w[v]":[c]"+r"(crc):[v]"r"(value))
#define CRC32H(crc, value) __asm__("crc32h %w[c], %w[c], %w[v]":[c]"+r"(crc):[v]"r"(value))
#define CRC32B(crc, value) __asm__("crc32b %w[c], %w[c], %w[v]":[c]"+r"(crc):[v]"r"(value))
#define CRC32CX(crc, value) __asm__("crc32cx %w[c], %w[c], %x[v]":[c]"+r"(crc):[v]"r"(value))
#define CRC32CW(crc, value) __asm__("crc32cw %w[c], %w[c], %w[v]":[c]"+r"(crc):[v]"r"(value))
#define CRC32CH(crc, value) __asm__("crc32ch %w[c], %w[c], %w[v]":[c]"+r"(crc):[v]"r"(value))
#define CRC32CB(crc, value) __asm__("crc32cb %w[c], %w[c], %w[v]":[c]"+r"(crc):[v]"r"(value))
#define RBIT(value) __asm__("rbit %w0, %w1": "=r"(value) : "r"(value))

#if 0
uint32_t crc32_arm64_le_hw(uint32_t crc, const uint8_t *p, unsigned int len)
{
    int64_t length = len;

    while ((length -= sizeof(uint64_t)) >= 0) {
        CRC32X(crc, *((uint64_t *)p));
        p += sizeof(uint64_t);
    }

    if (length & sizeof(uint32_t)) {
        CRC32W(crc, *((uint32_t *)p));
        p += sizeof(uint32_t);
    }
    if (length & sizeof(uint16_t)) {
        CRC32H(crc, *((uint16_t *)p));
        p += sizeof(uint16_t);
    }
    if (length & sizeof(uint8_t))
        CRC32B(crc, *p);

    return crc;
}
#endif

uint32_t
crc32_arm64_le_hw(uint32_t crc, const uint64_t *p, unsigned int len) {
    for (auto i = 0; i < 8; i++) {
        CRC32X(crc, *p);
    }
    RBIT(crc);
    return crc;
}

int main()
{
    unsigned char buf[64];
    unsigned int crcin = 0;
    unsigned int crcout = 0;

    memset(buf, 0xFF, sizeof(buf));
    for (auto c = 0; c < 1024 * 1024; c++) {
        crcout = crc32_arm64_le_hw(crcin, (uint64_t *)buf, sizeof(buf));
    }

    printf("%#x\n", crcout);
    return 0;
}

