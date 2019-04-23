//------------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// -----------------------------------------------------------------------------

#include "nic/apollo/agent/core/state.hpp"
#include "nic/apollo/agent/core/policy.hpp"
#include "nic/apollo/agent/trace.hpp"

namespace core {

static inline sdk_ret_t
policy_create_validate (pds_policy_spec_t *spec)
{
    // TODO
    return SDK_RET_OK;
}

sdk_ret_t
policy_create (pds_policy_key_t *key, pds_policy_spec_t *spec)
{
    sdk_ret_t ret;

    if (agent_state::state()->find_in_policy_db(key) != NULL) {
        PDS_TRACE_ERR("Failed to create policy {}, policy exists already",
                      spec->key.id);
        return SDK_RET_ENTRY_EXISTS;
    }
    if ((ret = policy_create_validate(spec)) != SDK_RET_OK) {
        PDS_TRACE_ERR("Failed to create policy {}, err {}", spec->key.id, ret);
        return ret;
    }
    if (!agent_state::state()->pds_mock_mode()) {
        if ((ret = pds_policy_create(spec)) != SDK_RET_OK) {
            PDS_TRACE_ERR("Failed to create policy {}, err {}",
                          spec->key.id, ret);
            return ret;
        }
    }
    if ((ret = agent_state::state()->add_to_policy_db(key, spec)) != SDK_RET_OK) {
        PDS_TRACE_ERR("Failed to add policy {} to db, err {}", spec->key.id, ret);
        return ret;
    }
    return SDK_RET_OK;
}

#if 0
sdk_ret_t
policy_delete (pds_policy_key_t *key)
{
    sdk_ret_t ret;
    pds_policy_spec_t *spec;

    if ((spec = agent_state::state()->find_in_policy_db(key)) == NULL) {
        PDS_TRACE_ERR("Failed to delete policy {}, policy not found", key->id);
        return SDK_RET_ENTRY_NOT_FOUND;
    }
    if (!agent_state::state()->pds_mock_mode()) {
        if ((ret = pds_policy_delete(key)) != SDK_RET_OK) {
            PDS_TRACE_ERR("Failed to delete policy {}, err {}", key->id, ret);
            return ret;
        }
    }
    if (spec->type == PDS_VPC_TYPE_SUBSTRATE) {
        agent_state::state()->substrate_policy_id_reset();
    }
    if (agent_state::state()->del_from_policy_db(key) == false) {
        PDS_TRACE_ERR("Failed to delete policy {} from db", key->id);
        return SDK_RET_ERR;
    }
    return SDK_RET_OK;
}

sdk_ret_t
policy_get (pds_policy_key_t *key, pds_policy_info_t *info)
{
    sdk_ret_t ret;
    pds_policy_spec_t *spec;

    spec = agent_state::state()->find_in_policy_db(key);
    if (spec == NULL) {
        PDS_TRACE_ERR("Failed to find policy {} in db", key->id);
        return SDK_RET_ENTRY_NOT_FOUND;
    }

    memcpy(&info->spec, spec, sizeof(pds_policy_spec_t));
    if (!agent_state::state()->pds_mock_mode()) {
        ret = pds_policy_read(key, info);
    }
    return ret;
}

static inline sdk_ret_t
policy_get_all_cb (pds_policy_spec_t *spec, void *ctxt)
{
    sdk_ret_t ret;
    pds_policy_info_t info;
    policy_db_cb_ctxt_t *cb_ctxt = (policy_db_cb_ctxt_t *)ctxt;

    memcpy(&info.spec, spec, sizeof(pds_policy_spec_t));
    if (!agent_state::state()->pds_mock_mode()) {
        ret = pds_policy_read(&spec->key, &info);
    }
    if (ret == SDK_RET_OK) {
        cb_ctxt->cb(&info, cb_ctxt->ctxt);
    }
    return ret;
}

sdk_ret_t
policy_get_all (policy_get_cb_t policy_get_cb, void *ctxt)
{
    policy_db_cb_ctxt_t cb_ctxt;

    cb_ctxt.cb = policy_get_cb;
    cb_ctxt.ctxt = ctxt;

    return agent_state::state()->policy_db_walk(policy_get_all_cb, &cb_ctxt);
}
#endif

}    // namespace core
