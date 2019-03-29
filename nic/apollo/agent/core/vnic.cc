//------------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// -----------------------------------------------------------------------------

#include "nic/apollo/agent/trace.hpp"
#include "nic/apollo/agent/core/state.hpp"
#include "nic/apollo/agent/core/tunnel.hpp"
#include "nic/apollo/agent/core/vnic.hpp"

extern bool g_pds_mock_mode;

namespace core {

static inline sdk_ret_t
vnic_create_validate (pds_vnic_spec_t *spec)
{
    // check if vnic exists
    if (agent_state::state()->find_in_vpc_db(&spec->vcn) == NULL) {
        PDS_TRACE_ERR("Failed to create vnic %u, vpc %u not found",
                      spec->key.id, spec->key.id);
        return sdk::SDK_RET_ENTRY_NOT_FOUND;
    }

    // check if subnet exists
    if (agent_state::state()->find_in_subnet_db(&spec->subnet) == NULL) {
        PDS_TRACE_ERR("Failed to create vnic %u, subnet %u not found",
                      spec->key.id, spec->subnet.id);
        return sdk::SDK_RET_ENTRY_NOT_FOUND;
    }
    return SDK_RET_OK;
}

sdk_ret_t
vnic_create (pds_vnic_key_t *key, pds_vnic_spec_t *spec)
{
    sdk_ret_t ret = SDK_RET_OK;

    if (agent_state::state()->find_in_vnic_db(key) != NULL) {
        PDS_TRACE_ERR("Failed to create vnic %u, vnic exists already",
                      spec->key.id);
        return sdk::SDK_RET_ENTRY_EXISTS;
    }

    if ((ret = vnic_create_validate(spec)) != SDK_RET_OK) {
        PDS_TRACE_ERR("vnic %u validation failure, err %u",
                      spec->key.id, ret);
        return ret;
    }
    
    if (!g_pds_mock_mode) {
        if ((ret = pds_vnic_create(spec)) != SDK_RET_OK) {
            PDS_TRACE_ERR("Failed to create vnic %u, err %u",
                          spec->key.id, ret);
            return ret;
        }
    }

    if ((ret = agent_state::state()->add_to_vnic_db(key, spec)) != SDK_RET_OK) {
        PDS_TRACE_ERR("Failed to add vnic %u to db, err %u",
                      spec->key.id, ret);
        return ret;
    }
    return SDK_RET_OK;
}

sdk_ret_t
vnic_delete (pds_vnic_key_t *key)
{
    sdk_ret_t ret = SDK_RET_OK;

    if (agent_state::state()->find_in_vnic_db(key) == NULL) {
        PDS_TRACE_ERR("Failed to delete vnic %u, vnic not found", key->id);
        return SDK_RET_ENTRY_NOT_FOUND;
    }

    if (!g_pds_mock_mode) {
        if ((ret = pds_vnic_delete(key)) != SDK_RET_OK) {
            PDS_TRACE_ERR("Failed to delete vnic %u, err %u", key->id, ret);
            return ret;
        }
    }

    if (agent_state::state()->del_from_vnic_db(key) == false) {
        PDS_TRACE_ERR("Failed to delete vnic %u from vnic db", key->id);
        return SDK_RET_ERR;
    }
    return SDK_RET_OK;
}

sdk_ret_t
vnic_get (pds_vnic_key_t *key, pds_vnic_info_t *info)
{
    sdk_ret_t ret = SDK_RET_OK;
    pds_vnic_spec_t *spec;

    spec = agent_state::state()->find_in_vnic_db(key);
    if (spec == NULL) {
        return SDK_RET_ENTRY_NOT_FOUND;
    }
    memcpy(&info->spec, spec, sizeof(pds_vnic_spec_t));
    if (!g_pds_mock_mode) {
        ret = pds_vnic_read(key, info);
    }
    return ret;
}

static inline sdk_ret_t
vnic_get_all_cb (pds_vnic_spec_t *spec, void *ctxt)
{
    sdk_ret_t ret = SDK_RET_OK;
    pds_vnic_info_t info;
    vnic_db_cb_ctxt_t *cb_ctxt = (vnic_db_cb_ctxt_t *)ctxt;

    memcpy(&info.spec, spec, sizeof(pds_vnic_spec_t));
    if (!g_pds_mock_mode) {
        ret = pds_vnic_read(&spec->key, &info);
    }
    if (ret == SDK_RET_OK) {
        cb_ctxt->cb(&info, cb_ctxt->ctxt);
    }
    return ret;
}

sdk_ret_t
vnic_get_all (vnic_get_cb_t vnic_get_cb, void *ctxt)
{
    vnic_db_cb_ctxt_t cb_ctxt;

    cb_ctxt.cb = vnic_get_cb;
    cb_ctxt.ctxt = ctxt;

     return agent_state::state()->vnic_db_walk(vnic_get_all_cb, &cb_ctxt);
}

}    // namespace core
