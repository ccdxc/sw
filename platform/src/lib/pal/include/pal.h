/*
 * Copyright (c) 2017, Pensando Systems Inc.
 */

#ifndef __PAL_H__
#define __PAL_H__

#ifdef __cplusplus
extern "C" {
#if 0
} /* close to calm emacs autoindent */
#endif
#endif

/*
 * PAL register read/write APIs.
 */
u_int32_t pal_reg_rd32(const u_int64_t pa);
void pal_reg_wr32(const u_int64_t pa, const u_int32_t val);

u_int64_t pal_reg_rd64(const u_int64_t pa);
void pal_reg_wr64(const u_int64_t pa, const u_int64_t val);

void pal_reg_rd32w(const u_int64_t pa,
                   u_int32_t *w,
                   const u_int32_t nw);

void pal_reg_wr32w(const u_int64_t pa,
                   const u_int32_t *w,
                   const u_int32_t nw);

int pal_reg_trace_control(const int on);

/*
 * PAL memory read/write APIs.
 */
void *pal_mem_map(const u_int64_t pa, const size_t sz);
void pal_mem_unmap(void *va);
u_int64_t pal_mem_vtop(const void *va);
int pal_mem_rd(const u_int64_t pa,       void *buf, const size_t sz);
int pal_mem_wr(const u_int64_t pa, const void *buf, const size_t sz);

int pal_mem_trace_control(const int on);

#ifdef __cplusplus
}
#endif

#endif /* __PAL_H__ */
