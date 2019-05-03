
/*
 * Copyright (c) 2019, Pensando Systems Inc.
 */

#ifndef __IOMEM_H__
#define __IOMEM_H__

#define SSRAM_BASE                  0x400000
#define SSRAM_SIZE                  0x20000

#define FLASH_BASE                  0x70000000
#define FLASH_SIZE                  0x8000000

#define CAP_MS_BASE                 0x6a000000

#define MS_(n)      (CAP_MS_BASE + CAP_MS_CSR_ ## n ## _BYTE_ADDRESS)

#ifndef __ASSEMBLER__
static inline uint32_t readreg(uint64_t addr)
{
    uint32_t r = *(volatile uint32_t *)addr;
    asm volatile("dsb sy" ::: "memory");
    return r;
}

static inline void writereg(uint64_t addr, uint32_t val)
{
    asm volatile("dsb sy" ::: "memory");
    *(volatile uint32_t *)addr = val;
}
#endif

#endif
