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
#include "gen/p4gen/p4/include/ftl.h"

using namespace sdk;

extern "C" {

pds_ret_t
pds_epoch_create (pds_epoch_spec_t *spec)
{
    p4pd_error_t                 p4pd_ret = P4PD_SUCCESS;
    uint32_t                     epoch_id = PDS_EPOCH_ID_MAX;
    config1_epoch_verify_entry_t entry = {0};
    
    if (!spec) {
        PDS_TRACE_ERR("spec is null");
        return PDS_RET_INVALID_ARG;
    }
    epoch_id = spec->key.epoch_id;
    if (epoch_id == 0 || epoch_id >= PDS_EPOCH_ID_MAX) {
        PDS_TRACE_ERR("epoch index %u is beyond range", epoch_id);
        return PDS_RET_INVALID_ARG;
    }

    entry.clear();
    entry.set_epoch(spec->data.epoch);
    p4pd_ret = entry.write(epoch_id);
    if (p4pd_ret != P4PD_SUCCESS) {
        PDS_TRACE_ERR("Failed to write epoch table at index %u",
                      epoch_id);
        return PDS_RET_HW_PROGRAM_ERR;
    }
    return PDS_RET_OK;
}

pds_ret_t
pds_epoch_read (pds_epoch_key_t *key, pds_epoch_info_t *info)
{
    p4pd_error_t                 p4pd_ret = P4PD_SUCCESS;
    uint32_t                     epoch_id = PDS_EPOCH_ID_MAX;
    config1_epoch_verify_entry_t entry = { 0 };
 
    if (!key || !info) {
        PDS_TRACE_ERR("key/info is null");
        return PDS_RET_INVALID_ARG;
    }
    epoch_id = key->epoch_id;
    if (epoch_id == 0 || epoch_id >= PDS_EPOCH_ID_MAX) {
        PDS_TRACE_ERR("epoch index %u is beyond range", epoch_id);
        return PDS_RET_INVALID_ARG;
    }

    entry.clear();
    p4pd_ret = entry.read(epoch_id);
    if (p4pd_ret != P4PD_SUCCESS) {
        PDS_TRACE_ERR("Failed to read epoch table at index %u",
                      epoch_id);
        return PDS_RET_HW_READ_ERR;
    }
    info->spec.data.epoch = entry.get_epoch();
    return PDS_RET_OK;
}

pds_ret_t
pds_epoch_update (pds_epoch_spec_t *spec)
{
    return pds_epoch_create(spec);
}

pds_ret_t
pds_epoch_delete (pds_epoch_key_t *key)
{
    p4pd_error_t                 p4pd_ret = P4PD_SUCCESS;
    uint32_t                     epoch_id = PDS_EPOCH_ID_MAX;
    config1_epoch_verify_entry_t entry = { 0 };
 
    if (!key) {
        PDS_TRACE_ERR("key is null");
        return PDS_RET_INVALID_ARG;
    }
    epoch_id = key->epoch_id;
    if (epoch_id == 0 || epoch_id >= PDS_EPOCH_ID_MAX) {
        PDS_TRACE_ERR("epoch index %u is beyond range", epoch_id);
        return PDS_RET_INVALID_ARG;
    }

    entry.clear();
    p4pd_ret = entry.write(epoch_id);
    if (p4pd_ret != P4PD_SUCCESS) {
        PDS_TRACE_ERR("Failed to clear epoch table at index %u",
                      epoch_id);
        return PDS_RET_HW_PROGRAM_ERR;
    }
    return PDS_RET_OK;
}

}
