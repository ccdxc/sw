//------------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// -----------------------------------------------------------------------------

#include "nic/apollo/agent/trace.hpp"
#include "nic/apollo/agent/core/state.hpp"
#include "nic/apollo/agent/core/tunnel.hpp"
#include "nic/apollo/agent/core/vnic.hpp"

namespace core {

static inline sdk_ret_t
vnic_create_validate (pds_vnic_spec_t *spec)
{
    // check if subnet exists
    if (agent_state::state()->find_in_subnet_db(&spec->subnet) == NULL) {
        PDS_TRACE_ERR("Failed to create vnic {}, subnet {} not found",
                      spec->key.str(), spec->subnet.str());
        return sdk::SDK_RET_ENTRY_NOT_FOUND;
    }
    return SDK_RET_OK;
}

sdk_ret_t
vnic_create (pds_obj_key_t *key, pds_vnic_spec_t *spec,
             pds_batch_ctxt_t bctxt)
{
    sdk_ret_t ret;

    if ((ret = vnic_create_validate(spec)) != SDK_RET_OK) {
        PDS_TRACE_ERR("vnic {} validation failure, err {}",
                      spec->key.str(), ret);
        return ret;
    }

    if (!agent_state::state()->pds_mock_mode()) {
        if ((ret = pds_vnic_create(spec, bctxt)) != SDK_RET_OK) {
            PDS_TRACE_ERR("Failed to create vnic {}, err {}",
                          spec->key.str(), ret);
            return ret;
        }
    }

    return SDK_RET_OK;
}

static inline sdk_ret_t
vnic_update_validate (pds_vnic_spec_t *spec)
{
    // check if subnet exists
    if (agent_state::state()->find_in_subnet_db(&spec->subnet) == NULL) {
        PDS_TRACE_ERR("Failed to update vnic {}, subnet {} not found",
                      spec->key.str(), spec->subnet.str());
        return sdk::SDK_RET_ENTRY_NOT_FOUND;
    }
    return SDK_RET_OK;
}

sdk_ret_t
vnic_update (pds_obj_key_t *key, pds_vnic_spec_t *spec,
             pds_batch_ctxt_t bctxt)
{
    sdk_ret_t ret;

    if ((ret = vnic_update_validate(spec)) != SDK_RET_OK) {
        PDS_TRACE_ERR("vnic {} validation failure, err {}",
                      spec->key.str(), ret);
        return ret;
    }

    if (!agent_state::state()->pds_mock_mode()) {
        if ((ret = pds_vnic_update(spec, bctxt)) != SDK_RET_OK) {
            PDS_TRACE_ERR("Failed to create vnic {}, err {}",
                          spec->key.str(), ret);
            return ret;
        }
    }

    return SDK_RET_OK;
}

sdk_ret_t
vnic_delete (pds_obj_key_t *key, pds_batch_ctxt_t bctxt)
{
    sdk_ret_t ret;

    if (!agent_state::state()->pds_mock_mode()) {
        if ((ret = pds_vnic_delete(key, bctxt)) != SDK_RET_OK) {
            PDS_TRACE_ERR("Failed to delete vnic {}, err {}", key->str(), ret);
            return ret;
        }
    }

    return SDK_RET_OK;
}

sdk_ret_t
vnic_get (pds_obj_key_t *key, pds_vnic_info_t *info)
{
    sdk_ret_t ret = SDK_RET_OK;

    if (!agent_state::state()->pds_mock_mode()) {
        ret = pds_vnic_read(key, info);
    } else {
        memset(&info->spec, 0, sizeof(info->spec));
        memset(&info->stats, 0, sizeof(info->stats));
        memset(&info->status, 0, sizeof(info->status));
    }
    return ret;
}

sdk_ret_t
vnic_get_all (vnic_read_cb_t vnic_read_cb, void *ctxt)
{
    return pds_vnic_read_all(vnic_read_cb, ctxt);
}

}    // namespace core
