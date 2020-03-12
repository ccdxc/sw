//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// athena epoch implementation
///
//----------------------------------------------------------------------------

#include "nic/sdk/include/sdk/base.hpp"
#include "nic/sdk/lib/p4/p4_api.hpp"
#include "nic/sdk/lib/p4/p4_utils.hpp"
#include "nic/apollo/core/trace.hpp"
#include "nic/apollo/api/include/athena/pds_epoch.h"
#include "gen/p4gen/athena/include/p4pd.h"

using namespace sdk;

extern "C" {

sdk_ret_t
pds_epoch_create (pds_epoch_spec_t *spec)
{
    p4pd_error_t p4pd_ret;
    uint32_t epoch_id;
    config1_actiondata_t config1_actiondata = { 0 };
    
    if (!spec) {
        PDS_TRACE_ERR("spec is null");
        return SDK_RET_INVALID_ARG;
    }
    epoch_id = spec->key.epoch_id;
    if (epoch_id == 0 || epoch_id >= PDS_EPOCH_ID_MAX) {
        PDS_TRACE_ERR("epoch index %u is beyond range", epoch_id);
        return SDK_RET_INVALID_ARG;
    }

    config1_actiondata.action_id = CONFIG1_CONFIG1_EPOCH_VERIFY_ID;
    config1_actiondata.action_u.config1_config1_epoch_verify.epoch =
        spec->data.epoch;
    p4pd_ret = p4pd_global_entry_write(P4TBL_ID_CONFIG1,
                                       epoch_id, NULL, NULL,
                                       &config1_actiondata);
    if (p4pd_ret != P4PD_SUCCESS) {
        PDS_TRACE_ERR("Failed to write epoch table at index %u",
                      epoch_id);
        return SDK_RET_HW_PROGRAM_ERR;
    }
    return SDK_RET_OK;
}

sdk_ret_t
pds_epoch_read (pds_epoch_key_t *key, pds_epoch_info_t *info)
{
    p4pd_error_t p4pd_ret;
    uint32_t epoch_id;
    config1_actiondata_t config1_actiondata = { 0 };
 
    if (!key || !info) {
        PDS_TRACE_ERR("key/info is null");
        return SDK_RET_INVALID_ARG;
    }
    epoch_id = key->epoch_id;
    if (epoch_id == 0 || epoch_id >= PDS_EPOCH_ID_MAX) {
        PDS_TRACE_ERR("epoch index %u is beyond range", epoch_id);
        return SDK_RET_INVALID_ARG;
    }

    p4pd_ret = p4pd_global_entry_read(P4TBL_ID_CONFIG1,
                                      epoch_id, NULL, NULL,
                                      &config1_actiondata);
    if (p4pd_ret != P4PD_SUCCESS) {
        PDS_TRACE_ERR("Failed to read epoch table at index %u",
                      epoch_id);
        return SDK_RET_HW_READ_ERR;
    }
    info->spec.data.epoch =
        config1_actiondata.action_u.config1_config1_epoch_verify.epoch;
    return SDK_RET_OK;
}

sdk_ret_t
pds_epoch_update (pds_epoch_spec_t *spec)
{
    return pds_epoch_create(spec);
}

sdk_ret_t
pds_epoch_delete (pds_epoch_key_t *key)
{
    p4pd_error_t p4pd_ret;
    uint32_t epoch_id;
    config1_actiondata_t config1_actiondata = { 0 };
 
    if (!key) {
        PDS_TRACE_ERR("key is null");
        return SDK_RET_INVALID_ARG;
    }
    epoch_id = key->epoch_id;
    if (epoch_id == 0 || epoch_id >= PDS_EPOCH_ID_MAX) {
        PDS_TRACE_ERR("epoch index %u is beyond range", epoch_id);
        return SDK_RET_INVALID_ARG;
    }

    p4pd_ret = p4pd_global_entry_write(P4TBL_ID_CONFIG1,
                                       epoch_id, NULL, NULL,
                                       &config1_actiondata);
    if (p4pd_ret != P4PD_SUCCESS) {
        PDS_TRACE_ERR("Failed to clear epoch table at index %u",
                      epoch_id);
        return SDK_RET_HW_PROGRAM_ERR;
    }
    return SDK_RET_OK;
}

}
