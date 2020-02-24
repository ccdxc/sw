//------------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// -----------------------------------------------------------------------------

#include "nic/apollo/agent/trace.hpp"
#include "nic/apollo/agent/core/interface.hpp"
#include "nic/apollo/agent/core/state.hpp"
#include "nic/metaswitch/stubs/mgmt/pds_ms_interface.hpp"

namespace core {

sdk_ret_t
interface_create (pds_if_spec_t *spec,
                  pds_batch_ctxt_t bctxt)
{
    sdk_ret_t ret;
    pds_if_info_t info;

    if (agent_state::state()->device()->overlay_routing_en) {
        // call the metaswitch api
        if ((ret = pds_ms::interface_create(spec, bctxt)) != SDK_RET_OK) {
            PDS_TRACE_ERR("Failed to create interface {}, err {}",
                          spec->key.str(), ret);
            return ret;
        }
    } else if (!agent_state::state()->pds_mock_mode()) {
        if (pds_if_read(&spec->key, &info) != SDK_RET_ENTRY_NOT_FOUND) {
            PDS_TRACE_ERR("Failed to create interface {}, interface "
                          "exists already", spec->key.str());
            return sdk::SDK_RET_ENTRY_EXISTS;
        }
        if ((ret = pds_if_create(spec, bctxt)) != SDK_RET_OK) {
            PDS_TRACE_ERR("Failed to create interface {}, err {}",
                          spec->key.str(), ret);
            return ret;
        }
    }

    return SDK_RET_OK;
}

sdk_ret_t
interface_update (pds_if_spec_t *spec,
                  pds_batch_ctxt_t bctxt)
{
    sdk_ret_t ret;
    pds_if_info_t info;

    if (agent_state::state()->device()->overlay_routing_en) {
        // call the metaswitch api
        if ((ret = pds_ms::interface_update(spec, bctxt)) != SDK_RET_OK) {
            PDS_TRACE_ERR("Failed to update interface {}, err {}",
                          spec->key.str(), ret);
            return ret;
        }
    } else if (!agent_state::state()->pds_mock_mode()) {
        if (pds_if_read(&spec->key, &info) != SDK_RET_OK) {
            PDS_TRACE_ERR("Failed to update interface {}, interface not found",
                          spec->key.str());
            return sdk::SDK_RET_ENTRY_EXISTS;
        }
        if ((ret = pds_if_update(spec, bctxt)) != SDK_RET_OK) {
            PDS_TRACE_ERR("Failed to update interface {}, err {}",
                          spec->key.str(), ret);
            return ret;
        }
    }

    return SDK_RET_OK;
}

sdk_ret_t
interface_delete (pds_obj_key_t *key, pds_batch_ctxt_t bctxt)
{
    sdk_ret_t ret;
    pds_if_info_t info;

    if (agent_state::state()->device()->overlay_routing_en) {
        // call the metaswitch api
        if ((ret = pds_ms::interface_delete(&info.spec, bctxt)) != SDK_RET_OK) {
            PDS_TRACE_ERR("Failed to delete interface {}, err {}",
                          key->str(), ret);
            return ret;
        }
    } else if (!agent_state::state()->pds_mock_mode()) {
        if (pds_if_read(key, &info) != SDK_RET_OK) {
            PDS_TRACE_ERR("Failed to delete interface {}, interface not found",
                          key->str());
            return sdk::SDK_RET_ENTRY_EXISTS;
        }
        if ((ret = pds_if_delete(key, bctxt)) != SDK_RET_OK) {
            PDS_TRACE_ERR("Failed to delete interface {}, err {}",
                          key->str(), ret);
            return ret;
        }
    }

    return SDK_RET_OK;
}

sdk_ret_t
interface_get (pds_obj_key_t *key, pds_if_info_t *info)
{
    sdk_ret_t ret = SDK_RET_OK;

    if (!agent_state::state()->pds_mock_mode()) {
        ret = pds_if_read(key, info);
    } else {
        memset(&info->spec, 0, sizeof(info->spec));
        memset(&info->stats, 0, sizeof(info->stats));
        memset(&info->status, 0, sizeof(info->status));
    }
    return ret;
}

sdk_ret_t
interface_get_all (if_read_cb_t cb, void *ctxt)
{
    return pds_if_read_all(cb, ctxt);
}

}    // namespace core
