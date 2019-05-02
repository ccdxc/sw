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

constexpr int k_max_v4_route_table = PDS_MAX_ROUTE_TABLE;
constexpr int k_max_v6_route_table = PDS_MAX_ROUTE_TABLE;
constexpr int k_max_route_per_tbl = PDS_MAX_ROUTE_PER_TABLE;

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
        test_case_params_t params;
        params.cfg_file = api_test::g_cfg_file;
        params.enable_fte = false;
        pds_test_base::SetUpTestCase(params);

        pds_batch_params_t batch_params = {0};
        pds_encap_t encap = {PDS_ENCAP_TYPE_MPLSoUDP, 0};

        batch_params.epoch = ++g_batch_epoch;
        ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
        // create max TEPs which can be used as NHs for routes
        ASSERT_TRUE(tep_util::many_create(
            PDS_MAX_TEP - 1, k_first_nh_ip_str, PDS_TEP_TYPE_WORKLOAD,
            encap) == sdk::SDK_RET_OK);
        ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);
    }
    static void TearDownTestCase() {
        pds_batch_params_t batch_params = {0};
        pds_encap_t encap = {PDS_ENCAP_TYPE_MPLSoUDP, 0};

        batch_params.epoch = ++g_batch_epoch;
        ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
        ASSERT_TRUE(tep_util::many_delete(PDS_MAX_TEP - 1, k_first_nh_ip_str,
            PDS_TEP_TYPE_WORKLOAD, encap) == sdk::SDK_RET_OK);
        ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);
    }
};

//----------------------------------------------------------------------------
// Route table test cases implementation
//----------------------------------------------------------------------------

/// \defgroup Route table
/// @{

/// \brief Create and delete max v4/v6 route tables in the same batch
/// The operation should be de-duped by framework and effectively
/// a NO-OP from hardware perspective
/// [ Create SetMax, Delete SetMax ] - Read
TEST_F(route_test, v4v6_route_table_workflow_1) {
    pds_batch_params_t batch_params = {0};
    pds_route_table_id_t first_v4_route_table_id = 22222;
    pds_route_table_id_t first_v6_route_table_id = 11111;

    // trigger - test max route tables with zero routes
    batch_params.epoch = ++g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(route_table_util::many_create(k_max_v4_route_table,
        first_v4_route_table_id, k_first_v4_pfx_str, k_first_nh_ip_str,
        IP_AF_IPV4, 0) == sdk::SDK_RET_OK);
    ASSERT_TRUE(route_table_util::many_create(k_max_v6_route_table,
        first_v6_route_table_id, k_first_v6_pfx_str, k_first_nh_ip_str,
        IP_AF_IPV6, 0) == sdk::SDK_RET_OK);
    ASSERT_TRUE(route_table_util::many_delete(k_max_v4_route_table,
        first_v4_route_table_id) == sdk::SDK_RET_OK);
    ASSERT_TRUE(route_table_util::many_delete(k_max_v6_route_table,
        first_v6_route_table_id) == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);

    // trigger - test max route tables with max routes
    batch_params.epoch = ++g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(route_table_util::many_create(k_max_v4_route_table,
        first_v4_route_table_id, k_first_v4_pfx_str, k_first_nh_ip_str,
        IP_AF_IPV4, k_max_route_per_tbl) == sdk::SDK_RET_OK);
    ASSERT_TRUE(route_table_util::many_create(k_max_v6_route_table,
        first_v6_route_table_id, k_first_v6_pfx_str, k_first_nh_ip_str,
        IP_AF_IPV6, k_max_route_per_tbl) == sdk::SDK_RET_OK);
    ASSERT_TRUE(route_table_util::many_delete(k_max_v4_route_table,
        first_v4_route_table_id) == sdk::SDK_RET_OK);
    ASSERT_TRUE(route_table_util::many_delete(k_max_v6_route_table,
        first_v6_route_table_id) == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);
}

