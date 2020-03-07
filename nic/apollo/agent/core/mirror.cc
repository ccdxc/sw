//------------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// -----------------------------------------------------------------------------

#include "nic/apollo/agent/core/state.hpp"
#include "nic/apollo/agent/trace.hpp"
#include "nic/apollo/agent/core/mirror.hpp"

namespace core {

sdk_ret_t
mirror_session_create (pds_obj_key_t *key,
                       pds_mirror_session_spec_t *spec,
                       pds_batch_ctxt_t bctxt)
{
    sdk_ret_t ret;

    if (!agent_state::state()->pds_mock_mode()) {
        if ((ret = pds_mirror_session_create(spec, bctxt)) != SDK_RET_OK) {
            PDS_TRACE_ERR("Failed to create mirror session {}, err {}",
                          spec->key.id, ret);
            return ret;
        }
    }
    return SDK_RET_OK;
}

sdk_ret_t
mirror_session_update (pds_obj_key_t *key,
                       pds_mirror_session_spec_t *spec,
                       pds_batch_ctxt_t bctxt)
{
    sdk_ret_t ret;

    if (!agent_state::state()->pds_mock_mode()) {
        if ((ret = pds_mirror_session_update(spec, bctxt)) != SDK_RET_OK) {
            PDS_TRACE_ERR("Failed to update mirror session {}, err {}",
                          spec->key.id, ret);
            return ret;
        }
    }
    return SDK_RET_OK;
}

sdk_ret_t
mirror_session_delete (pds_obj_key_t *key, pds_batch_ctxt_t bctxt)
{
    sdk_ret_t ret;

    if (!agent_state::state()->pds_mock_mode()) {
        if ((ret = pds_mirror_session_delete(key, bctxt)) != SDK_RET_OK) {
            PDS_TRACE_ERR("Failed to mirror session nh {}, err {}",
                          key->str(), ret);
            return ret;
        }
    }
    return SDK_RET_OK;
}

sdk_ret_t
mirror_session_get (pds_obj_key_t *key, pds_mirror_session_info_t *info)
{
    sdk_ret_t ret = SDK_RET_OK;

    if (!agent_state::state()->pds_mock_mode()) {
        ret = pds_mirror_session_read(key, info);
    } else {
        memset(&info->stats, 0, sizeof(info->stats));
        memset(&info->status, 0, sizeof(info->status));
    }
    return ret;
}

sdk_ret_t
mirror_session_get_all (mirror_session_read_cb_t mirror_session_read_cb,
                        void *ctxt)
{
    return pds_mirror_session_read_all(mirror_session_read_cb, ctxt);
}

}    // namespace core
