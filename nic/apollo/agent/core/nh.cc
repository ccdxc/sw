//------------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// -----------------------------------------------------------------------------

#include "nic/apollo/agent/core/state.hpp"
#include "nic/apollo/agent/core/nh.hpp"
#include "nic/apollo/agent/trace.hpp"

namespace core {

static inline sdk_ret_t
nh_create_validate (pds_nexthop_spec_t *spec)
{
    return SDK_RET_OK;
}

sdk_ret_t
nh_create (pds_nexthop_key_t *key, pds_nexthop_spec_t *spec)
{
    sdk_ret_t ret;

    if (agent_state::state()->find_in_nh_db(key) != NULL) {
        PDS_TRACE_ERR("Failed to create nh {}, nh exists already", spec->key);
        return SDK_RET_ENTRY_EXISTS;
    }
    if ((ret = nh_create_validate(spec)) != SDK_RET_OK) {
        PDS_TRACE_ERR("Failed to create nh {}, err {}", spec->key, ret);
        return ret;
    }
    if (!agent_state::state()->pds_mock_mode()) {
        if ((ret = pds_nexthop_create(spec)) != SDK_RET_OK) {
            PDS_TRACE_ERR("Failed to create nh {}, err {}", spec->key, ret);
            return ret;
        }
    }
    if ((ret = agent_state::state()->add_to_nh_db(key, spec)) != SDK_RET_OK) {
        PDS_TRACE_ERR("Failed to add nh {} to db, err {}", spec->key, ret);
        return ret;
    }
    return SDK_RET_OK;
}

static inline sdk_ret_t
nh_update_validate (pds_nexthop_key_t *key, pds_nexthop_spec_t *spec)
{
    return SDK_RET_OK;
}

sdk_ret_t
nh_update (pds_nexthop_key_t *key, pds_nexthop_spec_t *spec)
{
    sdk_ret_t ret;

    if ((ret = nh_update_validate(key, spec)) != SDK_RET_OK) {
        PDS_TRACE_ERR("Failed to update nh {}, err {}", spec->key, ret);
        return ret;
    }
    if (!agent_state::state()->pds_mock_mode()) {
        if ((ret = pds_nexthop_update(spec)) != SDK_RET_OK) {
            PDS_TRACE_ERR("Failed to update nh {}, err {}", spec->key, ret);
            return ret;
        }
    }
    if (agent_state::state()->del_from_nh_db(key) == false) {
        PDS_TRACE_ERR("Failed to delete nh {} from db", *key);
    }
    if ((ret = agent_state::state()->add_to_nh_db(key, spec)) != SDK_RET_OK) {
        PDS_TRACE_ERR("Failed to add nh {} to db, err {}", spec->key, ret);
        return ret;
    }
    return SDK_RET_OK;
}

sdk_ret_t
nh_delete (pds_nexthop_key_t *key)
{
    sdk_ret_t ret;
    pds_nexthop_spec_t *spec;

    if ((spec = agent_state::state()->find_in_nh_db(key)) == NULL) {
        PDS_TRACE_ERR("Failed to delete nh {}, nh not found", *key);
        return SDK_RET_ENTRY_NOT_FOUND;
    }
    if (!agent_state::state()->pds_mock_mode()) {
        if ((ret = pds_nexthop_delete(key)) != SDK_RET_OK) {
            PDS_TRACE_ERR("Failed to delete nh {}, err {}", *key, ret);
            return ret;
        }
    }
    if (agent_state::state()->del_from_nh_db(key) == false) {
        PDS_TRACE_ERR("Failed to delete nh {} from db", *key);
        return SDK_RET_ERR;
    }
    return SDK_RET_OK;
}

sdk_ret_t
nh_get (pds_nexthop_key_t *key, pds_nexthop_info_t *info)
{
    sdk_ret_t ret = SDK_RET_OK;
    pds_nexthop_spec_t *spec;

    spec = agent_state::state()->find_in_nh_db(key);
    if (spec == NULL) {
        PDS_TRACE_ERR("Failed to find nh {} in db", *key);
        return SDK_RET_ENTRY_NOT_FOUND;
    }

    memcpy(&info->spec, spec, sizeof(pds_nexthop_spec_t));
    if (!agent_state::state()->pds_mock_mode()) {
        ret = pds_nexthop_read(key, info);
    } else {
        memset(&info->stats, 0, sizeof(info->stats));
        memset(&info->status, 0, sizeof(info->status));
    }
    return ret;
}

static inline sdk_ret_t
nh_get_all_cb (pds_nexthop_spec_t *spec, void *ctxt)
{
    sdk_ret_t ret = SDK_RET_OK;
    pds_nexthop_info_t info;

    nh_db_cb_ctxt_t *cb_ctxt = (nh_db_cb_ctxt_t *)ctxt;

    memcpy(&info.spec, spec, sizeof(pds_nexthop_spec_t));
    if (!agent_state::state()->pds_mock_mode()) {
        ret = pds_nexthop_read(&spec->key, &info);
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
nh_get_all (nh_get_cb_t nh_get_cb, void *ctxt)
{
    nh_db_cb_ctxt_t cb_ctxt;

    cb_ctxt.cb = nh_get_cb;
    cb_ctxt.ctxt = ctxt;

    return agent_state::state()->nh_db_walk(nh_get_all_cb, &cb_ctxt);
}

}    // namespace core
