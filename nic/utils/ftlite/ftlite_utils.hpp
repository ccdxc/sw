//-----------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------
#ifndef __FTLITE_UTILS_HPP__
#define __FTLITE_UTILS_HPP__
#include <stdio.h>
#include <stdint.h>

#define FTLITE_SNPRINTF(_buf, _len, _fmt, _args...) snprintf(_buf, _len, _fmt, ##_args)
#define FTLITE_TRACE_VERBOSE(_msg, _args...) SDK_TRACE_DEBUG(_msg, ##_args)
#define FTLITE_TRACE_DEBUG(_msg, _args...) SDK_TRACE_DEBUG(_msg, ##_args)
#define FTLITE_TRACE_INFO(_msg, _args...) SDK_TRACE_INFO(_msg, ##_args)
#define FTLITE_TRACE_ERR(_msg, _args...) SDK_TRACE_ERR(_msg, ##_args)

#define FTLITE_RET_CHECK(_status, _msg, _args...) {\
    if (unlikely((_status) != sdk::SDK_RET_OK)) {\
        SDK_TRACE_ERR(_msg "ret:%d", ##_args, _status);\
        goto done;\
    }\
}
#define FTLITE_CHECK_AND_RETURN(_status) {\
    if (unlikely((_status) != SDK_RET_OK)) {\
        return ret;\
    }\
}

namespace ftlite {
namespace internal {

#ifdef USE_ARM64_SIMD
#include <arm_neon.h>
inline __attribute__((always_inline))
void swap8bytes(uint8_t* bs1, uint8_t* bs2) {
    auto a = vld1_u8(bs1);
    auto b = vld1_u8(bs2);
    a = vrev64_u8(a);
    b = vrev64_u8(b);
    vst1_u8(bs1, b);
    vst1_u8(bs2, a);
}
#else
inline __attribute__((always_inline))
void swap8bytes(uint8_t* bs1, uint8_t* bs2) {
    for (auto i = 0; i < 8; i++) {
        auto b = bs1[i];
        bs1[i] = bs2[i];
        bs2[i] = b;
    }
}
#endif

static inline
char* rawstr(void *data, uint32_t len) {
    static char str[512];
    uint32_t i = 0;
    uint32_t slen = 0;
    for (i = 0; i < len; i++) {
        slen += sprintf(str+slen, "%02x", ((uint8_t*)data)[i]);
    }
    return str;
}


} // namespace internal
} // namespace ftlite

#endif // __FTLITE_UTILS_HPP__
