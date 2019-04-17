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
    // check if vnic exists
    if (agent_state::state()->find_in_vpc_db(&spec->vcn) == NULL) {
        PDS_TRACE_ERR("Failed to create vnic {}, vpc {} not found",
                      spec->key.id, spec->key.id);
        return sdk::SDK_RET_ENTRY_NOT_FOUND;
    }

    // check if subnet exists
    if (agent_state::state()->find_in_subnet_db(&spec->subnet) == NULL) {
        PDS_TRACE_ERR("Failed to create vnic {}, subnet {} not found",
                      spec->key.id, spec->subnet.id);
        return sdk::SDK_RET_ENTRY_NOT_FOUND;
    }
    return SDK_RET_OK;
}

sdk_ret_t
vnic_create (pds_vnic_key_t *key, pds_vnic_spec_t *spec)
{
    sdk_ret_t ret;

    if (agent_state::state()->find_in_vnic_db(key) != NULL) {
        PDS_TRACE_ERR("Failed to create vnic {}, vnic exists already",
                      spec->key.id);
        return sdk::SDK_RET_ENTRY_EXISTS;
    }

    if ((ret = vnic_create_validate(spec)) != SDK_RET_OK) {
        PDS_TRACE_ERR("vnic {} validation failure, err {}",
                      spec->key.id, ret);
        return ret;
    }

    if (!agent_state::state()->pds_mock_mode()) {
        if ((ret = pds_vnic_create(spec)) != SDK_RET_OK) {
            PDS_TRACE_ERR("Failed to create vnic {}, err {}",
                          spec->key.id, ret);
            return ret;
        }
    }

    if ((ret = agent_state::state()->add_to_vnic_db(key, spec)) != SDK_RET_OK) {
        PDS_TRACE_ERR("Failed to add vnic {} to db, err {}",
                      spec->key.id, ret);
        return ret;
    }
    return SDK_RET_OK;
}

sdk_ret_t
vnic_delete (pds_vnic_key_t *key)
{
    sdk_ret_t ret;

    if (agent_state::state()->find_in_vnic_db(key) == NULL) {
        PDS_TRACE_ERR("Failed to delete vnic {}, vnic not found", key->id);
        return SDK_RET_ENTRY_NOT_FOUND;
    }

    if (!agent_state::state()->pds_mock_mode()) {
        if ((ret = pds_vnic_delete(key)) != SDK_RET_OK) {
            PDS_TRACE_ERR("Failed to delete vnic {}, err {}", key->id, ret);
            return ret;
        }
    }

    if (agent_state::state()->del_from_vnic_db(key) == false) {
        PDS_TRACE_ERR("Failed to delete vnic {} from vnic db", key->id);
        return SDK_RET_ERR;
    }
    return SDK_RET_OK;
}

sdk_ret_t
vnic_get (pds_vnic_key_t *key, pds_vnic_info_t *info)
{
    sdk_ret_t ret;
    pds_vnic_spec_t *spec;

    spec = agent_state::state()->find_in_vnic_db(key);
    if (spec == NULL) {
        return SDK_RET_ENTRY_NOT_FOUND;
    }
    memcpy(&info->spec, spec, sizeof(pds_vnic_spec_t));
    if (!agent_state::state()->pds_mock_mode()) {
        ret = pds_vnic_read(key, info);
    }
    return ret;
}

static inline sdk_ret_t
vnic_get_all_cb (pds_vnic_spec_t *spec, void *ctxt)
{
    sdk_ret_t ret;
    pds_vnic_info_t info;
    vnic_db_cb_ctxt_t *cb_ctxt = (vnic_db_cb_ctxt_t *)ctxt;

    memcpy(&info.spec, spec, sizeof(pds_vnic_spec_t));
    if (!agent_state::state()->pds_mock_mode()) {
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
