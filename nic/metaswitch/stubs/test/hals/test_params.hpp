//------------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//------------------------------------------------------------------------------

#ifndef __PDS_MS_TEST_PARAMS_HPP__
#define __PDS_MS_TEST_PARAMS_HPP__

#include <gtest/gtest.h>
#include "nic/apollo/test/api/utils/base.hpp"

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
 *-------------------------------------------------*/

namespace pds_ms_test {

class test_input_base_t {
public:
   virtual void trigger_create(void) = 0;
   virtual void modify(void) = 0;
   virtual void trigger_update(void) = 0;
   virtual void trigger_delete(void) = 0;

   virtual void next(void) = 0;
   virtual ~test_input_base_t(void) {};
   virtual bool ips_mock() = 0;
    virtual void cleanup(void) {};
};

class test_params_t;

class test_output_base_t {
public:
    virtual void expect_create() = 0;
    virtual void expect_update() = 0;
    virtual void expect_delete() = 0;
    // Should check for expected result and update the result in test_params
    virtual void validate() = 0;
    virtual void expect_pds_spec_op_fail() = 0;
    virtual void expect_pds_batch_commit_fail() = 0;
    virtual void expect_create_pds_async_fail() = 0;

    virtual void cleanup(void) = 0;
    virtual ~test_output_base_t(void) {};
};

class test_params_t {
public:
    test_input_base_t* test_input = nullptr;
    test_output_base_t* test_output = nullptr;
};

test_params_t* test_params();

// To be implemented by the test module
void vxlan_reg_test_input();
void vxlan_reg_test_output();

} // End namespace pds_ms_test

#endif
