#ifndef __QOS_HPP__
#define __QOS_HPP__

#include "nic/include/base.h"
#include "nic/include/hal_state.hpp"
#include "sdk/ht.hpp"
#include "nic/include/bitmap.hpp"
#include "nic/gen/proto/hal/qos.pb.h"
#include "nic/include/pd.hpp"

using sdk::lib::ht_ctxt_t;
using hal::utils::bitmap;

using qos::QosClassSpec;
using qos::QosClassKeyHandle;
using qos::QosClassRequestMsg;
using qos::QosClassResponse;
using qos::QosClassResponseMsg;
using qos::QosClassDeleteRequestMsg;
using qos::QosClassDeleteResponseMsg;
using qos::QosClassGetRequestMsg;
using qos::QosClassGetResponseMsg;
using qos::CoppSpec;
using qos::CoppKeyHandle;
using qos::CoppRequestMsg;
using qos::CoppResponse;
using qos::CoppResponseMsg;
using qos::CoppGetRequestMsg;
using qos::CoppGetResponseMsg;

namespace hal {

#define HAL_MAX_DOT1Q_PCP_VALS 8
#define HAL_MAX_IP_DSCP_VALS   64
// Maximum pfc cos values
#define HAL_MAX_PFC_COS_VALS HAL_MAX_DOT1Q_PCP_VALS

#define QOS_GROUPS(ENTRY)                                       \
    ENTRY(QOS_GROUP_DEFAULT,             0, "default")          \
    ENTRY(QOS_GROUP_USER_DEFINED_1,      1, "user-defined-1")   \
    ENTRY(QOS_GROUP_USER_DEFINED_2,      2, "user-defined-2")   \
    ENTRY(QOS_GROUP_USER_DEFINED_3,      3, "user-defined-3")   \
    ENTRY(QOS_GROUP_USER_DEFINED_4,      4, "user-defined-4")   \
    ENTRY(QOS_GROUP_USER_DEFINED_5,      5, "user-defined-5")   \
    ENTRY(QOS_GROUP_USER_DEFINED_6,      6, "user-defined-6")   \
    ENTRY(QOS_GROUP_ADMIN,               7, "admin")            \
    ENTRY(QOS_GROUP_CONTROL,             8, "control")          \
    ENTRY(QOS_GROUP_SPAN,                9, "span")             \
    ENTRY(QOS_GROUP_RX_PROXY_NO_DROP,   10, "rx-proxy-no-drop") \
    ENTRY(QOS_GROUP_RX_PROXY_DROP,      11, "rx-proxy-drop")    \
    ENTRY(QOS_GROUP_TX_PROXY_NO_DROP,   12, "tx-proxy-no-drop") \
    ENTRY(QOS_GROUP_TX_PROXY_DROP,      13, "tx-proxy-drop")    \
    ENTRY(QOS_GROUP_CPU_COPY,           14, "cpu-copy")         \
    ENTRY(NUM_QOS_GROUPS,               15, "num-qos-groups")         

DEFINE_ENUM(qos_group_t, QOS_GROUPS)
#undef QOS_GROUOPS

#define HAL_MAX_QOS_CLASSES NUM_QOS_GROUPS

typedef struct qos_class_key_s {
    qos_group_t qos_group;
} __PACK__ qos_class_key_t;

inline std::ostream& operator<<(std::ostream& os, const qos_class_key_t& s)
{
   return os << fmt::format("{{qos_group={}}}", s.qos_group);
}

typedef struct qos_buf_s {
    uint32_t reserved_mtus;       // Number of bytes reserved
    uint32_t headroom_mtus;       // Number of additional bytes reserved
                                  // Before this is used, xoff will be asserted
    uint32_t xon_threshold;       // Relative threshold from the
                                  // max occupancy at which xoff will be cleared
    uint32_t xoff_clear_limit;    // When the pool occupancy goes
                                  // below this limit, xoff will be cleared
} __PACK__ qos_buf_t;

#define QOS_SCHED_TYPES(ENTRY)                       \
    ENTRY(QOS_SCHED_TYPE_DWRR,          0, "dwrr")   \
    ENTRY(QOS_SCHED_TYPE_STRICT,        1, "strict")

DEFINE_ENUM(qos_sched_type_e, QOS_SCHED_TYPES);
#undef QOS_SCHED_TYPES

typedef struct qos_sched_s {
    qos_sched_type_e type;
    union {
        struct {
            uint32_t  bw;                // Bandwidth percentage for DWRR
        } dwrr;
        struct {
            uint32_t  bps;               // Rate for strict priority scheduling
        } strict;
    } __PACK__;
} __PACK__ qos_sched_t;

typedef struct qos_uplink_cmap_s {
    uint32_t    dot1q_pcp;
    bool        ip_dscp[HAL_MAX_IP_DSCP_VALS];
} __PACK__ qos_uplink_cmap_t;

typedef struct qos_marking_action_s {
    bool     pcp_rewrite_en;     // Rewrite the 802.1q pcp value
    uint32_t pcp;                // 802.1q pcp value to mark with
    bool     dscp_rewrite_en;    // Rewrite the dscp value
    uint32_t dscp;               // DSCP value to mark with
} __PACK__ qos_marking_action_t;

// QosClass structure
typedef struct qos_class_s {
    hal_spinlock_t       slock;          // lock to protect this structure
    qos_class_key_t      key;            // QOS group information

    uint32_t             mtu;            // MTU of the packets in bytes
    qos_buf_t            buffer;         // buffer configuration
    bool                 no_drop;        // No drop class
    uint32_t             pfc_cos;        // PFC cos the class
                                         // participates in
    qos_sched_t          sched;          // scheduler configuration
    qos_uplink_cmap_t    uplink_cmap;    // Uplink class map
    qos_marking_action_t marking;

                                         // operational state of qos-class
    hal_handle_t         hal_handle;     // HAL allocated handle

    pd::pd_qos_class_t   *pd;
} __PACK__               qos_class_t;

// CB data structures
typedef struct qos_class_create_app_ctxt_s {
} __PACK__ qos_class_create_app_ctxt_t;

typedef struct qos_class_update_app_ctxt_s {
} __PACK__ qos_class_update_app_ctxt_t;

// allocate a QosClass instance
static inline qos_class_t *
qos_class_alloc (void)
{
    qos_class_t    *qos_class;

    qos_class = (qos_class_t *)g_hal_state->qos_class_slab()->alloc();
    if (qos_class == NULL) {
        return NULL;
    }
    return qos_class;
}

// initialize a QosClass instance
static inline qos_class_t *
qos_class_init (qos_class_t *qos_class)
{
    if (!qos_class) {
        return NULL;
    }
    HAL_SPINLOCK_INIT(&qos_class->slock, PTHREAD_PROCESS_PRIVATE);

    return qos_class;
}

// allocate and initialize a qos_class instance
static inline qos_class_t *
qos_class_alloc_init (void)
{
    return qos_class_init(qos_class_alloc());
}

static inline hal_ret_t
qos_class_free (qos_class_t *qos_class)
{
    HAL_SPINLOCK_DESTROY(&qos_class->slock);
    hal::delay_delete_to_slab(HAL_SLAB_COPP, qos_class);
    return HAL_RET_OK;
}

static inline qos_group_t
qos_spec_qos_group_to_qos_group (qos::QosGroup qos_group)
{
    switch(qos_group) {
        case qos::DEFAULT: 
            return QOS_GROUP_DEFAULT;
        case qos::USER_DEFINED_1: 
            return QOS_GROUP_USER_DEFINED_1;
        case qos::USER_DEFINED_2: 
            return QOS_GROUP_USER_DEFINED_2;
        case qos::USER_DEFINED_3: 
            return QOS_GROUP_USER_DEFINED_3;
        case qos::USER_DEFINED_4: 
            return QOS_GROUP_USER_DEFINED_4;
        case qos::USER_DEFINED_5: 
            return QOS_GROUP_USER_DEFINED_5;
        case qos::USER_DEFINED_6: 
            return QOS_GROUP_USER_DEFINED_6;
        case qos::CONTROL: 
            return QOS_GROUP_CONTROL;
        case qos::SPAN: 
            return QOS_GROUP_SPAN;
        case qos::INTERNAL_RX_PROXY_NO_DROP: 
            return QOS_GROUP_RX_PROXY_NO_DROP;
        case qos::INTERNAL_RX_PROXY_DROP: 
            return QOS_GROUP_RX_PROXY_DROP;
        case qos::INTERNAL_TX_PROXY_NO_DROP: 
            return QOS_GROUP_TX_PROXY_NO_DROP;
        case qos::INTERNAL_TX_PROXY_DROP: 
            return QOS_GROUP_TX_PROXY_DROP;
        case qos::INTERNAL_CPU_COPY: 
            return QOS_GROUP_CPU_COPY;
        case qos::INTERNAL_ADMIN:
            return QOS_GROUP_ADMIN;
        default:
            HAL_ASSERT(0);
            return QOS_GROUP_DEFAULT;
    }
}

static inline qos_class_t *
find_qos_class_by_group (qos_group_t qos_group)
{
    hal_handle_id_ht_entry_t    *entry;
    qos_class_key_t             qos_class_key;
    qos_class_t                 *qos_class;

    qos_class_key.qos_group = qos_group;

    entry = (hal_handle_id_ht_entry_t *)g_hal_state->
        qos_class_ht()->lookup(&qos_class_key);
    if (entry) {
        // check for object type
        HAL_ASSERT(hal_handle_get_from_handle_id(entry->handle_id)->obj_id() == 
                   HAL_OBJ_ID_QOS_CLASS);
        qos_class = (qos_class_t *)hal_handle_get_obj(entry->handle_id);
        return qos_class;
    }
    return NULL;
}

static inline qos_class_t *
find_qos_class_by_handle (hal_handle_t handle)
{
    if (handle == HAL_HANDLE_INVALID) {
        return NULL;
    }
    auto hal_handle = hal_handle_get_from_handle_id(handle);
    if (!hal_handle) {
        HAL_TRACE_ERR("{}:failed to find object with handle:{}",
                        __FUNCTION__, handle);
        return NULL;
    }
    if (hal_handle->obj_id() != HAL_OBJ_ID_QOS_CLASS) {
        HAL_TRACE_ERR("{}:failed to find qos_class with handle:{}",
                        __FUNCTION__, handle);
        return NULL;
    }
    return (qos_class_t *)hal_handle_get_obj(handle);
}

static inline qos_class_t *
find_qos_class_by_key_handle (const QosClassKeyHandle& kh)
{
    if (kh.key_or_handle_case() == QosClassKeyHandle::kQosGroup) {
        return find_qos_class_by_group(qos_spec_qos_group_to_qos_group(kh.qos_group()));
    } else if (kh.key_or_handle_case() == QosClassKeyHandle::kQosClassHandle) {
        return find_qos_class_by_handle(kh.qos_class_handle());
    }
    return NULL;
}

extern void *qos_class_get_key_func(void *entry);
extern uint32_t qos_class_compute_hash_func(void *key, uint32_t ht_size);
extern bool qos_class_compare_key_func(void *key1, void *key2);

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

static inline bool
qos_class_is_default (qos_class_t *qos_class)
{
    return qos_class->key.qos_group == QOS_GROUP_DEFAULT;
}


// SVC CRUD APIs
hal_ret_t qos_class_create(qos::QosClassSpec& spec,
                           qos::QosClassResponse *rsp);
hal_ret_t qos_class_update(qos::QosClassSpec& spec,
                           qos::QosClassResponse *rsp);

typedef struct policer_s {
    uint32_t bps_rate; // rate in bytes-per-sec
    uint32_t burst_size; // Burst size in bytes
} __PACK__ policer_t;

static inline void
qos_policer_update_from_spec(const qos::PolicerSpec &spec, policer_t *policer)
{
    policer->bps_rate = spec.bps_rate();
    policer->burst_size = spec.burst_size();
}

static inline bool
qos_policer_spec_same (policer_t *p1, policer_t *p2)
{
    return !memcmp(p1, p2, sizeof(policer_t));
}

extern hal_ret_t
find_qos_cos_info_from_spec(QosClassKeyHandle kh, hal_handle_t pinned_uplink, 
                            uint32_t *cosA, uint32_t *cosB);

#define COPP_TYPES(ENTRY) \
    ENTRY(COPP_TYPE_FLOW_MISS,                  0, "flow-miss") \
    ENTRY(COPP_TYPE_ARP,                        1, "arp") \
    ENTRY(COPP_TYPE_DHCP,                       2, "dhcp") \
    ENTRY(NUM_COPP_TYPES,                       3, "num-copp-types")

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
    hal_spinlock_t slock;         // lock to protect this structure
    copp_key_t     key;           // copp key information
    policer_t      policer;       // policer values
    // operational state of copp
    hal_handle_t   hal_handle;    // HAL allocated handle