/// \brief Create, delete and create max v4/v6 route tables in the same batch
/// Create and delete should be de-deduped by framework and subsequent create
/// should result in successful creation
/// [ Create SetMax - Delete SetMax - Create SetMax ] - Read
TEST_F(route_test, v4v6_route_table_workflow_2) {
    pds_batch_params_t batch_params = {0};
    pds_route_table_id_t first_v4_route_table_id = 22222;
    pds_route_table_id_t first_v6_route_table_id = 11111;

    // trigger - test max route tables with zero routes
    batch_params.epoch = ++g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(route_table_util::many_create(k_max_v4_route_table,
        first_v4_route_table_id, k_first_v4_pfx_str, k_first_nh_ip_str,
        IP_AF_IPV4, 0) == sdk::SDK_RET_OK);
    ASSERT_TRUE(route_table_util::many_create(k_max_v6_route_table,
        first_v6_route_table_id, k_first_v6_pfx_str, k_first_nh_ip_str,
        IP_AF_IPV6, 0) == sdk::SDK_RET_OK);
    ASSERT_TRUE(route_table_util::many_delete(k_max_v4_route_table,
        first_v4_route_table_id) == sdk::SDK_RET_OK);
    ASSERT_TRUE(route_table_util::many_delete(k_max_v6_route_table,
        first_v6_route_table_id) == sdk::SDK_RET_OK);
    ASSERT_TRUE(route_table_util::many_create(k_max_v4_route_table,
        first_v4_route_table_id, k_first_v4_pfx_str, k_first_nh_ip_str,
        IP_AF_IPV4, 0) == sdk::SDK_RET_OK);
    ASSERT_TRUE(route_table_util::many_create(k_max_v6_route_table,
        first_v6_route_table_id, k_first_v6_pfx_str, k_first_nh_ip_str,
        IP_AF_IPV6, 0) == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);

    // cleanup
    batch_params.epoch = ++g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(route_table_util::many_delete(k_max_v4_route_table,
        first_v4_route_table_id) == sdk::SDK_RET_OK);
    ASSERT_TRUE(route_table_util::many_delete(k_max_v6_route_table,
        first_v6_route_table_id) == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);

    // trigger - test max route tables with max routes
    batch_params.epoch = ++g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(route_table_util::many_create(k_max_v4_route_table,
        first_v4_route_table_id, k_first_v4_pfx_str, k_first_nh_ip_str,
        IP_AF_IPV4, k_max_route_per_tbl) == sdk::SDK_RET_OK);
    ASSERT_TRUE(route_table_util::many_create(k_max_v6_route_table,
        first_v6_route_table_id, k_first_v6_pfx_str, k_first_nh_ip_str,
        IP_AF_IPV6, k_max_route_per_tbl) == sdk::SDK_RET_OK);
    ASSERT_TRUE(route_table_util::many_delete(k_max_v4_route_table,
        first_v4_route_table_id) == sdk::SDK_RET_OK);
    ASSERT_TRUE(route_table_util::many_delete(k_max_v6_route_table,
        first_v6_route_table_id) == sdk::SDK_RET_OK);
    ASSERT_TRUE(route_table_util::many_create(k_max_v4_route_table,
        first_v4_route_table_id, k_first_v4_pfx_str, k_first_nh_ip_str,
        IP_AF_IPV4, k_max_route_per_tbl) == sdk::SDK_RET_OK);
    ASSERT_TRUE(route_table_util::many_create(k_max_v6_route_table,
        first_v6_route_table_id, k_first_v6_pfx_str, k_first_nh_ip_str,
        IP_AF_IPV6, k_max_route_per_tbl) == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);

    // cleanup
    batch_params.epoch = ++g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(route_table_util::many_delete(k_max_v4_route_table,
        first_v4_route_table_id) == sdk::SDK_RET_OK);
    ASSERT_TRUE(route_table_util::many_delete(k_max_v6_route_table,
        first_v6_route_table_id) == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);
}

