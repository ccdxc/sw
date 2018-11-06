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

#include "pali2c.h"

enum {
    PAL_ENV_ASIC = 0,
    PAL_ENV_HAPS = 1,
    PAL_ENV_ZEBU = 2
};

typedef enum {
    MATTR_DEVICE,
    MATTR_UNCACHED,
    MATTR_CACHED
} mattr_t;

int pal_get_env(void);
int pal_is_asic(void);

void pal_init(char *application_name);

/* Allocs a region region_name of size size, we specify in flags,
 * if a moveable/non-moveable region is required - returns PA*/
u_int64_t pal_mem_alloc(char *region_name,
                	uint32_t size,
                	u_int32_t alloc_flags);

void pal_mem_free(char *region_name);

/* Move memory from source to destination */
u_int32_t pal_mem_move(uint64_t source_address,
                       uint64_t dest_address,
                       u_int32_t size);

/* Total space available for growth of moveable region */
u_int64_t pal_get_max_available(void);

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

int pal_reg_trace_control(const int on);

/*
 * PAL memory read/write APIs.
 */
void *pal_mem_map(const u_int64_t pa, const size_t sz, u_int32_t flags);
void pal_mem_unmap(void *va);
void *pal_mem_map_region(char *region_name);
void pal_mem_unmap_region(char *region_name);
u_int64_t pal_mem_region_pa(char *region_name);

u_int64_t pal_mem_vtop(const void *va);
void *pal_mem_ptov(const u_int64_t pa);
int pal_mem_rd(const u_int64_t pa,       void *buf, const size_t sz, u_int32_t flags);
int pal_mem_wr(const u_int64_t pa, const void *buf, const size_t sz, u_int32_t flags);
int pal_memset(const u_int64_t pa, u_int8_t c, const size_t sz, u_int32_t flags);

void pal_print_application_regions(char *application_name);
void pal_print_metadata();

int pal_mem_trace_control(const int on);

void pal_reg_trace(const char *fmt, ...)
    __attribute__((format (printf, 1, 2)));
void pal_mem_trace(const char *fmt, ...)
    __attribute__((format (printf, 1, 2)));

/* CPLD APIs */
int pal_is_qsfp_port_psnt(int port_no);
int pal_qsfp_set_port(int port);
int pal_qsfp_reset_port(int port);
int pal_qfsp_set_low_power_mode(int port);
int pal_qfsp_reset_low_power_mode(int port);

/*
 * I2C read/write API's
 */
int pal_i2c_read(u_int8_t *buffer, u_int32_t size, u_int32_t nretry,
                 u_int32_t bus, u_int32_t slaveaddr);

/*
 * I2C read/write API's
 */
int pal_fru_read(u_int8_t *buffer, u_int32_t size, u_int32_t nretry);
int pal_qsfp_read(u_int8_t *buffer, u_int32_t size,
                  u_int32_t nretry, u_int32_t port);
int pal_qsfp_dom_read(u_int8_t *buffer, u_int32_t size,
                      u_int32_t nretry, u_int32_t port);



#ifdef __cplusplus
}
#endif

#endif /* __PAL_H__ */
