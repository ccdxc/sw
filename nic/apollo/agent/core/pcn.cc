//------------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// -----------------------------------------------------------------------------

#include "nic/apollo/agent/core/state.hpp"
#include "nic/apollo/agent/core/pcn.hpp"

namespace core {

sdk_ret_t
pcn_create (pds_vcn_key_t *key, pds_vcn_spec_t *spec)
{
    if (agent_state::state()->find_in_pcn_db(key) != NULL) {
        return sdk::SDK_RET_ENTRY_EXISTS;
    }
    if (pds_vcn_create(spec) != sdk::SDK_RET_OK) {
        return sdk::SDK_RET_ERR;
    }
    if (agent_state::state()->add_to_pcn_db(key, spec) != sdk::SDK_RET_OK) {
        return sdk::SDK_RET_ERR;
    }
    return sdk::SDK_RET_OK;
}

sdk_ret_t
pcn_delete (pds_vcn_key_t *key)
{
    if (agent_state::state()->find_in_pcn_db(key) == NULL) {
        return sdk::SDK_RET_ENTRY_NOT_FOUND;
    }
    if (pds_vcn_delete(key) != sdk::SDK_RET_OK) {
        return sdk::SDK_RET_ERR;
    }
    if (agent_state::state()->del_from_pcn_db(key) == false) {
        return sdk::SDK_RET_ERR;
    }
    return sdk::SDK_RET_OK;
}

sdk_ret_t
pcn_get (pds_vcn_key_t *key, pds_vcn_info_t *info)
{
    sdk_ret_t ret;
    pds_vcn_spec_t *spec;

    spec = agent_state::state()->find_in_pcn_db(key);
    if (spec == NULL) {
        return sdk::SDK_RET_ENTRY_NOT_FOUND;
    }

    memcpy(&info->spec, spec, sizeof(pds_vcn_spec_t));
    ret = pds_vcn_read(key, info);
    return ret;
}

static inline sdk_ret_t
pcn_get_all_cb (pds_vcn_spec_t *spec, void *ctxt)
{
    sdk_ret_t ret;
    pds_vcn_info_t info;
    pcn_db_cb_ctxt_t *cb_ctxt = (pcn_db_cb_ctxt_t *)ctxt;

    memcpy(&info.spec, spec, sizeof(pds_vcn_spec_t));
    ret = pds_vcn_read(&spec->key, &info);
    if (ret == SDK_RET_OK) {
        cb_ctxt->cb(&info, cb_ctxt->ctxt);
    }
    return ret;
}

sdk_ret_t
pcn_get_all (pcn_get_cb_t pcn_get_cb, void *ctxt)
{
    pcn_db_cb_ctxt_t cb_ctxt;

    cb_ctxt.cb = pcn_get_cb;
    cb_ctxt.ctxt = ctxt;

    return agent_state::state()->pcn_db_walk(pcn_get_all_cb, &cb_ctxt);
}

}    // namespace core
