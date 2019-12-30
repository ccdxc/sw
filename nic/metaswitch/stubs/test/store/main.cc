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

sdk_ret_t pds_batch_commit(pds_batch_ctxt_t bctxt) {
    return SDK_RET_OK;
}
sdk_ret_t pds_batch_destroy(pds_batch_ctxt_t bctxt) {
    return SDK_RET_OK;
}

namespace api_test {

class tep_store_test : public ::testing::Test {
protected:
    tep_store_test() : state_thr_ctxt (state_t::thread_context()) {};
    virtual ~tep_store_test() {}

public:
    state_t::context_t state_thr_ctxt;
};

TEST_F(tep_store_test, create) {
    auto state = state_thr_ctxt.state();
    ASSERT_TRUE (state->get_slab_in_use (pds_ms::PDS_MS_TEP_SLAB_ID) == 0);
    ipv4_addr_t tep_ip;
    str2ipv4addr((char*) "10.1.2.54", &tep_ip);

    ip_addr_t tep_ip_prop;
    tep_ip_prop.af = IP_AF_IPV4;
    tep_ip_prop.addr.v4_addr = tep_ip; 

    state->tep_store().add_upd (tep_ip,   // TEP IP
                               tep_obj_uptr_t (new tep_obj_t (
                               tep_ip_prop, // MS bd id
                               11,     // Underlay ECMP Idx
                               501009, // PDS TEP Idx
                               501009  // PDS Overlay ECMP index
                               )
                              ));
    ASSERT_TRUE (state->get_slab_in_use (pds_ms::PDS_MS_TEP_SLAB_ID) == 1);
}

TEST_F(tep_store_test, get) {
    // Test the BD store
    auto state = state_thr_ctxt.state();
    ipv4_addr_t tep_ip;
    str2ipv4addr((char*) "10.1.2.54", &tep_ip);
    auto tep_obj = state->tep_store().get (tep_ip);
    ASSERT_TRUE (tep_obj != nullptr);
    ASSERT_TRUE (tep_obj->properties().tep_ip.addr.v4_addr == tep_ip);
    ASSERT_TRUE (tep_obj->properties().hal_uecmp_idx == 11);
    ASSERT_TRUE (tep_obj->properties().hal_tep_idx == 501009);
    ASSERT_TRUE (tep_obj->properties().hal_oecmp_idx == 501009);
}

TEST_F(tep_store_test, update) {
    // Update existing entry
    auto state = state_thr_ctxt.state();
    ipv4_addr_t tep_ip;
    str2ipv4addr((char*) "10.1.2.54", &tep_ip);
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
    ASSERT_TRUE (tep_obj_test->properties().tep_ip.addr.v4_addr == tep_ip);
    ASSERT_TRUE (tep_obj->properties().hal_uecmp_idx == 12);
    ASSERT_TRUE (tep_obj->properties().hal_tep_idx == 501009);
    ASSERT_TRUE (tep_obj->properties().hal_oecmp_idx == 501009);
}

TEST_F(tep_store_test, delete_store) {
    // Delete entry
    auto state = state_thr_ctxt.state();
    ipv4_addr_t tep_ip;
    str2ipv4addr((char*) "10.1.2.54", &tep_ip);
    state->tep_store().erase (tep_ip);
    auto tep_obj = state->tep_store().get (tep_ip);
    ASSERT_TRUE (tep_obj == nullptr);
    ASSERT_TRUE (state->get_slab_in_use (pds_ms::PDS_MS_TEP_SLAB_ID) == 0);
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


