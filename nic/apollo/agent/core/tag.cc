//------------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// -----------------------------------------------------------------------------

#include "nic/apollo/agent/trace.hpp"
#include "nic/apollo/agent/core/state.hpp"
#include "nic/apollo/agent/core/tunnel.hpp"
#include "nic/apollo/agent/core/tag.hpp"

namespace core {

static inline sdk_ret_t
tag_create_validate (pds_tag_spec_t *spec)
{
    return SDK_RET_OK;
}

sdk_ret_t
tag_create (pds_obj_key_t *key, pds_tag_spec_t *spec,
            pds_batch_ctxt_t bctxt)
{
    sdk_ret_t ret;

    if ((ret = tag_create_validate(spec)) != SDK_RET_OK) {
        PDS_TRACE_ERR("Tag {} validation failure, err {}",
                      spec->key.str(), ret);
        return ret;
    }

    if (!agent_state::state()->pds_mock_mode()) {
        if ((ret = pds_tag_create(spec, bctxt)) != SDK_RET_OK) {
            PDS_TRACE_ERR("Failed to create tag {}, err {}",
                          spec->key.str(), ret);
            return ret;
        }
    }

    return SDK_RET_OK;
}

static inline sdk_ret_t
tag_update_validate (pds_tag_spec_t *spec)
{
    return SDK_RET_OK;
}

sdk_ret_t
tag_update (pds_obj_key_t *key, pds_tag_spec_t *spec,
            pds_batch_ctxt_t bctxt)
{
    sdk_ret_t ret;

    if ((ret = tag_update_validate(spec)) != SDK_RET_OK) {
        PDS_TRACE_ERR("Tag {} validation failure, err {}",
                      spec->key.str(), ret);
        return ret;
    }

    if (!agent_state::state()->pds_mock_mode()) {
        if ((ret = pds_tag_update(spec, bctxt)) != SDK_RET_OK) {
            PDS_TRACE_ERR("Failed to create tag {}, err {}",
                          spec->key.str(), ret);
            return ret;
        }
    }

    return SDK_RET_OK;
}

sdk_ret_t
tag_delete (pds_obj_key_t *key, pds_batch_ctxt_t bctxt)
{
    sdk_ret_t ret;

    if (!agent_state::state()->pds_mock_mode()) {
        if ((ret = pds_tag_delete(key, bctxt)) != SDK_RET_OK) {
            PDS_TRACE_ERR("Failed to delete tag {}, err {}", key->str(), ret);
            return ret;
        }
    }

    return SDK_RET_OK;
}

sdk_ret_t
tag_get (pds_obj_key_t *key, pds_tag_info_t *info)
{
    sdk_ret_t ret = SDK_RET_OK;

    if (!agent_state::state()->pds_mock_mode()) {
        ret = pds_tag_read(key, info);
    } else {
        memset(&info->stats, 0, sizeof(info->stats));
        memset(&info->status, 0, sizeof(info->status));
    }
    return ret;
}

sdk_ret_t
tag_get_all (tag_read_cb_t tag_read_cb, void *ctxt)
{
    return pds_tag_read_all(tag_read_cb, ctxt);
}

}    // namespace core
