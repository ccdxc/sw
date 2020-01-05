//---------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// Route stub store
//---------------------------------------------------------------

#include "nic/metaswitch/stubs/common/pds_ms_route_store.hpp"
#include "nic/metaswitch/stubs/common/pds_ms_cookie.hpp"
#include "nic/metaswitch/stubs/common/pds_ms_error.hpp"
#include "nic/metaswitch/stubs/common/pds_ms_util.hpp"
#include "nic/apollo/api/include/pds_route.hpp"
#include <vector>
#include <unordered_map>


namespace pds_ms {
template<> sdk::lib::slab* slab_obj_t<route_table_obj_t>::slab_ = nullptr;

void
route_table_slab_init (slab_uptr_t slabs_[], sdk::lib::slab_id_t slab_id)
{
    slabs_[slab_id].
        reset(sdk::lib::slab::factory("PDS-MS-RTTABLE", slab_id,
                                      sizeof(route_table_obj_t),
                                      2,        // Route table per tenant
                                      true, true, true));
    if (unlikely (!slabs_[slab_id])) {
        throw Error("SLAB creation failed for Route Table");
    }
    route_table_obj_t::set_slab(slabs_[slab_id].get());
}

void route_table_obj_t::update_store (state_t* state, bool op_delete)
{
    if (!op_delete) {
        state->route_table_store().add_upd(this->key(), this);
    } else { 
        state->route_table_store().erase(this->key());
    }
}

void route_table_obj_t::add_upd_route(pds_route_t &route)
{
    const auto it = route_index_.find(route.prefix);
    if (it == route_index_.end()) {
        // Route not found. Add the route to the vector
        routes_.emplace_back(route);
        // Get the index position of the new route in the vector
        int idx = routes_.size() - 1;
        // Update the map with the index position
        route_index_[route.prefix] = idx;
    } else {
        // Route already present, update the contents in the vector
        int idx = it->second;
        routes_[idx] = route;
    }
    return;
}

void route_table_obj_t::del_route(ip_prefix_t &pfx)
{
    const auto it = route_index_.find(pfx);
    if (it != route_index_.end()) {
        // Get the position of the route in the vector
        int idx = it->second;
        // Vector is always compacted (no holes)
        // Copy the contents of the last index to the deleted index
        routes_[idx] = routes_.back();
        // Remove the last element in the vector
        routes_.pop_back();
        // Update the new vector position in the map
        route_index_[routes_[idx].prefix] = idx;
        // Delete the route from the map
        route_index_.erase(pfx);
    }
    return;
}

const pds_route_t* route_table_obj_t::get_route(ip_prefix_t &pfx)
{
    const auto it = route_index_.find(pfx);
    if (it != route_index_.end()) {
        // Get the position of the route in the vector
        int idx = it->second;
        // Return the pds_route_t struct ptr
        return (&routes_[idx]);
    } else {
        return (nullptr);
    }
}

} // End namespace
