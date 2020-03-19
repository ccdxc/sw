//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// route table database handling
///
//----------------------------------------------------------------------------

#include "nic/apollo/core/mem.hpp"
#include "nic/apollo/core/trace.hpp"
#include "nic/apollo/api/route_state.hpp"

namespace api {

/// \defgroup PDS_ROUTE_TABLE_STATE - route table state/db functionality
/// \ingroup PDS_ROUTE
/// \@{

route_table_state::route_table_state(sdk::lib::kvstore *kvs) {
    kvstore_ = kvs;
    route_table_ht_ = ht::factory(PDS_MAX_ROUTE_TABLE >> 2,
                                  route_table::route_table_key_func_get,
                                  sizeof(pds_obj_key_t));
    SDK_ASSERT(route_table_ht_ != NULL);

    route_table_slab_ = slab::factory("route-table", PDS_SLAB_ID_ROUTE_TABLE,
                                      sizeof(route_table), 16, true, true,
                                      true, NULL);
    SDK_ASSERT(route_table_slab_ != NULL);
}

route_table_state::~route_table_state() {
    ht::destroy(route_table_ht_);
    slab::destroy(route_table_slab_);
}

route_table *
route_table_state::alloc(void) {
    return ((route_table *)route_table_slab_->alloc());
}

sdk_ret_t
route_table_state::insert(route_table *table) {
    return route_table_ht_->insert_with_key(&table->key_, table,
                                            &table->ht_ctxt_);
}

route_table *
route_table_state::remove(route_table *table) {
    return (route_table *)(route_table_ht_->remove(&table->key_));
}

sdk_ret_t
route_table_state::update(route_table *curr_table, route_table *new_table) {
    // lock();
    // remove(curr_table);
    // insert(new_table);
    // unlock();
    return sdk::SDK_RET_INVALID_OP;
}

void
route_table_state::free(route_table *rtable) {
    route_table_slab_->free(rtable);
}

route_table *
route_table_state::find(pds_obj_key_t *route_table_key) const {
    return (route_table *)(route_table_ht_->lookup(route_table_key));
}

sdk_ret_t
route_table_state::persist(route_table *table, pds_route_table_spec_t *spec) {
    sdk_ret_t ret;

    if (table->key_.valid()) {
        ret = kvstore_->insert(&table->key_, sizeof(table->key_),
                               spec->route_info,
                               ROUTE_SET_SIZE(spec->route_info->num_routes));
        if (unlikely(ret != SDK_RET_OK)) {
            PDS_TRACE_ERR("Failed to persist route table %s in kvstore, err %u",
                          spec->key.str(), ret);
        }
        return ret;
    }
    return SDK_RET_OK;
}

sdk_ret_t
route_table_state::perish(const pds_obj_key_t& key) {
    sdk_ret_t ret;

    if (key.valid()) {
        ret = kvstore_->remove(&key, sizeof(key));
        if (unlikely(ret != SDK_RET_OK)) {
            PDS_TRACE_ERR("Failed to remove route table %s from kvstore, "
                          "err %u", key.str(), ret);
        }
        return ret;
    }
    return SDK_RET_OK;
}

sdk_ret_t
route_table_state::slab_walk(state_walk_cb_t walk_cb, void *ctxt) {
    walk_cb(route_table_slab_, ctxt);
    return SDK_RET_OK;
}

/// \@}    // end of PDS_ROUTE_TABLE_STATE

}    // namespace api
