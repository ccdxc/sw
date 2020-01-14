//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This file contains the unit tests for the common stores
///
//----------------------------------------------------------------------------

#include "nic/metaswitch/stubs/common/pds_ms_state.hpp"
#include "nic/metaswitch/stubs/common/pds_ms_tep_store.hpp"
#include "nic/apollo/api/include/pds_batch.hpp"
#include "nic/sdk/include/sdk/eth.hpp"
#include <gtest/gtest.h>
#include <ifaddrs.h>
#include <net/if.h>
#include <netpacket/packet.h>
#include <iostream>

using pds_ms::state_t;
using pds_ms::tep_store_t;
using pds_ms::tep_obj_t;
using pds_ms::tep_obj_uptr_t;
using pds_ms::route_table_store_t;
using pds_ms::route_table_obj_t;
using pds_ms::rttbl_obj_uptr_t;

sdk_ret_t pds_batch_commit(pds_batch_ctxt_t bctxt) {
    return SDK_RET_OK;
}
sdk_ret_t pds_batch_destroy(pds_batch_ctxt_t bctxt) {
    return SDK_RET_OK;
}

namespace api_test {

////////////////////////////////////////////////////////////////////////////////
// TEP store tests
////////////////////////////////////////////////////////////////////////////////
class tep_store_test : public ::testing::Test {
protected:
    tep_store_test() : state_thr_ctxt (state_t::thread_context()) {};
    virtual ~tep_store_test() {}

public:
    state_t::context_t state_thr_ctxt;
};
const char* g_pds_ms_store_ip_str = "172.17.0.6";

TEST_F(tep_store_test, create) {
    auto state = state_thr_ctxt.state();
    ASSERT_TRUE (state->get_slab_in_use (pds_ms::PDS_MS_TEP_SLAB_ID) == 0);
    ip_addr_t tep_ip;
    str2ipaddr((char*) g_pds_ms_store_ip_str, &tep_ip);
    state->tep_store().add_upd (tep_ip,   // TEP IP
                                tep_obj_uptr_t (new tep_obj_t (
                                        tep_ip, // TEP IP
                                        11,     // Underlay ECMP Idx
                                        501009 // PDS TEP Idx
                                        )));
    ASSERT_TRUE (state->get_slab_in_use (pds_ms::PDS_MS_TEP_SLAB_ID) == 1);
}

TEST_F(tep_store_test, get) {
    // Test the BD store
    auto state = state_thr_ctxt.state();
    ip_addr_t tep_ip;
    str2ipaddr((char*) g_pds_ms_store_ip_str, &tep_ip);
    auto tep_obj = state->tep_store().get (tep_ip);
    ASSERT_TRUE (tep_obj != nullptr);
    ASSERT_TRUE (tep_obj->properties().tep_ip == tep_ip);
    ASSERT_TRUE (tep_obj->properties().hal_uecmp_idx == 11);
    ASSERT_TRUE (tep_obj->properties().hal_tep_idx == 501009);
    ASSERT_TRUE (tep_obj->hal_oecmp_idx_guard->idx() == 1);
}

TEST_F(tep_store_test, update) {
    // Update existing entry
    auto state = state_thr_ctxt.state();
    ip_addr_t tep_ip;
    str2ipaddr((char*) g_pds_ms_store_ip_str, &tep_ip);
    auto tep_old = state->tep_store().get (tep_ip);

    // Make a new copy of the old object and update a field
    auto tep_obj = new tep_obj_t(*tep_old);
    tep_obj->properties().hal_uecmp_idx = 12;

    // After insert old object should be freed back to slab
    ASSERT_TRUE (state->get_slab_in_use (pds_ms::PDS_MS_TEP_SLAB_ID) == 2);
    state->tep_store().add_upd (tep_ip, tep_obj);
    ASSERT_TRUE (state->get_slab_in_use (pds_ms::PDS_MS_TEP_SLAB_ID) == 1);

    // Test the BD store
    auto tep_obj_test = state->tep_store().get (tep_ip);
    ASSERT_TRUE (tep_obj_test != nullptr);
    ASSERT_TRUE (tep_obj_test->properties().tep_ip == tep_ip);
    ASSERT_TRUE (tep_obj->properties().hal_uecmp_idx == 12);
    ASSERT_TRUE (tep_obj->properties().hal_tep_idx == 501009);
    ASSERT_TRUE (tep_obj->hal_oecmp_idx_guard->idx() == 1);
}

TEST_F(tep_store_test, delete_store) {
    // Delete entry
    auto state = state_thr_ctxt.state();
    ip_addr_t tep_ip;
    str2ipaddr((char*) g_pds_ms_store_ip_str, &tep_ip);
    state->tep_store().erase (tep_ip);
    auto tep_obj = state->tep_store().get (tep_ip);
    ASSERT_TRUE (tep_obj == nullptr);
    ASSERT_TRUE (state->get_slab_in_use (pds_ms::PDS_MS_TEP_SLAB_ID) == 0);
}

////////////////////////////////////////////////////////////////////////////////
// ROUTE store tests
////////////////////////////////////////////////////////////////////////////////
class route_store_test : public ::testing::Test {
protected:
    route_store_test() : state_thr_ctxt (state_t::thread_context()) {};
    virtual ~route_store_test() {}

public:
    state_t::context_t state_thr_ctxt;
};
static const pds_obj_key_t k_route_key = pds_ms::msidx2pdsobjkey(1);

TEST_F(route_store_test, create) {
    auto state = state_thr_ctxt.state();
    ASSERT_TRUE (state->get_slab_in_use (pds_ms::PDS_MS_RTTABLE_SLAB_ID) == 0);
    state->route_table_store().add_upd(k_route_key,
                        rttbl_obj_uptr_t (new route_table_obj_t(k_route_key)));
    ASSERT_TRUE (state->get_slab_in_use (pds_ms::PDS_MS_RTTABLE_SLAB_ID) == 1);

    auto rttbl = state->route_table_store().get(k_route_key);
    ASSERT_TRUE (rttbl != nullptr);
    ASSERT_TRUE (rttbl->num_routes() == 0);

    // Add route 1
    pds_route_t route = {0};
    route.prefix.addr.af = IP_AF_IPV4;
    route.prefix.addr.addr.v4_addr = 0x01020300;
    route.prefix.len = 20;
    route.prio = 100;
    rttbl->add_upd_route(route);
    ASSERT_TRUE (rttbl->num_routes() == 1);

    // Add route 2
    route.prefix.addr.af = IP_AF_IPV4;
    route.prefix.addr.addr.v4_addr = 0x11121300;
    route.prefix.len = 24;
    route.prio = 200;
    rttbl->add_upd_route(route);
    ASSERT_TRUE (rttbl->num_routes() == 2);
    
    // Add route 3
    route.prefix.addr.af = IP_AF_IPV4;
    route.prefix.addr.addr.v4_addr = 0x11121300;
    route.prefix.len = 31;
    route.prio = 500;
    rttbl->add_upd_route(route);
    ASSERT_TRUE (rttbl->num_routes() == 3);
    
    // Add route 4
    route.prefix.addr.af = IP_AF_IPV4;
    route.prefix.addr.addr.v4_addr = 0x11121300;
    route.prefix.len = 30;
    route.prio = 200;
    rttbl->add_upd_route(route);
    ASSERT_TRUE (rttbl->num_routes() == 4);
}

TEST_F(route_store_test, get) {
    // Test the route store
    auto state = state_thr_ctxt.state();
    ASSERT_TRUE (state->get_slab_in_use (pds_ms::PDS_MS_RTTABLE_SLAB_ID) == 1);
    auto rttbl = state->route_table_store().get(k_route_key);
    ASSERT_TRUE (rttbl != nullptr);

    // Get route 2
    ip_prefix_t prefix = {0};
    prefix.addr.af = IP_AF_IPV4;
    prefix.addr.addr.v4_addr = 0x11121300;
    prefix.len = 24;
    auto rt = rttbl->get_route(prefix);
    ASSERT_TRUE (rt != nullptr);
    ASSERT_TRUE (rt->prio == 200);
}

TEST_F(route_store_test, update) {
    // Update existing entry
    auto state = state_thr_ctxt.state();
    auto rttbl = state->route_table_store().get(k_route_key);
    // Update route 1
    pds_route_t route = {0};
    route.prefix.addr.af = IP_AF_IPV4;
    route.prefix.addr.addr.v4_addr = 0x01020300;
    route.prefix.len = 20;
    route.prio = 300;
    rttbl->add_upd_route(route);

    // Get route 1 and validate
    ip_prefix_t pfx = {0};
    pfx.addr.af = IP_AF_IPV4;
    pfx.addr.addr.v4_addr = 0x01020300;
    pfx.len = 20;
    auto rt = rttbl->get_route(pfx);
    ASSERT_TRUE (rt != nullptr);
    ASSERT_TRUE (rt->prio == 300);
}

TEST_F(route_store_test, del) {
    // Delete entry
    auto state = state_thr_ctxt.state();
    auto rttbl = state->route_table_store().get(k_route_key);

    // Delete route 2
    ip_prefix_t prefix = {0};
    prefix.addr.af = IP_AF_IPV4;
    prefix.addr.addr.v4_addr = 0x11121300;
    prefix.len = 24;
    rttbl->del_route(prefix);
    // Check if route is deleted
    auto rt = rttbl->get_route(prefix);
    ASSERT_TRUE (rt == nullptr);
    ASSERT_TRUE (rttbl->num_routes() == 3);

    // Delete route 4
    prefix = {0};
    prefix.addr.af = IP_AF_IPV4;
    prefix.addr.addr.v4_addr = 0x11121300;
    prefix.len = 30;
    rttbl->del_route(prefix);
    // Check if route is deleted
    rt = rttbl->get_route(prefix);
    ASSERT_TRUE (rt == nullptr);
    ASSERT_TRUE (rttbl->num_routes() == 2);

    // Delete route 1
    prefix = {0};
    prefix.addr.af = IP_AF_IPV4;
    prefix.addr.addr.v4_addr = 0x01020300;
    prefix.len = 20;
    rttbl->del_route(prefix);
    // Check if route is deleted
    rt = rttbl->get_route(prefix);
    ASSERT_TRUE (rt == nullptr);
    ASSERT_TRUE (rttbl->num_routes() == 1);

    // Get route 3 and validate
    ip_prefix_t pfx = {0};
    pfx.addr.af = IP_AF_IPV4;
    pfx.addr.addr.v4_addr = 0x11121300;
    pfx.len = 32;
    rt = rttbl->get_route(pfx);
    // Incorrect pfx, get should fail
    ASSERT_TRUE (rt == nullptr);
    // Fix the pfx
    pfx.len = 31;
    // Get should pass now
    rt = rttbl->get_route(pfx);
    ASSERT_TRUE (rt != nullptr);
    ASSERT_TRUE (rt->prio == 500);
    // Delete route 3
    rttbl->del_route(pfx);
    // Check if route is deleted
    rt = rttbl->get_route(pfx);
    ASSERT_TRUE (rt == nullptr);
    ASSERT_TRUE (rttbl->num_routes() == 0);

    // Delete complete rttbl
    state->route_table_store().erase(k_route_key);
    ASSERT_TRUE (state->get_slab_in_use (pds_ms::PDS_MS_RTTABLE_SLAB_ID) == 0);
}

} // namespace api_test

//----------------------------------------------------------------------------
// Entry point
//----------------------------------------------------------------------------

/// @private
int
main (int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    state_t::create();
    return RUN_ALL_TESTS();
}


