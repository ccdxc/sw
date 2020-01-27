//------------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// -----------------------------------------------------------------------------

#include "nic/apollo/agent/core/state.hpp"
#include "nic/apollo/agent/core/nh_group.hpp"
#include "nic/apollo/agent/trace.hpp"

namespace core {

static inline sdk_ret_t
nh_group_create_validate (pds_nexthop_group_spec_t *spec)
{
    return SDK_RET_OK;
}

sdk_ret_t
nh_group_create (pds_obj_key_t *key, pds_nexthop_group_spec_t *spec,
                 pds_batch_ctxt_t bctxt)
{
    sdk_ret_t ret;

    if (agent_state::state()->find_in_nh_group_db(key) != NULL) {
        PDS_TRACE_ERR("Failed to create nh_group {}, nh_group exists already",
                      spec->key.str());
        return SDK_RET_ENTRY_EXISTS;
    }
    if ((ret = nh_group_create_validate(spec)) != SDK_RET_OK) {
        PDS_TRACE_ERR("Failed to create nh_group {}, err {}",
                      spec->key.str(), ret);
        return ret;
    }
    if (!agent_state::state()->pds_mock_mode()) {
        if ((ret = pds_nexthop_group_create(spec, bctxt)) != SDK_RET_OK) {
            PDS_TRACE_ERR("Failed to create nh_group {}, err {}",
                          spec->key.str(), ret);
            return ret;
        }
    }
    if ((ret = agent_state::state()->add_to_nh_group_db(key, spec))
                                                             != SDK_RET_OK) {
        PDS_TRACE_ERR("Failed to add nh_group {} to db, err {}",
                      spec->key.str(), ret);
        return ret;
    }
    return SDK_RET_OK;
}

static inline sdk_ret_t
nh_group_update_validate (pds_obj_key_t *key,
                          pds_nexthop_group_spec_t *spec)
{
    if ((spec = agent_state::state()->find_in_nh_group_db(key)) == NULL) {
        PDS_TRACE_ERR("Failed to find nh_group {}, nh_group not found",
                      key->str());
        return SDK_RET_ENTRY_NOT_FOUND;
    }
    return SDK_RET_OK;
}

sdk_ret_t
nh_group_update (pds_obj_key_t *key, pds_nexthop_group_spec_t *spec,
                 pds_batch_ctxt_t bctxt)
{
    sdk_ret_t ret;

    if ((ret = nh_group_update_validate(key, spec)) != SDK_RET_OK) {
        PDS_TRACE_ERR("Failed to update nh_group {}, err {}",
                      spec->key.str(), ret);
        return ret;
    }
    if (!agent_state::state()->pds_mock_mode()) {
        if ((ret = pds_nexthop_group_update(spec, bctxt)) != SDK_RET_OK) {
            PDS_TRACE_ERR("Failed to update nh_group {}, err {}",
                          spec->key.str(), ret);
            return ret;
        }
    }
    if (agent_state::state()->del_from_nh_group_db(key) == false) {
        PDS_TRACE_ERR("Failed to delete nh_group {} from db", key->str());
    }
    if ((ret = agent_state::state()->add_to_nh_group_db(key, spec))
                                                         != SDK_RET_OK) {
        PDS_TRACE_ERR("Failed to add nh_group {} to db, err {}",
                      spec->key.str(), ret);
        return ret;
    }
    return SDK_RET_OK;
}

sdk_ret_t
nh_group_delete (pds_obj_key_t *key, pds_batch_ctxt_t bctxt)
{
    sdk_ret_t ret;
    pds_nexthop_group_spec_t *spec;

    if ((spec = agent_state::state()->find_in_nh_group_db(key)) == NULL) {
        PDS_TRACE_ERR("Failed to delete nh_group {}, nh_group not found",
                      key->str());
        return SDK_RET_ENTRY_NOT_FOUND;
    }
    if (!agent_state::state()->pds_mock_mode()) {
        if ((ret = pds_nexthop_group_delete(key, bctxt)) != SDK_RET_OK) {
            PDS_TRACE_ERR("Failed to delete nh_group {}, err {}",
                          key->str(), ret);
            return ret;
        }
    }
    if (agent_state::state()->del_from_nh_group_db(key) == false) {
        PDS_TRACE_ERR("Failed to delete nh_group {} from db", key->str());
        return SDK_RET_ERR;
    }
    return SDK_RET_OK;
}

sdk_ret_t
nh_group_get (pds_obj_key_t *key, pds_nexthop_group_info_t *info)
{
    sdk_ret_t ret = SDK_RET_OK;
    pds_nexthop_group_spec_t *spec;

    spec = agent_state::state()->find_in_nh_group_db(key);
    if (spec == NULL) {
        PDS_TRACE_ERR("Failed to find nh_group {} in db", key->str());
        return SDK_RET_ENTRY_NOT_FOUND;
    }

    memcpy(&info->spec, spec, sizeof(pds_nexthop_group_spec_t));
    if (!agent_state::state()->pds_mock_mode()) {
        ret = pds_nexthop_group_read(key, info);
    } else {
        memset(&info->stats, 0, sizeof(info->stats));
        memset(&info->status, 0, sizeof(info->status));
    }
    return ret;
}

static inline sdk_ret_t
nh_group_get_all_cb (pds_nexthop_group_spec_t *spec, void *ctxt)
{
    sdk_ret_t ret = SDK_RET_OK;
    pds_nexthop_group_info_t info;

    nh_group_db_cb_ctxt_t *cb_ctxt = (nh_group_db_cb_ctxt_t *)ctxt;

    memcpy(&info.spec, spec, sizeof(pds_nexthop_group_spec_t));
    if (!agent_state::state()->pds_mock_mode()) {
        ret = pds_nexthop_group_read(&spec->key, &info);
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
nh_group_get_all (nh_group_get_cb_t nh_group_get_cb, void *ctxt)
{
    nh_group_db_cb_ctxt_t cb_ctxt;

    cb_ctxt.cb = nh_group_get_cb;
    cb_ctxt.ctxt = ctxt;

    return agent_state::state()->nh_group_db_walk(nh_group_get_all_cb,
                                                  &cb_ctxt);
}

}    // namespace core