/// \brief Create Set1, Set2, Delete Set1, Create Set3 in same batch
/// The set1 route table should be de-duped and set2 and set3 should be
/// programmed in the hardware
/// [ Create Set1, Set2 - Delete Set1 - Create Set3 ] - Read
TEST_F(route_test, v4v6_route_table_workflow_3) {
    pds_batch_params_t batch_params = {0};
    uint32_t num_route_tables = 50;

    // trigger - test max route tables with zero routes
    batch_params.epoch = ++g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(route_table_util::many_create(num_route_tables,
        101, k_first_v4_pfx_str, k_first_nh_ip_str,
        IP_AF_IPV4, 0) == sdk::SDK_RET_OK);
    ASSERT_TRUE(route_table_util::many_create(num_route_tables,
        151, k_first_v6_pfx_str, k_first_nh_ip_str,
        IP_AF_IPV6, 0) == sdk::SDK_RET_OK);
    ASSERT_TRUE(route_table_util::many_create(num_route_tables,
        201, "125.100.100.1/21", k_first_nh_ip_str,
        IP_AF_IPV4, 0) == sdk::SDK_RET_OK);
    ASSERT_TRUE(route_table_util::many_create(num_route_tables,
        251, "125:100:100:1::0/64", k_first_nh_ip_str,
        IP_AF_IPV6, 0) == sdk::SDK_RET_OK);
    ASSERT_TRUE(route_table_util::many_delete(
        num_route_tables, 101) == sdk::SDK_RET_OK);
    ASSERT_TRUE(route_table_util::many_delete(
        num_route_tables, 151) == sdk::SDK_RET_OK);
    ASSERT_TRUE(route_table_util::many_create(num_route_tables,
        301, "150.100.100.1/21", k_first_nh_ip_str,
        IP_AF_IPV4, 0) == sdk::SDK_RET_OK);
    ASSERT_TRUE(route_table_util::many_create(num_route_tables,
        351, "150:100:100:1::0/64", k_first_nh_ip_str,
        IP_AF_IPV6, 0) == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);

    // cleanup
    batch_params.epoch = ++g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(route_table_util::many_delete(
        num_route_tables, 201) == sdk::SDK_RET_OK);
    ASSERT_TRUE(route_table_util::many_delete(
        num_route_tables, 251) == sdk::SDK_RET_OK);
    ASSERT_TRUE(route_table_util::many_delete(
        num_route_tables, 301) == sdk::SDK_RET_OK);
    ASSERT_TRUE(route_table_util::many_delete(
        num_route_tables, 351) == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);

    // trigger - test max route tables with max routes
    batch_params.epoch = ++g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(route_table_util::many_create(num_route_tables,
        101, k_first_v4_pfx_str, k_first_nh_ip_str,
        IP_AF_IPV4, k_max_route_per_tbl) == sdk::SDK_RET_OK);
    ASSERT_TRUE(route_table_util::many_create(num_route_tables,
        151, k_first_v6_pfx_str, k_first_nh_ip_str,
        IP_AF_IPV6, k_max_route_per_tbl) == sdk::SDK_RET_OK);
    ASSERT_TRUE(route_table_util::many_create(num_route_tables,
        201, "125.100.100.1/21", k_first_nh_ip_str,
        IP_AF_IPV4, k_max_route_per_tbl) == sdk::SDK_RET_OK);
    ASSERT_TRUE(route_table_util::many_create(num_route_tables,
        251, "125:100:100:1::0/64", k_first_nh_ip_str,
        IP_AF_IPV6, k_max_route_per_tbl) == sdk::SDK_RET_OK);
    ASSERT_TRUE(route_table_util::many_delete(
        num_route_tables, 101) == sdk::SDK_RET_OK);
    ASSERT_TRUE(route_table_util::many_delete(
        num_route_tables, 151) == sdk::SDK_RET_OK);
    ASSERT_TRUE(route_table_util::many_create(num_route_tables,
        301, "150.100.100.1/21", k_first_nh_ip_str,
        IP_AF_IPV4, k_max_route_per_tbl) == sdk::SDK_RET_OK);
    ASSERT_TRUE(route_table_util::many_create(num_route_tables,
        351, "150:100:100:1::0/64", k_first_nh_ip_str,
        IP_AF_IPV6, k_max_route_per_tbl) == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);

    // cleanup
    batch_params.epoch = ++g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(route_table_util::many_delete(
        num_route_tables, 201) == sdk::SDK_RET_OK);
    ASSERT_TRUE(route_table_util::many_delete(
        num_route_tables, 251) == sdk::SDK_RET_OK);
    ASSERT_TRUE(route_table_util::many_delete(
        num_route_tables, 301) == sdk::SDK_RET_OK);
    ASSERT_TRUE(route_table_util::many_delete(
        num_route_tables, 351) == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);
}

