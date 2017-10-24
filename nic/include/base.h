// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#ifndef __BASE_H__
#define __BASE_H__

#include <stdint.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include "nic/include/trace.hpp"

#define __IN__
#define __OUT__
#define __INOUT__

#define TRUE                                         1
#define FALSE                                        0

#define __PACK__ __attribute__ ((packed))

// MAC address
#define ETH_ADDR_LEN                                 6
typedef uint8_t    mac_addr_t[ETH_ADDR_LEN];

#define VRF_BIT_WIDTH                                10
#define BD_BIT_WIDTH                                 10
#define FW_NORM_ACTION_BIT_WIDTH                     2

#define HAL_IFINDEX_INVALID                          0
#define HAL_HANDLE_INVALID                           0
#define HAL_TENANT_ID_INVALID                        0
#define HAL_L2SEGMENT_ID_INVALID                     0
#define HAL_PC_INVALID                               0xFF
#define HAL_PORT_INVALID                             0xFF

//------------------------------------------------------------------------------
// basic types used in the packet processing path
// WARNING: some of these types may have more bits than needed (like VRF, BD
// etc.), so be careful in using these as key fields of other lookup data
// structures 
//------------------------------------------------------------------------------
typedef uint64_t        hal_handle_t;
typedef uint16_t        etype_t;
typedef uint16_t        bd_id_t;       // TBD - revisit
typedef uint16_t        vlan_id_t;
typedef uint16_t        vrf_id_t;
typedef uint16_t        vif_id_t;
typedef uint32_t        rule_id_t;
typedef uint16_t        policer_id_t;
typedef uint16_t        vlan_id_t;
typedef uint16_t        lport_id_t;     // TBD - revisit
typedef uint32_t        ifindex_t;
typedef uint32_t        seg_id_t;
typedef uint32_t        l2seg_id_t;
typedef uint32_t        encap_id_t;
typedef uint32_t        oif_list_id_t;

//------------------------------------------------------------------------------
// HAL config version type
//------------------------------------------------------------------------------
typedef uint32_t cfg_version_t;
#define HAL_CFG_VER_NONE            0

#define __ASSERT__(x)            assert(x)

#define HAL_ABORT(cond)                                    \
do {                                                       \
    if (!(cond)) {                                         \
        abort();                                           \
    }                                                      \
} while (FALSE)

