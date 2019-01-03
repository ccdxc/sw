// Copyright (c) 2018 Pensando Systems, Inc.

#include <stdio.h>
#include <gtest/gtest.h>
#include "nic/hal/apollo/test/oci_test_base.hpp"
#include "nic/hal/apollo/include/api/oci_batch.hpp"
#include "nic/hal/apollo/include/api/oci_vcn.hpp"
#include "nic/hal/apollo/include/api/oci_subnet.hpp"

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

static void
create_subnet(uint32_t vcn_id, uint32_t num_subnets) {
    sdk_ret_t       rv;
    oci_subnet_t    oci_subnet;

    for (uint32_t i = 1; i <= num_subnets; i++) {
        memset(&oci_subnet, 0, sizeof(oci_subnet));
        oci_subnet.key.vcn_id = vcn_id;
        oci_subnet.key.id = 1;
        oci_subnet.pfx.addr.af = IP_AF_IPV4;
        oci_subnet.pfx.addr.addr.v4_addr = ((vcn_id << 16) | (i << 8));
        oci_subnet.pfx.len = 24;
        oci_subnet.vr_ip.af = IP_AF_IPV4;
        oci_subnet.vr_ip.addr.v4_addr = ((vcn_id << 16) | (i << 8) | 1) ;
        MAC_UINT64_TO_ADDR(oci_subnet.vr_mac,
                           (uint64_t)oci_subnet.vr_ip.addr.v4_addr);
        rv = oci_subnet_create(&oci_subnet);
        ASSERT_TRUE(rv == SDK_RET_OK);
    }
}

static void
create_vcn(uint32_t num_vcns) {
    sdk_ret_t    rv;
    oci_vcn_t    oci_vcn;

    for (uint32_t i = 1; i <= num_vcns; i++) {
        memset(&oci_vcn, 0, sizeof(oci_vcn));
        oci_vcn.type = OCI_VCN_TYPE_TENANT;
        oci_vcn.key.id = i;
        rv = oci_vcn_create(&oci_vcn);
        ASSERT_TRUE(rv == SDK_RET_OK);
        create_subnet(i, 1);
    }
}

TEST_F(vcn_test, vcn_create_delete) {
    sdk_ret_t             rv;
    oci_batch_params_t    batch_params = { 0 };

    batch_params.epoch = 1;
    rv = oci_batch_start(&batch_params);
    ASSERT_TRUE(rv == SDK_RET_OK);
    create_vcn(1024);
    rv = oci_batch_commit();
    ASSERT_TRUE(rv == SDK_RET_OK);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
