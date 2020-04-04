//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This file contains the all svc mapping test cases
///
//----------------------------------------------------------------------------

#include "nic/apollo/test/api/utils/svc_mapping.hpp"
#include "nic/apollo/test/api/utils/vpc.hpp"
#include "nic/apollo/test/api/utils/workflow.hpp"

namespace test {
namespace api {

const uint32_t g_num_svc_map = 10;
const uint32_t g_max_svc_map = 1024;
/// \cond
//----------------------------------------------------------------------------
// Service mapping test class
//----------------------------------------------------------------------------

class svc_mapping_test : public ::pds_test_base {
protected:
    svc_mapping_test() {}
    virtual ~svc_mapping_test() {}
    virtual void SetUp() {}
    virtual void TearDown() {}
    static void SetUpTestCase() {
        if (!agent_mode())
            pds_test_base::SetUpTestCase(g_tc_params);
        pds_batch_ctxt_t bctxt = batch_start();
        sample1_vpc_setup(bctxt, PDS_VPC_TYPE_TENANT);
        batch_commit(bctxt);
    }
    static void TearDownTestCase() {
        pds_batch_ctxt_t bctxt = batch_start();
        sample1_vpc_teardown(bctxt, PDS_VPC_TYPE_TENANT);
        batch_commit(bctxt);
        if (!agent_mode())
            pds_test_base::TearDownTestCase();
    }
};

//----------------------------------------------------------------------------
// Service mapping test cases implementation
//----------------------------------------------------------------------------

/// \defgroup SVC_MAPPING_TEST Service Mapping Tests
/// @{

/// \brief Service mapping WF_B1
/// \ref WF_B1
TEST_F(svc_mapping_test, svc_mapping_workflow_b1) {
    if (!apulu()) return;

    svc_mapping_feeder feeder;
    pds_obj_key_t key = int2pdsobjkey(1);
    feeder.init(key, "10.1.1.1", 10, int2pdsobjkey(2), "20.1.1.1", 20,
                "30.1.1.1", 1);
    workflow_b1<svc_mapping_feeder>(feeder);
}

/// \brief Service mapping WF_B2
/// \ref WF_B2
TEST_F(svc_mapping_test, svc_mapping_workflow_b2) {
    if (!apulu()) return;

    svc_mapping_feeder feeder1, feeder1A;
    pds_obj_key_t key = int2pdsobjkey(1);

    feeder1.init(key, "10.1.1.1", 10, int2pdsobjkey(2), "20.1.1.1", 20,
                 "30.1.1.1", 1);
    feeder1A.init(key, "10.1.1.2", 10, int2pdsobjkey(2), "20.1.1.1", 20,
                  "30.1.1.2", 1);
    workflow_b2<svc_mapping_feeder>(feeder1, feeder1A);
}

/// \brief Service mapping WF_1
/// \ref WF_1
TEST_F(svc_mapping_test, svc_mapping_workflow_1) {
    if (!apulu()) return;

    svc_mapping_feeder feeder;
    pds_obj_key_t key = int2pdsobjkey(1);

    feeder.init(key, "10.1.1.1", 10, int2pdsobjkey(2), "20.1.1.1", 20,
                "30.1.1.1", g_num_svc_map);
    workflow_1<svc_mapping_feeder>(feeder);
}

/// \brief Service mapping WF_2
/// \ref WF_2
TEST_F(svc_mapping_test, svc_mapping_workflow_2) {
    if (!apulu()) return;

    svc_mapping_feeder feeder;
    pds_obj_key_t key = int2pdsobjkey(1);

    feeder.init(key, "10.1.1.1", 10, int2pdsobjkey(2), "20.1.1.1", 20,
                "30.1.1.1", g_max_svc_map);
    workflow_2<svc_mapping_feeder>(feeder);
}

/// \brief Service mapping WF_3
/// \ref WF_3
TEST_F(svc_mapping_test, svc_mapping_workflow_3) {
    if (!apulu()) return;

    svc_mapping_feeder feeder1, feeder2, feeder3;
    pds_obj_key_t key1 = int2pdsobjkey(10);
    pds_obj_key_t key2 = int2pdsobjkey(40);
    pds_obj_key_t key3 = int2pdsobjkey(70);


    feeder1.init(key1, "10.1.1.1", 10, int2pdsobjkey(2), "20.1.1.1", 20,
                 "30.1.1.1", g_num_svc_map);
    feeder2.init(key2, "10.2.1.1", 10, int2pdsobjkey(3), "20.2.1.1", 20,
                 "30.2.1.1", g_num_svc_map);
    feeder3.init(key3, "10.3.1.1", 10, int2pdsobjkey(4), "20.3.1.1", 20,
                 "30.3.1.1", g_num_svc_map);
    workflow_3<svc_mapping_feeder>(feeder1, feeder2, feeder3);
}

/// \brief Service mapping WF_4
/// \ref WF_4
TEST_F(svc_mapping_test, svc_mapping_workflow_4) {
    if (!apulu()) return;

    svc_mapping_feeder feeder;
    pds_obj_key_t key = int2pdsobjkey(1);

    feeder.init(key, "10.1.1.1", 10, int2pdsobjkey(2), "20.1.1.1", 20,
                "30.1.1.1", g_max_svc_map);
    workflow_4<svc_mapping_feeder>(feeder);
}

/// \brief Service mapping WF_5
/// \ref WF_5
TEST_F(svc_mapping_test, svc_mapping_workflow_5) {
    if (!apulu()) return;

    svc_mapping_feeder feeder1, feeder2, feeder3;
    pds_obj_key_t key1 = int2pdsobjkey(10);
    pds_obj_key_t key2 = int2pdsobjkey(40);
    pds_obj_key_t key3 = int2pdsobjkey(70);

    feeder1.init(key1, "10.1.1.1", 10, int2pdsobjkey(2), "20.1.1.1", 20,
                 "30.1.1.1", g_num_svc_map);
    feeder2.init(key2, "10.2.1.1", 10, int2pdsobjkey(3), "20.2.1.1", 20,
                 "30.2.1.1", g_num_svc_map);
    feeder3.init(key3, "10.3.1.1", 10, int2pdsobjkey(4), "20.3.1.1", 20,
                 "30.3.1.1", g_num_svc_map);
    workflow_5<svc_mapping_feeder>(feeder1, feeder2, feeder3);
}

/// \brief Service mapping WF_6
/// \ref WF_6
TEST_F(svc_mapping_test, svc_mapping_workflow_6) {
    if (!apulu()) return;

    svc_mapping_feeder feeder1, feeder1A, feeder1B;
    pds_obj_key_t key = int2pdsobjkey(1);

    feeder1.init(key, "10.1.1.1", 10, int2pdsobjkey(2), "20.1.1.1", 20,
                 "30.1.1.1", g_max_svc_map);
    feeder1A.init(key, "10.2.1.1", 10, int2pdsobjkey(2), "20.1.1.1", 20,
                  "30.1.1.1", g_max_svc_map);
    feeder1B.init(key, "10.3.1.1", 5000, int2pdsobjkey(2), "20.1.1.1",
                  20, "30.1.1.1",
                  g_max_svc_map);
    workflow_6<svc_mapping_feeder>(feeder1, feeder1A, feeder1B);
}

/// \brief Service mapping WF_7
/// \ref WF_7
TEST_F(svc_mapping_test, svc_mapping_workflow7) {
    if (!apulu()) return;

    svc_mapping_feeder feeder1, feeder1A, feeder1B;
    pds_obj_key_t key = int2pdsobjkey(1);

    feeder1.init(key, "10.1.1.1", 10, int2pdsobjkey(2), "20.1.1.1", 20,
                 "30.1.1.1", g_max_svc_map);
    feeder1A.init(key, "10.2.1.1", 10, int2pdsobjkey(2), "20.1.1.1", 20,
                  "30.1.1.1", g_max_svc_map);
    feeder1B.init(key, "10.3.1.1", 5000, int2pdsobjkey(2), "20.1.1.1", 20,
                  "30.1.1.1", g_max_svc_map);
    workflow_7<svc_mapping_feeder>(feeder1, feeder1A, feeder1B);
}

/// \brief Service mapping WF_8
/// \ref WF_8
TEST_F(svc_mapping_test, svc_mapping_workflow8) {
    if (!apulu()) return;

    svc_mapping_feeder feeder1, feeder1A, feeder1B;
    pds_obj_key_t key = int2pdsobjkey(1);

    feeder1.init(key, "10.1.1.1", 10, int2pdsobjkey(2), "20.1.1.1", 20,
                 "30.1.1.1", g_max_svc_map);
    feeder1A.init(key, "10.2.1.1", 10, int2pdsobjkey(2), "20.1.1.1", 20,
                  "30.1.1.1", g_max_svc_map);
    feeder1B.init(key, "10.3.1.1", 5000, int2pdsobjkey(2), "20.1.1.1", 20,
                  "30.1.1.1", g_max_svc_map);
    workflow_8<svc_mapping_feeder>(feeder1, feeder1A, feeder1B);
}

/// \brief Service mapping WF_9
/// \ref WF_9
TEST_F(svc_mapping_test, svc_mapping_workflow9) {
    if (!apulu()) return;

    svc_mapping_feeder feeder1, feeder1A;
    pds_obj_key_t key = int2pdsobjkey(1);

    feeder1.init(key, "10.1.1.1", 10, int2pdsobjkey(2), "20.1.1.1", 20,
                 "30.1.1.1", g_max_svc_map);
    feeder1A.init(key, "10.3.1.1", 5000, int2pdsobjkey(2), "20.1.1.1", 20,
                  "30.1.1.1", g_max_svc_map);
    workflow_9<svc_mapping_feeder>(feeder1, feeder1A);
}

/// \brief Service mapping WF_10
/// \ref WF_10
TEST_F(svc_mapping_test, DISABLED_svc_mapping_workflow10) {
    if (!apulu()) return;

    svc_mapping_feeder feeder1, feeder2, feeder2A, feeder3, feeder3A, feeder4;
    pds_obj_key_t key1 = int2pdsobjkey(10), key2 = int2pdsobjkey(40);
    pds_obj_key_t key3 = int2pdsobjkey(70), key4 = int2pdsobjkey(100);

    feeder1.init(key1, "10.1.1.1", 10, int2pdsobjkey(2), "20.1.1.1", 20,
                 "30.1.1.1", g_num_svc_map);
    feeder2.init(key2, "10.2.1.1", 10, int2pdsobjkey(3), "20.2.1.1", 20,
                 "30.2.1.1", g_num_svc_map);
    feeder2A.init(key2, "10.2.1.1", 10, int2pdsobjkey(3), "20.2.20.1", 2020,
                  "30.2.30.1", g_num_svc_map);
    feeder3.init(key3, "10.3.1.1", 10, int2pdsobjkey(4), "20.3.1.1", 20,
                 "30.3.1.1", g_num_svc_map);
    feeder3A.init(key3, "10.3.1.1", 10, int2pdsobjkey(4), "20.3.30.1", 3030,
                  "30.3.30.1", g_num_svc_map);
    feeder4.init(key4, "10.4.1.1", 10, int2pdsobjkey(5), "20.4.1.1", 20,
                 "30.4.1.1", g_num_svc_map);
    workflow_10<svc_mapping_feeder>(feeder1, feeder2, feeder2A,
                            feeder3, feeder3A, feeder4);
}

/// \brief Service mapping WF_N_1
/// \ref WF_N_1
TEST_F(svc_mapping_test, svc_mapping_workflow_neg_1) {
    if (!apulu()) return;

    svc_mapping_feeder feeder;
    pds_obj_key_t key = int2pdsobjkey(1);

    feeder.init(key, "10.1.1.1", 10, int2pdsobjkey(2), "20.1.1.1", 20,
                "30.1.1.1", g_max_svc_map);
    workflow_neg_1<svc_mapping_feeder>(feeder);
}

/// \brief Service mapping WF_N_2
/// \ref WF_N_2
TEST_F(svc_mapping_test, DISABLED_svc_mapping_workflow_neg_2) {
    if (!apulu()) return;

    svc_mapping_feeder feeder;
    pds_obj_key_t key = int2pdsobjkey(1);

    feeder.init(key, "10.1.1.1", 10, int2pdsobjkey(2), "20.1.1.1", 20, "30.1.1.1",
                g_max_svc_map + 1);
    workflow_neg_2<svc_mapping_feeder>(feeder);
}

/// \brief Service mapping WF_N_3
/// \ref WF_N_3
TEST_F(svc_mapping_test, svc_mapping_workflow_neg_3) {
    if (!apulu()) return;

    svc_mapping_feeder feeder;
    pds_obj_key_t key = int2pdsobjkey(1);

    feeder.init(key, "10.1.1.1", 10, int2pdsobjkey(2), "20.1.1.1", 20,
                "30.1.1.1", g_num_svc_map);
    workflow_neg_3<svc_mapping_feeder>(feeder);
}

/// \brief Service mapping WF_N_4
/// \ref WF_N_4
TEST_F(svc_mapping_test, svc_mapping_workflow_neg_4) {
    if (!apulu()) return;

    svc_mapping_feeder feeder1, feeder2;
    pds_obj_key_t key1 = int2pdsobjkey(10), key2 = int2pdsobjkey(40);

    feeder1.init(key1, "10.1.1.1", 10, int2pdsobjkey(2), "20.1.1.1", 20,
                 "30.1.1.1", g_num_svc_map);
    feeder2.init(key2, "10.2.1.1", 10, int2pdsobjkey(3), "20.2.1.1", 20,
                 "30.2.1.1", g_num_svc_map);
    workflow_neg_4<svc_mapping_feeder>(feeder1, feeder2);
}

/// \brief Service mapping WF_N_5
/// \ref WF_N_5
TEST_F(svc_mapping_test, svc_mapping_workflow_neg_5) {
    svc_mapping_feeder feeder1, feeder1A;
    pds_obj_key_t key = int2pdsobjkey(1);

    feeder1.init(key, "10.1.1.1", 10, int2pdsobjkey(2), "20.1.1.1", 20,
                 "30.1.1.1", g_max_svc_map);
    feeder1A.init(key, "10.3.1.1", 5000, int2pdsobjkey(2), "20.1.1.1", 20,
                  "30.1.1.1", g_max_svc_map);

    workflow_neg_5<svc_mapping_feeder>(feeder1, feeder1A);
}

/// \brief Service mapping WF_N_6
/// \ref WF_N_6
TEST_F(svc_mapping_test, DISABLED_svc_mapping_workflow_neg_6) {
    svc_mapping_feeder feeder1, feeder1A;

    workflow_neg_6<svc_mapping_feeder>(feeder1, feeder1A);
}

/// \brief Service mapping WF_N_7
/// \ref WF_N_7
TEST_F(svc_mapping_test, DISABLED_svc_mapping_workflow_neg_7) {
    svc_mapping_feeder feeder1, feeder1A, feeder2;
    pds_obj_key_t key1 = int2pdsobjkey(10), key2 = int2pdsobjkey(40);

    feeder1.init(key1, "10.1.1.1", 10, int2pdsobjkey(2), "20.1.1.1", 20,
                 "30.1.1.1", g_max_svc_map);
    feeder1A.init(key1, "10.2.1.1", 10, int2pdsobjkey(2), "20.1.1.1", 20,
                  "30.1.1.1", g_max_svc_map);
    feeder2.init(key2, "10.2.1.1", 10, int2pdsobjkey(3), "20.2.1.1", 20,
                 "30.2.1.1", g_num_svc_map);

    workflow_neg_7<svc_mapping_feeder>(feeder1, feeder1A, feeder2);
}

/// \brief Service mapping WF_N_8
/// \ref WF_N_8
TEST_F(svc_mapping_test, svc_mapping_workflow_neg_8) {
    svc_mapping_feeder feeder1, feeder2;
    pds_obj_key_t key1 = int2pdsobjkey(10), key2 = int2pdsobjkey(40);

    feeder1.init(key1, "10.1.1.1", 10, int2pdsobjkey(2), "20.1.1.1", 20,
                 "30.1.1.1", g_max_svc_map);
    feeder2.init(key2, "10.2.1.1", 10, int2pdsobjkey(3), "20.2.1.1", 20,
                 "30.2.1.1", g_num_svc_map);


    workflow_neg_8<svc_mapping_feeder>(feeder1, feeder2);
}

/// @}

}    // namespace api
}    // namespace test

//----------------------------------------------------------------------------
// Entry point
//----------------------------------------------------------------------------

int
main (int argc, char **argv)
{
    return api_test_program_run(argc, argv);
}
