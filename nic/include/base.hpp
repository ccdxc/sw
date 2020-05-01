// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#ifndef __BASE_HPP__
#define __BASE_HPP__

#include <stdint.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include "nic/hal/hal_trace.hpp"
#include "nic/sdk/include/sdk/base.hpp"

#define __IN__
#define __OUT__
#define __INOUT__

#define TRUE                                         1
#define FALSE                                        0

#ifndef __PACK__
#define __PACK__ __attribute__ ((packed))
#endif

#define VRF_BIT_WIDTH                                10
#define BD_BIT_WIDTH                                 10
#define FW_NORM_ACTION_BIT_WIDTH                     2

#define HAL_IFINDEX_INVALID                          0
#define HAL_HANDLE_INVALID                           0
#define HAL_VRF_ID_INVALID                           0
#define HAL_L2SEGMENT_ID_INVALID                     0
#define HAL_PC_INVALID                               0xFF
#define HAL_PORT_INVALID                             0xFF
#define HAL_GFT_INVALID_PROFILE_ID                   0
#define HAL_GFT_INVALID_TABLE_ID                     0
#define HAL_GFT_INVALID_FLOW_ENTRY_ID                0

using sdk::sdk_ret_t;

//------------------------------------------------------------------------------
// basic types used in the packet processing path
// WARNING: some of these types may have more bits than needed (like VRF, BD
// etc.), so be careful in using these as key fields of other lookup data
// structures
//------------------------------------------------------------------------------
typedef uint64_t        hal_handle_t;
typedef uint64_t        l2seg_id_t;
typedef uint64_t        vrf_id_t;
typedef uint64_t        if_id_t;
typedef uint64_t        qid_t;
typedef uint64_t        policer_id_t;
typedef uint32_t        etype_t;
typedef uint32_t        vlan_id_t;
typedef uint32_t        oif_list_id_t;
typedef uint32_t        cpucb_id_t;
typedef uint32_t        nwsec_profile_id_t;
typedef uint32_t        tlscb_id_t;
typedef uint32_t        tls_proxy_cb_id_t;
typedef uint32_t        wring_id_t;
typedef uint32_t        dos_policy_id_t;
typedef uint32_t        acl_id_t;
typedef uint32_t        ipseccb_id_t;
typedef uint32_t        ipsec_sa_id_t;
typedef uint32_t        ipsec_rule_id_t;
typedef uint32_t        tcp_proxy_rule_id_t;
typedef uint32_t        lif_id_t;
typedef uint32_t        nwsec_policy_cfg_id_t;
typedef uint32_t        nwsec_group_id_t;
typedef uint32_t        lif_id_t;
typedef uint32_t        qtype_t;
typedef uint32_t        port_num_t;
typedef uint32_t        rawccb_id_t;
typedef uint32_t        queue_id_t;
typedef uint32_t        proxyrcb_id_t;
typedef uint32_t        proxyccb_id_t;
typedef uint32_t        rawrcb_id_t;
typedef uint32_t        mirror_session_id_t;
typedef uint32_t        tcpcb_id_t;
typedef uint32_t        session_id_t;
typedef uint32_t        app_session_id_t;
typedef uint32_t        appid_id_t;
typedef uint32_t        hal_stats_t;
typedef uint32_t        nat_pool_id_t;
typedef uint32_t        nh_id_t;
typedef uint32_t        sg_id_t;
typedef uint32_t        hal_timer_id_t;
typedef uint32_t        tcp_proxy_cb_id_t;
typedef uint32_t        nvme_sesscb_id_t;
typedef uint32_t        nvme_ns_id_t;
typedef uint32_t        nvme_sq_id_t;
typedef uint32_t        nvme_cq_id_t;

#define HAL_ABORT(cond)                                    \
do {                                                       \
    if (unlikely(!(cond))) {                               \
        abort();                                           \
    }                                                      \
} while (FALSE)

#define HAL_ABORT_TRACE(cond, args...)                     \
do {                                                       \
    if (unlikely(!(cond))) {                               \
        HAL_TRACE_ERR(args);                               \
        abort();                                           \
    }                                                      \
} while (FALSE)

#define HAL_EXIT(cond)                                     \
do {                                                       \
    if (unlikely(!(cond))) {                               \
        exit(1);                                           \
    }                                                      \
} while (FALSE)

