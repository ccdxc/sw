//------------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// -----------------------------------------------------------------------------

#include "nic/apollo/agent/trace.hpp"
#include "nic/apollo/agent/core/state.hpp"
#include "nic/apollo/agent/core/tunnel.hpp"

namespace core {

static inline sdk_ret_t
tep_create_validate (pds_tep_spec_t *spec)
{
    if (spec->type == PDS_TEP_TYPE_SERVICE) {
        // service TEPs must have vxlan encap
        if ((spec->encap.type != PDS_ENCAP_TYPE_VXLAN) ||
            (spec->encap.val.vnid == 0)) {
            PDS_TRACE_ERR("Service TEP %s has invalid vxlan encap (%u, %u)",
                          ipaddr2str(&spec->key.ip_addr),
                          spec->encap.type, spec->encap.val.vnid);
            return sdk::SDK_RET_INVALID_ARG;
        }
    }
    return SDK_RET_OK;
}

sdk_ret_t
tep_create (uint32_t key, pds_tep_spec_t *spec)
{
    sdk_ret_t ret;

    if (agent_state::state()->find_in_tep_db(key) != NULL) {
        return sdk::SDK_RET_ENTRY_EXISTS;
    }
    if (!agent_state::state()->pds_mock_mode()) {
        if ((ret = pds_tep_create(spec)) != sdk::SDK_RET_OK) {
            return ret;
        }
    }
    if (agent_state::state()->add_to_tep_db(key, spec) != sdk::SDK_RET_OK) {
        return sdk::SDK_RET_ERR;
    }
    return sdk::SDK_RET_OK;
}

sdk_ret_t
tep_update (uint32_t key, pds_tep_spec_t *spec)
{
    sdk_ret_t ret;

    if (agent_state::state()->find_in_tep_db(key) == NULL) {
        return sdk::SDK_RET_ENTRY_NOT_FOUND;
    }
    if (!agent_state::state()->pds_mock_mode()) {
        if ((ret = pds_tep_update(spec)) != sdk::SDK_RET_OK) {
            return ret;
        }
    }
    if (agent_state::state()->del_from_tep_db(key) == false) {
    }
    if (agent_state::state()->add_to_tep_db(key, spec) != sdk::SDK_RET_OK) {
        return sdk::SDK_RET_ERR;
    }
    return sdk::SDK_RET_OK;
}

sdk_ret_t
tep_delete (uint32_t key)
{
    sdk_ret_t ret;
    pds_tep_spec_t *spec;

    spec = agent_state::state()->find_in_tep_db(key);
    if (spec == NULL) {
        return sdk::SDK_RET_ENTRY_NOT_FOUND;
    }
    if (!agent_state::state()->pds_mock_mode()) {
        if ((ret = pds_tep_delete(&spec->key)) != sdk::SDK_RET_OK) {
            return ret;
        }
    }
    if (agent_state::state()->del_from_tep_db(key) == false) {
        return sdk::SDK_RET_ERR;
    }
    return sdk::SDK_RET_OK;
}

sdk_ret_t
tep_get (uint32_t key, pds_tep_info_t *info)
{
    sdk_ret_t ret = SDK_RET_OK;
    pds_tep_spec_t *spec;

    spec = agent_state::state()->find_in_tep_db(key);
    if (spec == NULL) {
        return sdk::SDK_RET_ENTRY_NOT_FOUND;
    }
    memcpy(&info->spec, spec, sizeof(pds_tep_spec_t));
    if (!agent_state::state()->pds_mock_mode()) {
        ret = pds_tep_read(&spec->key, info);
    } else {
        memset(&info->stats, 0, sizeof(info->stats));
        memset(&info->status, 0, sizeof(info->status));
    }
    return ret;
}

static inline sdk_ret_t
tep_get_all_cb (pds_tep_spec_t *spec, void *ctxt)
{
    sdk_ret_t ret = SDK_RET_OK;
    pds_tep_info_t info;
    tep_db_cb_ctxt_t *cb_ctxt = (tep_db_cb_ctxt_t *)ctxt;

    memcpy(&info.spec, spec, sizeof(pds_tep_spec_t));
    if (!agent_state::state()->pds_mock_mode()) {
        ret = pds_tep_read(&spec->key, &info);
    } else {
        memset(&info.stats, 0, sizeof(info.stats));
        memset(&info.status, 0, sizeof(info.status));
    }
    if (ret == SDK_RET_OK) {
        cb_ctxt->cb(&info, cb_ctxt->ctxt);
    }
    return ret;
}

sdk_ret_t
tep_get_all (tep_get_cb_t tep_get_cb, void *ctxt)
{
    tep_db_cb_ctxt_t cb_ctxt;

    cb_ctxt.cb = tep_get_cb;
    cb_ctxt.ctxt = ctxt;
    return agent_state::state()->tep_db_walk(tep_get_all_cb, &cb_ctxt);
}

}    // namespace core
