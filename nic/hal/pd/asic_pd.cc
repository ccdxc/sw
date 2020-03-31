// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#include "nic/include/pd_api.hpp"
#include "nic/include/hal.hpp"
#include "nic/hal/pd/asic_pd.hpp"
#include "nic/sdk/lib/pal/pal.hpp"
#include "nic/sdk/asic/cmn/asic_hbm.hpp"
#include "asic/asic.hpp"
#include "asic/pd/pd.hpp"
#include "platform/utils/mpartition.hpp"

namespace hal {
namespace pd {

//------------------------------------------------------------------------------
// public API for saving cpu packet
//------------------------------------------------------------------------------
sdk_ret_t
asic_step_cpu_pkt (const uint8_t* pkt, size_t pkt_len)
{
    pal_ret_t prc = sdk::lib::pal_step_cpu_pkt(pkt, pkt_len);
    return IS_PAL_API_SUCCESS(prc) ? SDK_RET_OK : SDK_RET_ERR;
}

hal_ret_t
pd_hbm_read_mem (pd_func_args_t *pd_func_args)
{
    pd_hbm_read_mem_args_t *args = pd_func_args->pd_hbm_read_mem;
    sdk::asic::asic_mem_read(args->addr, args->buf, args->size);
    return HAL_RET_OK;
}

hal_ret_t
pd_hbm_write_mem (pd_func_args_t *pd_func_args)
{
    mpartition_region_t *reg = NULL;
    p4plus_cache_action_t action = P4PLUS_CACHE_ACTION_NONE;
    pd_hbm_write_mem_args_t *args = pd_func_args->pd_hbm_write_mem;
    sdk::asic::asic_mem_write(args->addr, (uint8_t *)args->buf, args->size);

    reg = sdk::asic::pd::asicpd_get_hbm_region_by_address(args->addr);
    SDK_ASSERT(reg != NULL);

    if(is_region_cache_pipe_p4plus_all(reg)) {
        action = P4PLUS_CACHE_INVALIDATE_BOTH;
    } else if (is_region_cache_pipe_p4plus_rxdma(reg)) {
        action = P4PLUS_CACHE_INVALIDATE_RXDMA;
    } else if (is_region_cache_pipe_p4plus_txdma(reg)) {
        action = P4PLUS_CACHE_INVALIDATE_TXDMA;
    }

    if (action != P4PLUS_CACHE_ACTION_NONE) {
        sdk::asic::pd::asicpd_p4plus_invalidate_cache(args->addr,
                                                      args->size, action);
    }

    return HAL_RET_OK;
}

hal_ret_t
pd_program_label_to_offset (pd_func_args_t *pd_func_args)
{
    pd_program_label_to_offset_args_t *args =
        pd_func_args->pd_program_label_to_offset;

    sdk::p4::p4_program_label_to_offset(args->handle, args->prog_name,
                                  args->label_name, args->offset);
    return HAL_RET_OK;
}

hal_ret_t
pd_program_to_base_addr (pd_func_args_t *pd_func_args)
{
    pd_program_to_base_addr_args_t *args =
        pd_func_args->pd_program_to_base_addr;

    if(sdk::p4::p4_program_to_base_addr(args->handle,
                                        args->prog_name, args->base_addr) != 0)
        return HAL_RET_ERR;
    return HAL_RET_OK;
}

hal_ret_t
pd_get_opaque_tag_addr (pd_func_args_t *pd_func_args)
{
    pd_get_opaque_tag_addr_args_t *args = pd_func_args->pd_get_opaque_tag_addr;
    return (get_opaque_tag_addr((barco_rings_t)args->ring_type, args->addr) ==
            SDK_RET_OK ? HAL_RET_OK : HAL_RET_ERR);
}

hal_ret_t
pd_quiesce_start (pd_func_args_t *pd_func_args)
{
    sdk_ret_t sdk_ret;
    sdk_ret = sdk::asic::pd::asicpd_quiesce_start();
    return hal_sdk_ret_to_hal_ret(sdk_ret);
}

hal_ret_t
pd_quiesce_stop (pd_func_args_t *pd_func_args)
{
    sdk_ret_t sdk_ret;
    sdk_ret = sdk::asic::pd::asicpd_quiesce_stop();
    return hal_sdk_ret_to_hal_ret(sdk_ret);
}

}    // namespace pd
}    // namespace hal
