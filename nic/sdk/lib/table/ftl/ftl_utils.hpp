//-----------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------
#ifndef __FTL_UTILS_HPP__
#define __FTL_UTILS_HPP__

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <cinttypes>
#include "ftl_includes.hpp"

#ifdef SIM
#define FTL_TRACES_ENABLE
#endif

#ifdef FTL_TRACES_ENABLE
#define FTL_TRACE_VERBOSE(_msg, _args...) SDK_TRACE_VERBOSE(_msg, ##_args)
#define FTL_TRACE_DEBUG(_msg, _args...) SDK_TRACE_DEBUG(_msg, ##_args)
#else
#define FTL_TRACE_VERBOSE(_msg, _args...)
#define FTL_TRACE_DEBUG(_msg, _args...)
#endif

#define FTL_TRACE_INFO(_msg, _args...) SDK_TRACE_INFO(_msg, ##_args)
#define FTL_TRACE_ERR(_msg, _args...) SDK_TRACE_ERR(_msg, ##_args)
#define FTL_SNPRINTF(_buf, _len, _fmt, _args...) snprintf(_buf, _len, _fmt, ##_args)

#define FTL_RET_CHECK_AND_GOTO(_status, _label, _msg, _args...) {\
    if (unlikely((_status) != sdk::SDK_RET_OK)) {\
        SDK_TRACE_ERR(_msg, ##_args);\
        goto _label;\
    }\
}

#ifndef SIM
#define CRC32X(crc, value) __asm__("crc32x %w[c], %w[c], %x[v]":[c]"+r"(crc):[v]"r"(value))
#define RBITX(value) __asm__("rbit %x0, %x1": "=r"(value) : "r"(value))
#define RBITW(value) __asm__("rbit %w0, %w1": "=r"(value) : "r"(value))
#define REVX(value) __asm__("rev %x0, %x1": "=r"(value) : "r"(value))
static inline uint32_t
crc32_aarch64(const uint64_t *p)
{
    uint32_t crc = 0;
    for (auto i = 0; i < 8; i++) {
        auto v = p[i];
        RBITX(v);
        REVX(v);
        CRC32X(crc, v);
    }
    RBITW(crc);
    return crc;
}
#endif

static inline void *
get_sw_entry_pointer (base_table_entry_t *p)
{
    return (uint8_t *)p + sizeof(base_table_entry_t);
}

static inline char *
ftlu_rawstr (void *data, uint32_t len)
{
    static thread_local char str[512];
    uint32_t i = 0;
    uint32_t slen = 0;
    for (i = 0; i < len; i++) {
        slen += sprintf(str+slen, "%02x", ((uint8_t*)data)[i]);
    }
    return str;
}

#endif    // __FTL_UTILS_HPP__