    pd::pd_copp_t  *pd;
} __PACK__ copp_t;

// CB data structures
typedef struct copp_create_app_ctxt_s {
} __PACK__ copp_create_app_ctxt_t;

typedef struct copp_update_app_ctxt_s {
} __PACK__ copp_update_app_ctxt_t;

// allocate a Copp instance
static inline copp_t *
copp_alloc (void)
{
    copp_t    *copp;

    copp = (copp_t *)g_hal_state->copp_slab()->alloc();
    if (copp == NULL) {
        return NULL;
    }
    return copp;
}

// initialize a Copp instance
static inline copp_t *
copp_init (copp_t *copp)
{
    if (!copp) {
        return NULL;
    }
    HAL_SPINLOCK_INIT(&copp->slock, PTHREAD_PROCESS_PRIVATE);

    return copp;
}

// allocate and initialize a copp instance
static inline copp_t *
copp_alloc_init (void)
{
    return copp_init(copp_alloc());
}

static inline hal_ret_t
copp_free (copp_t *copp)
{
    HAL_SPINLOCK_DESTROY(&copp->slock);
    hal::delay_delete_to_slab(HAL_SLAB_COPP, copp);
    return HAL_RET_OK;
}

static inline copp_type_t
copp_spec_copp_type_to_copp_type (qos::CoppType copp_type)
{
    switch(copp_type) {
        case qos::COPP_TYPE_FLOW_MISS:
            return COPP_TYPE_FLOW_MISS;
        case qos::COPP_TYPE_ARP:
            return COPP_TYPE_ARP;
        case COPP_TYPE_DHCP:
            return COPP_TYPE_DHCP;
        default:
            HAL_ASSERT(0);
            return COPP_TYPE_FLOW_MISS;
    }
}