#define HAL_EXIT_TRACE(cond, args...)                      \
do {                                                       \
    if (unlikely(!(cond))) {                               \
        HAL_TRACE_ERR(args);                               \
        std::terminate();                                  \
    }                                                      \
} while (FALSE)

#define HAL_NOP                                      ((void) FALSE)
#define HAL_GET_SYSTEM_CLOCK(_val)   \
    clock_gettime(CLOCK_MONOTONIC, _val);

//-----------------------------------------------------------------------------
// X-Macro for defining enums (generates enum definition and string formatter)
//
// Example:
//
// #define SAMPLE_ENUM_ENTRIES(ENTRY)
//    ENTRY(OK, 0, "ok")
//    ENTRY(ERR, 1, "err")
//
// DEFINE_ENUM(sample_enum_t, SAMPLE_ENUM_ENTRIES)
//------------------------------------------------------------------------------

#define _ENUM_FIELD(_name, _val, _desc) _name = _val,
#define _ENUM_STR(_name, _val, _desc) \
    inline const char *_name ## _str() {return _desc; }
#define _ENUM_CASE(_name, _val, _desc) case _name: return os << #_desc;
#define _ENUM_CASE_STR(_name, _val, _desc) case _name: return #_name;

#define DEFINE_ENUM(_typ, _entries)                                     \
    typedef enum { _entries(_ENUM_FIELD) } _typ;                        \
    inline std::ostream& operator<<(std::ostream& os, _typ c)           \
    {                                                                   \
        switch (c) {                                                    \
            _entries(_ENUM_CASE);                                       \
        }                                                               \
        return os;                                                      \
    }                                                                   \
    _entries(_ENUM_STR)                                                 \

#define DEFINE_ENUM_TO_STR(_typ, _entries)                              \
    inline const char *_entries##_str(_typ c)                           \
    {                                                                   \
        switch (c) {                                                    \
            _entries(_ENUM_CASE_STR);                                   \
        }                                                               \
        return "unknown";                                                \
    }                                                                   \

