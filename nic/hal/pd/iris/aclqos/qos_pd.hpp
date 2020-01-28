// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
#ifndef __HAL_QOS_PD_HPP__
#define __HAL_QOS_PD_HPP__

#include "nic/include/base.hpp"
#include "nic/hal/plugins/cfg/aclqos/qos.hpp"
#include "nic/include/pd.hpp"
#include "nic/include/pd_api.hpp"
#include "platform/capri/capri_tm_rw.hpp"
#include "platform/capri/capri_tm_utils.hpp"
#include "nic/hal/iris/datapath/p4/include/defines.h"
#include <memory>
#include <map>

using namespace sdk::platform::capri;

namespace hal {
namespace pd {

#define HAL_PD_QOS_MAX_IQS                  32
#define HAL_PD_QOS_MAX_OQS                  32

#define HAL_MAX_TXDMA_IQS                   16
#define HAL_MAX_UPLINK_IQS                  8

#define HAL_MAX_COMMON_OQS                  16
#define HAL_MAX_RXDMA_ONLY_OQS              16

// Using a non-zero value for admic cos so that any uninitialized
// rings do not use this 
#define HAL_QOS_ADMIN_COS                   1

#define HAL_PD_QOS_MAX_TX_QUEUES_PER_CLASS  2

#define HAL_DEFAULT_POLICER_REFRESH_INTERVAL 250  // us - min 4K PPS
#define HAL_DEFAULT_COPP_REFRESH_INTERVAL    2000 // us - min 512 PPS
#define HAL_MAX_POLICER_TOKENS_PER_INTERVAL ((1ull<<39)-1)

#define HAL_PD_QOS_IQS(ENTRY)                                        \
    ENTRY(HAL_PD_QOS_IQ_COMMON,                 0, "common")         \
    ENTRY(HAL_PD_QOS_IQ_TX_UPLINK_GROUP_0,      1, "uplink-group-0") \
    ENTRY(HAL_PD_QOS_IQ_TX_UPLINK_GROUP_1,      2, "uplink-group-1") \
    ENTRY(HAL_PD_QOS_IQ_RX,                     3, "rx")             \
    ENTRY(HAL_PD_QOS_NUM_IQ_TYPES,              4, "num-iq-types")

DEFINE_ENUM(pd_qos_iq_type_e, HAL_PD_QOS_IQS)
#undef HAL_PD_QOS_IQS

#define HAL_PD_QOS_OQS(ENTRY)                                \
    ENTRY(HAL_PD_QOS_OQ_COMMON,                 0, "common") \
    ENTRY(HAL_PD_QOS_OQ_RXDMA,                  1, "rxdma")
DEFINE_ENUM(pd_qos_oq_type_e, HAL_PD_QOS_OQS)
#undef HAL_PD_QOS_OQS

typedef struct pd_qos_uplink_iq_s {
    tm_q_t   iq;
} __PACK__ pd_qos_uplink_iq_t;

typedef struct pd_qos_txdma_iq_s {
    tm_q_t   iq;
} __PACK__ pd_qos_txdma_iq_t;

struct pd_qos_class_s {
    pd_qos_uplink_iq_t uplink;
    pd_qos_txdma_iq_t  txdma[HAL_PD_QOS_MAX_TX_QUEUES_PER_CLASS];
    tm_q_t             p4_ig_q[HAL_PD_QOS_NUM_IQ_TYPES];
    tm_q_t             p4_eg_q[HAL_PD_QOS_NUM_IQ_TYPES];
    pd_qos_oq_type_e   dest_oq_type;
    tm_q_t             dest_oq;
    bool               pcie_oq; // indicates if the rxdma oq is towards pcie/hbm

