//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This file contains all VXLAN tunnel test cases
///
//----------------------------------------------------------------------------

#include "nic/metaswitch/stubs/test/hals/test_params.hpp"
#include "nic/metaswitch/stubs/test/hals/vxlan_test_params.hpp"
#include "nic/metaswitch/stubs/test/hals/phy_port_test_params.hpp"
#include "nic/metaswitch/stubs/common/pdsa_state_init.hpp"
#include "nic/metaswitch/stubs/hals/pdsa_hal_init.hpp"
#include "nic/metaswitch/stubs/pdsa_stubs_init.hpp"
#include "nic/apollo/api/include/pds_init.hpp"
#include "nic/apollo/test/utils/base.hpp"
#include <iostream>
#include <fstream>

/*------- Test class hierarchy and dependencies -----
 *
 * xxx - stands for an area that is being tested
 *       e.g. vxlan - tests vxlan tunnel create,update,delete
 *
 *                 test_params 
 *                 /         \
 *                /           \
 *       test_input_base   test_output_base 
 *                |             \
 *                |            pds_api_mock
 *          xxx_test_params        \
 *           /           \          \
 *          /             \          \
 *         /               \          \
 *    xxx_ips_feeder        xxx_pds_mock
 *        (or)                   (or)
 *    xxx_proto_feeder      xxx_hal_pds_verif
 *
 *-------------------------------------------------*/
  
namespace pdsa_test {
test_params_t* test_params() {    
    static test_params_t  g_test_params;
    return &g_test_params;
}
} // End namespace pdsa_test

class pdsa_hals_test: public ::testing::Test {
protected:
    void TearDown(void) {
        pdsa_test::test_params()->test_output->cleanup();
        pdsa_test::test_params()->test_input = nullptr;
        pdsa_test::test_params()->test_output = nullptr;
    }
};

TEST_F(pdsa_hals_test, phy_port_test) {
    pdsa_test::load_phy_port_test();
    auto test_input = pdsa_test::test_params()->test_input;
    auto test_output = pdsa_test::test_params()->test_output;

    // Initialize
    auto phy_port_input = dynamic_cast<pdsa_test::phy_port_input_params_t*>
                              (test_input);
    phy_port_input->init ();

    // Create
    std::cout << "=== IF Create test ===" << std::endl;
    test_output->expect_create();
    test_input->trigger_create();
    test_output->validate();

    // Update
    std::cout << "=== IF Update test ===" << std::endl;
    test_input->modify(); // Change any field
    test_output->expect_update();
    test_input->trigger_update();
    test_output->validate();

    // Delete
    std::cout << "=== IF Delete test ===" << std::endl;
    test_output->expect_delete();
    test_input->trigger_delete();
    test_output->validate();

    // Mock batch update failure
    std::cout << "=== IF Spec Create failure test ===" << std::endl;
    test_input->next();
    test_output->expect_pds_spec_op_fail();
    test_input->trigger_create();
    test_output->validate();

    // Mock batch commit failure
    std::cout << "=== IF Spec Batch commit failure test ===" << std::endl;
    test_input->next();
    test_output->expect_pds_batch_commit_fail();
    test_input->trigger_create();
    test_output->validate();

    // Mock async batch failure return
    std::cout << "=== IF Spec Batch commit async failure test ===" << std::endl;
    test_input->next();
    test_output->expect_create_pds_async_fail();
    test_input->trigger_create();
    test_output->validate();
}

TEST_F(pdsa_hals_test, vxlan_test) {
    pdsa_test::load_vxlan_test();
    auto test_input = pdsa_test::test_params()->test_input;
    auto test_output = pdsa_test::test_params()->test_output;

    // Initialize
    auto vxlan_input = dynamic_cast<pdsa_test::vxlan_input_params_t*>(test_input);
    vxlan_input->init ("10.0.0.1",  // Source IP
                       "20.0.0.1"); // Dest IP
    // Create
    std::cout << "=== TEP Create test ===" << std::endl;
    test_output->expect_create();
    test_input->trigger_create();
    test_output->validate();

    // Update
    std::cout << "=== TEP Update test ===" << std::endl;
    test_input->modify(); // Change any field
    test_output->expect_update();
    test_input->trigger_update();
    test_output->validate();

    // Delete
    std::cout << "=== TEP Delete test ===" << std::endl;
    test_output->expect_delete();
    test_input->trigger_delete();
    test_output->validate();

    // Mock batch update failure
    std::cout << "=== TEP Spec Create failure test ===" << std::endl;
    test_input->next();
    test_output->expect_pds_spec_op_fail();
    test_input->trigger_create();
    test_output->validate();

    // Mock batch commit failure
    std::cout << "=== TEP Spec Batch commit failure test ===" << std::endl;
    test_input->next();
    test_output->expect_pds_batch_commit_fail();
    test_input->trigger_create();
    test_output->validate();

    // Mock async batch failure return
    std::cout << "=== TEP Spec Batch commit async failure test ===" << std::endl;
    test_input->next();
    test_output->expect_create_pds_async_fail();
    test_input->trigger_create();
    test_output->validate();
}

//----------------------------------------------------------------------------
// Entry point
//----------------------------------------------------------------------------

int
main (int argc, char **argv)
{
    // Call the mock pds init
    pds_init(nullptr);
    // Initialize the State and HAL stubs
    pdsa_stub::state_init();
    pdsa_stub::hal_init();

#ifdef PDS_MOCKAPI
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
#else
    return api_test_program_run(argc, argv);
#endif
}