/// \brief Create and delete max v4/v6 route tables in two batches
/// The hardware should create and delete route table correctly.
///  Validate using reads when it is supported
/// [ Create SetMax ] - Read - [ Delete SetMax ] - Read
TEST_F(route_test, v4v6_route_table_workflow_4) {
    pds_batch_params_t batch_params = {0};
    pds_route_table_id_t first_v4_route_table_id = 22222;
    pds_route_table_id_t first_v6_route_table_id = 11111;

    // trigger - test max route tables with zero routes
    batch_params.epoch = ++g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(route_table_util::many_create(k_max_v4_route_table,
        first_v4_route_table_id, k_first_v4_pfx_str, k_first_nh_ip_str,
        IP_AF_IPV4, 0) == sdk::SDK_RET_OK);
    ASSERT_TRUE(route_table_util::many_create(k_max_v6_route_table,
        first_v6_route_table_id, k_first_v6_pfx_str, k_first_nh_ip_str,
        IP_AF_IPV6, 0) == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);

    batch_params.epoch = ++g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(route_table_util::many_delete(k_max_v4_route_table,
        first_v4_route_table_id) == sdk::SDK_RET_OK);
    ASSERT_TRUE(route_table_util::many_delete(k_max_v6_route_table,
        first_v6_route_table_id) == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);

    // trigger - test max route tables with max routes
    batch_params.epoch = ++g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(route_table_util::many_create(k_max_v4_route_table,
        first_v4_route_table_id, k_first_v4_pfx_str, k_first_nh_ip_str,
        IP_AF_IPV4, k_max_route_per_tbl) == sdk::SDK_RET_OK);
    ASSERT_TRUE(route_table_util::many_create(k_max_v6_route_table,
        first_v6_route_table_id, k_first_v6_pfx_str, k_first_nh_ip_str,
        IP_AF_IPV6, k_max_route_per_tbl) == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);

    batch_params.epoch = ++g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(route_table_util::many_delete(k_max_v4_route_table,
        first_v4_route_table_id) == sdk::SDK_RET_OK);
    ASSERT_TRUE(route_table_util::many_delete(k_max_v6_route_table,
        first_v6_route_table_id) == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);
}

