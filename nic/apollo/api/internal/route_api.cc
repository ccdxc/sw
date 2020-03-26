//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This file implements internal route related APIs
///
//----------------------------------------------------------------------------

#include "nic/apollo/core/trace.hpp"
#include "nic/apollo/api/include/pds_route.hpp"
#include "nic/apollo/api/internal/pds_route.hpp"
#include "nic/apollo/api/pds_state.hpp"

void pds_route_table_spec_s::deepcopy_(const pds_route_table_spec_t& route_table) {
    // self-assignment guard
    if (this == &route_table) {
        return;
    }

    if (route_table.route_info == NULL) {
        if (route_info) {
            // incorrect usage !!
            SDK_ASSERT(FALSE);
        } else {
            // no routes to copy, just copy the key
             memcpy(this, &route_table, sizeof(pds_route_table_spec_t));
             return;
        }
    }

    // free internally allocated memory, if any
    if (route_info && priv_mem_) {
        PDS_TRACE_VERBOSE("Freeing internally allocated route info");
        SDK_FREE(PDS_MEM_ALLOC_ID_ROUTE_TABLE, route_info);
    }
    PDS_TRACE_VERBOSE("Deep copying route table spec");
    route_info =
        (route_info_t *)SDK_MALLOC(PDS_MEM_ALLOC_ID_ROUTE_TABLE,
                            ROUTE_INFO_SIZE(route_table.route_info->num_routes));
    SDK_ASSERT(route_info != NULL);
    priv_mem_ = true;
    key = route_table.key;
    memcpy(route_info, route_table.route_info,
           ROUTE_INFO_SIZE(route_table.route_info->num_routes));
}

void pds_route_table_spec_s::move_(pds_route_table_spec_t&& route_table) {
    // self-assignment guard
    if (this == &route_table) {
        return;
    }

    if (route_table.route_info == NULL) {
        if (route_info) {
            // incorrect usage !!
            SDK_ASSERT(FALSE);
        }
        // fall through
    }
    // free internally allocated memory, if any
    if (route_info && priv_mem_) {
        PDS_TRACE_VERBOSE("Freeing internally allocated route info");
        SDK_FREE(PDS_MEM_ALLOC_ID_ROUTE_TABLE, route_info);
    }
    // shallow copy the spec
    PDS_TRACE_VERBOSE("Shallow copying route table spec");
    memcpy(this, &route_table, sizeof(pds_route_table_spec_t));
    // transfer the ownership of the memory
    route_table.route_info = NULL;
}

namespace api {

#define PDS_MAX_UNDERLAY_ROUTES    1
uint32_t g_num_routes = 0;
typedef struct route_entry_s {
    uint8_t valid:1;
    uint8_t rsvd:7;
    pds_route_spec_t spec;
} route_entry_t;
static route_entry_t g_route_db[PDS_MAX_UNDERLAY_ROUTES];

sdk_ret_t
pds_underlay_route_create (_In_ pds_route_spec_t *spec)
{
    if (g_num_routes >= PDS_MAX_UNDERLAY_ROUTES) {
        PDS_TRACE_ERR("Failed to create route %s, underlay route table is full",
                      spec->key.str());
        return SDK_RET_NO_RESOURCE;
    }
    if (route_table_db()->find(&spec->route_table) == NULL) {
        PDS_TRACE_ERR("Failed to create route %s, route table %s not found",
                      spec->key.str(), spec->route_table.str());
        return SDK_RET_INVALID_ARG;
    }
    g_route_db[g_num_routes].valid = TRUE;
    g_num_routes++;

    return SDK_RET_OK;
}

sdk_ret_t
pds_underlay_route_update (_In_ pds_route_spec_t *spec)
{
    for (uint32_t i = 0; i < g_num_routes; i++) {
        if (g_route_db[i].valid && (g_route_db[i].spec.key == spec->key)) {
            g_route_db[i].spec = *spec;
            // TODO: do TEP walk !!
            return SDK_RET_OK;
        }
    }
    PDS_TRACE_ERR("Failed to update route %s, route not found",
                  spec->key.str());
    return SDK_RET_ENTRY_NOT_FOUND;
}

sdk_ret_t
pds_underlay_route_delete (_In_ pds_obj_key_t *key)
{
    for (uint32_t i = 0; i < g_num_routes; i++) {
        if (g_route_db[i].valid && (g_route_db[i].spec.key == *key)) {
            // replace this with the last valid route
            g_route_db[i] = g_route_db[g_num_routes - 1];
            g_route_db[g_num_routes - 1].valid = FALSE;
            g_num_routes--;
            return SDK_RET_OK;
        }
    }
    PDS_TRACE_ERR("Failed to delete route %s, route not found", key->str());
    return SDK_RET_INVALID_OP;
}

pds_obj_key_t
pds_underlay_nexthop (_In_ ip_addr_t ip_addr)
{
    // TODO
    return k_pds_obj_key_invalid;
}

}    // namespace api
