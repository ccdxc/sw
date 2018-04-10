// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
#ifndef __HAL_QOS_PD_HPP__
#define __HAL_QOS_PD_HPP__

#include "nic/include/base.h"
#include "nic/hal/src/aclqos/qos.hpp"
#include "nic/include/pd.hpp"
#include "nic/include/pd_api.hpp"
#include "nic/hal/pd/capri/capri_tm_rw.hpp"
#include "nic/p4/iris/include/defines.h"


namespace hal {
namespace pd {

#define HAL_MAX_TXDMA_IQS                   16
#define HAL_MAX_UPLINK_IQS                  8

#define HAL_MAX_COMMON_OQS                  16
#define HAL_MAX_RXDMA_ONLY_OQS              16

// Using a non-zero value for admic cos so that any uninitialized
// rings do not use this 
#define HAL_QOS_ADMIN_COS                   1

#define HAL_PD_QOS_MAX_TX_QUEUES_PER_CLASS  2
#define HAL_PD_QOS_MAX_QUEUES_PER_CLASS     (1 + HAL_PD_QOS_MAX_TX_QUEUES_PER_CLASS)

#define HAL_DEFAULT_POLICER_REFRESH_INTERVAL 4000 // us
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
    qos_class->uplink.iq = HAL_TM_INVALID_Q;
    for (unsigned i = 0; i < HAL_ARRAY_SIZE(qos_class->p4_ig_q); i++) {
        qos_class->p4_ig_q[i] = HAL_TM_INVALID_Q;
    }
    for (unsigned i = 0; i < HAL_ARRAY_SIZE(qos_class->p4_eg_q); i++) {
        qos_class->p4_eg_q[i] = HAL_TM_INVALID_Q;
    }
    for (unsigned i = 0; i < HAL_ARRAY_SIZE(qos_class->txdma); i++) {
        qos_class->txdma[i].iq = HAL_TM_INVALID_Q;
    }
    qos_class->dest_oq = HAL_TM_INVALID_Q;

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


static hal_ret_t
policer_rate_per_sec_to_token_rate (uint64_t rate_per_sec, uint64_t refresh_interval_us, 
                                    uint64_t *token_rate_p, uint64_t burst)
{
    uint64_t rate_tokens;

    *token_rate_p = 0;

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

    return HAL_RET_OK;
}

}   // namespace pd
}   // namespace hal

#endif    // __HAL_QOS_PD_HPP__
