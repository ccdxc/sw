//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// athena connection tracking implementation
///
//----------------------------------------------------------------------------

#include "nic/sdk/include/sdk/base.hpp"
#include "nic/sdk/lib/p4/p4_api.hpp"
#include "nic/sdk/lib/p4/p4_utils.hpp"
#include "nic/apollo/core/trace.hpp"
#include "nic/apollo/api/include/athena/pds_conntrack.h"
#include "gen/p4gen/athena/include/p4pd.h"

using namespace sdk;

extern "C" {

sdk_ret_t
pds_conntrack_state_create (pds_conntrack_spec_t *spec)
{
    p4pd_error_t p4pd_ret;
    uint32_t conntrack_id;
    conntrack_actiondata_t conntrack_actiondata = { 0 };

    if (!spec) {
        PDS_TRACE_ERR("spec is null");
        return SDK_RET_INVALID_ARG;
    }
    conntrack_id = spec->key.conntrack_id;
    if (conntrack_id >= PDS_CONNTRACK_ID_MAX) {
        PDS_TRACE_ERR("conntrack id %u is beyond range", conntrack_id);
        return SDK_RET_INVALID_ARG;
    }

    conntrack_actiondata.action_id = CONNTRACK_CONNTRACK_ID;
    conntrack_actiondata.action_u.conntrack_conntrack.flow_type =
        spec->data.flow_type;
    conntrack_actiondata.action_u.conntrack_conntrack.flow_state =
        spec->data.flow_state;
    p4pd_ret = p4pd_global_entry_write(P4TBL_ID_CONNTRACK,
                                       conntrack_id, NULL, NULL,
                                       &conntrack_actiondata);
    if (p4pd_ret != P4PD_SUCCESS) {
        PDS_TRACE_ERR("Failed to write conntrack table at index %u",
                      conntrack_id);
        return SDK_RET_HW_PROGRAM_ERR;
    }
    return SDK_RET_OK;
}

sdk_ret_t
pds_conntrack_state_read (pds_conntrack_key_t *key,
                          pds_conntrack_info_t *info)
{
    p4pd_error_t p4pd_ret;
    uint32_t conntrack_id;
    conntrack_actiondata_t conntrack_actiondata = { 0 };

    if (!key || !info) {
        PDS_TRACE_ERR("key/info is null");
        return SDK_RET_INVALID_ARG;
    }
    conntrack_id = key->conntrack_id;
    if (conntrack_id >= PDS_CONNTRACK_ID_MAX) {
        PDS_TRACE_ERR("conntrack id %u is beyond range", conntrack_id);
        return SDK_RET_INVALID_ARG;
    }

    p4pd_ret = p4pd_global_entry_read(P4TBL_ID_CONNTRACK,
                                      conntrack_id, NULL, NULL,
                                      &conntrack_actiondata);
    if (p4pd_ret != P4PD_SUCCESS) {
        PDS_TRACE_ERR("Failed to read conntrack table at index %u",
                      conntrack_id);
        return SDK_RET_HW_READ_ERR;
    }
    info->spec.data.flow_type =
        (pds_flow_type_t)conntrack_actiondata.action_u.conntrack_conntrack.flow_type;
    info->spec.data.flow_state =
        (pds_flow_state_t)conntrack_actiondata.action_u.conntrack_conntrack.flow_state;
    info->status.timestamp = conntrack_actiondata.action_u.conntrack_conntrack.timestamp;
    return SDK_RET_OK;
}

sdk_ret_t
pds_conntrack_state_update (pds_conntrack_spec_t *spec)
{
    return pds_conntrack_state_create(spec);
}

sdk_ret_t
pds_conntrack_state_delete (pds_conntrack_key_t *key)
{
    p4pd_error_t p4pd_ret;
    uint32_t conntrack_id;
    conntrack_actiondata_t conntrack_actiondata = { 0 };

    if (!key) {
        PDS_TRACE_ERR("key is null");
        return SDK_RET_INVALID_ARG;
    }
    conntrack_id = key->conntrack_id;
    if (conntrack_id >= PDS_CONNTRACK_ID_MAX) {
        PDS_TRACE_ERR("conntrack id %u is beyond range", conntrack_id);
        return SDK_RET_INVALID_ARG;
    }

    p4pd_ret = p4pd_global_entry_write(P4TBL_ID_CONNTRACK,
                                       conntrack_id, NULL, NULL,
                                       &conntrack_actiondata);
    if (p4pd_ret != P4PD_SUCCESS) {
        PDS_TRACE_ERR("Failed to clear conntrack table at index %u",
                      conntrack_id);
        return SDK_RET_HW_PROGRAM_ERR;
    }
    return SDK_RET_OK;
}

}
