//------------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// -----------------------------------------------------------------------------

#include "nic/apollo/agent/core/state.hpp"
#include "nic/apollo/agent/core/route.hpp"

namespace core {

sdk_ret_t
route_table_create (pds_route_table_key_t *key, pds_route_table_spec_t *spec)
{
    sdk_ret_t               ret;

    if (agent_state::state()->find_in_route_table_db(key) != NULL) {
        return SDK_RET_ENTRY_EXISTS;
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

sdk_ret_t
route_table_update (pds_route_table_key_t *key, pds_route_table_spec_t *spec)
{
    sdk_ret_t               ret;

    if (agent_state::state()->find_in_route_table_db(key) == NULL) {
        return SDK_RET_ENTRY_NOT_FOUND;
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
