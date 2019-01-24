#ifndef _PAL_REG_H_
#define _PAL_REG_H_
#include <inttypes.h>

/*
 * PAL register read/write APIs.
 */
uint8_t pal_reg_rd8(const uint64_t pa);
void pal_reg_wr8(const uint64_t pa, const uint8_t val);

uint16_t pal_reg_rd16(const uint64_t pa);
void pal_reg_wr16(const uint64_t pa, const uint16_t val);

uint32_t pal_reg_rd32(const uint64_t pa);
void pal_reg_wr32(const uint64_t pa, const uint32_t val);

uint64_t pal_reg_rd64(const uint64_t pa);
void pal_reg_wr64(const uint64_t pa, const uint64_t val);

uint64_t pal_reg_rd64_safe(const uint64_t pa);
void pal_reg_wr64_safe(const uint64_t pa, const uint64_t val);

void pal_reg_rd32w(const uint64_t pa,
                   uint32_t *w,
                   const uint32_t nw);

static inline void pal_reg_rd32_1w(const uint64_t pa,
                                   uint32_t *w)
{
    pal_reg_rd32w(pa, w, 1);
}

void pal_reg_wr32w(const uint64_t pa,
                   const uint32_t *w,
                   const uint32_t nw);

static inline void pal_reg_wr32_1w(const uint64_t pa,
                                   uint32_t *w)
{
    pal_reg_wr32w(pa, w, 1);
}

#endif

