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

namespace api_test {

class bd_store_test : public ::testing::Test {
protected:
    bd_store_test() {
        state->create();
    }
    virtual ~bd_store_test() {}

public:
    state_t *state;
};

TEST_F(bd_store_test, create) {
    state_t::state()->bd_store();
}

TEST_F(bd_store_test, get) {
    // Test the BD store
}

TEST_F(bd_store_test, update) {
    // Test the BD store
}

TEST_F(bd_store_test, delete_store) {
    // Test the BD store
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
    return RUN_ALL_TESTS();
}


