//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This file contains the unit tests for the common stores
///
//----------------------------------------------------------------------------

#include "nic/metaswitch/stubs/common/pdsa_state.hpp"
#include "nic/metaswitch/stubs/common/pdsa_bd_store.hpp"
#include "nic/apollo/api/include/pds_batch.hpp"
#include "nic/sdk/include/sdk/eth.hpp"
#include <gtest/gtest.h>
#include <ifaddrs.h>
#include <net/if.h>
#include <netpacket/packet.h>
#include <iostream>

using pdsa_stub::state_t;
using pdsa_stub::bd_store_t;
using pdsa_stub::bd_obj_t;
using pdsa_stub::bd_obj_uptr_t;

sdk_ret_t pds_batch_commit(pds_batch_ctxt_t bctxt) {
    return SDK_RET_OK;
}
sdk_ret_t pds_batch_destroy(pds_batch_ctxt_t bctxt) {
    return SDK_RET_OK;
}

namespace api_test {

class bd_store_test : public ::testing::Test {
protected:
    bd_store_test() : state_thr_ctxt (state_t::thread_context()) {};
    virtual ~bd_store_test() {}

public:
    state_t::context_t state_thr_ctxt;
};

TEST_F(bd_store_test, create) {
    auto state = state_thr_ctxt.state();
    ASSERT_TRUE (state->get_slab_in_use (pdsa_stub::PDSA_BD_SLAB_ID) == 0);
    state->bd_store().add_upd (100,   // MS bd id
                               bd_obj_uptr_t (new bd_obj_t (
                               {      // bd properties
                               100,    // MS bd id
                               501009, // vni
                               100     // hal bd index
                               })
                              ));
    ASSERT_TRUE (state->get_slab_in_use (pdsa_stub::PDSA_BD_SLAB_ID) == 1);
}

TEST_F(bd_store_test, get) {
    // Test the BD store
    auto state = state_thr_ctxt.state();
    auto bd_obj = state->bd_store().get (100);
    ASSERT_TRUE (bd_obj != nullptr);
    ASSERT_TRUE (bd_obj->properties().bd_id == 100);
    ASSERT_TRUE (bd_obj->properties().vni == 501009);
    ASSERT_TRUE (bd_obj->properties().hal_idx == 100);
}

TEST_F(bd_store_test, update) {
    // Update existing entry
    auto state = state_thr_ctxt.state();
    auto bd_old = state->bd_store().get (100);

    // Make a new copy of the old object and update a field
    auto bd_obj = new bd_obj_t(*bd_old);
    bd_obj->properties().hal_idx = 101;

    // After insert old object should be freed back to slab
    ASSERT_TRUE (state->get_slab_in_use (pdsa_stub::PDSA_BD_SLAB_ID) == 2);
    state->bd_store().add_upd (100, bd_obj);
    ASSERT_TRUE (state->get_slab_in_use (pdsa_stub::PDSA_BD_SLAB_ID) == 1);

    // Test the BD store
    auto bd_obj_test = state->bd_store().get (100);
    ASSERT_TRUE (bd_obj_test != nullptr);
    ASSERT_TRUE (bd_obj_test->properties().bd_id == 100);
    ASSERT_TRUE (bd_obj_test->properties().vni == 501009);
    ASSERT_TRUE (bd_obj_test->properties().hal_idx == 101);
}

TEST_F(bd_store_test, delete_store) {
    // Delete entry
    auto state = state_thr_ctxt.state();
    state->bd_store().erase (100);
    auto bd_obj = state->bd_store().get (100);
    ASSERT_TRUE (bd_obj == nullptr);
    ASSERT_TRUE (state->get_slab_in_use (pdsa_stub::PDSA_BD_SLAB_ID) == 0);
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


