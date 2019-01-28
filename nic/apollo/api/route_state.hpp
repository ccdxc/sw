/**
 * Copyright (c) 2019 Pensando Systems, Inc.
 *
 * @file    route_state.hpp
 *
 * @brief   route table database handling
 */

#if !defined (__ROUTE_STATE_HPP__)
#define __ROUTE_STATE_HPP__

#include "nic/sdk/lib/slab/slab.hpp"
#include "nic/sdk/lib/ht/ht.hpp"
#include "nic/apollo/api/route.hpp"

namespace api {

/**
 * @defgroup OCI_ROUTE_TABLE_STATE - route table state/db functionality
 * @ingroup OCI_ROUTE
 * @{
 */

/**
 * @brief    state maintained for route tables
 */
class route_table_state : public obj_base {
public:
    /**
     * @brief    constructor
     */
    route_table_state();

    /**
     * @brief    destructor
     */
    ~route_table_state();

    /**
     * @brief    allocate memory required for a route table instance
     * @return pointer to the allocated route table instance, NULL if no memory
     */
    route_table *route_table_alloc(void);

    /**
     * @brief      free route table instance back to slab
     * @param[in]  route_table   pointer to the allocated route table instance
     */
    void route_table_free(route_table *table);

    /**
     * @brief     lookup a route table in database given the key
     * @param[in] route_table_key route table key
     */
    route_table *route_table_find(oci_route_table_key_t *route_table_key) const;

    friend void slab_delay_delete_cb(void *timer, uint32_t slab_id, void *elem);

private:
    ht *route_table_ht(void) { return route_table_ht_; }
    slab *rout_table_slab(void) { return route_table_slab_; }
    friend class route_table;   /**< route_table class is friend of route_table_state */

private:
    ht      *route_table_ht_;      /**< route table database */
    slab    *route_table_slab_;    /**< slab to allocate route table instance */
};

/** @} */    // end of OCI_ROUTE_TABLE_STATE

}    // namespace api

using api::route_table_state;

#endif    /** __ROUTE_STATE_HPP__ */
