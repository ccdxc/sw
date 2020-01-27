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

    if (agent_state::state()->find_in_tag_db(key) != NULL) {
        PDS_TRACE_ERR("Failed to create tag {}, tag exists already",
                      spec->key.str());
        return sdk::SDK_RET_ENTRY_EXISTS;
    }

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

    if ((ret = agent_state::state()->add_to_tag_db(key, spec)) != SDK_RET_OK) {
        PDS_TRACE_ERR("Failed to add tag {} to db, err {}",
                      spec->key.str(), ret);
        return ret;
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

    if (agent_state::state()->find_in_tag_db(key) == NULL) {
        PDS_TRACE_ERR("Failed to update tag {}, tag doesn't exist",
                      spec->key.str());
        return SDK_RET_ENTRY_NOT_FOUND;
    }

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

    if (agent_state::state()->del_from_tag_db(key) == false) {
        PDS_TRACE_ERR("Failed to delete tag {} from tag db", key->str());
    }

    if ((ret = agent_state::state()->add_to_tag_db(key, spec)) != SDK_RET_OK) {
        PDS_TRACE_ERR("Failed to add tag {} to db, err {}",
                      spec->key.str(), ret);
        return ret;
    }

    return SDK_RET_OK;
}

sdk_ret_t
tag_delete (pds_obj_key_t *key, pds_batch_ctxt_t bctxt)
{
    sdk_ret_t ret;

    if (agent_state::state()->find_in_tag_db(key) == NULL) {
        PDS_TRACE_ERR("Failed to delete tag {}, tag not found", key->str());
        return SDK_RET_ENTRY_NOT_FOUND;
    }

    if (!agent_state::state()->pds_mock_mode()) {
        if ((ret = pds_tag_delete(key, bctxt)) != SDK_RET_OK) {
            PDS_TRACE_ERR("Failed to delete tag {}, err {}", key->str(), ret);
            return ret;
        }
    }

    if (agent_state::state()->del_from_tag_db(key) == false) {
        PDS_TRACE_ERR("Failed to delete tag {} from tag db", key->str());
        return SDK_RET_ERR;
    }
    return SDK_RET_OK;
}

sdk_ret_t
tag_get (pds_obj_key_t *key, pds_tag_info_t *info)
{
    sdk_ret_t ret = SDK_RET_OK;
    pds_tag_spec_t *spec;

    spec = agent_state::state()->find_in_tag_db(key);
    if (spec == NULL) {
        return SDK_RET_ENTRY_NOT_FOUND;
    }
    info->spec = *spec;
    if (!agent_state::state()->pds_mock_mode()) {
        ret = pds_tag_read(key, info);
    } else {
        memset(&info->stats, 0, sizeof(info->stats));
        memset(&info->status, 0, sizeof(info->status));
    }
    return ret;
}

static inline sdk_ret_t
tag_get_all_cb (pds_tag_spec_t *spec, void *ctxt)
{
    sdk_ret_t ret = SDK_RET_OK;
    pds_tag_info_t info;
    tag_db_cb_ctxt_t *cb_ctxt = (tag_db_cb_ctxt_t *)ctxt;

    info.spec = *spec;
    if (!agent_state::state()->pds_mock_mode()) {
        ret = pds_tag_read(&spec->key, &info);
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
tag_get_all (tag_get_cb_t tag_get_cb, void *ctxt)
{
    tag_db_cb_ctxt_t cb_ctxt;

    cb_ctxt.cb = tag_get_cb;
    cb_ctxt.ctxt = ctxt;

     return agent_state::state()->tag_db_walk(tag_get_all_cb, &cb_ctxt);
}

}    // namespace core
