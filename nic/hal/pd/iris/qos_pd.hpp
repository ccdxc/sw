#ifndef __HAL_QOS_PD_HPP__
#define __HAL_QOS_PD_HPP__

#include "nic/include/base.h"
#include "nic/hal/src/qos.hpp"
#include "nic/include/pd.hpp"
#include "nic/include/pd_api.hpp"
#include "nic/hal/pd/capri/capri_tm_rw.hpp"
#include "nic/p4/nw/include/defines.h"


namespace hal {
namespace pd {

#define HAL_MAX_COMMON_OQS                  16
#define HAL_MAX_RXDMA_ONLY_OQS              16

#define HAL_PD_QOS_MAX_TX_QUEUES_PER_CLASS  2
#define HAL_PD_QOS_MAX_QUEUES_PER_CLASS     (1 + HAL_PD_QOS_MAX_TX_QUEUES_PER_CLASS)

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

struct pd_qos_class_s {
    tm_q_t           uplink_iq;
    tm_q_t           txdma_iq[HAL_PD_QOS_MAX_TX_QUEUES_PER_CLASS];
    tm_q_t           p4_q[HAL_PD_QOS_NUM_IQ_TYPES];
    tm_q_t           p4_eg_q[HAL_PD_QOS_NUM_IQ_TYPES];
    pd_qos_oq_type_e dest_oq_type;
    tm_q_t           dest_oq;
    bool             pcie_oq; // indicates if the rxdma oq is towards pcie/hbm
    uint32_t         cells_per_mtu;

    // pi ptr
    void             *pi_qos_class;
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
    qos_class->uplink_iq = HAL_TM_INVALID_Q;
    for (unsigned i = 0; i < HAL_ARRAY_SIZE(qos_class->p4_q); i++) {
        qos_class->p4_q[i] = HAL_TM_INVALID_Q;
    }
    for (unsigned i = 0; i < HAL_ARRAY_SIZE(qos_class->p4_eg_q); i++) {
        qos_class->p4_eg_q[i] = HAL_TM_INVALID_Q;
    }
    for (unsigned i = 0; i < HAL_ARRAY_SIZE(qos_class->txdma_iq); i++) {
        qos_class->txdma_iq[i] = HAL_TM_INVALID_Q;
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

// freeing Qos-class PD
static inline hal_ret_t
qos_class_pd_free (pd_qos_class_t *qos_class)
{
    g_hal_state_pd->qos_class_pd_slab()->free(qos_class);
    return HAL_RET_OK;
}


// freeing Qos-class PD memory
static inline hal_ret_t
qos_class_pd_mem_free (pd_qos_class_t *qos_class)
{
    g_hal_state_pd->qos_class_pd_slab()->free(qos_class);
    return HAL_RET_OK;
}

hal_ret_t qos_class_pd_alloc_res(pd_qos_class_t *up_qos_class);
hal_ret_t qos_class_pd_dealloc_res(pd_qos_class_t *up_qos_class);
hal_ret_t qos_class_pd_cleanup(pd_qos_class_t *qos_class_pd);


hal_ret_t qos_class_pd_program_hw(pd_qos_class_t *pd_qos_class);
void qos_class_link_pi_pd(pd_qos_class_t *pd_qos_class, qos_class_t *pi_qos_class);
void qos_class_delink_pi_pd(pd_qos_class_t *pd_qos_class, qos_class_t *pi_up_qos_class);

}   // namespace pd
}   // namespace hal

#endif    // __HAL_QOS_PD_HPP__
