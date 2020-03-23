/*
 * Copyright (c) 2017-18, Pensando Systems Inc.
 *
 * pal.h : This header file includes all abstraction headers.
 *
 * PAL is supposed to be a high performance library to access
 * Naples resources. It is NOT THREAD SAFE and does not support
 * Re-entrancy. The consumers of PAL are responsible for ensuring
 * synchronized access to PAL among their threads.
 *
 * To add a new abstraction API, ensure a new header file
 * for the component being abstracted and include it here.
 *
 * The definition for the abstraction APIs live in src directory.
 * Try to ensure there is symmetry in naming between the abstraction
 * header file and the file definint the abstraction.
 *
 * for. eg.
 * To create a new abstraction for component "foo" -
 *
 * 1. Create a header file under include which declare the APIs
 *    viz. pal_foo.h
 *
 * 2. Include the header "pal_foo.h" in pal.h
 *
 * 3. Define the functions declared in pal_foo.h in foo.c file
 *    in source
 *
 */

#ifndef __PAL_H__
#define __PAL_H__

#ifdef __cplusplus
extern "C" {
#if 0
} /* close to calm emacs autoindent */
#endif
#endif

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include "pal_types.h"
#include "pal_locks.h"
#include "pal_i2c.h"
#include "pal_cpld.h"
#include "pal_mem.h"
#include "pal_reg.h"
#include "pal_mm.h"
#include "pal_watchdog.h"
#include "pal_int.h"

int pal_get_env(void);
int pal_is_asic(void);
void pal_init(char *application_name);
int pal_reg_trace_control(const int on);
int pal_mem_trace_control(const int on);
void pal_reg_trace(const char *fmt, ...) __attribute__((format (printf, 1, 2)));
void pal_mem_trace(const char *fmt, ...) __attribute__((format (printf, 1, 2)));

static inline bool
pal_swm_enabled (void)
{
    return (cpld_reg_rd(CPLD_REGISTER_CTRL) & CPLD_ALOM_PRESENT_BIT);
}

static inline size_t
pal_memcpy (void *dst, const void *src, size_t n)
{
    size_t i;
    volatile u_int8_t *d = (u_int8_t*)dst;
    u_int8_t *s = (u_int8_t*)src;
    u_int64_t bytes_left = n;

    if (src != NULL) {
        if (bytes_left >= 8 &&
            (((intptr_t)src | (intptr_t)dst) & 0x07) == 0) {
            u_int64_t ndwords = bytes_left >> 3;
            volatile u_int64_t *d = (u_int64_t*)dst;
            u_int64_t *d_end = (u_int64_t*)d + ndwords;
            u_int64_t *s = (u_int64_t*)src;
            while(d != d_end) {
                *d++ = *s++;
                bytes_left -= 8;
            }
            dst = (void*) d;
            src = (void*) s;
        }

        if (bytes_left >= 4 &&
            (((intptr_t)src | (intptr_t)dst) & 0x03) == 0) {
            u_int32_t nwords = bytes_left >> 2;
            volatile u_int32_t *d = (u_int32_t*)dst;
            u_int32_t *d_end = (u_int32_t*)d + nwords;
            u_int32_t *s = (u_int32_t*)src;

            while(d != d_end) {
                *d++ = *s++;
                bytes_left -= 4;
            }
            dst = (void*) d;
            src = (void*) s;
        }

        if(bytes_left > 0) {
            d = (u_int8_t*)dst;
            s = (u_int8_t*)src;

            for (i = 0; i < bytes_left; i++) {
                *d++ = *s++;
            }
        }
    } else {
        /* TODO : Remove this.
         *
         *        This is placed here to satisfy a hack within HAL which uses
         *        the pal_mem_wr interface to perform ZERO-ing of memory by
         *        passing src as NULL.
         */
        for (i = 0; i < n; i++) {
            *d++ = 0;
        }
    }
    return n;
}

#ifdef __cplusplus
}
#endif

#endif /* __PAL_H__ */

