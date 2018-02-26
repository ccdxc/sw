// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#include "nic/include/base.h"
#include "nic/gen/gft/include/p4pd.h"
#include "nic/hal/pd/gft/gft_state.hpp"
#include "nic/hal/pd/hal_pd.hpp"
#include "sdk/tcam.hpp"

using sdk::table::tcam;

namespace hal {
namespace pd {

static hal_ret_t
p4pd_rx_vport_init (void)
{
    hal_ret_t               ret;
    sdk_ret_t               sdk_ret;
    uint32_t                idx; 
    rx_vport_swkey_t        key;
    rx_vport_swkey_mask_t   mask;
    rx_vport_actiondata     data;
    tcam                    *rx_vport;

    rx_vport = g_hal_state_pd->tcam_table(P4TBL_ID_RX_VPORT);
    HAL_ASSERT(rx_vport!= NULL);

    memset(&key, 0, sizeof(key));
    memset(&mask, 0, sizeof(mask));
    memset(&data, 0, sizeof(data));

    sdk_ret = rx_vport->insert(&key, &mask, &data, &idx, false);
    ret = hal_sdk_ret_to_hal_ret(sdk_ret);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("rx_vport tcam write failure, "
                      "idx : {}, err : {}", idx, ret);
        return ret;
    }
    HAL_TRACE_DEBUG("rx_vport tcam write, "
                    "idx : {}, ret: {}", idx, ret);

    return ret;
}

hal_ret_t
p4pd_table_defaults_init (p4pd_def_cfg_t *p4pd_def_cfg)
{
    // hack for initial set of DOLs for Uplink -> Uplink
    HAL_ASSERT(p4pd_rx_vport_init() == HAL_RET_OK);

    return HAL_RET_OK;
}

}    // namespace pd
}    // namespace hal