//------------------------------------------------------------------------------
// HAL return codes
// NOTE: please number these enums properly for easier troubleshooting
//------------------------------------------------------------------------------
#define HAL_RET_ENTRIES(ENTRY)                                                          \
    ENTRY(HAL_RET_OK,                         0,   "ok")                                \
    ENTRY(HAL_RET_OOM,                        1,   "out of memory error")               \
    ENTRY(HAL_RET_INVALID_ARG,                2,   "invalid arg")                       \
    ENTRY(HAL_RET_INVALID_OP,                 3,   "invalid operation")                 \
    ENTRY(HAL_RET_ENTRY_NOT_FOUND,            4,   "lookup failure")                    \
    ENTRY(HAL_RET_ENTRY_EXISTS,               5,   "already present")                   \
    ENTRY(HAL_RET_NO_RESOURCE,                6,   "resource exhaustion")               \
    ENTRY(HAL_RET_BATCH_FAIL,                 7,   "batch processing failed")           \
    ENTRY(HAL_RET_BATCH_PARTIAL_FAIL,         8,   "partial batch processing error")    \
    ENTRY(HAL_RET_HW_FAIL,                    9,   "read/write into hw failure")        \
    ENTRY(HAL_RET_TABLE_FULL,                 10,  "hw table full")                     \
    ENTRY(HAL_RET_OTCAM_FULL,                 11,  "otcam table full")                  \
    ENTRY(HAL_RET_DUP_INS_FAIL_DEPRECATED,    12,  "deprecated: duplicate insert fail") \
    ENTRY(HAL_RET_HW_KEY_BLD_FAIL,            13,  "p4 key builder fail")               \
    ENTRY(HAL_RET_OOB,                        14,  "out-of-bound error")                \
    ENTRY(HAL_RET_COLLISION,                  15,  "collision detected")                \
    ENTRY(HAL_RET_FLOW_OFLOW_FULL,            16,  "flow oflow table full")             \
    ENTRY(HAL_RET_FLOW_LIMT_REACHED,          17,  "allowed flow limit reached")        \
    ENTRY(HAL_RET_DUP_FREE,                   18,  "freeing multiple times")            \
    ENTRY(HAL_RET_VRF_NOT_FOUND,              19,  "vrf not found")                     \
    ENTRY(HAL_RET_L2SEG_NOT_FOUND,            20,  "L2 segment not found")              \
    ENTRY(HAL_RET_IF_NOT_FOUND,               21,  "interface not found")               \
    ENTRY(HAL_RET_EP_NOT_FOUND,               22,  "endpoint not found")                \
    ENTRY(HAL_RET_FLOW_NOT_FOUND,             23,  "flow not found")                    \
    ENTRY(HAL_RET_SESSION_NOT_FOUND,          24,  "session not found")                 \
    ENTRY(HAL_RET_HANDLE_INVALID,             25,  "invalid hal handle")                \
    ENTRY(HAL_RET_LIF_NOT_FOUND,              26,  "LIF not found")                     \
    ENTRY(HAL_RET_HW_PROG_ERR,                27,  "h/w programming error")             \
    ENTRY(HAL_RET_SECURITY_PROFILE_NOT_FOUND, 28,  "security profile not found")        \
    ENTRY(HAL_RET_TLS_CB_NOT_FOUND,           29,  "TLS CB not found")                  \
    ENTRY(HAL_RET_TCP_CB_NOT_FOUND,           30,  "TCP CB not found")                  \
    ENTRY(HAL_RET_QUEUE_NOT_FOUND,            31,  "interface not found")               \
    ENTRY(HAL_RET_NETWORK_NOT_FOUND,          32,  "network not found")                 \
    ENTRY(HAL_RET_WRING_NOT_FOUND,            33,  "Wring not found")                   \
    ENTRY(HAL_RET_PROXY_NOT_FOUND,            34,  "Proxy Service not found")           \
    ENTRY(HAL_RET_QOS_CLASS_NOT_FOUND,        35,  "Qos class not found")               \
    ENTRY(HAL_RET_IPSEC_CB_NOT_FOUND,         36,  "IPSEC CB not found")                \
    ENTRY(HAL_RET_CPU_CB_NOT_FOUND,           37,  "CPU CB not found")                  \
    ENTRY(HAL_RET_RETRY,                      38,  "retry the operation")               \
    ENTRY(HAL_RET_IF_ENIC_TYPE_INVALID,       39,  "enic type invalid")                 \
    ENTRY(HAL_RET_IF_ENIC_INFO_INVALID,       40,  "enic info invalid")                 \
    ENTRY(HAL_RET_IF_INFO_INVALID,            41,  "if info invalid")                   \
    ENTRY(HAL_RET_VRF_ID_INVALID,             42,  "vrf id invalid")                    \
    ENTRY(HAL_RET_NWSEC_ID_INVALID,           43,  "nwsec id invalid")                  \
    ENTRY(HAL_RET_INTERFACE_ID_INVALID,       44,  "interface id invalid")              \
    ENTRY(HAL_RET_DOS_POLICY_ID_INVALID,      45,  "dos policy id invalid")             \
    ENTRY(HAL_RET_RAWR_CB_NOT_FOUND,          46,  "Raw redirect CB not found")         \
    ENTRY(HAL_RET_L2SEG_ID_INVALID,           47,  "l2seg id invalid")                  \
    ENTRY(HAL_RET_OBJECT_IN_USE,              48,  "object in use")                     \
    ENTRY(HAL_RET_SG_ID_INVALID,              49,  "seg id invalid")                    \
    ENTRY(HAL_RET_SG_NOT_FOUND,               50,  "seg id not found")                  \
    ENTRY(HAL_RET_NOOP,                       51,  "Hal No operation")                  \
    ENTRY(HAL_RET_RAWC_CB_NOT_FOUND,          52,  "Raw redirect CB not found")         \
    ENTRY(HAL_RET_FTE_RULE_NO_MATCH,          53,  "Firewall rule no match")            \
    ENTRY(HAL_RET_APP_REDIR_FORMAT_UNKNOWN,   54,  "App redir hdr fmt unknown")         \
    ENTRY(HAL_RET_APP_REDIR_HDR_LEN_ERR,      55,  "App redir hdr_len error")           \
    ENTRY(HAL_RET_APP_REDIR_HDR_ERR,          56,  "App redir hdr error")               \
    ENTRY(HAL_RET_PORT_NOT_FOUND,             57,  "port not found ")                   \
    ENTRY(HAL_RET_NW_HANDLE_NOT_FOUND,        58,  "nw handle not found")               \
    ENTRY(HAL_RET_EP_HANDLE_NOT_FOUND,        59,  "ep handle not found")               \
    ENTRY(HAL_RET_EXISTING_PROXY_SESSION,     60,  "Existing proxy session")            \
    ENTRY(HAL_RET_PROXYR_CB_NOT_FOUND,        61,  "Proxy redirect CB not found")       \
    ENTRY(HAL_RET_PROXYC_CB_NOT_FOUND,        62,  "Proxy chain CB not found")          \
    ENTRY(HAL_RET_FLOW_LKUP_ID_NOT_FOUND,     63,  "flow lookup id not found")          \
    ENTRY(HAL_RET_ACL_NOT_FOUND,              64,  "acl not found")                     \
    ENTRY(HAL_RET_COPP_NOT_FOUND,             65,  "copp not found")                    \
    ENTRY(HAL_RET_OPENSSL_ENGINE_NOT_FOUND,   66,  "openssl engine not found")          \
    ENTRY(HAL_RET_SSL_CERT_KEY_ADD_ERR,       67,  "failed to add cert/key to SSL")     \
    ENTRY(HAL_RET_KEY_HANDLE_NOT_SPECIFIED,   68,  "key and handle not set")            \
    ENTRY(HAL_RET_CERT_NOT_FOUND,             69,  "Certificate not found")             \
    ENTRY(HAL_RET_SECURITY_POLICY_NOT_FOUND,  70,  "Security policy not found")         \
    ENTRY(HAL_RET_NEXTHOP_NOT_FOUND,          71,  "Nexthop not found")                 \
    ENTRY(HAL_RET_NAT_POOL_NOT_FOUND,         72,  "NAT pool not found")                \
    ENTRY(HAL_RET_NOT_SUPPORTED,              73,  "Feature not supported")             \
    ENTRY(HAL_RET_ROUTE_NOT_FOUND,            74,  "Route not found")                   \
    ENTRY(HAL_RET_NAT_MAPPING_NOT_FOUND,      75,  "NAT mapping not found")             \
    ENTRY(HAL_RET_IPSEC_ALGO_NOT_SUPPORTED,   76,  "IPSec Algo not supported")          \
    ENTRY(HAL_RET_NAT_POLICY_NOT_FOUND,       77,  "NAT policy not found")              \
    ENTRY(HAL_RET_IPSEC_RULE_NOT_FOUND,       78,  "IPSec rule not found")              \
    ENTRY(HAL_RET_ASYNC,                      79,  "Async operation in progress")       \
    ENTRY(HAL_RET_TCP_PROXY_RULE_NOT_FOUND,   80,  "Tcp Proxy rule not found")          \
    ENTRY(HAL_RET_FILTER_NOT_FOUND,           81,  "filter not found")                  \
    ENTRY(HAL_RET_INVALID_CTRL_SESSION_OP,    82,  "Invalid op on ALG control session") \
    ENTRY(HAL_RET_FTE_SPAN,                   83,  "fte span packet")                   \
    ENTRY(HAL_RET_SNAKE_TEST_NOT_FOUND,       84,  "snake test not found ")             \
    ENTRY(HAL_RET_QOS_CLASS_MTU_INVALID,                        85, "qos class mtu invalid")                        \
    ENTRY(HAL_RET_QOS_CLASS_XON_THRESHOLD_INVALID,              86, "qos class xon threshold invalid")              \
    ENTRY(HAL_RET_QOS_CLASS_XOFF_THRESHOLD_INVALID,             87, "qos class xoff threshold invalid")             \
    ENTRY(HAL_RET_QOS_CLASS_SCHEDULER_NOT_SET,                  88, "qos class scheduler not set")                  \
    ENTRY(HAL_RET_QOS_CLASS_DWRR_INVALID,                       89, "qos class dwrr invalid")                       \
    ENTRY(HAL_RET_QOS_CLASS_QOS_GROUP_INVALID,                  90, "qos class qos group invalid")                  \
    ENTRY(HAL_RET_QOS_CLASS_UPLINK_CLASS_MAP_NOT_SET,           91, "qos class uplink class map not set")           \
    ENTRY(HAL_RET_QOS_CLASS_UPLINK_CLASS_MAP_SET,               92, "qos class uplink class map set")               \
    ENTRY(HAL_RET_QOS_CLASS_DOT1Q_PCP_INVALID,                  93, "qos class dot1q pcp invalid")                  \
    ENTRY(HAL_RET_QOS_CLASS_IP_DSCP_INVALID,                    94, "qos class ip dscp invalid")                    \
    ENTRY(HAL_RET_QOS_CLASS_DOT1Q_PCP_MARKING_INVALID,          95, "qos class dot1q pcp marking invalid")          \
    ENTRY(HAL_RET_QOS_CLASS_IP_DSCP_MARKING_INVALID,            96, "qos class ip dscp marking invalid")            \
    ENTRY(HAL_RET_QOS_CLASS_DOT1Q_PCP_ALREADY_IN_USE,           97, "qos class dot1q pcp already in use")           \
    ENTRY(HAL_RET_QOS_CLASS_IP_DSCP_ALREADY_IN_USE,             98, "qos class ip dscp already in use")             \
    ENTRY(HAL_RET_QOS_CLASS_DROP_NO_DROP_CHANGE_NOT_ALLOWED,    99, "qos class drop no-drop change not allowed")    \
    ENTRY(HAL_RET_QOS_CLASS_PAUSE_TYPE_INVALID,                 100, "qos class invalid pause type")                \
    ENTRY(HAL_RET_RETRANSMISSION,                               101,"retransmission detected in flow miss path")    \
    ENTRY(HAL_RET_FTE_SRC_LIF_MISMATCH,                         102,"src lif mismatch detected in fte")             \
    ENTRY(HAL_RET_POLICY_EXIST,                                 103, "security policy exists")                      \
    ENTRY(HAL_RET_CONN_CLOSED,                                  104, "connection closed")                           \
    ENTRY(HAL_RET_BOND0_NOT_RESOLVED,                           105, "Bond0 not resoloved for Netflow collector")   \
    ENTRY(HAL_RET_ERR,                                          255, "catch all generic error")

