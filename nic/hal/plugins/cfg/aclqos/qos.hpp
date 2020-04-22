//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#ifndef __QOS_HPP__
#define __QOS_HPP__

#include "nic/include/base.hpp"
#include "lib/ht/ht.hpp"
#include "lib/bitmap/bitmap.hpp"
#include "gen/proto/qos.pb.h"
#include "nic/include/pd.hpp"
#include "nic/sdk/include/sdk/lock.hpp"
#include "nic/utils/block_list/block_list.hpp"

using sdk::lib::ht_ctxt_t;
using hal::utils::block_list;

using kh::QosClassKeyHandle;
using qos::QosClassSpec;
using qos::QosClassStatus;
using qos::QosClassResponse;
using qos::QosClassRequestMsg;
using qos::QosClassResponseMsg;
using qos::QosClassDeleteRequest;
using qos::QosClassDeleteResponse;
using qos::QosClassDeleteRequestMsg;
using qos::QosClassDeleteResponseMsg;
using qos::QosClassSetGlobalPauseTypeRequest;
using qos::QosClassSetGlobalPauseTypeRequestMsg;
using qos::QosClassSetGlobalPauseTypeResponseMsg;
using qos::QosClassGetRequest;
using qos::QosClassGetRequestMsg;
using qos::QosClassGetResponse;
using qos::QosClassGetResponseMsg;
using qos::QosClassThresholdsGetRequest;
using qos::QosClassThresholdsGetRequestMsg;
using qos::QosClassThresholdsGetResponse;
using qos::QosClassThresholdsGetResponseMsg;
using qos::QosClassStats;
using kh::CoppKeyHandle;
using qos::CoppSpec;
using qos::CoppStatus;
using qos::CoppResponse;
using qos::CoppRequestMsg;
using qos::CoppResponseMsg;
using qos::CoppDeleteRequest;
using qos::CoppDeleteResponse;
using qos::CoppGetRequest;
using qos::CoppGetRequestMsg;
using qos::CoppGetResponse;
using qos::CoppGetResponseMsg;
using qos::CoppStats;

