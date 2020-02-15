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
policy_create (pds_obj_key_t *key, pds_policy_spec_t *spec,
               pds_batch_ctxt_t bctxt)
{
    sdk_ret_t ret;

    if ((ret = policy_create_validate(spec)) != SDK_RET_OK) {
        PDS_TRACE_ERR("Failed to create policy {}, err {}",
                      spec->key.str(), ret);
        return ret;
    }
    if (!agent_state::state()->pds_mock_mode()) {
        if ((ret = pds_policy_create(spec, bctxt)) != SDK_RET_OK) {
            PDS_TRACE_ERR("Failed to create policy {}, err {}",
                          spec->key.str(), ret);
            return ret;
        }
    }
    return SDK_RET_OK;
}

static inline sdk_ret_t
policy_update_validate (pds_policy_spec_t *spec)
{
    // TODO
    return SDK_RET_OK;
}

sdk_ret_t
policy_update (pds_obj_key_t *key, pds_policy_spec_t *spec,
               pds_batch_ctxt_t bctxt)
{
    sdk_ret_t ret;

    if ((ret = policy_update_validate(spec)) != SDK_RET_OK) {
        PDS_TRACE_ERR("Failed to update policy {}, err {}",
                      spec->key.str(), ret);
        return ret;
    }
    if (!agent_state::state()->pds_mock_mode()) {
        if ((ret = pds_policy_update(spec, bctxt)) != SDK_RET_OK) {
            PDS_TRACE_ERR("Failed to update policy {}, err {}",
                          spec->key.str(), ret);
            return ret;
        }
    }
    return SDK_RET_OK;
}

sdk_ret_t
policy_delete (pds_obj_key_t *key, pds_batch_ctxt_t bctxt)
{
    sdk_ret_t ret;

    if (!agent_state::state()->pds_mock_mode()) {
        if ((ret = pds_policy_delete(key, bctxt)) != SDK_RET_OK) {
            PDS_TRACE_ERR("Failed to delete policy {}, err {}",
                          key->str(), ret);
            return ret;
        }
    }
    return SDK_RET_OK;
}

sdk_ret_t
policy_get (pds_obj_key_t *key, pds_policy_info_t *info)
{
    sdk_ret_t ret = SDK_RET_OK;

    memset(info, 0, sizeof(pds_policy_info_t));
    if (!agent_state::state()->pds_mock_mode()) {
        ret = pds_policy_read(key, info);
    }
    return ret;
}

sdk_ret_t
policy_get_all (policy_read_cb_t policy_read_cb, void *ctxt)
{
    return pds_policy_read_all(policy_read_cb, ctxt);
}

}    // namespace core