/// \brief Create and delete mix and match of v4/v6 route tables in two batches
/// [ Create Set1, Set2 ] - Read - [Delete Set1 - Create Set3 ] - Read
TEST_F(route_test, v4v6_route_table_workflow_5) {
    pds_batch_params_t batch_params = {0};
    uint32_t num_route_tables = 50;

    // trigger - test max route tables with zero routes
    batch_params.epoch = ++g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(route_table_util::many_create(num_route_tables,
        101, k_first_v4_pfx_str, k_first_nh_ip_str,
        IP_AF_IPV4, 0) == sdk::SDK_RET_OK);
    ASSERT_TRUE(route_table_util::many_create(num_route_tables,
        151, k_first_v6_pfx_str, k_first_nh_ip_str,
        IP_AF_IPV6, 0) == sdk::SDK_RET_OK);
    ASSERT_TRUE(route_table_util::many_create(num_route_tables,
        201, "125.100.100.1/21", k_first_nh_ip_str,
        IP_AF_IPV4, 0) == sdk::SDK_RET_OK);
    ASSERT_TRUE(route_table_util::many_create(num_route_tables,
        251, "125:100:100:1::0/64", k_first_nh_ip_str,
        IP_AF_IPV6, 0) == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);

    batch_params.epoch = ++g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(route_table_util::many_delete(
        num_route_tables, 101) == sdk::SDK_RET_OK);
    ASSERT_TRUE(route_table_util::many_delete(
        num_route_tables, 151) == sdk::SDK_RET_OK);
    ASSERT_TRUE(route_table_util::many_create(num_route_tables,
        301, "150.100.100.1/21", k_first_nh_ip_str,
        IP_AF_IPV4, 0) == sdk::SDK_RET_OK);
    ASSERT_TRUE(route_table_util::many_create(num_route_tables,
        351, "150:100:100:1::0/64", k_first_nh_ip_str,
        IP_AF_IPV6, 0) == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);

    // cleanup
    batch_params.epoch = ++g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(route_table_util::many_delete(
        num_route_tables, 201) == sdk::SDK_RET_OK);
    ASSERT_TRUE(route_table_util::many_delete(
        num_route_tables, 251) == sdk::SDK_RET_OK);
    ASSERT_TRUE(route_table_util::many_delete(
        num_route_tables, 301) == sdk::SDK_RET_OK);
    ASSERT_TRUE(route_table_util::many_delete(
        num_route_tables, 351) == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);

    // trigger - test max route tables with max routes
    batch_params.epoch = ++g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(route_table_util::many_create(num_route_tables,
        101, k_first_v4_pfx_str, k_first_nh_ip_str,
        IP_AF_IPV4, k_max_route_per_tbl) == sdk::SDK_RET_OK);
    ASSERT_TRUE(route_table_util::many_create(num_route_tables,
        151, k_first_v6_pfx_str, k_first_nh_ip_str,
        IP_AF_IPV6, k_max_route_per_tbl) == sdk::SDK_RET_OK);
    ASSERT_TRUE(route_table_util::many_create(num_route_tables,
        201, "125.100.100.1/21", k_first_nh_ip_str,
        IP_AF_IPV4, k_max_route_per_tbl) == sdk::SDK_RET_OK);
    ASSERT_TRUE(route_table_util::many_create(num_route_tables,
        251, "125:100:100:1::0/64", k_first_nh_ip_str,
        IP_AF_IPV6, k_max_route_per_tbl) == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);

    batch_params.epoch = ++g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(route_table_util::many_delete(
        num_route_tables, 101) == sdk::SDK_RET_OK);
    ASSERT_TRUE(route_table_util::many_delete(
        num_route_tables, 151) == sdk::SDK_RET_OK);
    ASSERT_TRUE(route_table_util::many_create(num_route_tables,
        301, "150.100.100.1/21", k_first_nh_ip_str,
        IP_AF_IPV4, k_max_route_per_tbl) == sdk::SDK_RET_OK);
    ASSERT_TRUE(route_table_util::many_create(num_route_tables,
        351, "150:100:100:1::0/64", k_first_nh_ip_str,
        IP_AF_IPV6, k_max_route_per_tbl) == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);

    // cleanup
    batch_params.epoch = ++g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(route_table_util::many_delete(
        num_route_tables, 201) == sdk::SDK_RET_OK);
    ASSERT_TRUE(route_table_util::many_delete(
        num_route_tables, 251) == sdk::SDK_RET_OK);
    ASSERT_TRUE(route_table_util::many_delete(
        num_route_tables, 301) == sdk::SDK_RET_OK);
    ASSERT_TRUE(route_table_util::many_delete(
        num_route_tables, 351) == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);
}

/// \brief Create maximum number of v4v6 route tables in two batches
/// [ Create SetMax ] - [ Create SetMax ] - Read
TEST_F(route_test, v4v6_route_table_workflow_neg_1) {
    pds_batch_params_t batch_params = {0};
    pds_route_table_id_t first_v4_route_table_id = 22222;
    pds_route_table_id_t first_v6_route_table_id = 11111;

    // trigger - test max route tables with zero routes
    batch_params.epoch = ++g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(route_table_util::many_create(k_max_v4_route_table,
        first_v4_route_table_id, k_first_v4_pfx_str, k_first_nh_ip_str,
        IP_AF_IPV4, 0) == sdk::SDK_RET_OK);
    ASSERT_TRUE(route_table_util::many_create(k_max_v6_route_table,
        first_v6_route_table_id, k_first_v6_pfx_str, k_first_nh_ip_str,
        IP_AF_IPV6, 0) == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);

    batch_params.epoch = ++g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(route_table_util::many_create(k_max_v4_route_table,
        first_v4_route_table_id, k_first_v4_pfx_str, k_first_nh_ip_str,
        IP_AF_IPV4, 0) == sdk::SDK_RET_OK);
    ASSERT_TRUE(route_table_util::many_create(k_max_v6_route_table,
        first_v6_route_table_id, k_first_v6_pfx_str, k_first_nh_ip_str,
        IP_AF_IPV6, 0) == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_INVALID_OP);
    ASSERT_TRUE(pds_batch_abort() == sdk::SDK_RET_OK);

    // cleanup
    batch_params.epoch = ++g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(route_table_util::many_delete(k_max_v4_route_table,
        first_v4_route_table_id) == sdk::SDK_RET_OK);
    ASSERT_TRUE(route_table_util::many_delete(k_max_v6_route_table,
        first_v6_route_table_id) == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);

    // trigger - test max route tables with max routes
    batch_params.epoch = ++g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(route_table_util::many_create(k_max_v4_route_table,
        first_v4_route_table_id, k_first_v4_pfx_str, k_first_nh_ip_str,
        IP_AF_IPV4, k_max_route_per_tbl) == sdk::SDK_RET_OK);
    ASSERT_TRUE(route_table_util::many_create(k_max_v6_route_table,
        first_v6_route_table_id, k_first_v6_pfx_str, k_first_nh_ip_str,
        IP_AF_IPV6, k_max_route_per_tbl) == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);

    batch_params.epoch = ++g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(route_table_util::many_create(k_max_v4_route_table,
        first_v4_route_table_id, k_first_v4_pfx_str, k_first_nh_ip_str,
        IP_AF_IPV4, k_max_route_per_tbl) == sdk::SDK_RET_OK);
    ASSERT_TRUE(route_table_util::many_create(k_max_v6_route_table,
        first_v6_route_table_id, k_first_v6_pfx_str, k_first_nh_ip_str,
        IP_AF_IPV6, k_max_route_per_tbl) == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_INVALID_OP);
    ASSERT_TRUE(pds_batch_abort() == sdk::SDK_RET_OK);

    // cleanup
    batch_params.epoch = ++g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(route_table_util::many_delete(k_max_v4_route_table,
        first_v4_route_table_id) == sdk::SDK_RET_OK);
    ASSERT_TRUE(route_table_util::many_delete(k_max_v6_route_table,
        first_v6_route_table_id) == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);
}