static inline copp_t *
find_copp_by_copp_type (copp_type_t copp_type)
{
    hal_handle_id_ht_entry_t *entry;
    copp_key_t       copp_key;
    copp_t           *copp;

    copp_key.copp_type = copp_type;

    entry = (hal_handle_id_ht_entry_t *)g_hal_state->
        copp_ht()->lookup(&copp_key);
    if (entry) {
        // check for object type
        HAL_ASSERT(hal_handle_get_from_handle_id(entry->handle_id)->obj_id() == 
                   HAL_OBJ_ID_COPP);
        copp = (copp_t *)hal_handle_get_obj(entry->handle_id);
        return copp;
    }
    return NULL;
}

static inline copp_t *
find_copp_by_handle (hal_handle_t handle)
{
    if (handle == HAL_HANDLE_INVALID) {
        return NULL;
    }
    auto hal_handle = hal_handle_get_from_handle_id(handle);
    if (!hal_handle) {
        HAL_TRACE_ERR("{}:failed to find object with handle:{}",
                        __FUNCTION__, handle);
        return NULL;
    }
    if (hal_handle->obj_id() != HAL_OBJ_ID_COPP) {
        HAL_TRACE_ERR("{}:failed to find copp with handle:{}",
                        __FUNCTION__, handle);
        return NULL;
    }
    return (copp_t *)hal_handle_get_obj(handle);
}

static inline copp_t *
find_copp_by_key_handle (const CoppKeyHandle& kh)
{
    if (kh.key_or_handle_case() == CoppKeyHandle::kCoppType) {
        return find_copp_by_copp_type(copp_spec_copp_type_to_copp_type(kh.copp_type()));
    } else if (kh.key_or_handle_case() == CoppKeyHandle::kCoppHandle) {
        return find_copp_by_handle(kh.copp_handle());
    }
    return NULL;
}

extern void *copp_get_key_func(void *entry);
extern uint32_t copp_compute_hash_func(void *key, uint32_t ht_size);
extern bool copp_compare_key_func(void *key1, void *key2);

// SVC CRUD APIs
hal_ret_t copp_create(qos::CoppSpec& spec,
                              qos::CoppResponse *rsp);
hal_ret_t copp_update(qos::CoppSpec& spec,
                              qos::CoppResponse *rsp);


}    // namespace hal

#endif    // __QOS_HPP__
