#ifndef _PAL_REG_H_
#define _PAL_REG_H_
#include <inttypes.h>

/*
 * PAL register read/write APIs.
 */
u_int16_t pal_reg_rd16(const u_int64_t pa);
void pal_reg_wr16(const u_int64_t pa, const u_int16_t val);

u_int32_t pal_reg_rd32(const u_int64_t pa);
void pal_reg_wr32(const u_int64_t pa, const u_int32_t val);

u_int64_t pal_reg_rd64(const u_int64_t pa);
void pal_reg_wr64(const u_int64_t pa, const u_int64_t val);

u_int64_t pal_reg_rd64_safe(const u_int64_t pa);
void pal_reg_wr64_safe(const u_int64_t pa, const u_int64_t val);

void pal_reg_rd32w(const u_int64_t pa,
                   u_int32_t *w,
                   const u_int32_t nw);

static inline void pal_reg_rd32_1w(const u_int64_t pa,
                                   u_int32_t *w)
{
    pal_reg_rd32w(pa, w, 1);
}

void pal_reg_wr32w(const u_int64_t pa,
                   const u_int32_t *w,
                   const u_int32_t nw);

static inline void pal_reg_wr32_1w(const u_int64_t pa,
                                   u_int32_t *w)
{
    pal_reg_wr32w(pa, w, 1);
}

#endif