/// \brief Create more than max number of v4/v6 route tables supported
/// [ Create SetMax+1] - Read
TEST_F(route_test, DISABLED_v4v6_route_table_workflow_neg_2) {
    pds_batch_params_t batch_params = {0};
    pds_route_table_id_t first_v4_route_table_id = 22222;
    pds_route_table_id_t first_v6_route_table_id = 11111;

    // trigger - test route tables with zero routes
    batch_params.epoch = ++g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(route_table_util::many_create(k_max_v4_route_table + 1,
        first_v4_route_table_id, k_first_v4_pfx_str, k_first_nh_ip_str,
        IP_AF_IPV4, 0) == sdk::SDK_RET_OK);
    ASSERT_TRUE(route_table_util::many_create(k_max_v6_route_table + 1,
        first_v6_route_table_id, k_first_v6_pfx_str, k_first_nh_ip_str,
        IP_AF_IPV6, 0) == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_NO_RESOURCE);
    ASSERT_TRUE(pds_batch_abort() == sdk::SDK_RET_OK);

    // trigger - test route tables with max routes
    batch_params.epoch = ++g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(route_table_util::many_create(k_max_v4_route_table + 1,
        first_v4_route_table_id, k_first_v4_pfx_str, k_first_nh_ip_str,
        IP_AF_IPV4, k_max_route_per_tbl) == sdk::SDK_RET_OK);
    ASSERT_TRUE(route_table_util::many_create(k_max_v6_route_table + 1,
        first_v6_route_table_id, k_first_v6_pfx_str, k_first_nh_ip_str,
        IP_AF_IPV6, k_max_route_per_tbl) == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_NO_RESOURCE);
    ASSERT_TRUE(pds_batch_abort() == sdk::SDK_RET_OK);

    // trigger - test route tables with max+1 routes
    batch_params.epoch = ++g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(route_table_util::many_create(k_max_v4_route_table,
        first_v4_route_table_id, k_first_v4_pfx_str, k_first_nh_ip_str,
        IP_AF_IPV4, k_max_route_per_tbl+1) == sdk::SDK_RET_OK);
    ASSERT_TRUE(route_table_util::many_create(k_max_v6_route_table,
        first_v6_route_table_id, k_first_v6_pfx_str, k_first_nh_ip_str,
        IP_AF_IPV6, k_max_route_per_tbl+1) == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_NO_RESOURCE);
    ASSERT_TRUE(pds_batch_abort() == sdk::SDK_RET_OK);
}

