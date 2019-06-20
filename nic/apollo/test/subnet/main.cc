//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This file contains all subnet test cases
///
//----------------------------------------------------------------------------

#include "nic/apollo/test/utils/utils.hpp"
#include "nic/apollo/test/utils/base.hpp"
#include "nic/apollo/test/utils/batch.hpp"
#include "nic/apollo/test/utils/api_base.hpp"
#include "nic/apollo/test/utils/workflow1.hpp"
#include "nic/apollo/test/utils/subnet.hpp"
#include "nic/apollo/test/utils/vpc.hpp"

namespace api_test {

// Globals
static constexpr uint32_t k_max_subnet = PDS_MAX_SUBNET + 1;

//----------------------------------------------------------------------------
// Subnet test class
//----------------------------------------------------------------------------

class subnet : public ::pds_test_base {
protected:
    subnet() {}
    virtual ~subnet() {}
    virtual void SetUp() {}
    virtual void TearDown() {}
    static void SetUpTestCase() {
        pds_test_base::SetUpTestCase(g_tc_params);

        batch_start();
        sample_vpc_setup(PDS_VPC_TYPE_TENANT);
        batch_commit();
    }
    static void TearDownTestCase() {
        batch_start();
        sample_vpc_teardown(PDS_VPC_TYPE_TENANT);
        batch_commit();
        pds_test_base::TearDownTestCase();
    }
};

//----------------------------------------------------------------------------
// Subnet test cases implementation
//----------------------------------------------------------------------------

/// \defgroup SUBNET_TEST
/// @{

/// \brief Subnet WF_1
TEST_F(subnet, subnet_workflow_1) {
    pds_subnet_key_t key = {.id = 1};
    subnet_feeder feeder;

    feeder.init(key, k_vpc_key, "10.0.0.0/16", k_max_subnet);
    workflow_1<subnet_feeder>(feeder);
}

/// \brief Subnet WF_2
TEST_F(subnet, subnet_workflow_2) {
    pds_subnet_key_t key = {.id = 1};
    subnet_feeder feeder;

    feeder.init(key, k_vpc_key, "10.0.0.0/16", k_max_subnet);
    workflow_2<subnet_feeder>(feeder);
}

/// \brief Subnet WF_3
TEST_F(subnet, subnet_workflow_3) {
    pds_subnet_key_t key1 = {.id = 10}, key2 = {.id = 40}, key3 = {.id = 70};
    subnet_feeder feeder1, feeder2, feeder3;

    feeder1.init(key1, k_vpc_key, "10.0.0.0/16", 20);
    feeder2.init(key2, k_vpc_key, "30.0.0.0/16", 20);
    feeder3.init(key3, k_vpc_key, "60.0.0.0/16", 20);
    workflow_3<subnet_feeder>(feeder1, feeder2, feeder3);
}

/// \brief Subnet WF_4
TEST_F(subnet, subnet_workflow_4) {
    pds_subnet_key_t key = {.id = 1};
    subnet_feeder feeder = {};

    feeder.init(key, k_vpc_key, "10.0.0.0/16", k_max_subnet);
    workflow_4<subnet_feeder>(feeder);
}

/// \brief Subnet WF_5
TEST_F(subnet, subnet_workflow_5) {
    pds_subnet_key_t key1 = {.id = 10}, key2 = {.id = 40}, key3 = {.id = 70};
    subnet_feeder feeder1, feeder2, feeder3;

    feeder1.init(key1, k_vpc_key, "10.0.0.0/16", 20);
    feeder2.init(key2, k_vpc_key, "30.0.0.0/16", 20);
    feeder3.init(key3, k_vpc_key, "60.0.0.0/16", 20);
    workflow_5<subnet_feeder>(feeder1, feeder2, feeder3);
}

/// \brief Subnet WF_6
TEST_F(subnet, subnet_workflow_6) {
    pds_subnet_key_t key = {.id = 1};
    subnet_feeder feeder1, feeder1A, feeder1B;

    feeder1.init(key, k_vpc_key, "10.0.0.0/16", k_max_subnet);
    feeder1A.init(key, k_vpc_key, "11.0.0.0/16", k_max_subnet);
    feeder1B.init(key, k_vpc_key, "12.0.0.0/16", k_max_subnet);
    workflow_6<subnet_feeder>(feeder1, feeder1A, feeder1B);
}

/// \brief Subnet WF_7
TEST_F(subnet, subnet_workflow_7) {
    pds_subnet_key_t key = {.id = 1};
    subnet_feeder feeder1, feeder1A, feeder1B;

    feeder1.init(key, k_vpc_key, "10.0.0.0/16", k_max_subnet);
    feeder1A.init(key, k_vpc_key, "11.0.0.0/16", k_max_subnet);
    feeder1B.init(key, k_vpc_key, "12.0.0.0/16", k_max_subnet);
    workflow_7<subnet_feeder>(feeder1, feeder1A, feeder1B);
}

/// \brief Subnet WF_8
TEST_F(subnet, DISABLED_subnet_workflow_8) {
    pds_subnet_key_t key = {.id = 1};
    subnet_feeder feeder1, feeder1A, feeder1B;

    feeder1.init(key, k_vpc_key, "10.0.0.0/16", k_max_subnet);
    feeder1A.init(key, k_vpc_key, "11.0.0.0/16", k_max_subnet);
    feeder1B.init(key, k_vpc_key, "12.0.0.0/16", k_max_subnet);
    workflow_8<subnet_feeder>(feeder1, feeder1A, feeder1B);
}

/// \brief Subnet WF_9
TEST_F(subnet, subnet_workflow_9) {
    pds_subnet_key_t key = {.id = 1};
    subnet_feeder feeder1, feeder1A;

    feeder1.init(key, k_vpc_key, "10.0.0.0/16", k_max_subnet);
    feeder1A.init(key, k_vpc_key, "11.0.0.0/16", k_max_subnet);
    workflow_9<subnet_feeder>(feeder1, feeder1A);
}

/// \brief Subnet WF_10
TEST_F(subnet, DISABLED_subnet_workflow_10) {
    pds_subnet_key_t key1 = {.id = 10}, key2 = {.id = 40}, key3 = {.id = 70},
                     key4 = {.id = 100};
    subnet_feeder feeder1, feeder2, feeder3, feeder4, feeder2A, feeder3A;

    feeder1.init(key1, k_vpc_key, "10.0.0.0/16", 20);
    feeder2.init(key2, k_vpc_key, "40.0.0.0/16", 20);
    feeder2A.init(key2, k_vpc_key, "140.0.0.0/16", 20);
    feeder3.init(key3, k_vpc_key, "70.0.0.0/16", 20);
    feeder3A.init(key3, k_vpc_key, "170.0.0.0/16", 20);
    feeder4.init(key4, k_vpc_key, "100.0.0.0/16", 20);
    workflow_10<subnet_feeder>(
        feeder1, feeder2, feeder2A, feeder3, feeder3A, feeder4);
}

/// \brief Subnet WF_N_1
TEST_F(subnet, subnet_workflow_neg_1) {
    pds_subnet_key_t key = {.id = 1};
    subnet_feeder feeder;

    feeder.init(key, k_vpc_key, "10.0.0.0/16", k_max_subnet);
    workflow_neg_1<subnet_feeder>(feeder);
}

/// \brief Subnet WF_N_2
TEST_F(subnet, subnet_workflow_neg_2) {
    pds_subnet_key_t key = {.id = 1};
    subnet_feeder feeder;

    feeder.init(key, k_vpc_key, "10.0.0.0/16", k_max_subnet + 1);
    workflow_neg_2<subnet_feeder>(feeder);
}

/// \brief Subnet WF_N_3
TEST_F(subnet, subnet_workflow_neg_3) {
    pds_subnet_key_t key = {.id = 1};
    subnet_feeder feeder;

    feeder.init(key, k_vpc_key, "10.0.0.0/16", k_max_subnet);
    workflow_neg_3<subnet_feeder>(feeder);
}

/// \brief Subnet WF_N_4
TEST_F(subnet, subnet_workflow_neg_4) {
    pds_subnet_key_t key1 = {.id = 10}, key2 = {.id = 40};
    subnet_feeder feeder1, feeder2;

    feeder1.init(key1, k_vpc_key, "10.0.0.0/16", 20);
    feeder2.init(key2, k_vpc_key, "40.0.0.0/16", 20);
    workflow_neg_4<subnet_feeder>(feeder1, feeder2);
}

/// \brief Subnet WF_N_5
TEST_F(subnet, DISABLED_subnet_workflow_neg_5) {
    pds_subnet_key_t key = {.id = 1};
    subnet_feeder feeder1, feeder1A;

    feeder1.init(key, k_vpc_key, "10.0.0.0/16", k_max_subnet);
    feeder1A.init(key, k_vpc_key, "11.0.0.0/16", k_max_subnet);
    workflow_neg_5<subnet_feeder>(feeder1, feeder1A);
}

/// \brief Subnet WF_N_6
TEST_F(subnet, DISABLED_subnet_workflow_neg_6) {
    pds_subnet_key_t key = {.id = 1};
    subnet_feeder feeder1, feeder1A;

    feeder1.init(key, k_vpc_key, "10.0.0.0/16", k_max_subnet);
    feeder1A.init(key, k_vpc_key, "11.0.0.0/16", k_max_subnet + 1);
    workflow_neg_6<subnet_feeder>(feeder1, feeder1A);
}

/// \brief Subnet WF_N_7
TEST_F(subnet, subnet_workflow_neg_7) {
    pds_subnet_key_t key1 = {.id = 10}, key2 = {.id = 40};
    subnet_feeder feeder1, feeder1A, feeder2;

    feeder1.init(key1, k_vpc_key, "10.0.0.0/16", 20);
    feeder1A.init(key1, k_vpc_key, "11.0.0.0/16", 20);
    feeder2.init(key2, k_vpc_key, "12.0.0.0/16", 20);
    workflow_neg_7<subnet_feeder>(feeder1, feeder1A, feeder2);
}

/// \brief Subnet WF_N_8
TEST_F(subnet, subnet_workflow_neg_8) {
    pds_subnet_key_t key1 = {.id = 10}, key2 = {.id = 40};
    subnet_feeder feeder1, feeder2;

    feeder1.init(key1, k_vpc_key, "10.0.0.0/16", 20);
    feeder2.init(key2, k_vpc_key, "11.0.0.0/16", 20);
    workflow_neg_8<subnet_feeder>(feeder1, feeder2);
}

/// @}

}    // namespace api_test

//----------------------------------------------------------------------------
// Entry point
//----------------------------------------------------------------------------

int
main (int argc, char **argv)
{
    api_test_program_run(argc, argv);
}
