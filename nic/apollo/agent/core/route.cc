//------------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// -----------------------------------------------------------------------------

#include "nic/apollo/agent/core/state.hpp"
#include "nic/apollo/agent/core/route.hpp"
#include "nic/apollo/agent/trace.hpp"

namespace core {

static inline sdk_ret_t
route_table_create_validate (pds_route_table_key_t *key, pds_route_table_spec_t *spec)
{
    for (uint32_t i = 0; i < spec->num_routes; i ++) {
        if (spec->routes[i].nh_type == PDS_NH_TYPE_IP) {
            if (agent_state::state()->find_in_nh_db(&spec->routes[i].nh) == NULL) {
                PDS_TRACE_ERR("Failed to create route table {}, nexthop {} not found",
                              spec->key.id, spec->routes[i].nh);
                return SDK_RET_INVALID_ARG;
            }
        } else if (spec->routes[i].nh_type == PDS_NH_TYPE_PEER_VPC) {
            if (agent_state::state()->find_in_vpc_db(&spec->routes[i].vpc) == NULL) {
                PDS_TRACE_ERR("Failed to create route table {}, vpc {} not found",
                              spec->key.id, spec->routes[i].vpc.id);
                return SDK_RET_INVALID_ARG;
            }
        }
    }
    return SDK_RET_OK;
}

sdk_ret_t
route_table_create (pds_route_table_key_t *key, pds_route_table_spec_t *spec)
{
    sdk_ret_t               ret;

    if (agent_state::state()->find_in_route_table_db(key) != NULL) {
        return SDK_RET_ENTRY_EXISTS;
    }
    if ((ret = route_table_create_validate(key, spec)) != SDK_RET_OK) {
        PDS_TRACE_ERR("Failed to create route table {}, err {}", spec->key.id, ret);
        return ret;
    }
    if (!agent_state::state()->pds_mock_mode()) {
        if ((ret = pds_route_table_create(spec)) != sdk::SDK_RET_OK) {
            return ret;
        }
    }
    if (agent_state::state()->add_to_route_table_db(key, spec) != SDK_RET_OK) {
        return SDK_RET_ERR;
    }
    return SDK_RET_OK;
}

static inline sdk_ret_t
route_table_update_validate (pds_route_table_key_t *key, pds_route_table_spec_t *spec)
{
    for (uint32_t i = 0; i < spec->num_routes; i ++) {
        if (spec->routes[i].nh_type == PDS_NH_TYPE_IP) {
            if (agent_state::state()->find_in_nh_db(&spec->routes[i].nh) == NULL) {
                PDS_TRACE_ERR("Failed to update route table {}, nexthop {} not found",
                              spec->key.id, spec->routes[i].nh);
                return SDK_RET_INVALID_ARG;
            }
        } else if (spec->routes[i].nh_type == PDS_NH_TYPE_PEER_VPC) {
            if (agent_state::state()->find_in_vpc_db(&spec->routes[i].vpc) == NULL) {
                PDS_TRACE_ERR("Failed to update route table {}, vpc {} not found",
                              spec->key.id, spec->routes[i].vpc.id);
                return SDK_RET_INVALID_ARG;
            }
        }
    }
    return SDK_RET_OK;
}

sdk_ret_t
route_table_update (pds_route_table_key_t *key, pds_route_table_spec_t *spec)
{
    sdk_ret_t               ret;

    if (agent_state::state()->find_in_route_table_db(key) == NULL) {
        return SDK_RET_ENTRY_NOT_FOUND;
    }
    if ((ret = route_table_update_validate(key, spec)) != SDK_RET_OK) {
        PDS_TRACE_ERR("Failed to update route table {}, err {}", spec->key.id, ret);
        return ret;
    }
    if (!agent_state::state()->pds_mock_mode()) {
        if ((ret = pds_route_table_update(spec)) != sdk::SDK_RET_OK) {
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
route_table_delete (pds_route_table_key_t *key)
{
    sdk_ret_t               ret;
    pds_route_table_spec_t *spec;

    spec = agent_state::state()->find_in_route_table_db(key);
    if (spec == NULL) {
        return SDK_RET_ENTRY_NOT_FOUND;
    }
    if (!agent_state::state()->pds_mock_mode()) {
        if ((ret = pds_route_table_delete(key)) != sdk::SDK_RET_OK) {
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

}    // namespace core
