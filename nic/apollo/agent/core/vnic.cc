//------------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// -----------------------------------------------------------------------------

#include "nic/apollo/agent/core/state.hpp"
#include "nic/apollo/agent/core/tunnel.hpp"
#include "nic/apollo/agent/core/vnic.hpp"

namespace core {

sdk_ret_t
vnic_create (pds_vnic_key_t *key, pds_vnic_spec_t *spec)
{
    if (agent_state::state()->find_in_vnic_db(key) != NULL) {
        return sdk::SDK_RET_ENTRY_EXISTS;
    }
    if (pds_vnic_create(spec) != sdk::SDK_RET_OK) {
        return sdk::SDK_RET_ERR;
    }
    if (agent_state::state()->add_to_vnic_db(key, spec) != sdk::SDK_RET_OK) {
        return sdk::SDK_RET_ERR;
    }
    return sdk::SDK_RET_OK;
}

sdk_ret_t
vnic_delete (pds_vnic_key_t *key)
{
    if (agent_state::state()->find_in_vnic_db(key) == NULL) {
        return sdk::SDK_RET_ENTRY_NOT_FOUND;
    }
    if (pds_vnic_delete(key) != sdk::SDK_RET_OK) {
        return sdk::SDK_RET_ERR;
    }
    if (agent_state::state()->del_from_vnic_db(key) == false) {
        return sdk::SDK_RET_ERR;
    }
    return sdk::SDK_RET_OK;
}

sdk_ret_t
vnic_get (pds_vnic_key_t *key, pds_vnic_info_t *info)
{
    sdk_ret_t ret;
    pds_vnic_spec_t *spec;

    spec = agent_state::state()->find_in_vnic_db(key);
    if (spec == NULL) {
        return sdk::SDK_RET_ENTRY_NOT_FOUND;
    }

    memcpy(&info->spec, spec, sizeof(pds_vnic_spec_t));
    ret = pds_vnic_read(key, info);
    return ret;
}

static inline sdk_ret_t
vnic_get_all_cb (pds_vnic_spec_t *spec, void *ctxt)
{
    sdk_ret_t ret;
    pds_vnic_info_t info;
    vnic_db_cb_ctxt_t *cb_ctxt = (vnic_db_cb_ctxt_t *)ctxt;

    memcpy(&info.spec, spec, sizeof(pds_vnic_spec_t));
    ret = pds_vnic_read(&spec->key, &info);
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