#define HAL_ASSERT_RETURN(cond, rv)                        \
do {                                                       \
    if (!(cond)) {                                         \
        HAL_TRACE_ERR("ASSERT FAILURE(" #cond ")");      \
        __ASSERT__(FALSE);                                 \
        return rv;                                         \
    }                                                      \
} while (FALSE)

#define HAL_ASSERT_RETURN_VOID(cond)                       \
do {                                                       \
    if (!(cond)) {                                         \
        HAL_TRACE_ERR("ASSERT FAILURE(" #cond ")");      \
        __ASSERT__(FALSE);                                 \
        return;                                            \
    }                                                      \
} while (FALSE)

#define HAL_ASSERT_GOTO(cond, label)                       \
do {                                                       \
    if (!(cond)) {                                         \
        HAL_TRACE_ERR("ASSERT FAILURE(" #cond ")");      \
        __ASSERT__(FALSE);                                 \
        goto label;                                        \
    }                                                      \
} while (FALSE)

#define HAL_ASSERT(cond)                             __ASSERT__(cond)
#define HAL_NOP                                      ((void) FALSE)

//-----------------------------------------------------------------------------
// X-Macro for defining enums (generates enum definition and string formatter)
//
// Example:
//
// #define SAMPLE_ENUM_ENTRIES(ENTRY)           
//    ENTRY(OK, 0, "ok")                        
//    ENTRY(ERR, 1, "err")
//
// ENUM_DEFINE(sample_enum_t, SAMPLE_ENUM_ENTRIES)
//------------------------------------------------------------------------------

#define _ENUM_FIELD(_name, _val, _desc) _name = _val,
#define _ENUM_CASE(_name, _val, _desc) case _name: return os << #_desc;

#define DEFINE_ENUM(_typ, _entries)                                     \
    typedef enum { _entries(_ENUM_FIELD) } _typ;                        \
    inline std::ostream& operator<<(std::ostream& os, _typ c)           \
    {                                                                   \
        switch (c) {                                                    \
            _entries(_ENUM_CASE);                                       \
        }                                                               \
        return os;                                                      \
    }


//------------------------------------------------------------------------------
// TODO - following are strictly temporary !!
//        use them when one off allocation is needed, otherwise use slabs
//        we can overload new operator later on
//------------------------------------------------------------------------------
#define HAL_MALLOC(type_id, size)        malloc(size)
#define HAL_CALLOC(type_id, size)        calloc(1, size)
#define HAL_MALLOCZ(type_id, size)       calloc(1, size)
#define HAL_FREE(type_id, ptr)           ::free(ptr)

//------------------------------------------------------------------------------
// TODO: we need atomic increment operations for ARM
// gnu gcc builin functions aren't availabe for ARM, ARM has its own library
//------------------------------------------------------------------------------
#define HAL_ATOMIC_INC_UINT32(ptr, val)     __atomic_add_fetch(ptr, val, __ATOMIC_SEQ_CST)
#define HAL_ATOMIC_DEC_UINT32(ptr, val)     __atomic_sub_fetch(ptr, val, __ATOMIC_SEQ_CST)
#define HAL_ATOMIC_STORE_UINT32(ptr, vptr)  __atomic_store(ptr, vptr, __ATOMIC_SEQ_CST)
#define HAL_ATOMIC_LOAD_UINT32(ptr, vptr)   __atomic_load(ptr, vptr, __ATOMIC_SEQ_CST)
#define HAL_ATOMIC_INC_UINT64(ptr, val)     __atomic_add_fetch(ptr, val, __ATOMIC_SEQ_CST)
#define HAL_ATOMIC_DEC_UINT64(ptr, val)     __atomic_sub_fetch(ptr, val, __ATOMIC_SEQ_CST)

#define HAL_ARRAY_SIZE(arr)                (sizeof((arr))/sizeof((arr)[0]))

//------------------------------------------------------------------------------
// HAL return codes
// NOTE: please number these enums properly for easier troubleshooting
//------------------------------------------------------------------------------
#define HAL_RET_ENTRIES(ENTRY)                                                 \
    ENTRY(HAL_RET_OK,                         0, "ok")                         \
    ENTRY(HAL_RET_OOM,                        1, "out of memory error")        \
    ENTRY(HAL_RET_INVALID_ARG,                2, "invalid arg")                \
    ENTRY(HAL_RET_INVALID_OP,                 3, "invalid operation")          \
    ENTRY(HAL_RET_ENTRY_NOT_FOUND,            4, "lookup failure")             \
    ENTRY(HAL_RET_ENTRY_EXISTS,               5, "already present")            \
    ENTRY(HAL_RET_NO_RESOURCE,                6, "resource exhaustion")        \
    ENTRY(HAL_RET_BATCH_FAIL,                 7, "batch processing failed")    \
    ENTRY(HAL_RET_BATCH_PARTIAL_FAIL,         8, "partial batch processing error") \
    ENTRY(HAL_RET_HW_FAIL,                    9, "read/write into hw failure") \
    ENTRY(HAL_RET_TABLE_FULL,                 10,"hw table full")              \
    ENTRY(HAL_RET_OTCAM_FULL,                 11,"otcam table full")           \
    ENTRY(HAL_RET_DUP_INS_FAIL,               12,"duplicate insert fail")      \
    ENTRY(HAL_RET_HW_KEY_BLD_FAIL,            13,"p4 key builder fail")        \
    ENTRY(HAL_RET_OOB,                        14,"out-of-bound error")         \
    ENTRY(HAL_RET_FLOW_COLL,                  15,"flow collision")             \
    ENTRY(HAL_RET_FLOW_OFLOW_FULL,            16,"flow oflow table full")      \
    ENTRY(HAL_RET_FLOW_LIMT_REACHED,          17,"allowed flow limit reached") \
    ENTRY(HAL_RET_DUP_FREE,                   18,"freeing multiple times")     \
    ENTRY(HAL_RET_TENANT_NOT_FOUND,           19,"tenant not found")           \
    ENTRY(HAL_RET_L2SEG_NOT_FOUND,            20,"L2 segment not found")       \
    ENTRY(HAL_RET_IF_NOT_FOUND,               21,"interface not found")        \
    ENTRY(HAL_RET_EP_NOT_FOUND,               22,"endpoint not found")         \
    ENTRY(HAL_RET_FLOW_NOT_FOUND,             23,"flow not found")             \
    ENTRY(HAL_RET_SESSION_NOT_FOUND,          24,"session not found")          \
    ENTRY(HAL_RET_HANDLE_INVALID,             25,"invalid hal handle")         \
    ENTRY(HAL_RET_LIF_NOT_FOUND,              26,"LIF not found")              \
    ENTRY(HAL_RET_HW_PROG_ERR,                27,"h/w programming error")      \
    ENTRY(HAL_RET_SECURITY_PROFILE_NOT_FOUND, 28, "qsecurity profile not found") \
    ENTRY(HAL_RET_TLS_CB_NOT_FOUND,           29, "TLS CB not found")          \
    ENTRY(HAL_RET_TCP_CB_NOT_FOUND,           30, "TCP CB not found")          \
    ENTRY(HAL_RET_QUEUE_NOT_FOUND,            31,"interface not found")        \
    ENTRY(HAL_RET_NETWORK_NOT_FOUND,          32,"network not found")          \
    ENTRY(HAL_RET_WRING_NOT_FOUND,            33, "Wring not found")           \
    ENTRY(HAL_RET_PROXY_NOT_FOUND,            34, "Proxy Service not found")   \
    ENTRY(HAL_RET_POLICER_NOT_FOUND,          35, "Policer not found")         \
    ENTRY(HAL_RET_OQUEUE_NOT_FOUND,           36, "Output queue not found")    \
    ENTRY(HAL_RET_IPSEC_CB_NOT_FOUND,         37, "IPSEC CB not found")        \
    ENTRY(HAL_RET_CPU_CB_NOT_FOUND,           38, "CPU CB not found")          \
    ENTRY(HAL_RET_RETRY,                      39, "retry the operation")       \
    ENTRY(HAL_RET_IF_ENIC_TYPE_INVALID,       40, "enic type invalid")         \
    ENTRY(HAL_RET_IF_ENIC_INFO_INVALID,       41, "enic info invalid")         \
    ENTRY(HAL_RET_IF_INFO_INVALID,            42, "if info invalid")           \
    ENTRY(HAL_RET_TENANT_ID_INVALID,          43, "tenant id invalid")         \
    ENTRY(HAL_RET_NWSEC_ID_INVALID,           44, "nwsec id invalid")          \
    ENTRY(HAL_RET_INTERFACE_ID_INVALID,       45, "interface id invalid")      \
    ENTRY(HAL_RET_DOS_POLICY_ID_INVALID,      46, "dos policy id invalid")     \
    ENTRY(HAL_RET_RAWR_CB_NOT_FOUND,          47, "Raw redirect CB not found") \
    ENTRY(HAL_RET_L2SEG_ID_INVALID,           48, "l2seg id invalid")          \
    ENTRY(HAL_RET_OBJECT_IN_USE,              49, "object in use")             \
    ENTRY(HAL_RET_SG_ID_INVALID,              50, "seg id invalid")            \
    ENTRY(HAL_RET_SG_NOT_FOUND,               51, "seg id not found")          \
    ENTRY(HAL_RET_NOOP,                       52, "Hal No operation")          \
    ENTRY(HAL_RET_RAWC_CB_NOT_FOUND,          53, "Raw redirect CB not found") \
                                                                               \
    ENTRY(HAL_RET_ERR,                        255, "catch all generic error")

DEFINE_ENUM(hal_ret_t, HAL_RET_ENTRIES)

#undef HAL_RET_ENTRIES

#define htonll(x) ((1==htonl(1)) ? (x) : ((uint64_t)htonl((x) & 0xFFFFFFFF) << 32) | htonl((x) >> 32))
#define ntohll(x) ((1==ntohl(1)) ? (x) : ((uint64_t)ntohl((x) & 0xFFFFFFFF) << 32) | ntohl((x) >> 32))

#endif    // __BASE_H__

