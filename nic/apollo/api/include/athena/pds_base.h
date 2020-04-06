//------------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//
// base PDS header file that goes into rest of the PDS
//------------------------------------------------------------------------------

#ifndef __PDS_BASE_H__
#define __PDS_BASE_H__

#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>

#define ETH_ADDR_LEN 6
#define IP6_ADDR8_LEN 16

#ifndef __PACK__
#define __PACK__            __attribute__((packed))
#endif


#define _PDS_ENUM_FIELD(_name, _val, _desc) _name = _val,
#define _PDS_ENUM_STR(_name, _val, _desc)                               \
            inline const char *_name ## _str() {return _desc; }
#define _PDS_ENUM_CASE_STR(_name, _val, _desc) case _name: return #_name;

#define PDS_DEFINE_ENUM(_type, _entries)                                \
    typedef enum { _entries(_PDS_ENUM_FIELD) } _type;                   \
    _entries(_PDS_ENUM_STR)

#define PDS_DEFINE_ENUM_TO_STR(_typ, _entries)                          \
    inline const char *_entries##_str(_typ c)                           \
    {                                                                   \
        switch (c) {                                                    \
            _entries(_PDS_ENUM_CASE_STR);                               \
        }                                                               \
        return "uknown";                                                \
    }

#define PDS_RET_ENTRIES(ENTRY)                                                 \
    ENTRY(PDS_RET_OK,                         0,    "ok")                      \
    ENTRY(PDS_RET_OOM,                        1,    "out of memory error")     \
    ENTRY(PDS_RET_INVALID_ARG,                2,    "invalid arg")             \
    ENTRY(PDS_RET_INVALID_OP,                 3,    "invalid operation")       \
    ENTRY(PDS_RET_ENTRY_NOT_FOUND,            4,    "lookup failure")          \
    ENTRY(PDS_RET_ENTRY_EXISTS,               5,    "already present")         \
    ENTRY(PDS_RET_NO_RESOURCE,                6,    "resource exhaustion")     \
    ENTRY(PDS_RET_TABLE_FULL,                 7,    "hw table full")           \
    ENTRY(PDS_RET_OOB,                        8,    "out-of-bound error")      \
    ENTRY(PDS_RET_HW_PROGRAM_ERR,             9,    "h/w programming error")   \
    ENTRY(PDS_RET_RETRY,                      10,   "retry the operation")     \
    ENTRY(PDS_RET_NOOP,                       11,   "No operation performed")  \
    ENTRY(PDS_RET_DUPLICATE_FREE,             12,   "duplicate free")          \
    ENTRY(PDS_RET_COLLISION,                  13,   "collision detected")      \
    ENTRY(PDS_RET_MAX_RECIRC_EXCEED,          14,   "maximum recirc exceeded") \
    ENTRY(PDS_RET_HW_READ_ERR,                15,   "h/w read error")          \
    ENTRY(PDS_RET_TXN_NOT_FOUND,              16,   "transaction not found")   \
    ENTRY(PDS_RET_TXN_EXISTS,                 17,   "transaction exists")      \
    ENTRY(PDS_RET_TXN_INCOMPLETE,             18,   "transaction incomplete")  \
    ENTRY(PDS_RET_COMM_FAIL,                  19,   "communication failure")   \
    ENTRY(PDS_RET_HW_SW_OO_SYNC,              20,   "h/w, s/w out of sync")    \
    ENTRY(PDS_RET_OBJ_CLONE_ERR,              21,   "object clone failure")    \
    ENTRY(PDS_RET_IN_PROGRESS,                22,   "operation in progress")   \
    ENTRY(PDS_RET_UPG_CRITICAL,               23,   "upgrade critcal failure") \
    ENTRY(PDS_RET_MAX,                        24,   "max return value")        \
    ENTRY(PDS_RET_ERR,                        255,  "catch all generic error") 

PDS_DEFINE_ENUM(pds_ret_t, PDS_RET_ENTRIES)
PDS_DEFINE_ENUM_TO_STR(pds_ret_t, PDS_RET_ENTRIES)

#undef PDS_RET_ENTRIES




#endif


