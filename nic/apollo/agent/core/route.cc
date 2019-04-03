//------------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// -----------------------------------------------------------------------------

#include "nic/apollo/agent/core/state.hpp"
#include "nic/apollo/agent/core/route.hpp"

namespace core {

sdk_ret_t
route_table_create (pds_route_table_key_t *key, pds_route_table_spec_t *spec)
{
    if (agent_state::state()->find_in_route_table_db(key) != NULL) {
        return SDK_RET_ENTRY_EXISTS;
    }
    if (pds_route_table_create(spec) != SDK_RET_OK) {
        return SDK_RET_ERR;
    }
    if (agent_state::state()->add_to_route_table_db(key, spec) != SDK_RET_OK) {
        return SDK_RET_ERR;
    }
    return SDK_RET_OK;
}

sdk_ret_t
route_table_delete (pds_route_table_key_t *key)
{
    pds_route_table_spec_t *spec;

    spec = agent_state::state()->find_in_route_table_db(key);
    if (spec == NULL) {
        return SDK_RET_ENTRY_NOT_FOUND;
    }
    if (pds_route_table_delete(key) != SDK_RET_OK) {
        return SDK_RET_ERR;
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