namespace hal {

#define HAL_MAX_DOT1Q_PCP_VALS 8
#define HAL_MAX_IP_DSCP_VALS   64
// Maximum pfc cos values
#define HAL_MAX_PFC_COS_VALS HAL_MAX_DOT1Q_PCP_VALS
#define HAL_MIN_MTU            1500
#define HAL_JUMBO_MTU          9216
#define QOS_DEFAULT_CPU_BPS    10000

#define QOS_GROUPS(ENTRY)                                       \
    ENTRY(QOS_GROUP_DEFAULT,             0, "default")          \
    ENTRY(QOS_GROUP_USER_DEFINED_1,      1, "user-defined-1")   \
    ENTRY(QOS_GROUP_USER_DEFINED_2,      2, "user-defined-2")   \
    ENTRY(QOS_GROUP_USER_DEFINED_3,      3, "user-defined-3")   \
    ENTRY(QOS_GROUP_USER_DEFINED_4,      4, "user-defined-4")   \
    ENTRY(QOS_GROUP_USER_DEFINED_5,      5, "user-defined-5")   \
    ENTRY(QOS_GROUP_USER_DEFINED_6,      6, "user-defined-6")   \
    ENTRY(QOS_GROUP_CONTROL,             7, "control")          \
    ENTRY(QOS_GROUP_SPAN,                8, "span")             \
    ENTRY(QOS_GROUP_RX_PROXY_NO_DROP,    9, "rx-proxy-no-drop") \
    ENTRY(QOS_GROUP_RX_PROXY_DROP,      10, "rx-proxy-drop")    \
    ENTRY(QOS_GROUP_TX_PROXY_NO_DROP,   11, "tx-proxy-no-drop") \
    ENTRY(QOS_GROUP_TX_PROXY_DROP,      12, "tx-proxy-drop")    \
    ENTRY(QOS_GROUP_CPU_COPY,           13, "cpu-copy")         \
    ENTRY(NUM_QOS_GROUPS,               14, "num-qos-groups")

DEFINE_ENUM(qos_group_t, QOS_GROUPS)
#undef QOS_GROUPS

#define HAL_MAX_QOS_CLASSES NUM_QOS_GROUPS

typedef struct qos_class_key_s {
    qos_group_t qos_group;
} __PACK__ qos_class_key_t;

inline std::ostream& operator<<(std::ostream& os, const qos_class_key_t& s)
{
   return os << fmt::format("{{qos_group={}}}", s.qos_group);
}

typedef struct qos_pause_s {
    uint32_t xon_threshold;       // Threshold at which to send xon
    uint32_t xoff_threshold;      // Free buffer threshold at which to send xoff
    bool     pfc_enable;          // PFC or link-level pause
    uint32_t pfc_cos;             // PFC cos
} __PACK__   qos_pause_t;

#define QOS_SCHED_TYPES(ENTRY)                       \
    ENTRY(QOS_SCHED_TYPE_DWRR,          0, "dwrr")   \
    ENTRY(QOS_SCHED_TYPE_STRICT,        1, "strict")

DEFINE_ENUM(qos_sched_type_e, QOS_SCHED_TYPES);
#undef QOS_SCHED_TYPES

typedef enum qos_pause_type_e {
    QOS_PAUSE_TYPE_NONE,
    QOS_PAUSE_TYPE_LINK_LEVEL,
    QOS_PAUSE_TYPE_PFC
} qos_pause_type_t;

typedef struct qos_sched_s {
    qos_sched_type_e type;
    union {
        struct {
            uint32_t  bw;                // Bandwidth percentage for DWRR
        } dwrr;
        struct {
            uint64_t  bps;               // Rate for strict priority scheduling
        } strict;
    } __PACK__;
} __PACK__ qos_sched_t;

#define QOS_CMAP_TYPES(ENTRY)                     \
    ENTRY(QOS_CMAP_TYPE_NONE,         0, "none")  \
    ENTRY(QOS_CMAP_TYPE_PCP,          1, "pcp")   \
    ENTRY(QOS_CMAP_TYPE_DSCP,         2, "dscp")  \
    ENTRY(QOS_CMAP_TYPE_PCP_DSCP,     3, "pcp-dscp")

DEFINE_ENUM(qos_cmap_type_e, QOS_CMAP_TYPES);
#undef QOS_CMAP_TYPES

typedef struct qos_cmap_s {
    qos_cmap_type_e type;
    uint32_t        dot1q_pcp;
    bool            ip_dscp[HAL_MAX_IP_DSCP_VALS];
} __PACK__ qos_cmap_t;

typedef struct qos_marking_action_s {
    bool     pcp_rewrite_en;     // Rewrite the 802.1q pcp value
    uint32_t pcp;                // 802.1q pcp value to mark with
    bool     dscp_rewrite_en;    // Rewrite the dscp value
    uint32_t dscp;               // DSCP value to mark with
} __PACK__ qos_marking_action_t;

// QosClass structure
typedef struct qos_class_s {
    sdk_spinlock_t       slock;          // lock to protect this structure
    qos_class_key_t      key;            // QOS group information

    uint32_t             mtu;            // MTU of the packets in bytes
    qos_pause_t          pause;          // pause configuration
    bool                 no_drop;        // No drop class
    qos_sched_t          sched;          // scheduler configuration
    qos_cmap_t           cmap;           // Uplink class map
    qos_marking_action_t marking;

                                         // operational state of qos-class
    hal_handle_t         hal_handle;     // HAL allocated handle

    // Back references
    block_list           *lif_list_rx;   // Lif list, rx
    block_list           *lif_list_tx;   // Lif list, tx

    pd::pd_qos_class_t   *pd;
} __PACK__ qos_class_t;

// cb data structures
typedef struct qos_class_create_app_ctxt_s {
} __PACK__ qos_class_create_app_ctxt_t;

typedef struct qos_class_update_app_ctxt_s {
    bool mtu_changed;
    bool threshold_changed;
    bool dot1q_pcp_changed;
    uint32_t dot1q_pcp_remove;
    bool ip_dscp_changed;
    bool ip_dscp_remove[HAL_MAX_IP_DSCP_VALS];
    bool pfc_changed;
    bool scheduler_changed;
    bool marking_changed;
    bool pfc_cos_changed;
} __PACK__ qos_class_update_app_ctxt_t;

static inline bool
valid_qos_group (qos_group_t qos_group)
{
    return qos_group < NUM_QOS_GROUPS;
}

static inline kh::QosGroup
qos_group_to_qos_spec_qos_group (qos_group_t qos_group)
{
    switch(qos_group) {
    case QOS_GROUP_DEFAULT:
        return kh::DEFAULT;
    case QOS_GROUP_USER_DEFINED_1:
        return kh::USER_DEFINED_1;
    case QOS_GROUP_USER_DEFINED_2:
        return kh::USER_DEFINED_2;
    case QOS_GROUP_USER_DEFINED_3:
        return kh::USER_DEFINED_3;
    case QOS_GROUP_USER_DEFINED_4:
        return kh::USER_DEFINED_4;
    case QOS_GROUP_USER_DEFINED_5:
        return kh::USER_DEFINED_5;
    case QOS_GROUP_USER_DEFINED_6:
        return kh::USER_DEFINED_6;
    case QOS_GROUP_CONTROL:
        return kh::CONTROL;
    case QOS_GROUP_SPAN:
        return kh::SPAN;
    case QOS_GROUP_RX_PROXY_NO_DROP:
        return kh::INTERNAL_RX_PROXY_NO_DROP;
    case QOS_GROUP_RX_PROXY_DROP:
        return kh::INTERNAL_RX_PROXY_DROP;
    case QOS_GROUP_TX_PROXY_NO_DROP:
        return kh::INTERNAL_TX_PROXY_NO_DROP;
    case QOS_GROUP_TX_PROXY_DROP:
        return kh::INTERNAL_TX_PROXY_DROP;
    case QOS_GROUP_CPU_COPY:
        return kh::INTERNAL_CPU_COPY;
    default:
        HAL_TRACE_ERR("pi-qos:{}: Invalid qos group {}",
                      __func__, qos_group);
        return kh::DEFAULT;
    }
}

static inline qos_group_t
qos_spec_qos_group_to_qos_group (kh::QosGroup qos_group)
{
    switch(qos_group) {
    case kh::DEFAULT:
        return QOS_GROUP_DEFAULT;
    case kh::USER_DEFINED_1:
        return QOS_GROUP_USER_DEFINED_1;
    case kh::USER_DEFINED_2:
        return QOS_GROUP_USER_DEFINED_2;
    case kh::USER_DEFINED_3:
        return QOS_GROUP_USER_DEFINED_3;
    case kh::USER_DEFINED_4:
        return QOS_GROUP_USER_DEFINED_4;
    case kh::USER_DEFINED_5:
        return QOS_GROUP_USER_DEFINED_5;
    case kh::USER_DEFINED_6:
        return QOS_GROUP_USER_DEFINED_6;
    case kh::CONTROL:
        return QOS_GROUP_CONTROL;
    case kh::SPAN:
        return QOS_GROUP_SPAN;
    case kh::INTERNAL_RX_PROXY_NO_DROP:
        return QOS_GROUP_RX_PROXY_NO_DROP;
    case kh::INTERNAL_RX_PROXY_DROP:
        return QOS_GROUP_RX_PROXY_DROP;
    case kh::INTERNAL_TX_PROXY_NO_DROP:
        return QOS_GROUP_TX_PROXY_NO_DROP;
    case kh::INTERNAL_TX_PROXY_DROP:
        return QOS_GROUP_TX_PROXY_DROP;
    case kh::INTERNAL_CPU_COPY:
        return QOS_GROUP_CPU_COPY;
    default:
        HAL_TRACE_ERR("pi-qos:{}: Invalid qos group {}",
                      __func__, qos_group);
        return NUM_QOS_GROUPS;
    }
}

extern void *qos_class_get_key_func(void *entry);
extern uint32_t qos_class_key_size(void);
qos_class_t *find_qos_class_by_group(qos_group_t qos_group);
qos_class_t *find_qos_class_by_handle(hal_handle_t handle);
qos_class_t *find_qos_class_by_key_handle(const QosClassKeyHandle& kh);

static inline bool
qos_group_is_user_defined (qos_group_t qos_group)
{
    switch(qos_group) {
        case QOS_GROUP_USER_DEFINED_1:
        case QOS_GROUP_USER_DEFINED_2:
        case QOS_GROUP_USER_DEFINED_3:
        case QOS_GROUP_USER_DEFINED_4:
        case QOS_GROUP_USER_DEFINED_5:
        case QOS_GROUP_USER_DEFINED_6:
            return true;
        default:
            return false;
    }
    return false;
}

static inline bool
qos_class_is_user_defined (qos_class_t *qos_class)
{
    return qos_group_is_user_defined(qos_class->key.qos_group);
}

// SVC CRUD APIs
hal_ret_t qosclass_create(qos::QosClassSpec& spec,
                          qos::QosClassResponse *rsp);
hal_ret_t qosclass_update(qos::QosClassSpec& spec,
                          qos::QosClassResponse *rsp);
hal_ret_t qosclass_delete(qos::QosClassDeleteRequest& req,
                          qos::QosClassDeleteResponse *rsp);
hal_ret_t qosclass_get(qos::QosClassGetRequest& req,
                       qos::QosClassGetResponseMsg *rsp);
hal_ret_t qos_class_thresholds_get(qos::QosClassThresholdsGetRequest& req,
                                   qos::QosClassThresholdsGetResponseMsg *rsp);
hal_ret_t qos_class_set_global_pause_type(
                        qos::QosClassSetGlobalPauseTypeRequest& req,
                        qos::QosClassSetGlobalPauseTypeResponseMsg *rsp);
hal_ret_t qos_class_init_tc_to_iq_map(void);
hal_ret_t qos_class_store_cb(void *obj, uint8_t *mem,
                             uint32_t len, uint32_t *mlen);
uint32_t qos_class_restore_cb(void *obj, uint32_t len);

// SWM related APIs
hal_ret_t qos_swm_queue_init(uint32_t swm_uplink_port, uint64_t dmac);
hal_ret_t qos_swm_queue_deinit(uint32_t swm_uplink_port);
uint32_t qos_get_span_tm_oq(void);


#define POLICER_TYPES(ENTRY) \
    ENTRY(POLICER_TYPE_PPS,                 0, "pps") \
    ENTRY(POLICER_TYPE_BPS,                 1, "bps")

DEFINE_ENUM(policer_type_t, POLICER_TYPES)
#undef POLICER_TYPES

typedef struct policer_s {
    policer_type_t type;
    uint64_t rate; // rate in bytes-per-sec or packets-per-sec
    uint64_t burst; // burst size in bytes or packets
} __PACK__ policer_t;

inline std::ostream& operator<<(std::ostream& os, const policer_t& p)
{
    return os << fmt::format("{{type: {}, rate: {}, burst: {}}}",
                             p.type, p.rate, p.burst);
}


static inline void
qos_policer_update_from_spec(const qos::PolicerSpec &spec, policer_t *policer)
{
    if (spec.has_pps_policer()) {
        policer->type = POLICER_TYPE_PPS;
        policer->rate = spec.pps_policer().packets_per_sec();
        policer->burst = spec.pps_policer().burst_packets();
    } else {
        policer->type = POLICER_TYPE_BPS;
        policer->rate = spec.bps_policer().bytes_per_sec();
        policer->burst = spec.bps_policer().burst_bytes();
    }
}

static inline void
qos_policer_to_spec (policer_t *policer, qos::PolicerSpec *spec)
{
    if (policer->type == POLICER_TYPE_PPS) {
        spec->mutable_pps_policer()->set_packets_per_sec(policer->rate);
        spec->mutable_pps_policer()->set_burst_packets(policer->burst);
    } else {
        spec->mutable_bps_policer()->set_bytes_per_sec(policer->rate);
        spec->mutable_bps_policer()->set_burst_bytes(policer->burst);
    }
}

static inline bool
qos_policer_spec_same (policer_t *p1, policer_t *p2)
{
    return !memcmp(p1, p2, sizeof(policer_t));
}

extern hal_ret_t
find_qos_cos_info_from_spec(QosClassKeyHandle kh, hal_handle_t pinned_uplink,
                            uint32_t *cos);

#define COPP_TYPES(ENTRY) \
    ENTRY(COPP_TYPE_FLOW_MISS,                  0, "flow-miss") \
    ENTRY(COPP_TYPE_ARP,                        1, "arp") \
    ENTRY(COPP_TYPE_DHCP,                       2, "dhcp") \
    ENTRY(COPP_TYPE_SWM,                        3, "swm") \
    ENTRY(COPP_TYPE_INBAND_MGMT,                4, "inband") \
    ENTRY(COPP_TYPE_OOB_MGMT,                   5, "oob") \
    ENTRY(NUM_COPP_TYPES,                       6, "num-copp-types")

DEFINE_ENUM(copp_type_t, COPP_TYPES)
#undef COPP_TYPES

#define HAL_MAX_COPPS NUM_COPP_TYPES

typedef struct copp_key_s {
    copp_type_t copp_type;
} __PACK__ copp_key_t;

inline std::ostream& operator<<(std::ostream& os, const copp_key_t& s)
{
   return os << fmt::format("{{copp_type={}}}", s.copp_type);
}

typedef struct copp_s {
    sdk_spinlock_t slock;         // lock to protect this structure
    copp_key_t     key;           // copp key information
    policer_t      policer;       // policer values
    // operational state of copp
    hal_handle_t   hal_handle;    // HAL allocated handle

    // Back references
    block_list     *acl_list;   // Acl list

    pd::pd_copp_t  *pd;
} __PACK__ copp_t;

// cb data structures
typedef struct copp_create_app_ctxt_s {
} __PACK__ copp_create_app_ctxt_t;

typedef struct copp_update_app_ctxt_s {
    bool policer_changed;
} __PACK__ copp_update_app_ctxt_t;

static inline copp_type_t
copp_spec_copp_type_to_copp_type (kh::CoppType copp_type)
{
    switch(copp_type) {
    case kh::COPP_TYPE_FLOW_MISS:
        return COPP_TYPE_FLOW_MISS;
    case kh::COPP_TYPE_ARP:
        return COPP_TYPE_ARP;
    case kh::COPP_TYPE_DHCP:
        return COPP_TYPE_DHCP;
    case kh::COPP_TYPE_SWM:
        return COPP_TYPE_SWM;
    case kh::COPP_TYPE_INBAND_MGMT:
        return COPP_TYPE_INBAND_MGMT;
    case kh::COPP_TYPE_OOB_MGMT:
        return COPP_TYPE_OOB_MGMT;
    default:
        SDK_ASSERT(0);
        return COPP_TYPE_FLOW_MISS;
    }
}

static inline kh::CoppType
copp_type_to_spec_type (copp_type_t copp_type)
{
    switch(copp_type) {
    case COPP_TYPE_FLOW_MISS:
        return kh::COPP_TYPE_FLOW_MISS;
    case COPP_TYPE_ARP:
        return kh::COPP_TYPE_ARP;
    case COPP_TYPE_DHCP:
        return kh::COPP_TYPE_DHCP;
    case COPP_TYPE_SWM:
        return kh::COPP_TYPE_SWM;
    case COPP_TYPE_INBAND_MGMT:
        return kh::COPP_TYPE_INBAND_MGMT;
    case COPP_TYPE_OOB_MGMT:
        return kh::COPP_TYPE_OOB_MGMT;
    default:
        SDK_ASSERT(0);
        return kh::COPP_TYPE_FLOW_MISS;
    }
}

static inline bool
spec_cmap_type_pcp(qos::QosClassMapType type) {
    return type == qos::QOS_CLASS_MAP_TYPE_PCP;
}

static inline bool
spec_cmap_type_dscp(qos::QosClassMapType type) {
    return type == qos::QOS_CLASS_MAP_TYPE_DSCP;
}

static inline bool
cmap_type_pcp(qos_cmap_type_e type) {
    return type == QOS_CMAP_TYPE_PCP;
}

static inline bool
cmap_type_dscp(qos_cmap_type_e type) {
    return type == QOS_CMAP_TYPE_DSCP;
}

extern void *copp_get_key_func(void *entry);
extern uint32_t copp_key_size(void);
extern copp_t *find_copp_by_copp_type(copp_type_t copp_type);
extern copp_t *find_copp_by_handle(hal_handle_t handle);
extern copp_t *find_copp_by_key_handle(const CoppKeyHandle& kh);

// SVC CRUD APIs
hal_ret_t copp_create(qos::CoppSpec& spec,
                      qos::CoppResponse *rsp);
hal_ret_t copp_update(qos::CoppSpec& spec,
                      qos::CoppResponse *rsp);
hal_ret_t copp_delete(qos::CoppDeleteRequest& req,
                      qos::CoppDeleteResponse *rsp);
hal_ret_t copp_get(qos::CoppGetRequest& req,
                   qos::CoppGetResponseMsg *rsp);
hal_ret_t copp_store_cb(void *obj, uint8_t *mem,
                        uint32_t len, uint32_t *mlen);
uint32_t copp_restore_cb(void *obj, uint32_t len);

}    // namespace hal

#endif    // __QOS_HPP__
