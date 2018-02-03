// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#include "nic/include/pd_api.hpp"
#include "nic/hal/pd/p4pd_api.hpp"

namespace hal {
namespace pd {

hal_ret_t
#if 0
pd_debug_cli_read(uint32_t tableid,
                  uint32_t index,
                  void     *swkey,
                  void     *swkey_mask,
                  void     *actiondata)
#endif
pd_debug_cli_read(pd_debug_cli_read_args_t *args)
{
    hal_ret_t    ret    = HAL_RET_OK;
    p4pd_error_t pd_err = P4PD_SUCCESS;
    uint32_t tableid = args->tableid;
    uint32_t index = args->index;
    void     *swkey = args->swkey;
    void     *swkey_mask = args->swkey_mask;
    void     *actiondata = args->actiondata;

    pd_err = p4pd_global_entry_read(tableid,
                                    index,
                                    swkey,
                                    swkey_mask,
                                    actiondata);
    if (pd_err != P4PD_SUCCESS) {
        HAL_TRACE_DEBUG("{}: Hardware read fail",
                        __FUNCTION__);
        ret = HAL_RET_ERR;
    }

    return ret;
}


hal_ret_t
#if 0
pd_debug_cli_write(uint32_t tableid,
                   uint32_t index,
                   void     *swkey,
                   void     *swkey_mask,
                   void     *actiondata)
#endif
pd_debug_cli_write(pd_debug_cli_write_args_t *args)
{
    hal_ret_t    ret              = HAL_RET_OK;
    p4pd_error_t pd_err           = P4PD_SUCCESS;
    uint32_t     hwkey_len        = 0;
    uint32_t     hwkeymask_len    = 0;
    uint32_t     hwactiondata_len = 0;
    void         *hwkey           = NULL;
    void         *hwkeymask       = NULL;
    uint32_t tableid = args->tableid;
    uint32_t index = args->index;
    void     *swkey = args->swkey;
    void     *swkey_mask = args->swkey_mask;
    void     *actiondata = args->actiondata;

    p4pd_hwentry_query(tableid, &hwkey_len,
                       &hwkeymask_len, &hwactiondata_len);

    HAL_TRACE_DEBUG("hwkeylen: {}, hwkeymask_len: {}",
                    hwkey_len, hwkeymask_len);

    // build hw key & mask
    hwkey      = HAL_MALLOC(HAL_MEM_ALLOC_DEBUG_CLI,
                            (hwkey_len     + 7)/8);

    hwkeymask  = HAL_MALLOC(HAL_MEM_ALLOC_DEBUG_CLI,
                            (hwkeymask_len + 7)/8);

    memset(hwkey,     0, (hwkey_len     + 7)/8);
    memset(hwkeymask, 0, (hwkeymask_len + 7)/8);

    pd_err = p4pd_hwkey_hwmask_build(tableid,
                                     swkey,
                                     swkey_mask,
                                     (uint8_t*)hwkey,
                                     (uint8_t*)hwkeymask);
    if (pd_err != P4PD_SUCCESS) {
        HAL_TRACE_DEBUG("{}: Hardware mask build fail", __FUNCTION__);
        ret = HAL_RET_ERR;
    }

    pd_err = p4pd_global_entry_write(tableid,
                                     index,
                                     (uint8_t*)hwkey,
                                     (uint8_t*)hwkeymask,
                                     actiondata);

    if (pd_err != P4PD_SUCCESS) {
        HAL_TRACE_DEBUG("{}: Hardware write fail", __FUNCTION__);
        ret = HAL_RET_ERR;
    }

    if (hwkey)     HAL_FREE(HAL_MEM_ALLOC_DEBUG_CLI, hwkey);
    if (hwkeymask) HAL_FREE(HAL_MEM_ALLOC_DEBUG_CLI, hwkeymask);

    return ret;
}

}    // namespace pd
}    // namespace hal
