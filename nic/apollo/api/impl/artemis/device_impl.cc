//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// datapath implementation of device
///
//----------------------------------------------------------------------------

#include "nic/sdk/include/sdk/base.hpp"
#include "nic/sdk/asic/pd/pd.hpp"
#include "nic/apollo/core/mem.hpp"
#include "nic/apollo/core/trace.hpp"
#include "nic/apollo/api/device.hpp"
#include "nic/apollo/api/impl/device_impl.hpp"
#include "gen/p4gen/artemis/include/p4pd.h"
#include "nic/sdk/lib/utils/utils.hpp"

namespace api {
namespace impl {

// as there is no state in this impl, single instance is good enough
device_impl    g_device_impl;

device_impl *
device_impl::factory(pds_device_spec_t *pds_device) {
    return &g_device_impl;
}

void
device_impl::destroy(device_impl *impl) {
    return;
}

void
device_impl::fill_spec_(pds_device_spec_t *spec) {
}

void
device_impl::fill_ing_drop_stats_(pds_device_ing_drop_stats_t *ing_drop_stats) {
#if 0
    p4pd_error_t pd_err = P4PD_SUCCESS;
    uint64_t pkts = 0;
    p4i_drop_stats_swkey_t key = { 0 };
    p4i_drop_stats_swkey_mask_t key_mask = { 0 };
    p4i_drop_stats_actiondata_t data = { 0 };

    for (uint32_t i = P4I_DROP_REASON_MIN; i <= P4I_DROP_REASON_MAX; ++i) {
        if (p4pd_global_entry_read(P4TBL_ID_P4I_DROP_STATS, i,
                                   &key, &key_mask, &data) == P4PD_SUCCESS) {
            memcpy(&pkts,
                   data.action_u.p4i_drop_stats_p4i_drop_stats.drop_stats_pkts,
                   sizeof(data.action_u.p4i_drop_stats_p4i_drop_stats.drop_stats_pkts));
            ing_drop_stats->drop_stats_pkts[i] = pkts;
        }
    }
#endif
    return;
}

sdk_ret_t
device_impl::read_hw(pds_device_info_t *info) {
    fill_spec_(&info->spec);
    fill_ing_drop_stats_(&info->stats.ing_drop_stats);
    return sdk::SDK_RET_OK;
}

sdk_ret_t
device_impl::activate_hw(api_base *api_obj, pds_epoch_t epoch,
                         api_op_t api_op, obj_ctxt_t *obj_ctxt) {
    return SDK_RET_OK;
}

}    // namespace impl
}    // namespace api