DEFINE_ENUM(hal_ret_t, HAL_RET_ENTRIES)
DEFINE_ENUM_TO_STR(hal_ret_t, HAL_RET_ENTRIES)

#undef HAL_RET_ENTRIES

#define BW(x, curr_x, interval)     \
    ((x == curr_x) ? 0 :            \
    (((curr_x > x) ? (curr_x - x) : ((UINT64_MAX-x) + curr_x)) / interval))


#define htonll(x) ((1==htonl(1)) ? (x) : ((uint64_t)htonl((x) & 0xFFFFFFFF) << 32) | htonl((x) >> 32))
#define ntohll(x) ((1==ntohl(1)) ? (x) : ((uint64_t)ntohl((x) & 0xFFFFFFFF) << 32) | ntohl((x) >> 32))

static inline hal_ret_t
hal_sdk_ret_to_hal_ret(sdk_ret_t sdk_ret)
{
    switch (sdk_ret) {
    case sdk::SDK_RET_OK:
        return HAL_RET_OK;
    case sdk::SDK_RET_OOM:
        return HAL_RET_OOM;
    case sdk::SDK_RET_INVALID_ARG:
        return HAL_RET_INVALID_ARG;
    case sdk::SDK_RET_INVALID_OP:
        return HAL_RET_INVALID_OP;
    case sdk::SDK_RET_ENTRY_NOT_FOUND:
        return HAL_RET_ENTRY_NOT_FOUND;
    case sdk::SDK_RET_ENTRY_EXISTS:
        return HAL_RET_ENTRY_EXISTS;
    case sdk::SDK_RET_NO_RESOURCE:
        return HAL_RET_NO_RESOURCE;
    case sdk::SDK_RET_TABLE_FULL:
        return HAL_RET_TABLE_FULL;
    case sdk::SDK_RET_OOB:
        return HAL_RET_OOB;
    case sdk::SDK_RET_HW_PROGRAM_ERR:
        return HAL_RET_HW_PROG_ERR;
    case sdk::SDK_RET_RETRY:
        return HAL_RET_RETRY;
    case sdk::SDK_RET_NOOP:
        return HAL_RET_NOOP;
    case sdk::SDK_RET_DUPLICATE_FREE:
        return HAL_RET_DUP_FREE;
    case sdk::SDK_RET_COLLISION:
        return HAL_RET_COLLISION;
    case sdk::SDK_RET_ERR:
    default:
        return HAL_RET_ERR;
    }
}

#endif    // __BASE_HPP__

