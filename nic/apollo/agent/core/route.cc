//------------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// -----------------------------------------------------------------------------

#include "nic/apollo/agent/core/state.hpp"
#include "nic/apollo/agent/core/route.hpp"
#include "nic/apollo/agent/trace.hpp"

namespace core {

static inline sdk_ret_t
route_table_create_validate (pds_obj_key_t *key,
                             pds_route_table_spec_t *spec)
{
    for (uint32_t i = 0; i < spec->num_routes; i ++) {
        if (spec->routes[i].nh_type == PDS_NH_TYPE_IP) {
            if (agent_state::state()->find_in_nh_db(&spec->routes[i].nh) ==
                    NULL) {
                PDS_TRACE_ERR("Failed to create route table {}, nexthop {} "
                              "not found", spec->key.str(),
                              spec->routes[i].nh.str());
                return SDK_RET_INVALID_ARG;
            }
        } else if (spec->routes[i].nh_type == PDS_NH_TYPE_PEER_VPC) {
            if (agent_state::state()->find_in_vpc_db(&spec->routes[i].vpc) ==
                    NULL) {
                PDS_TRACE_ERR("Failed to create route table {}, vpc {} "
                              "not found", spec->key.str(),
                              spec->routes[i].vpc.str());
                return SDK_RET_INVALID_ARG;
            }
        }
    }
    return SDK_RET_OK;
}

sdk_ret_t
route_table_create (pds_obj_key_t *key, pds_route_table_spec_t *spec,
                    pds_batch_ctxt_t bctxt)
{
    sdk_ret_t               ret;

    if (agent_state::state()->find_in_route_table_db(key) != NULL) {
        return SDK_RET_ENTRY_EXISTS;
    }
    if ((ret = route_table_create_validate(key, spec)) != SDK_RET_OK) {
        PDS_TRACE_ERR("Failed to create route table {}, err {}",
                      spec->key.str(), ret);
        return ret;
    }
    if (!agent_state::state()->pds_mock_mode()) {
        if ((ret = pds_route_table_create(spec, bctxt)) != SDK_RET_OK) {
            return ret;
        }
    }
    if (agent_state::state()->add_to_route_table_db(key, spec) != SDK_RET_OK) {
        return SDK_RET_ERR;
    }
    return SDK_RET_OK;
}

static inline sdk_ret_t
route_table_update_validate (pds_obj_key_t *key, pds_route_table_spec_t *spec)
{
    for (uint32_t i = 0; i < spec->num_routes; i ++) {
        if (spec->routes[i].nh_type == PDS_NH_TYPE_IP) {
            if (agent_state::state()->find_in_nh_db(&spec->routes[i].nh) ==
                    NULL) {
                PDS_TRACE_ERR("Failed to update route table {}, nexthop {} "
                              "not found", spec->key.str(),
                              spec->routes[i].nh.str());
                return SDK_RET_INVALID_ARG;
            }
        } else if (spec->routes[i].nh_type == PDS_NH_TYPE_PEER_VPC) {
            if (agent_state::state()->find_in_vpc_db(&spec->routes[i].vpc) ==
                    NULL) {
                PDS_TRACE_ERR("Failed to update route table {}, vpc {} "
                              "not found", spec->key.str(),
                              spec->routes[i].vpc.str());
                return SDK_RET_INVALID_ARG;
            }
        }
    }
    return SDK_RET_OK;
}

sdk_ret_t
route_table_update (pds_obj_key_t *key, pds_route_table_spec_t *spec,
                    pds_batch_ctxt_t bctxt)
{
    sdk_ret_t               ret;

    if (agent_state::state()->find_in_route_table_db(key) == NULL) {
        return SDK_RET_ENTRY_NOT_FOUND;
    }
    if ((ret = route_table_update_validate(key, spec)) != SDK_RET_OK) {
        PDS_TRACE_ERR("Failed to update route table {}, err {}",
                      spec->key.str(), ret);
        return ret;
    }
    if (!agent_state::state()->pds_mock_mode()) {
        if ((ret = pds_route_table_update(spec, bctxt)) != SDK_RET_OK) {
            return ret;
        }
    }
    if (agent_state::state()->del_from_route_table_db(key) == false) {
    }
    if (agent_state::state()->add_to_route_table_db(key, spec) != SDK_RET_OK) {
        return SDK_RET_ERR;
    }
    return SDK_RET_OK;
}

sdk_ret_t
route_table_delete (pds_obj_key_t *key, pds_batch_ctxt_t bctxt)
{
    sdk_ret_t               ret;
    pds_route_table_spec_t *spec;

    spec = agent_state::state()->find_in_route_table_db(key);
    if (spec == NULL) {
        return SDK_RET_ENTRY_NOT_FOUND;
    }
    if (!agent_state::state()->pds_mock_mode()) {
        if ((ret = pds_route_table_delete(key, bctxt)) != SDK_RET_OK) {
            return ret;
        }
    }
    if (spec->routes) {
        SDK_FREE(PDS_MEM_ALLOC_ID_ROUTE_TABLE, spec->routes);
        spec->routes = NULL;
    }
    if (agent_state::state()->del_from_route_table_db(key) == false) {
        return SDK_RET_ERR;
    }
    return SDK_RET_OK;
}

sdk_ret_t
route_table_get (pds_obj_key_t *key, pds_route_table_info_t *info)
{
    sdk_ret_t ret = SDK_RET_OK;
    pds_route_table_spec_t *spec;

    memset(info, 0, sizeof(pds_route_table_info_t));
    spec = agent_state::state()->find_in_route_table_db(key);
    if (spec == NULL) {
        PDS_TRACE_ERR("Failed to find route table {} in db", key->str());
        return SDK_RET_ENTRY_NOT_FOUND;
    }

    memcpy(&info->spec, spec, sizeof(pds_route_table_spec_t));
    if (!agent_state::state()->pds_mock_mode()) {
        ret = pds_route_table_read(key, info);
    }
    return ret;
}

static inline sdk_ret_t
route_table_get_all_cb (pds_route_table_spec_t *spec, void *ctxt)
{
    sdk_ret_t ret = SDK_RET_OK;
    pds_route_table_info_t info;
    route_table_db_cb_ctxt_t *cb_ctxt = (route_table_db_cb_ctxt_t *)ctxt;

    memset(&info, 0, sizeof(pds_route_table_info_t));
    memcpy(&info.spec, spec, sizeof(pds_route_table_spec_t));
    if (!agent_state::state()->pds_mock_mode()) {
        ret = pds_route_table_read(&spec->key, &info);
    }
    if (ret == SDK_RET_OK) {
        cb_ctxt->cb(&info, cb_ctxt->ctxt);
    }
    return ret;
}

sdk_ret_t
route_table_get_all (route_table_get_cb_t route_table_get_cb, void *ctxt)
{
    route_table_db_cb_ctxt_t cb_ctxt;

    cb_ctxt.cb = route_table_get_cb;
    cb_ctxt.ctxt = ctxt;

    return agent_state::state()->route_table_db_walk(route_table_get_all_cb,
                                                     &cb_ctxt);
}

}    // namespace core
