//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#include "nic/hal/plugins/cfg/nw/interface.hpp"
#include "nic/include/base.hpp"
#include "nic/hal/hal.hpp"
#include "nic/sdk/include/sdk/lock.hpp"
#include "nic/hal/iris/include/hal_state.hpp"
#include "nic/hal/src/internal/proxy.hpp"
#include "nic/include/pd_api.hpp"
#include "nic/hal/plugins/cfg/lif/lif.hpp"
#include "nic/hal/plugins/cfg/nw/interface.hpp"

namespace hal {

hal_ret_t
service_create_cpucb(uint8_t cpucb_id)
{
    hal_ret_t               ret = HAL_RET_OK;

    ret = cpucb_create(cpucb_id);
    if(ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to create cpucb: {}", ret);
    }

    return ret;
}

hal_ret_t
service_create_cpucb(void)
{
    hal_ret_t   ret = HAL_RET_OK;
    for(int i = 0; i < MAX_CPU_CBID; i++) {
        ret = service_create_cpucb(i);
        if(ret != HAL_RET_OK) {
            HAL_TRACE_ERR("Failed to create cpucb for id: {}, ret: {}", i, ret);
            return ret;
        }
    }

    return ret;

}

hal_ret_t
program_cpu_lif(void)
{
    hal_ret_t                         ret = HAL_RET_OK;
    sdk_ret_t                         sret = SDK_RET_OK;
    intf::LifSpec                     lif_spec;
    intf::LifResponse                 rsp;
    lif_hal_info_t                    lif_hal_info = {0};
    pd::pd_lif_get_lport_id_args_t    args = { 0 };
    pd::pd_func_args_t                pd_func_args = {0};

    // if(lif_manager()->LIFRangeAlloc(HAL_LIF_CPU, 1) <= 0)
    sret = lif_manager()->reserve_id(HAL_LIF_CPU, 1);
    if (sret != SDK_RET_OK) {
        HAL_TRACE_ERR("Failed to reserve service LIF");
        return HAL_RET_NO_RESOURCE;
    }


    lif_spec.Clear();

    lif_spec.mutable_key_or_handle()->set_lif_id(HAL_LIF_CPU);
    lif_spec.set_admin_status(intf::IF_STATUS_UP);
    lif_spec.set_name("HAL_LIF_CPU");
    lif_hal_info.with_hw_lif_id = true;
    lif_hal_info.hw_lif_id = HAL_LIF_CPU;
    lif_hal_info.dont_zero_qstate_mem = true;
    lif_spec.add_lif_qstate_map();
    lif_spec.mutable_lif_qstate_map(0)->set_type_num(0);
    lif_spec.mutable_lif_qstate_map(0)->set_size(2);
    lif_spec.mutable_lif_qstate_map(0)->set_entries(MAX_CPU_CBID);

    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = lif_create(lif_spec, &rsp, &lif_hal_info);
    if(ret != HAL_RET_OK) {
        HAL_TRACE_ERR("lif creation failed for cpu lif {} ", HAL_LIF_CPU);
        hal::hal_cfg_db_close();
        return ret;
    } else {
        hal::hal_cfg_db_close();
    }

    // get lport-id for this lif
    lif_t* lif = find_lif_by_id(HAL_LIF_CPU);
    SDK_ASSERT_RETURN((NULL != lif), HAL_RET_LIF_NOT_FOUND);

    args.pi_lif = lif;
    pd_func_args.pd_lif_get_lport_id = &args;
    ret = pd::hal_pd_call(pd::PD_FUNC_ID_LIF_GET_LPORTID, &pd_func_args);
    HAL_TRACE_DEBUG("Received lport-id: {} for lif: {}",
                    args.lport_id, HAL_LIF_CPU);

    service_create_cpucb();
    return HAL_RET_OK;
}


} //namespace hal
