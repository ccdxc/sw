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
    return SDK_RET_OK;
}

static inline sdk_ret_t
nh_group_update_validate (pds_obj_key_t *key,
                          pds_nexthop_group_spec_t *spec)
{
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
    return SDK_RET_OK;
}

sdk_ret_t
nh_group_delete (pds_obj_key_t *key, pds_batch_ctxt_t bctxt)
{
    sdk_ret_t ret;

    if (!agent_state::state()->pds_mock_mode()) {
        if ((ret = pds_nexthop_group_delete(key, bctxt)) != SDK_RET_OK) {
            PDS_TRACE_ERR("Failed to delete nh_group {}, err {}",
                          key->str(), ret);
            return ret;
        }
    }
    return SDK_RET_OK;
}

sdk_ret_t
nh_group_get (pds_obj_key_t *key, pds_nexthop_group_info_t *info)
{
    sdk_ret_t ret = SDK_RET_OK;

    if (!agent_state::state()->pds_mock_mode()) {
        ret = pds_nexthop_group_read(key, info);
    } else {
        memset(&info->spec, 0, sizeof(info->spec));
        memset(&info->stats, 0, sizeof(info->stats));
        memset(&info->status, 0, sizeof(info->status));
    }
    return ret;
}

sdk_ret_t
nh_group_get_all (nexthop_group_read_cb_t nh_group_read_cb, void *ctxt)
{
    return pds_nexthop_group_read_all(nh_group_read_cb, ctxt);
}

}    // namespace core
