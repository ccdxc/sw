//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This file contains the unit tests for the common stores
///
//----------------------------------------------------------------------------

#include "nic/metaswitch/stubs/test/utils/base.hpp"
#include "nic/metaswitch/stubs/common/pdsa_state.hpp"
#include "nic/metaswitch/stubs/common/pdsa_bd_store.hpp"

using pdsa_stub::state_t;
using pdsa_stub::bd_store_t;
using pdsa_stub::bd_obj_t;
using pdsa_stub::bd_obj_uptr_t;

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
    auto bd_obj = new bd_obj_t(*bd_old);
    ASSERT_TRUE (bd_obj != nullptr);
    bd_obj->properties().hal_idx = 101;
    ASSERT_TRUE (bd_obj->properties().hal_idx == 101);
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


