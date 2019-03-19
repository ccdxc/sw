//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This file contains the all route table test cases
///
//----------------------------------------------------------------------------
#include <getopt.h>
#include "nic/apollo/api/include/pds_batch.hpp"
#include "nic/apollo/test/utils/base.hpp"
#include "nic/apollo/test/utils/tep.hpp"
#include "nic/apollo/test/utils/route.hpp"

using std::cerr;
using std::cout;
using std::endl;

namespace api_test {

// Globals
char *g_cfg_file = NULL;
static pds_epoch_t g_batch_epoch = PDS_EPOCH_INVALID;
const char *k_first_nh_ip_str = "30.30.30.1";
const char *k_first_v4_pfx_str = "100.100.100.1/21";
const char *k_first_v6_pfx_str = "100:100:100:1::1/64";

//----------------------------------------------------------------------------
// Route table test class
//----------------------------------------------------------------------------

class route_test : public pds_test_base {
protected:
    route_test() {}
    virtual ~route_test() {}
    virtual void SetUp() {}
    virtual void TearDown() {}
    static void SetUpTestCase() {
        pds_test_base::SetUpTestCase(api_test::g_cfg_file, false);
    }
};

//----------------------------------------------------------------------------
// Route table test cases implementation
//----------------------------------------------------------------------------

/// \defgroup Route table
/// @{

/// \brief Setup testcase
/// Does initial setup for route table testcases
TEST_F(route_test, initial_setup) {
    pds_batch_params_t batch_params = {0};

    batch_params.epoch = ++api_test::g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    // create max TEPs which can be used as NHs for routes
    ASSERT_TRUE(tep_util::many_create(PDS_MAX_TEP-1, k_first_nh_ip_str,
        PDS_TEP_ENCAP_TYPE_VNIC) == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);
    // TODO: Because of delayed delete,
    // immediate TEP creation in the successive test case is not possible.
    // once that is supported, move this to route_test::SetUp()
}

/// \brief Create & Delete single V4 route table in separate batches
/// Create a v4 route table with ten routes and then delete the same
TEST_F(route_test, v4_route_table_create_delete) {
    pds_batch_params_t batch_params = {0};
    pds_route_table_info_t info;
    pds_route_table_id_t rt_table_id = 1;
    uint32_t num_routes = 10;
    route_table_util rt_obj(rt_table_id, "100.100.0.0/16",
                            k_first_nh_ip_str, IP_AF_IPV4, num_routes);

    batch_params.epoch = ++api_test::g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(rt_obj.create() == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);

    // TODO: read API is NOT available yet.
    // Add read cases for all TCs
    //ASSERT_TRUE(rt_obj->read(&info) == sdk::SDK_RET_OK);

    batch_params.epoch = ++api_test::g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(rt_obj.del() == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);

    //ASSERT_TRUE(rt_obj->read(&info) == sdk::SDK_RET_ENTRY_NOT_FOUND);
}

/// \brief Create & Delete single V6 route table in separate batches
/// Create a v6 route table with ten routes and then delete the same
TEST_F(route_test, v6_route_table_create_delete) {
    pds_batch_params_t batch_params = {0};
    pds_route_table_id_t rt_table_id = 2;
    uint32_t num_routes = 10;
    route_table_util rt_obj(rt_table_id, k_first_v6_pfx_str,
                            k_first_nh_ip_str, IP_AF_IPV6, num_routes);

    batch_params.epoch = ++api_test::g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(rt_obj.create() == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);

    batch_params.epoch = ++api_test::g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(rt_obj.del() == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);
}

/// \brief Create & Delete max V4-V6 route tables
/// Create max v4 & v6 route tables with max routes and then delete the same
TEST_F(route_test, DISABLED_v4_v6_route_table_multi_create_delete) {
    pds_batch_params_t batch_params = {0};
    pds_route_table_id_t first_v4_rt_table_id = 1;
    pds_route_table_id_t first_v6_rt_table_id = 1000;
    uint32_t num_v4_route_tables = PDS_MAX_ROUTE_TABLE / 2;
    uint32_t num_v6_route_tables = PDS_MAX_ROUTE_TABLE - num_v4_route_tables;
    uint32_t num_routes = PDS_MAX_ROUTE_PER_TABLE;

    batch_params.epoch = ++api_test::g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(route_table_util::many_create(num_v4_route_tables,
        first_v4_rt_table_id, k_first_v4_pfx_str, k_first_nh_ip_str,
        IP_AF_IPV4, num_routes) == sdk::SDK_RET_OK);
    ASSERT_TRUE(route_table_util::many_create(num_v6_route_tables,
        first_v6_rt_table_id, k_first_v6_pfx_str, k_first_nh_ip_str,
        IP_AF_IPV6, num_routes) == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);

    batch_params.epoch = ++api_test::g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(route_table_util::many_delete(num_v4_route_tables,
        first_v4_rt_table_id) == sdk::SDK_RET_OK);
    ASSERT_TRUE(route_table_util::many_delete(num_v6_route_tables,
        first_v6_rt_table_id) == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);
}

/// \brief Create and delete max v4 route tables in the same batch
/// The operation should be de-duped by framework and effectively
/// a NO-OP from hardware perspective
TEST_F(route_test, DISABLED_v4_route_table_workflow_1) {
    pds_batch_params_t batch_params = {0};
    pds_route_table_id_t first_rt_table_id = 1;
    uint32_t num_route_tables = PDS_MAX_ROUTE_TABLE;
    uint32_t num_routes = PDS_MAX_ROUTE_PER_TABLE;

    batch_params.epoch = ++api_test::g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(route_table_util::many_create(num_route_tables,
        first_rt_table_id, k_first_v4_pfx_str, k_first_nh_ip_str,
        IP_AF_IPV4, num_routes) == sdk::SDK_RET_OK);
    ASSERT_TRUE(route_table_util::many_delete(num_route_tables,
        first_rt_table_id) == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);
}

/// \brief Create, delete and create max v4 route tables in the same batch
/// The operation should program and unprogram route tables in hardware
/// and return success after final create
TEST_F(route_test, DISABLED_v4_route_table_workflow_2) {
    pds_batch_params_t batch_params = {0};
    pds_route_table_id_t first_rt_table_id = 1;
    uint32_t num_route_tables = PDS_MAX_ROUTE_TABLE;
    uint32_t num_routes = PDS_MAX_ROUTE_PER_TABLE;

    batch_params.epoch = ++api_test::g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(route_table_util::many_create(num_route_tables,
        first_rt_table_id, k_first_v4_pfx_str, k_first_nh_ip_str,
        IP_AF_IPV4, num_routes) == sdk::SDK_RET_OK);
    ASSERT_TRUE(route_table_util::many_delete(num_route_tables,
        first_rt_table_id) == sdk::SDK_RET_OK);
    ASSERT_TRUE(route_table_util::many_create(num_route_tables,
        first_rt_table_id, k_first_v4_pfx_str, k_first_nh_ip_str,
        IP_AF_IPV4, num_routes) == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);
}

/// \brief Create, delete some and create diff set of v4 route table in a batch
/// The operation should program final set of route tables in HW post de-dup
/// and return success after final create
TEST_F(route_test, DISABLED_v4_route_table_workflow_3) {
    pds_batch_params_t batch_params = {0};
    uint32_t num_routes = PDS_MAX_ROUTE_PER_TABLE, num_route_tables = 100;

    // [ Create Set1, Set2 - Delete Set1 - Create Set3 ] - Read
    batch_params.epoch = ++api_test::g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    // create set1 of 100 (101-200) v4 route tables
    ASSERT_TRUE(route_table_util::many_create(num_route_tables,
        101, k_first_v4_pfx_str, k_first_nh_ip_str,
        IP_AF_IPV4, num_routes) == sdk::SDK_RET_OK);
    // create set2 of 100 (201-300) v4 route tables
    ASSERT_TRUE(route_table_util::many_create(num_route_tables,
        201, "125.100.100.1/21", k_first_nh_ip_str,
        IP_AF_IPV4, num_routes) == sdk::SDK_RET_OK);
    // delete set1 of 100 (101-200) v4 route tables
    ASSERT_TRUE(route_table_util::many_delete(num_route_tables,
        101) == sdk::SDK_RET_OK);
    // create set3 of 100 (301-400) v4 route tables
    ASSERT_TRUE(route_table_util::many_create(num_route_tables,
        301, "150.100.100.1/21", k_first_nh_ip_str,
        IP_AF_IPV4, num_routes) == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);

    // teardown
    batch_params.epoch = ++api_test::g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    // delete set2 of 100 (201-300) v4 route tables
    ASSERT_TRUE(route_table_util::many_delete(num_route_tables,
        201) == sdk::SDK_RET_OK);
    // delete set3 of 100 (301-400) v4 route tables
    ASSERT_TRUE(route_table_util::many_delete(num_route_tables,
        301) == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);
}

/// \brief Create and delete max V4 route tables in different batches
/// The hardware should create route tables correctly and
///  return entry not found after delete
TEST_F(route_test, DISABLED_v4_route_table_workflow_4) {
    pds_batch_params_t batch_params = {0};
    pds_route_table_id_t first_rt_table_id = 1;
    uint32_t num_route_tables = PDS_MAX_ROUTE_TABLE;
    uint32_t num_routes = PDS_MAX_ROUTE_PER_TABLE;

    batch_params.epoch = ++api_test::g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(route_table_util::many_create(num_route_tables,
        first_rt_table_id, k_first_v4_pfx_str, k_first_nh_ip_str,
        IP_AF_IPV4, num_routes) == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);

    batch_params.epoch = ++api_test::g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(route_table_util::many_delete(num_route_tables,
        first_rt_table_id) == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);
}

/// \brief Create & delete different sets of v4 route table in separate batches
/// The hardware should create v4 route tables correctly and
/// return entry not found after delete for deleted ones
TEST_F(route_test, DISABLED_v4_route_table_workflow_5) {
    pds_batch_params_t batch_params = {0};
    uint32_t num_routes = PDS_MAX_ROUTE_PER_TABLE, num_route_tables = 100;

    // [ Create Set1, Set2 ] - Read - [Delete Set1 - Create Set3 ] - Read

    batch_params.epoch = ++api_test::g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    // create set1 of 100 (101-200) v4 route tables
    ASSERT_TRUE(route_table_util::many_create(num_route_tables,
        101, k_first_v4_pfx_str, k_first_nh_ip_str,
        IP_AF_IPV4, num_routes) == sdk::SDK_RET_OK);
    // create set2 of 100 (201-300) v4 route tables
    ASSERT_TRUE(route_table_util::many_create(num_route_tables,
        201, "125.100.100.1/21", k_first_nh_ip_str,
        IP_AF_IPV4, num_routes) == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);

    batch_params.epoch = ++api_test::g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    // delete set1 of 100 (101-200) v4 route tables
    ASSERT_TRUE(route_table_util::many_delete(num_route_tables,
        101) == sdk::SDK_RET_OK);
    // create set3 of 100 (301-400) v4 route tables
    ASSERT_TRUE(route_table_util::many_create(num_route_tables,
        301, "150.100.100.1/21", k_first_nh_ip_str,
        IP_AF_IPV4, num_routes) == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);

    // teardown
    batch_params.epoch = ++api_test::g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    // delete set2 of 100 (201-300) v4 route tables
    ASSERT_TRUE(route_table_util::many_delete(num_route_tables,
        201) == sdk::SDK_RET_OK);
    // delete set3 of 100 (301-400) v4 route tables
    ASSERT_TRUE(route_table_util::many_delete(num_route_tables,
        301) == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);
}

/// \brief Create and recreate max V4 route tables in two batches
/// The hardware should program device correctly in case of
/// first create and return error in second create operation
TEST_F(route_test, DISABLED_v4_route_table_workflow_neg_1) {
    pds_batch_params_t batch_params = {0};
    pds_route_table_id_t first_rt_table_id = 1;
    uint32_t num_route_tables = PDS_MAX_ROUTE_TABLE;
    uint32_t num_routes = PDS_MAX_ROUTE_PER_TABLE;

    // [ Create SetMax ] - [ Create SetMax ] - Read

    batch_params.epoch = ++api_test::g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(route_table_util::many_create(num_route_tables,
        first_rt_table_id, k_first_v4_pfx_str, k_first_nh_ip_str,
        IP_AF_IPV4, num_routes) == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);

    batch_params.epoch = ++api_test::g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(route_table_util::many_create(num_route_tables,
        first_rt_table_id, k_first_v4_pfx_str, k_first_nh_ip_str,
        IP_AF_IPV4, num_routes) == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_INVALID_OP);
    ASSERT_TRUE(pds_batch_abort() == sdk::SDK_RET_OK);

    // teardown
    batch_params.epoch = ++api_test::g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(route_table_util::many_delete(num_route_tables,
        first_rt_table_id) == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);
}

/// \brief Create more than max V4 route tables
TEST_F(route_test, DISABLED_v4_route_table_workflow_neg_2) {
    pds_batch_params_t batch_params = {0};
    pds_route_table_id_t first_rt_table_id = 1;
    // TODO: fails only for PDS_MAX_ROUTE_TABLE + 2
    // but expectation is PDS_MAX_ROUTE_TABLE + 1
    uint32_t num_route_tables = PDS_MAX_ROUTE_TABLE + 1;
    uint32_t num_routes = PDS_MAX_ROUTE_PER_TABLE;

    // [ Create SetMax+1 ] - Read

    batch_params.epoch = ++api_test::g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(route_table_util::many_create(num_route_tables,
        first_rt_table_id, k_first_v4_pfx_str, k_first_nh_ip_str,
        IP_AF_IPV4, num_routes) == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_NO_RESOURCE);
    ASSERT_TRUE(pds_batch_abort() == sdk::SDK_RET_OK);
}

/// \brief Create more than max V4 routes for max v4 route tables
TEST_F(route_test, DISABLED_v4_route_table_workflow_neg_2a) {
    pds_batch_params_t batch_params = {0};
    pds_route_table_id_t first_rt_table_id = 1;
    uint32_t num_route_tables = PDS_MAX_ROUTE_TABLE;
    // TODO: PDS_MAX_ROUTE_PER_TABLE + 1 will crash on lpm creation
    // so keeping this testcase in disabled state
    uint32_t num_routes = PDS_MAX_ROUTE_PER_TABLE + 1;

    // [ Create SetMax (each with max+1 route scale) ] - Read

    batch_params.epoch = ++api_test::g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(route_table_util::many_create(num_route_tables,
        first_rt_table_id, k_first_v4_pfx_str, k_first_nh_ip_str,
        IP_AF_IPV4, num_routes) == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_NO_RESOURCE);
    ASSERT_TRUE(pds_batch_abort() == sdk::SDK_RET_OK);
}

/// \brief Create 0 routes for each of max v4 route tables
TEST_F(route_test, v4_route_table_workflow_neg_2b) {
    pds_batch_params_t batch_params = {0};
    pds_route_table_id_t first_rt_table_id = 77777;
    uint32_t num_route_tables = PDS_MAX_ROUTE_TABLE;
    uint32_t num_routes = 0;

    // [ Create SetMax (each with 0 route scale) ] - Read

    batch_params.epoch = ++api_test::g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(route_table_util::many_create(num_route_tables,
        first_rt_table_id, k_first_v4_pfx_str, k_first_nh_ip_str,
        IP_AF_IPV4, num_routes) == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);
}

/// \brief Read and delete a non existing v4 route table
/// The hardware should return entry not found
TEST_F(route_test, v4_route_table_workflow_neg_3) {
    pds_batch_params_t batch_params = {0};
    pds_route_table_info_t info;
    pds_route_table_id_t rt_table_id = 133333;
    uint32_t num_routes = 10;
    route_table_util rt_obj(rt_table_id, "100.100.0.0/16",
                            k_first_nh_ip_str, IP_AF_IPV4, num_routes);

    //ASSERT_TRUE(rt_obj->read(&info) == sdk::SDK_RET_ENTRY_NOT_FOUND);

    batch_params.epoch = ++api_test::g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(rt_obj.del() == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_ENTRY_NOT_FOUND);
    ASSERT_TRUE(pds_batch_abort() == sdk::SDK_RET_OK);
}

/// \brief Create and delete different V4 route tables in different batches
/// The hardware should create v4 route tables correctly and
/// delete batch (containing valid & invalid entries) is expected to fail
/// because of lookup failure.
TEST_F(route_test, DISABLED_v4_route_table_workflow_neg_4) {
    pds_batch_params_t batch_params = {0};
    pds_route_table_id_t first_rt_table_id = 144444;
    uint32_t num_route_tables = 10;
    uint32_t num_routes = 10;
    route_table_util rt_obj(100, k_first_v4_pfx_str,
                            k_first_nh_ip_str, IP_AF_IPV4, num_routes);

    batch_params.epoch = ++api_test::g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(route_table_util::many_create(num_route_tables,
        first_rt_table_id, k_first_v4_pfx_str, k_first_nh_ip_str,
        IP_AF_IPV4, num_routes) == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);

    batch_params.epoch = ++api_test::g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(route_table_util::many_delete(num_route_tables,
        first_rt_table_id) == sdk::SDK_RET_OK);
    // TODO: rollback is failing here and causing a crash
    // so keeping this TC in DISABLED state.
    ASSERT_TRUE(rt_obj.del() == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_ENTRY_NOT_FOUND);
    ASSERT_TRUE(pds_batch_abort() == sdk::SDK_RET_OK);

    // teardown
    batch_params.epoch = ++api_test::g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(route_table_util::many_delete(num_route_tables,
        first_rt_table_id) == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);
}

/// \brief Create and delete max v6 route tables in the same batch
/// The operation should be de-duped by framework and effectively
/// a NO-OP from hardware perspective
TEST_F(route_test, DISABLED_v6_route_table_workflow_1) {
    pds_batch_params_t batch_params = {0};
    pds_route_table_id_t first_rt_table_id = 1;
    uint32_t num_route_tables = PDS_MAX_ROUTE_TABLE;
    uint32_t num_routes = PDS_MAX_ROUTE_PER_TABLE;

    batch_params.epoch = ++api_test::g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(route_table_util::many_create(num_route_tables,
        first_rt_table_id, k_first_v6_pfx_str, k_first_nh_ip_str,
        IP_AF_IPV6, num_routes) == sdk::SDK_RET_OK);
    ASSERT_TRUE(route_table_util::many_delete(num_route_tables,
        first_rt_table_id) == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);
}

/// \brief Create, delete and create max v6 route tables in the same batch
/// The operation should program and unprogram route tables in hardware
/// and return success after final create
TEST_F(route_test, DISABLED_v6_route_table_workflow_2) {
    pds_batch_params_t batch_params = {0};
    pds_route_table_id_t first_rt_table_id = 1;
    uint32_t num_route_tables = PDS_MAX_ROUTE_TABLE;
    uint32_t num_routes = PDS_MAX_ROUTE_PER_TABLE;

    batch_params.epoch = ++api_test::g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(route_table_util::many_create(num_route_tables,
        first_rt_table_id, k_first_v6_pfx_str, k_first_nh_ip_str,
        IP_AF_IPV6, num_routes) == sdk::SDK_RET_OK);
    ASSERT_TRUE(route_table_util::many_delete(num_route_tables,
        first_rt_table_id) == sdk::SDK_RET_OK);
    ASSERT_TRUE(route_table_util::many_create(num_route_tables,
        first_rt_table_id, k_first_v6_pfx_str, k_first_nh_ip_str,
        IP_AF_IPV6, num_routes) == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);
}

/// \brief Create, delete some and create diff set of v6 route table in a batch
/// The operation should program final set of route tables in HW post de-dup
/// and return success after final create
TEST_F(route_test, DISABLED_v6_route_table_workflow_3) {
    pds_batch_params_t batch_params = {0};
    uint32_t num_routes = PDS_MAX_ROUTE_PER_TABLE, num_route_tables = 100;

    // [ Create Set1, Set2 - Delete Set1 - Create Set3 ] - Read
    batch_params.epoch = ++api_test::g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    // create set1 of 100 (101-200) v6 route tables
    ASSERT_TRUE(route_table_util::many_create(num_route_tables,
        101, k_first_v6_pfx_str, k_first_nh_ip_str,
        IP_AF_IPV6, num_routes) == sdk::SDK_RET_OK);
    // create set2 of 100 (201-300) v6 route tables
    ASSERT_TRUE(route_table_util::many_create(num_route_tables,
        201, "125:100:100:1::0/64", k_first_nh_ip_str,
        IP_AF_IPV6, num_routes) == sdk::SDK_RET_OK);
    // delete set1 of 100 (101-200) v6 route tables
    ASSERT_TRUE(route_table_util::many_delete(num_route_tables,
        101) == sdk::SDK_RET_OK);
    // create set3 of 100 (301-400) v6 route tables
    ASSERT_TRUE(route_table_util::many_create(num_route_tables,
        301, "150:100:100:1::0/64", k_first_nh_ip_str,
        IP_AF_IPV6, num_routes) == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);

    // teardown
    batch_params.epoch = ++api_test::g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    // delete set2 of 100 (201-300) v6 route tables
    ASSERT_TRUE(route_table_util::many_delete(num_route_tables,
        201) == sdk::SDK_RET_OK);
    // delete set3 of 100 (301-400) v6 route tables
    ASSERT_TRUE(route_table_util::many_delete(num_route_tables,
        301) == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);
}

/// \brief Create and delete max v6 route tables in different batches
/// The hardware should create route tables correctly and
///  return entry not found after delete
TEST_F(route_test, DISABLED_v6_route_table_workflow_4) {
    pds_batch_params_t batch_params = {0};
    pds_route_table_id_t first_rt_table_id = 1;
    uint32_t num_route_tables = PDS_MAX_ROUTE_TABLE;
    uint32_t num_routes = PDS_MAX_ROUTE_PER_TABLE;

    batch_params.epoch = ++api_test::g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(route_table_util::many_create(num_route_tables,
        first_rt_table_id, k_first_v6_pfx_str, k_first_nh_ip_str,
        IP_AF_IPV6, num_routes) == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);

    batch_params.epoch = ++api_test::g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(route_table_util::many_delete(num_route_tables,
        first_rt_table_id) == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);
}

/// \brief Create & delete different sets of v6 route table in separate batches
/// The hardware should create v6 route tables correctly and
/// return entry not found after delete for deleted ones
TEST_F(route_test, DISABLED_v6_route_table_workflow_5) {
    pds_batch_params_t batch_params = {0};
    uint32_t num_routes = PDS_MAX_ROUTE_PER_TABLE, num_route_tables = 100;

    // [ Create Set1, Set2 ] - Read - [Delete Set1 - Create Set3 ] - Read

    batch_params.epoch = ++api_test::g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    // create set1 of 100 (101-200) v6 route tables
    ASSERT_TRUE(route_table_util::many_create(num_route_tables,
        101, k_first_v6_pfx_str, k_first_nh_ip_str,
        IP_AF_IPV6, num_routes) == sdk::SDK_RET_OK);
    // create set2 of 100 (201-300) v6 route tables
    ASSERT_TRUE(route_table_util::many_create(num_route_tables,
        201, "125:100:100:1::0/64", k_first_nh_ip_str,
        IP_AF_IPV6, num_routes) == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);

    batch_params.epoch = ++api_test::g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    // delete set1 of 100 (101-200) v6 route tables
    ASSERT_TRUE(route_table_util::many_delete(num_route_tables,
        101) == sdk::SDK_RET_OK);
    // create set3 of 100 (301-400) v6 route tables
    ASSERT_TRUE(route_table_util::many_create(num_route_tables,
        301, "150:100:100:1::0/64", k_first_nh_ip_str,
        IP_AF_IPV6, num_routes) == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);

    // teardown
    batch_params.epoch = ++api_test::g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    // delete set2 of 100 (201-300) v6 route tables
    ASSERT_TRUE(route_table_util::many_delete(num_route_tables,
        201) == sdk::SDK_RET_OK);
    // delete set3 of 100 (301-400) v6 route tables
    ASSERT_TRUE(route_table_util::many_delete(num_route_tables,
        301) == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);
}

/// \brief Create and recreate max v6 route tables in two batches
/// The hardware should program device correctly in case of
/// first create and return error in second create operation
TEST_F(route_test, DISABLED_v6_route_table_workflow_neg_1) {
    pds_batch_params_t batch_params = {0};
    pds_route_table_id_t first_rt_table_id = 1;
    uint32_t num_route_tables = PDS_MAX_ROUTE_TABLE;
    uint32_t num_routes = PDS_MAX_ROUTE_PER_TABLE;

    // [ Create SetMax ] - [ Create SetMax ] - Read

    batch_params.epoch = ++api_test::g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(route_table_util::many_create(num_route_tables,
        first_rt_table_id, k_first_v6_pfx_str, k_first_nh_ip_str,
        IP_AF_IPV6, num_routes) == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);

    batch_params.epoch = ++api_test::g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(route_table_util::many_create(num_route_tables,
        first_rt_table_id, k_first_v6_pfx_str, k_first_nh_ip_str,
        IP_AF_IPV6, num_routes) == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_INVALID_OP);
    ASSERT_TRUE(pds_batch_abort() == sdk::SDK_RET_OK);

    // teardown
    batch_params.epoch = ++api_test::g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(route_table_util::many_delete(num_route_tables,
        first_rt_table_id) == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);
}

/// \brief Create more than max v6 route tables
TEST_F(route_test,  DISABLED_v6_route_table_workflow_neg_2) {
    pds_batch_params_t batch_params = {0};
    pds_route_table_id_t first_rt_table_id = 1;
    // TODO: fails only for PDS_MAX_ROUTE_TABLE + 2
    // but expectation is PDS_MAX_ROUTE_TABLE + 1
    uint32_t num_route_tables = PDS_MAX_ROUTE_TABLE + 1;
    uint32_t num_routes = PDS_MAX_ROUTE_PER_TABLE;

    // [ Create SetMax+1 ] - Read

    batch_params.epoch = ++api_test::g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(route_table_util::many_create(num_route_tables,
        first_rt_table_id, k_first_v6_pfx_str, k_first_nh_ip_str,
        IP_AF_IPV6, num_routes) == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_NO_RESOURCE);
    ASSERT_TRUE(pds_batch_abort() == sdk::SDK_RET_OK);
}

/// \brief Create more than max v6 routes for max v6 route tables
TEST_F(route_test, DISABLED_v6_route_table_workflow_neg_2a) {
    pds_batch_params_t batch_params = {0};
    pds_route_table_id_t first_rt_table_id = 1;
    uint32_t num_route_tables = PDS_MAX_ROUTE_TABLE;
    // TODO: PDS_MAX_ROUTE_PER_TABLE + 1 will crash on lpm creation
    // so keeping this testcase in disabled state
    uint32_t num_routes = PDS_MAX_ROUTE_PER_TABLE + 1;

    // [ Create SetMax (each with max+1 route scale) ] - Read

    batch_params.epoch = ++api_test::g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(route_table_util::many_create(num_route_tables,
        first_rt_table_id, k_first_v6_pfx_str, k_first_nh_ip_str,
        IP_AF_IPV6, num_routes) == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_NO_RESOURCE);
    ASSERT_TRUE(pds_batch_abort() == sdk::SDK_RET_OK);
}

/// \brief Create 0 routes for each of max v6 route tables
TEST_F(route_test, DISABLED_v6_route_table_workflow_neg_2b) {
    pds_batch_params_t batch_params = {0};
    pds_route_table_id_t first_rt_table_id = 88888;
    uint32_t num_route_tables = PDS_MAX_ROUTE_TABLE;
    uint32_t num_routes = 0;

    // [ Create SetMax (each with 0 route scale) ] - Read

    batch_params.epoch = ++api_test::g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(route_table_util::many_create(num_route_tables,
        first_rt_table_id, k_first_v6_pfx_str, k_first_nh_ip_str,
        IP_AF_IPV4, num_routes) == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);
}

/// \brief Read and delete a non existing v6 route table
/// The hardware should return entry not found
TEST_F(route_test, v6_route_table_workflow_neg_3) {
    pds_batch_params_t batch_params = {0};
    pds_route_table_info_t info;
    pds_route_table_id_t rt_table_id = 233333;
    uint32_t num_routes = 10;
    route_table_util rt_obj(rt_table_id, "99:99:99::99/64",
                            k_first_nh_ip_str, IP_AF_IPV6, num_routes);

    //ASSERT_TRUE(rt_obj->read(&info) == sdk::SDK_RET_ENTRY_NOT_FOUND);

    batch_params.epoch = ++api_test::g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(rt_obj.del() == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_ENTRY_NOT_FOUND);
    ASSERT_TRUE(pds_batch_abort() == sdk::SDK_RET_OK);
}

/// \brief Create and delete different v6 route tables in different batches
/// The hardware should create v6 route tables correctly and
/// delete batch (containing valid & invalid entries) is expected to fail
/// because of lookup failure.
TEST_F(route_test, DISABLED_v6_route_table_workflow_neg_4) {
    pds_batch_params_t batch_params = {0};
    pds_route_table_id_t first_rt_table_id = 244444;
    uint32_t num_route_tables = 10;
    uint32_t num_routes = 10;
    route_table_util rt_obj(100, k_first_v6_pfx_str,
                            k_first_nh_ip_str, IP_AF_IPV6, num_routes);

    batch_params.epoch = ++api_test::g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(route_table_util::many_create(num_route_tables,
        first_rt_table_id, k_first_v6_pfx_str, k_first_nh_ip_str,
        IP_AF_IPV6, num_routes) == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);

    batch_params.epoch = ++api_test::g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(route_table_util::many_delete(num_route_tables,
        first_rt_table_id) == sdk::SDK_RET_OK);
    // TODO: rollback is failing here and causing a crash
    // so keeping this TC in DISABLED state.
    ASSERT_TRUE(rt_obj.del() == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_ENTRY_NOT_FOUND);
    ASSERT_TRUE(pds_batch_abort() == sdk::SDK_RET_OK);

    // teardown
    batch_params.epoch = ++api_test::g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(route_table_util::many_delete(num_route_tables,
        first_rt_table_id) == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);
}

/// \brief Teardown testcase
/// Does cleanup for route table testcases
TEST_F(route_test, DISABLED_final_teardown) {
    pds_batch_params_t batch_params = {0};

    batch_params.epoch = ++api_test::g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    // delete the TEPs created as part of SetUp()
    ASSERT_TRUE(tep_util::many_delete(PDS_MAX_TEP-1, k_first_nh_ip_str,
        PDS_TEP_ENCAP_TYPE_VNIC) == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);
    // TODO: Because of delayed delete,
    // immediate TEP creation in the successive test case is not possible.
    // once that is supported, need to move this to route_test::TearDown()
}

/// @}

}    // namespace api_test

//----------------------------------------------------------------------------
// Entry point
//----------------------------------------------------------------------------

static inline void
route_test_usage_print (char **argv)
{
    cout << "Usage : " << argv[0] << " -c <hal.json>" << endl;
    return;
}

static inline sdk_ret_t
route_test_options_validate (void)
{
    if (!api_test::g_cfg_file) {
        cerr << "HAL config file is not specified" << endl;
        return sdk::SDK_RET_ERR;
    }
    return sdk::SDK_RET_OK;
}

static void
route_test_options_parse (int argc, char **argv)
{
    int oc = -1;
    struct option longopts[] = {{"config", required_argument, NULL, 'c'},
                                {"help", no_argument, NULL, 'h'},
                                {0, 0, 0, 0}};

    while ((oc = getopt_long(argc, argv, ":hc:", longopts, NULL)) != -1) {
        switch (oc) {
        case 'c':
            api_test::g_cfg_file = optarg;
            break;
        default:    // ignore all other options
            break;
        }
    }
    return;
}

int
main (int argc, char **argv)
{
    route_test_options_parse(argc, argv);
    if (route_test_options_validate() != sdk::SDK_RET_OK) {
        route_test_usage_print(argv);
        exit(1);
    }

    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
