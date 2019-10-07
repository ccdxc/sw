//------------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// -----------------------------------------------------------------------------

#include "nic/apollo/agent/core/state.hpp"
#include "nic/apollo/agent/trace.hpp"
#include "nic/apollo/agent/core/mirror.hpp"

namespace core {

static inline sdk_ret_t
mirror_session_create_validate (pds_mirror_session_spec_t *spec)
{
    if (spec->key.id > PDS_MAX_MIRROR_SESSION) {
        PDS_TRACE_ERR("Mirror session id {} is more than supported value {}",
                      spec->key.id, PDS_MAX_MIRROR_SESSION);
        return SDK_RET_INVALID_ARG;
    }
    if (spec->type > PDS_MIRROR_SESSION_TYPE_MAX) {
        PDS_TRACE_ERR("Unknown mirror session type {} in mirror session {}",
                      spec->type, spec->key.id);
        return SDK_RET_INVALID_ARG;
    }
    if (spec->type == PDS_MIRROR_SESSION_TYPE_ERSPAN) {
        // check if VPC exists, if mirror session type is ERSPAN
        if (agent_state::state()->find_in_vpc_db(&spec->erspan_spec.vpc)
                == NULL) {
            PDS_TRACE_ERR("Failed to create erspan mirror session {}, "
                          "vpc {} not found", spec->key.id,
                          spec->erspan_spec.vpc.id);
            return sdk::SDK_RET_ENTRY_NOT_FOUND;
        }
    }
    return SDK_RET_OK;
}

sdk_ret_t
mirror_session_create (pds_mirror_session_key_t *key,
                       pds_mirror_session_spec_t *spec,
                       pds_batch_ctxt_t bctxt)
{
    sdk_ret_t ret;

    if (agent_state::state()->find_in_mirror_session_db(key) != NULL) {
        PDS_TRACE_ERR("Failed to create mirror session {}, mirror session with "
                      "that id exists already", spec->key.id);
        return SDK_RET_ENTRY_EXISTS;
    }
    if ((ret = mirror_session_create_validate(spec)) != SDK_RET_OK) {
        PDS_TRACE_ERR("Failed to create mirror session {}, err {}",
                      spec->key.id, ret);
        return ret;
    }
    if (!agent_state::state()->pds_mock_mode()) {
        if ((ret = pds_mirror_session_create(spec, bctxt)) != SDK_RET_OK) {
            PDS_TRACE_ERR("Failed to create mirror session {}, err {}",
                          spec->key.id, ret);
            return ret;
        }
    }
    if (agent_state::state()->add_to_mirror_session_db(key, spec) !=
            SDK_RET_OK) {
        PDS_TRACE_ERR("Failed to add mirror session {} to db, err {}",
                      spec->key.id, ret);
        return SDK_RET_ERR;
    }
    return SDK_RET_OK;
}

static inline sdk_ret_t
mirror_session_update_validate (pds_mirror_session_spec_t *spec)
{
    if (spec->key.id > PDS_MAX_MIRROR_SESSION) {
        PDS_TRACE_ERR("Mirror session id {} is more than supported value {}",
                      spec->key.id, PDS_MAX_MIRROR_SESSION);
        return SDK_RET_INVALID_ARG;
    }
    if (spec->type > PDS_MIRROR_SESSION_TYPE_MAX) {
        PDS_TRACE_ERR("Unknown mirror session type {} in mirror session {}",
                      spec->type, spec->key.id);
        return SDK_RET_INVALID_ARG;
    }
    if (spec->type == PDS_MIRROR_SESSION_TYPE_ERSPAN) {
        // check if VPC exists, if mirror session type is ERSPAN
        if (agent_state::state()->find_in_vpc_db(&spec->erspan_spec.vpc)
                == NULL) {
            PDS_TRACE_ERR("Failed to create erspan mirror session {}, "
                          "vpc {} not found", spec->key.id,
                          spec->erspan_spec.vpc.id);
            return sdk::SDK_RET_ENTRY_NOT_FOUND;
        }
    }
    return SDK_RET_OK;
}

sdk_ret_t
mirror_session_update (pds_mirror_session_key_t *key,
                       pds_mirror_session_spec_t *spec,
                       pds_batch_ctxt_t bctxt)
{
    sdk_ret_t ret;

    if (agent_state::state()->find_in_mirror_session_db(key) == NULL) {
        PDS_TRACE_ERR("Failed to update mirror session {}, mirror session with "
                      "that id doesn't exist", spec->key.id);
        return SDK_RET_ENTRY_NOT_FOUND;
    }
    if ((ret = mirror_session_update_validate(spec)) != SDK_RET_OK) {
        PDS_TRACE_ERR("Failed to update mirror session {}, err {}",
                      spec->key.id, ret);
        return ret;
    }
    if (!agent_state::state()->pds_mock_mode()) {
        if ((ret = pds_mirror_session_update(spec, bctxt)) != SDK_RET_OK) {
            PDS_TRACE_ERR("Failed to update mirror session {}, err {}",
                          spec->key.id, ret);
            return ret;
        }
    }
    if (agent_state::state()->del_from_mirror_session_db(key) == false) {
        PDS_TRACE_ERR("Failed to delete mirror session {} from db, err {}",
                      spec->key.id, ret);
    }
    if (agent_state::state()->add_to_mirror_session_db(key, spec) !=
            SDK_RET_OK) {
        PDS_TRACE_ERR("Failed to add mirror session {} to db, err {}",
                      spec->key.id, ret);
        return SDK_RET_ERR;
    }
    return SDK_RET_OK;
}

sdk_ret_t
mirror_session_delete (pds_mirror_session_key_t *key, pds_batch_ctxt_t bctxt)
{
    sdk_ret_t ret;
    pds_mirror_session_spec_t *spec;

    spec = agent_state::state()->find_in_mirror_session_db(key);
    if (spec == NULL) {
        return SDK_RET_ENTRY_NOT_FOUND;
    }
    if (!agent_state::state()->pds_mock_mode()) {
        if ((ret = pds_mirror_session_delete(&spec->key, bctxt)) != SDK_RET_OK) {
            return ret;
        }
    }
    if (agent_state::state()->del_from_mirror_session_db(key) == false) {
        return SDK_RET_ERR;
    }
    return SDK_RET_OK;
}

sdk_ret_t
mirror_session_get (pds_mirror_session_key_t *key,
                    pds_mirror_session_info_t *info)
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

static inline sdk_ret_t
mirror_session_get_all_cb (pds_mirror_session_spec_t *spec, void *ctxt)
{
    sdk_ret_t ret = SDK_RET_OK;
    pds_mirror_session_info_t info;
    mirror_session_db_cb_ctxt_t *cb_ctxt = (mirror_session_db_cb_ctxt_t *)ctxt;

    memcpy(&info.spec, spec, sizeof(pds_mirror_session_spec_t));
    if (!agent_state::state()->pds_mock_mode()) {
        ret = pds_mirror_session_read(&spec->key, &info);
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
mirror_session_get_all (mirror_session_get_cb_t mirror_session_get_cb,
                        void *ctxt)
{
    mirror_session_db_cb_ctxt_t cb_ctxt;

    cb_ctxt.cb = mirror_session_get_cb;
    cb_ctxt.ctxt = ctxt;
    return agent_state::state()->mirror_session_db_walk(mirror_session_get_all_cb,
                                                        &cb_ctxt);
}

}    // namespace core
