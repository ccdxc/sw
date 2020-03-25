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