    // pi ptr
    qos_class_t        *pi_qos_class;
} __PACK__;

typedef struct pd_qos_dscp_cos_map_s {
    uint8_t     is_dscp : 1;
    uint8_t     rsvd1: 7;
    uint8_t     no_drop[16]; // 128-bits. 2 bits per DSCP/PCP. 
    uint8_t     txdma_iq[32];
    uint8_t     no_drop1_txdma_iq : 4;
    uint8_t     no_drop2_txdma_iq : 4;
    uint8_t     no_drop3_txdma_iq : 4;
    uint8_t     rsvd2: 4;
    uint8_t     rsvd3[13];
} __PACK__ pd_qos_dscp_cos_map_t;


inline std::ostream& operator<<(std::ostream& os, const struct pd_qos_class_s& s) 
{
    os << fmt::format("{{");
    os << fmt::format("uplink iq {} ", s.uplink.iq);
    for (unsigned i = 0; i < SDK_ARRAY_SIZE(s.txdma); i++) {
        os << fmt::format("txdma iq {} ", s.txdma[i].iq);
    }
    for (unsigned i = 0; i < SDK_ARRAY_SIZE(s.p4_ig_q); i++) {
        os << fmt::format("p4_ig_q iq {} ", s.p4_ig_q[i]);
    }
    for (unsigned i = 0; i < SDK_ARRAY_SIZE(s.p4_eg_q); i++) {
        os << fmt::format("p4_eg_q iq {} ", s.p4_eg_q[i]);
    }
    os << fmt::format("dest_oq_type {} dest_oq {} pcie_oq {} ",
                      s.dest_oq_type, s.dest_oq, s.pcie_oq);

    return os << fmt::format("}}");
}

// allocate Qos-class Instance
static inline pd_qos_class_t *
qos_class_pd_alloc (void)
{
    pd_qos_class_t    *qos_class;

    qos_class = (pd_qos_class_t *)g_hal_state_pd->qos_class_pd_slab()->alloc();
    if (qos_class == NULL) {
        return NULL;
    }


    return qos_class;
}

// initialize Qos-class PD instance
static inline pd_qos_class_t *
qos_class_pd_init (pd_qos_class_t *qos_class)
{
    // Nothing to do currently
    if (!qos_class) {
        return NULL;
    }

    // Set here if you want to initialize any fields
    qos_class->uplink.iq = CAPRI_TM_INVALID_Q;
    for (unsigned i = 0; i < SDK_ARRAY_SIZE(qos_class->p4_ig_q); i++) {
        qos_class->p4_ig_q[i] = CAPRI_TM_INVALID_Q;
    }
    for (unsigned i = 0; i < SDK_ARRAY_SIZE(qos_class->p4_eg_q); i++) {
        qos_class->p4_eg_q[i] = CAPRI_TM_INVALID_Q;
    }
    for (unsigned i = 0; i < SDK_ARRAY_SIZE(qos_class->txdma); i++) {
        qos_class->txdma[i].iq = CAPRI_TM_INVALID_Q;
    }
    qos_class->dest_oq = CAPRI_TM_INVALID_Q;

    return qos_class;
}

// allocate and Initialize Qos-class PD Instance
static inline pd_qos_class_t *
qos_class_pd_alloc_init(void)
{
    return qos_class_pd_init(qos_class_pd_alloc());
}

// freeing Qos-class PD memory
static inline hal_ret_t
qos_class_pd_mem_free (pd_qos_class_t *pd_qos_class)
{
    if (pd_qos_class) {
        hal::pd::delay_delete_to_slab(HAL_SLAB_QOS_CLASS_PD, pd_qos_class);
    }
    return HAL_RET_OK;
}

// freeing Qos-class PD
static inline hal_ret_t
qos_class_pd_free (pd_qos_class_t *pd_qos_class)
{
    return qos_class_pd_mem_free(pd_qos_class);
}


static inline hal_ret_t
policer_to_token_rate (policer_t *policer, uint64_t refresh_interval_us, 
                       uint64_t *token_rate_p, uint64_t *token_burst_p)
{
    uint64_t rate_per_sec = policer->rate;
    uint64_t burst = policer->burst;
    uint64_t rate_tokens;

    if (rate_per_sec > UINT64_MAX/refresh_interval_us) {
        HAL_TRACE_ERR("Policer rate {} is too high", rate_per_sec);
        return HAL_RET_INVALID_ARG;
    }
    rate_tokens = (refresh_interval_us * rate_per_sec)/1000000;

    if (!rate_tokens) {
        HAL_TRACE_ERR("Policer rate {} is too low for the refresh interval {}us",
                      rate_per_sec, refresh_interval_us);
        return HAL_RET_INVALID_ARG;
    }

    if ((burst + rate_tokens) > HAL_MAX_POLICER_TOKENS_PER_INTERVAL) {
        HAL_TRACE_ERR("Policer rate {} is too high for the "
                      "refresh interval {}us", 
                      rate_per_sec, refresh_interval_us);
        return HAL_RET_INVALID_ARG;
    }

    *token_rate_p = rate_tokens;
    *token_burst_p = rate_tokens + burst;

    return HAL_RET_OK;
}

static inline void
qos_class_pd_port_to_packet_buffer_port (tm_port_t port, qos::PacketBufferPort *buf_port)
{
    if ((port >= TM_UPLINK_PORT_BEGIN) && (port <= TM_UPLINK_PORT_END)) {
        buf_port->set_port_type(qos::PACKET_BUFFER_PORT_TYPE_UPLINK);
        buf_port->set_port_num(port-TM_UPLINK_PORT_BEGIN);
    } else if ((port >= TM_DMA_PORT_BEGIN) && (port <= TM_DMA_PORT_END)) {
        buf_port->set_port_type(qos::PACKET_BUFFER_PORT_TYPE_DMA);
        buf_port->set_port_num(TM_PORT_DMA);
    } else if (port == TM_PORT_INGRESS) {
        buf_port->set_port_type(qos::PACKET_BUFFER_PORT_TYPE_P4IG);
        buf_port->set_port_num(TM_PORT_INGRESS);
    } else if (port == TM_PORT_EGRESS) {
        buf_port->set_port_type(qos::PACKET_BUFFER_PORT_TYPE_P4EG);
        buf_port->set_port_num(TM_PORT_EGRESS);
    } else {
        SDK_ASSERT(0);
    }
}

static inline bool 
qos_is_user_defined_class (qos_group_t qos_group) {
    
    if ((qos_group > QOS_GROUP_DEFAULT) && (qos_group <= QOS_GROUP_USER_DEFINED_6)) {
        return true;
    }
    return false;
}

static inline qos_group_t 
qos_group_get_next_user_defined(qos_group_t qos_group)
{
    switch(qos_group) {
        case QOS_GROUP_DEFAULT:
            return QOS_GROUP_USER_DEFINED_1;
        case QOS_GROUP_USER_DEFINED_1:
            return QOS_GROUP_USER_DEFINED_2;
        case QOS_GROUP_USER_DEFINED_2:
            return QOS_GROUP_USER_DEFINED_3;
        case QOS_GROUP_USER_DEFINED_3:
            return QOS_GROUP_USER_DEFINED_4;
        case QOS_GROUP_USER_DEFINED_4:
            return QOS_GROUP_USER_DEFINED_5;
        case QOS_GROUP_USER_DEFINED_5:
            return QOS_GROUP_USER_DEFINED_6;
        case QOS_GROUP_USER_DEFINED_6:
        default:
            break;
    }
    return NUM_QOS_GROUPS;
}

void
qos_class_queue_stats_to_proto_stats(qos::QosClassQueueStats *q_stats,
                                     sdk::platform::capri::capri_queue_stats_t *qos_queue_stats);

}   // namespace pd
}   // namespace hal

#endif    // __HAL_QOS_PD_HPP__
