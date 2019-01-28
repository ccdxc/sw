/**
 * Copyright (c) 2019 Pensando Systems, Inc.
 *
 * @file    route_state.cc
 *
 * @brief   route table database handling
 */

#include "nic/apollo/core/mem.hpp"
#include "nic/apollo/api/route_state.hpp"

namespace api {

/**
 * @defgroup OCI_ROUTE_TABLE_STATE - route table database functionality
 * @ingroup OCI_ROUTE
 * @{
 */

/**
 * @brief    constructor
 */
route_table_state::route_table_state() {
    // TODO: need to tune multi-threading related params later
    route_table_ht_ = ht::factory(OCI_MAX_ROUTE_TABLE >> 2,
                           route_table::route_table_key_func_get,
                           route_table::route_table_hash_func_compute,
                           route_table::route_table_key_func_compare);
    SDK_ASSERT(route_table_ht_ != NULL);

    route_table_slab_ = slab::factory("route-table", OCI_SLAB_ID_ROUTE_TABLE,
                                      sizeof(route_table), 16, true, true,
                                      true, NULL);
    SDK_ASSERT(route_table_slab_ != NULL);
}

/**
 * @brief    destructor
 */
route_table_state::~route_table_state() {
    ht::destroy(route_table_ht_);
    slab::destroy(route_table_slab_);
}

/**
 * @brief     allocate route table instance
 * @return    pointer to the allocated route table, NULL if no memory
 */
route_table *
route_table_state::route_table_alloc(void) {
    return ((route_table *)route_table_slab_->alloc());
}

/**
 * @brief      free route table instance back to slab
 * @param[in]  rtrable pointer to the allocated route table instance
 */
void
route_table_state::route_table_free(route_table *rtable) {
    route_table_slab_->free(rtable);
}

/**
 * @brief        lookup route table in database with given key
 * @param[in]    route_table_key route table key
 * @return       pointer to the route table instance found or NULL
 */
route_table *
route_table_state::route_table_find(oci_route_table_key_t *route_table_key) const {
    return (route_table *)(route_table_ht_->lookup(route_table_key));
}

/** @} */    // end of OCI_ROUTE_TABLE_STATE

}    // namespace api
