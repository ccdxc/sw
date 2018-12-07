// Copyright (c) 2018 Pensando Systems, Inc.

#include <stdio.h>
#include <gtest/gtest.h>
#include "nic/hal/apollo/test/oci_test_base.hpp"
#include "nic/hal/apollo/include/api/oci_batch.hpp"
#include "nic/hal/apollo/include/api/oci_vcn.hpp"

class vcn_test : public oci_test_base {
protected:
    vcn_test() {}
    virtual ~vcn_test() {}
    /**< called immediately after the constructor before each test */
    virtual void SetUp() {}
    /**< called immediately after each test before the destructor */
    virtual void TearDown() {}
    /**< called at the beginning of all test cases in this class */
    static void SetUpTestCase() {
        /**< call base class function */
        oci_test_base::SetUpTestCase(false);
    }
};

TEST_F(vcn_test, vcn_create_delete) {
    sdk_ret_t    rv;
    oci_vcn_t    oci_vcn;
    oci_batch_params_t    batch_params = { 0 };

    batch_params.epoch = 1;
    rv = oci_batch_start(&batch_params);
    ASSERT_TRUE(rv == sdk::SDK_RET_OK);

    oci_vcn.type = OCI_VCN_TYPE_TENANT;
    oci_vcn.key.id = 1;
    rv = oci_vcn_create(&oci_vcn);
    ASSERT_TRUE(rv == sdk::SDK_RET_OK);
    rv = oci_batch_commit();
    ASSERT_TRUE(rv == sdk::SDK_RET_OK);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
