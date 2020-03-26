//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//
// sw PHV injection
//-----------------------------------------------------------------------------

#include "nic/sdk/include/sdk/lock.hpp"
#include "nic/sdk/asic/pd/pd.hpp"
#include "nic/include/pd_api.hpp"
#include "nic/hal/src/internal/proxy.hpp"
#include "nic/include/pd.hpp"
#include "nic/hal/pd/iris/hal_state_pd.hpp"
#include "nic/sdk/lib/p4/p4_api.hpp"
#include "nic/hal/pd/asicpd/asic_pd_common.hpp"
#include "gen/p4gen/tcp_proxy_txdma/include/ingress_phv.h"
#include "nic/sdk/asic/pd/pd.hpp"
#undef __INGRESS_PHV__
#include "gen/p4gen/tcp_proxy_rxdma/include/ingress_phv.h"
#undef __INGRESS_PHV__
#ifndef P4_16 //ISSUE_1222
#include "gen/p4gen/p4/include/ingress_phv.h"
#endif
// FIXME: Need to fix an issue with generated egress_phv.h
// #include "gen/p4gen/p4/include/egress_phv.h"
//
using namespace sdk::asic;
using namespace sdk::asic::pd;

namespace hal {
namespace pd {

// Iris PHV size in bytes
#define PD_IRIS_PHV_SIZE (4096 / 8)

// pd_swphv_inject injects software PHV into a pipeline
hal_ret_t
pd_swphv_inject (pd_func_args_t *pd_func_args)
{
    pd_swphv_inject_args_t *args = pd_func_args->pd_swphv_inject;
    uint8_t data[PD_IRIS_PHV_SIZE];
    hal_ret_t   ret = HAL_RET_OK;
    sdk_ret_t   sdk_ret;
    uint32_t    lif_id = -1;
    int         phv_size = 0;

    bzero(data, PD_IRIS_PHV_SIZE);
    // switch based on pipeline type
    switch(args->type) {
    case PD_SWPHV_TYPE_RXDMA:
    {
        tcp_proxy_rxdma_ingress_phv_t *phv = (tcp_proxy_rxdma_ingress_phv_t *)data;
        phv_size = sizeof(tcp_proxy_rxdma_ingress_phv_t);
        lif_id = SERVICE_LIF_IPSEC_ESP;

        // initialize PHV intrinsic fields
        phv->p4_intr_global_drop = 1;
        phv->p4_intr_global_bypass = 1;
        phv->p4_intr_no_data = 1;
        phv->p4_intr_global_lif = lif_id;
        phv->p4_rxdma_intr_qid = 0;
        phv->p4_rxdma_intr_qtype = 0;

	break;
    }
    case PD_SWPHV_TYPE_TXDMA:
    {
        lif_id = HAL_LIF_CPU;
        tcp_proxy_txdma_ingress_phv_t *phv = (tcp_proxy_txdma_ingress_phv_t *)data;
        phv_size = sizeof(tcp_proxy_txdma_ingress_phv_t);

        // initialize PHV intrinsic fields
        phv->p4_intr_global_drop = 1;
        phv->p4_intr_global_bypass = 1;
        phv->p4_intr_no_data = 1;
        phv->p4_intr_global_lif = lif_id;
        phv->p4_txdma_intr_qid = 0;
        phv->p4_txdma_intr_qtype = 0;

	break;
    }
#ifndef P4_16 //ISSUE_1222
    case PD_SWPHV_TYPE_INGRESS:
    {
        p4_ingress_phv_t *phv = (p4_ingress_phv_t *)data;
        phv_size = sizeof(p4_ingress_phv_t);

        // initialize PHV intrinsic fields
        phv->capri_intrinsic_drop = 1;
        phv->capri_intrinsic_bypass = 1;
        phv->capri_p4_intrinsic_no_data = 1;
        phv->capri_intrinsic_lif = lif_id;
        phv->control_metadata_qid = 0;
        // phv->control_metadata_qtype = 0;

	break;
    }
    case PD_SWPHV_TYPE_EGRESS:
    {
        p4_ingress_phv_t *phv = (p4_ingress_phv_t *)data;
        phv_size = sizeof(p4_ingress_phv_t);

        // initialize PHV intrinsic fields
        phv->capri_intrinsic_drop = 1;
        phv->capri_intrinsic_bypass = 1;
        phv->control_metadata_ingress_bypass = 1;
        phv->capri_p4_intrinsic_no_data = 1;
        phv->capri_intrinsic_lif = lif_id;

	break;
    }
#endif
    }

    HAL_TRACE_DEBUG("pd_swphv_inject: Injecting Software PHV type {} on lif {}. size of phv: {}", args->type, lif_id, phv_size);

    // Inject the phv
    sdk_ret = asicpd_sw_phv_inject((asic_swphv_type_t)args->type, 0, 0, 1, data);

    ret = hal_sdk_ret_to_hal_ret(sdk_ret);
    return ret;
}

// pd_swphv_get_state
// get the current state of SW phv
hal_ret_t
pd_swphv_get_state (pd_func_args_t *pd_func_args)
{
    pd_swphv_get_state_args_t *state = pd_func_args->pd_swphv_get_state;
    asic_sw_phv_state_t        hw_state;
    hal_ret_t                  ret = HAL_RET_OK;
    sdk_ret_t                  sdk_ret;

    HAL_TRACE_DEBUG("pd_swphv_get_state: getting Software PHV state for type {}", state->type);

    // get it from asic PD
    sdk_ret = asicpd_sw_phv_get((asic_swphv_type_t)state->type, 0, &hw_state);

    ret = hal_sdk_ret_to_hal_ret(sdk_ret);
    if (ret != HAL_RET_OK) {
        return ret;
    }

    // translate state
    state->enabled           = hw_state.enabled;
    state->done              = hw_state.done;
    state->current_cntr      = hw_state.current_cntr;
    state->no_data_cntr      = hw_state.no_data_cntr;
    state->drop_no_data_cntr = hw_state.drop_no_data_cntr;

    return HAL_RET_OK;
}

}    // namespace pd
}    // namespace hal