/// \brief Read and delete non existing v4/v6 route tables and routes
/// Read NonEx, [ Delete NonExMax ]
TEST_F(route_test, v4v6_route_table_workflow_neg_3) {
    pds_batch_params_t batch_params = {0};
    pds_route_table_id_t first_v4_route_table_id = 22222;
    pds_route_table_id_t first_v6_route_table_id = 11111;

    // trigger - read is not supported yet; validate later

    // trigger
    batch_params.epoch = ++g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(route_table_util::many_delete(k_max_v4_route_table,
        111) == sdk::SDK_RET_OK);
    ASSERT_TRUE(route_table_util::many_delete(k_max_v6_route_table,
        222) == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_ENTRY_NOT_FOUND);
    ASSERT_TRUE(pds_batch_abort() == sdk::SDK_RET_OK);
}

/// \brief Create and delete different v4v6 route tables in different batches
/// The hardware should create v4v6 route tables correctly and
/// delete batch (containing valid & invalid entries) is expected to fail
/// because of lookup failure.
/// [ Create Set1 ] - [Delete Set1, Set2 ] - Read
TEST_F(route_test, v4v6_route_table_workflow_neg_4) {
    pds_batch_params_t batch_params = {0};
    uint32_t num_route_tables = 40;

    // trigger- test max route tables with zero routes
    batch_params.epoch = ++g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(route_table_util::many_create(num_route_tables,
        100, k_first_v4_pfx_str, k_first_nh_ip_str,
        IP_AF_IPV4, 0) == sdk::SDK_RET_OK);
    ASSERT_TRUE(route_table_util::many_create(num_route_tables,
        150, k_first_v6_pfx_str, k_first_nh_ip_str,
        IP_AF_IPV6, 0) == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);

    batch_params.epoch = ++g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(route_table_util::many_delete(num_route_tables,
        200) == sdk::SDK_RET_OK);
    ASSERT_TRUE(route_table_util::many_delete(num_route_tables,
        250) == sdk::SDK_RET_OK);
    ASSERT_TRUE(route_table_util::many_create(num_route_tables,
        300, k_first_v4_pfx_str, k_first_nh_ip_str,
        IP_AF_IPV4, 0) == sdk::SDK_RET_OK);
    ASSERT_TRUE(route_table_util::many_create(num_route_tables,
        350, k_first_v6_pfx_str, k_first_nh_ip_str,
        IP_AF_IPV6, 0) == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_ENTRY_NOT_FOUND);
    ASSERT_TRUE(pds_batch_abort() == sdk::SDK_RET_OK);

    // cleanup
    batch_params.epoch = ++g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(route_table_util::many_delete(num_route_tables,
        100) == sdk::SDK_RET_OK);
    ASSERT_TRUE(route_table_util::many_delete(num_route_tables,
        150) == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);

    // trigger- test max route tables with max routes
    batch_params.epoch = ++g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(route_table_util::many_create(num_route_tables,
        100, k_first_v4_pfx_str, k_first_nh_ip_str,
        IP_AF_IPV4, k_max_route_per_tbl) == sdk::SDK_RET_OK);
    ASSERT_TRUE(route_table_util::many_create(num_route_tables,
        150, k_first_v6_pfx_str, k_first_nh_ip_str,
        IP_AF_IPV6, k_max_route_per_tbl) == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);

    batch_params.epoch = ++g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(route_table_util::many_delete(num_route_tables,
        200) == sdk::SDK_RET_OK);
    ASSERT_TRUE(route_table_util::many_delete(num_route_tables,
        250) == sdk::SDK_RET_OK);
    ASSERT_TRUE(route_table_util::many_create(num_route_tables,
        300, k_first_v4_pfx_str, k_first_nh_ip_str,
        IP_AF_IPV4, k_max_route_per_tbl) == sdk::SDK_RET_OK);
    ASSERT_TRUE(route_table_util::many_create(num_route_tables,
        350, k_first_v6_pfx_str, k_first_nh_ip_str,
        IP_AF_IPV6, k_max_route_per_tbl) == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_ENTRY_NOT_FOUND);
    ASSERT_TRUE(pds_batch_abort() == sdk::SDK_RET_OK);

    // cleanup
    batch_params.epoch = ++g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(route_table_util::many_delete(num_route_tables,
        100) == sdk::SDK_RET_OK);
    ASSERT_TRUE(route_table_util::many_delete(num_route_tables,
        150) == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);
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
