//------------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//
// base SDK header file that goes into rest of the SDK
//------------------------------------------------------------------------------

#ifndef __SDK_BASE_HPP__
#define __SDK_BASE_HPP__

#include <ostream>
#include <stdint.h>
#include <assert.h>
#include <stdlib.h>
#include <map>

#include "lib/logger/logger.hpp"

namespace sdk {

#define TRUE                                         1
#define FALSE                                        0

#define SDK_MAX_NAME_LEN                             16

#define _In_
#define _Out_
#define _Inout_

#define likely(x)           __builtin_expect(!!(x), 1)
#define unlikely(x)         __builtin_expect(!!(x), 0)

#ifndef __PACK__
#define __PACK__            __attribute__((packed))
#endif

#define __UNUSED__          __attribute__((unused))

#define __ALIGN__           __attribute__((__aligned__(n)))
#define __CACHE_ALIGN__     __ALIGN__(CACHE_LINE_SIZE)
#define __ASSERT__(x)       assert(x)

#define SDK_MAX(a, b)                                      \
    ({ __typeof__ (a) _a = (a);                            \
       __typeof__ (b) _b = (b);                            \
       _a > _b ? _a : _b;                                  \
     })

#define SDK_MIN(a, b)                                      \
    ({ __typeof__ (a) _a = (a);                            \
       __typeof__ (b) _b = (b);                            \
       _a < _b ? _a : _b;                                  \
     })

#define SDK_ASSERT_RETURN(cond, rv)                        \
do {                                                       \
    if (unlikely(!(cond))) {                               \
        SDK_TRACE_ERR("ASSERT FAILURE(" #cond ")");        \
        __ASSERT__(FALSE);                                 \
        return rv;                                         \
    }                                                      \
} while (FALSE)

#define SDK_ASSERT_TRACE_RETURN(cond, rv, args...)         \
do {                                                       \
    if (unlikely(!(cond))) {                               \
        SDK_TRACE_ERR("ASSERT FAILURE(" #cond ")");        \
        SDK_TRACE_ERR(args);                               \
        __ASSERT__(FALSE);                                 \
        return rv;                                         \
    }                                                      \
} while (FALSE)


#define SDK_ASSERT_RETURN_VOID(cond)                       \
do {                                                       \
    if (unlikely(!(cond))) {                               \
        SDK_TRACE_ERR("ASSERT FAILURE(" #cond ")");        \
        __ASSERT__(FALSE);                                 \
        return;                                            \
    }                                                      \
} while (FALSE)

#define SDK_ASSERT_GOTO(cond, label)                       \
do {                                                       \
    if (unlikely(!(cond))) {                               \
        SDK_TRACE_ERR("ASSERT FAILURE(" #cond ")");        \
        goto label;                                        \
    }                                                      \
} while (FALSE)

#define SDK_ASSERT_TRACE_GOTO(cond, label, args...)        \
do {                                                       \
    if (unlikely(!(cond))) {                               \
        SDK_TRACE_ERR("ASSERT FAILURE(" #cond ")");        \
        SDK_TRACE_ERR(args);                               \
        __ASSERT__(FALSE);                                 \
        goto label;                                         \
    }                                                      \
} while (FALSE)

#define SDK_ASSERT(cond)        __ASSERT__(cond)

#define SDK_ABORT(cond)                                    \
do {                                                       \
    if (unlikely(!(cond))) {                               \
        abort();                                           \
    }                                                      \
} while (FALSE)

#define SDK_ABORT_TRACE(cond, args...)                     \
do {                                                       \
    if (unlikely(!(cond))) {                               \
        SDK_TRACE_ERR(args);                               \
        abort();                                           \
    }                                                      \
} while (FALSE)

//-----------------------------------------------------------------------------
// X-Macro for defining enums (generates enum definition and string formatter)
//
// Example:
//
// #define SAMPLE_ENUM_ENTRIES(ENTRY)
//    ENTRY(OK, 0, "ok")
//    ENTRY(ERR, 1, "err")
//
// SDK_DEFINE_ENUM(sample_enum_t, SAMPLE_ENUM_ENTRIES)
//------------------------------------------------------------------------------
#define _SDK_ENUM_FIELD(_name, _val, _desc) _name = _val,
#define _SDK_ENUM_STR(_name, _val, _desc)                               \
            inline const char *_name ## _str() {return _desc; }
#define _SDK_ENUM_CASE(_name, _val, _desc) case _name: return os << #_desc;
#define _SDK_ENUM_CASE_STR(_name, _val, _desc) case _name: return #_name;
#define _SDK_MAP_ENTRY(_name, _val, _desc) {_desc, _name},

#define SDK_DEFINE_ENUM(_type, _entries)                                \
    typedef enum { _entries(_SDK_ENUM_FIELD) } _type;                   \
    inline std::ostream& operator<<(std::ostream& os, _type t) {        \
        switch (t) {                                                    \
            _entries(_SDK_ENUM_CASE);                                   \
        }                                                               \
        return os;                                                      \
    }                                                                   \
    _entries(_SDK_ENUM_STR)

#define SDK_DEFINE_ENUM_TO_STR(_typ, _entries)                          \
    inline const char *_entries##_str(_typ c)                           \
    {                                                                   \
        switch (c) {                                                    \
            _entries(_SDK_ENUM_CASE_STR);                               \
        }                                                               \
        return "uknown";                                                \
    }

#define SDK_DEFINE_MAP(_typ, _entries)                                  \
    std::map<std::string, _typ> _entries##_map =                        \
    {                                                                   \
        _entries(_SDK_MAP_ENTRY)                                        \
    };

#define SDK_DEFINE_MAP_EXTERN(_typ, _entries)                           \
    extern std::map<std::string, _typ> _entries##_map;


//------------------------------------------------------------------------------
// atomic operations
//------------------------------------------------------------------------------
#define SDK_ATOMIC_INC_UINT32(ptr, val)     __atomic_add_fetch(ptr, val, __ATOMIC_SEQ_CST)
#define SDK_ATOMIC_DEC_UINT32(ptr, val)     __atomic_sub_fetch(ptr, val, __ATOMIC_SEQ_CST)
#define SDK_ATOMIC_STORE_UINT32(ptr, vptr)  __atomic_store(ptr, vptr, __ATOMIC_SEQ_CST)
#define SDK_ATOMIC_LOAD_UINT32(ptr, vptr)   __atomic_load(ptr, vptr, __ATOMIC_SEQ_CST)

#define SDK_ATOMIC_INC_UINT64(ptr, val)     SDK_ATOMIC_INC_UINT32(ptr, val)
#define SDK_ATOMIC_DEC_UINT64(ptr, val)     SDK_ATOMIC_DEC_UINT32(ptr, val)
#define SDK_ATOMIC_STORE_UINT64(ptr, vptr)  SDK_ATOMIC_STORE_UINT32(ptr, vptr)
#define SDK_ATOMIC_LOAD_UINT64(ptr, vptr)   SDK_ATOMIC_LOAD_UINT32(ptr, vptr)

#define SDK_ATOMIC_FETCH_ADD(ptr, val)      __atomic_fetch_add(ptr, val, __ATOMIC_RELAXED)
#define SDK_ATOMIC_FETCH_SUB(ptr, val)      __atomic_fetch_sub(ptr, val, __ATOMIC_RELAXED)

#define SDK_ATOMIC_LOAD_BOOL(ptr)           __atomic_load_n(ptr, __ATOMIC_SEQ_CST)
#define SDK_ATOMIC_STORE_BOOL(ptr, val)     __atomic_store_n(ptr, val, __ATOMIC_SEQ_CST)

#define SDK_ATOMIC_LOAD_UINT16(ptr)         SDK_ATOMIC_LOAD_BOOL(ptr)

#define SDK_ATOMIC_COMPARE_EXCHANGE_WEAK(ptr, vptr, val) \
   __atomic_compare_exchange_n(ptr, vptr, val, true, __ATOMIC_RELEASE, __ATOMIC_RELAXED)

#define SDK_ARRAY_SIZE(arr)                (sizeof((arr))/sizeof((arr)[0]))

//------------------------------------------------------------------------------
// SDK return codes
//------------------------------------------------------------------------------
#define SDK_RET_ENTRIES(ENTRY)                                                 \
    ENTRY(SDK_RET_OK,                         0,    "ok")                      \
    ENTRY(SDK_RET_OOM,                        1,    "out of memory error")     \
    ENTRY(SDK_RET_INVALID_ARG,                2,    "invalid arg")             \
    ENTRY(SDK_RET_INVALID_OP,                 3,    "invalid operation")       \
    ENTRY(SDK_RET_ENTRY_NOT_FOUND,            4,    "lookup failure")          \
    ENTRY(SDK_RET_ENTRY_EXISTS,               5,    "already present")         \
    ENTRY(SDK_RET_NO_RESOURCE,                6,    "resource exhaustion")     \
    ENTRY(SDK_RET_TABLE_FULL,                 7,    "hw table full")           \
    ENTRY(SDK_RET_OOB,                        8,    "out-of-bound error")      \
    ENTRY(SDK_RET_HW_PROGRAM_ERR,             9,    "h/w programming error")   \
    ENTRY(SDK_RET_RETRY,                      10,   "retry the operation")     \
    ENTRY(SDK_RET_NOOP,                       11,   "No operation performed")  \
    ENTRY(SDK_RET_DUPLICATE_FREE,             12,   "duplicate free")          \
    ENTRY(SDK_RET_COLLISION,                  13,   "collision detected")      \
    ENTRY(SDK_RET_MAX_RECIRC_EXCEED,          14,   "maximum recirc exceeded") \
    ENTRY(SDK_RET_HW_READ_ERR,                15,   "h/w read error")          \
    ENTRY(SDK_RET_TXN_NOT_FOUND,              16,   "transaction not found")   \
    ENTRY(SDK_RET_TXN_EXISTS,                 17,   "transaction exists")      \
    ENTRY(SDK_RET_TXN_INCOMPLETE,             18,   "transaction incomplete")  \
    ENTRY(SDK_RET_COMM_FAIL,                  19,   "communication failure")   \
    ENTRY(SDK_RET_HW_SW_OO_SYNC,              20,   "h/w, s/w out of sync")    \
    ENTRY(SDK_RET_OBJ_CLONE_ERR,              21,   "object clone failure")    \
    ENTRY(SDK_RET_IN_PROGRESS,                22,   "operation in progress")   \
    ENTRY(SDK_RET_UPG_CRITICAL,               23,   "upgrade critcal failure") \
    ENTRY(SDK_RET_MAPPING_CONFLICT,           24,   "resource mapping conflict")\
    ENTRY(SDK_RET_MAX,                        25,   "max return value")        \
    ENTRY(SDK_RET_ERR,                        255,  "catch all generic error")

SDK_DEFINE_ENUM(sdk_ret_t, SDK_RET_ENTRIES)
SDK_DEFINE_ENUM_TO_STR(sdk_ret_t, SDK_RET_ENTRIES)
#undef SDK_RET_ENTRIES

typedef enum sdk_status_e {
    SDK_STATUS_NONE,
    SDK_STATUS_ASIC_INIT_DONE,
    SDK_STATUS_MEM_INIT_DONE,
    SDK_STATUS_PACKET_BUFFER_INIT_DONE,
    SDK_STATUS_DATA_PLANE_INIT_DONE,
    SDK_STATUS_SCHEDULER_INIT_DONE,
    SDK_STATUS_UP
} sdk_status_t;
#define INVALID_INDEXER_INDEX                        0xFFFFFFFF

#define htonll(x) ((1==htonl(1)) ? (x) : ((uint64_t)htonl((x) & 0xFFFFFFFF) << 32) | htonl((x) >> 32))
#define ntohll(x) ((1==ntohl(1)) ? (x) : ((uint64_t)ntohl((x) & 0xFFFFFFFF) << 32) | ntohl((x) >> 32))

}    // namespace sdk

using sdk::sdk_ret_t;
using sdk::SDK_RET_OK;
using sdk::SDK_RET_ERR;
using sdk::SDK_RET_INVALID_ARG;
using sdk::SDK_RET_INVALID_OP;
using sdk::SDK_RET_ENTRY_NOT_FOUND;
using sdk::SDK_RET_ENTRY_EXISTS;
using sdk::SDK_RET_COMM_FAIL;
using sdk::SDK_RET_OOM;
using sdk::SDK_RET_OOB;
using sdk::SDK_RET_NO_RESOURCE;
using sdk::SDK_RET_OBJ_CLONE_ERR;
using sdk::SDK_RET_RETRY;
using sdk::SDK_RET_HW_PROGRAM_ERR;
using sdk::SDK_RET_HW_SW_OO_SYNC;
using sdk::SDK_RET_IN_PROGRESS;
using sdk::SDK_RET_NOOP;
using sdk::SDK_RET_MAPPING_CONFLICT;
using sdk::sdk_status_t;

#endif    // __SDK_BASE_HPP__

