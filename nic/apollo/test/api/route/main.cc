//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This file contains the all route test cases
///
//----------------------------------------------------------------------------

#include "nic/apollo/test/api/utils/api_base.hpp"
#include "nic/apollo/test/api/utils/batch.hpp"
#include "nic/apollo/test/api/utils/nexthop.hpp"
#include "nic/apollo/test/api/utils/route.hpp"
#include "nic/apollo/test/api/utils/device.hpp"
#include "nic/apollo/test/api/utils/tep.hpp"
#include "nic/apollo/test/api/utils/vpc.hpp"
#include "nic/apollo/test/api/utils/subnet.hpp"
#include "nic/apollo/test/api/utils/if.hpp"
#include "nic/apollo/test/api/utils/nexthop.hpp"
#include "nic/apollo/test/api/utils/nexthop_group.hpp"

namespace test {
namespace api {

// globals
static const std::string k_base_v4_pfx    = "100.100.100.1/16";
static const std::string k_base_v4_pfx_2  = "100.200.100.1/16";
static const std::string k_base_v4_pfx_3  = "100.225.100.1/16";

//----------------------------------------------------------------------------
// Route test class
//----------------------------------------------------------------------------

class route_test : public pds_test_base {
protected:
    route_test() {}
    virtual ~route_test() {}
    virtual void SetUp() {}
    virtual void TearDown() {}
    static void SetUpTestCase() {
        if (!agent_mode()) {
            pds_test_base::SetUpTestCase(g_tc_params);
        }
        pds_batch_ctxt_t bctxt = batch_start();
        sample1_vpc_setup(bctxt, PDS_VPC_TYPE_TENANT);
        sample_device_setup(bctxt);
        sample_if_setup(bctxt);
        sample_nexthop_setup(bctxt);
        sample_nexthop_group_setup(bctxt);
        sample_tep_setup(bctxt);
        sample_subnet_setup(bctxt);
        batch_commit(bctxt);
    }
    static void TearDownTestCase() {
        pds_batch_ctxt_t bctxt = batch_start();
        sample_subnet_teardown(bctxt);
        sample_tep_teardown(bctxt);
        sample_nexthop_group_teardown(bctxt);
        sample_nexthop_teardown(bctxt);
        sample_if_teardown(bctxt);
        sample_device_teardown(bctxt);
        sample1_vpc_teardown(bctxt, PDS_VPC_TYPE_TENANT);
        batch_commit(bctxt);
        if (!agent_mode()) {
            pds_test_base::TearDownTestCase();
        }
    }
};

static uint32_t k_num_init_routes = 10;
static uint32_t k_num_route_add = 5;
static uint32_t k_num_route_del = 2;
static uint32_t k_num_route_tables = 1;
static uint32_t k_route_table_id = 1;

//----------------------------------------------------------------------------
// Route test cases implementation
//----------------------------------------------------------------------------

void
route_spec_fill (pds_route_spec_t *route_spec, uint32_t route_id,
                 ip_prefix_t *ip_pfx)
{
    uint32_t base_tep_id = 2;

    route_spec->key.route_id = int2pdsobjkey(route_id);
    route_spec->key.route_table_id = int2pdsobjkey(k_route_table_id);
    route_spec->attrs.prefix = *ip_pfx;
    route_spec->attrs.nh_type = PDS_NH_TYPE_OVERLAY;
    route_spec->attrs.tep = int2pdsobjkey(base_tep_id);
}

void
route_table_setup (pds_batch_ctxt_t bctxt, std::string base_pfx)
{
    route_table_feeder rt_feeder;

    rt_feeder.init(base_pfx, IP_AF_IPV4, k_num_init_routes,
                   k_num_route_tables, k_route_table_id);
    many_create(bctxt, rt_feeder);
}

void
route_table_teardown (pds_batch_ctxt_t bctxt)
{
    route_table_feeder rt_feeder;

    rt_feeder.init("0.0.0.0/0", IP_AF_IPV4, PDS_MAX_ROUTE_PER_TABLE,
                   k_num_route_tables, k_route_table_id);
    many_delete(bctxt, rt_feeder);
}

void
route_table_add_routes (pds_batch_ctxt_t bctxt, std::string base_pfx)
{
    pds_route_spec_t route_spec;
    uint32_t route_id = 100;
    ip_prefix_t ip_pfx;
    ip_addr_t ip_addr;
    sdk_ret_t ret;

    test::extract_ip_pfx(base_pfx.c_str(), &ip_pfx);
    for (uint32_t i = 0; i < k_num_route_add; i++) {
        ip_prefix_ip_next(&ip_pfx, &ip_addr);
        ip_pfx.addr = ip_addr;
        route_spec_fill(&route_spec, route_id + i, &ip_pfx);
        ret = pds_route_create(&route_spec, bctxt);
        ASSERT_TRUE(ret == SDK_RET_OK);
    }
}

void
route_table_add_routes_verify (void)
{
    pds_route_table_info_t info;
    pds_obj_key_t key;
    sdk_ret_t ret;

    memset(&info, 0, sizeof(pds_route_table_info_t));
    info.spec.route_info =
        (route_info_t *)SDK_CALLOC(PDS_MEM_ALLOC_ID_ROUTE_TABLE,
                                   ROUTE_INFO_SIZE(0));
    key = int2pdsobjkey(k_route_table_id);
    ret = pds_route_table_read(&key, &info);
    ASSERT_TRUE(ret == SDK_RET_OK);
    ASSERT_TRUE(info.spec.route_info->num_routes ==
                (k_num_init_routes + k_num_route_add));
    SDK_FREE(PDS_MEM_ALLOC_ID_ROUTE_TABLE, info.spec.route_info);
}

void
route_table_delete_routes (pds_batch_ctxt_t bctxt, std::string base_pfx)
{
    sdk_ret_t ret;
    pds_route_key_t key;
    uint32_t route_id = 100;

    for (uint32_t i = 0; i < k_num_route_del; i++) {
        key.route_id = int2pdsobjkey(i + 1);
        key.route_table_id = int2pdsobjkey(k_route_table_id);
        ret = pds_route_delete(&key, bctxt);
        ASSERT_TRUE(ret == SDK_RET_OK);
    }
}

void
route_table_delete_routes_verify (std::string base_pfx)
{
}

void
route_table_update_route (pds_batch_ctxt_t bctxt, std::string base_pfx)
{
    pds_route_spec_t route_spec;
    uint32_t route_id = 1;
    ip_prefix_t ip_pfx;
    sdk_ret_t ret;

    test::extract_ip_pfx(base_pfx.c_str(), &ip_pfx);
    route_spec_fill(&route_spec, route_id, &ip_pfx);
    ret = pds_route_update(&route_spec, bctxt);
    ASSERT_TRUE(ret == SDK_RET_OK);
}

void
route_table_update_route_verify (std::string base_pfx)
{
    pds_route_table_info_t info;
    ip_prefix_t ip_pfx;
    pds_obj_key_t key;
    sdk_ret_t ret;

    test::extract_ip_pfx(base_pfx.c_str(), &ip_pfx);
    memset(&info, 0, sizeof(pds_route_table_info_t));
    info.spec.route_info =
        (route_info_t *)SDK_CALLOC(PDS_MEM_ALLOC_ID_ROUTE_TABLE,
                                   ROUTE_INFO_SIZE(1));
    info.spec.route_info->num_routes = 1;
    key = int2pdsobjkey(k_route_table_id);
    ret = pds_route_table_read(&key, &info);
    ASSERT_TRUE(ret == SDK_RET_OK);
    ASSERT_TRUE(memcmp(&info.spec.route_info->routes[0].attrs.prefix,
                       &ip_pfx, sizeof(ip_prefix_t)) == 0);
    SDK_FREE(PDS_MEM_ALLOC_ID_ROUTE_TABLE, info.spec.route_info);
}

/// \defgroup ROUTE_TESTS Route tests
/// @{

/// do route table and individual route(s) add in separate batches
TEST_F(route_test, route_add_1) {
    pds_batch_ctxt_t bctxt;

    bctxt = batch_start();
    route_table_setup(bctxt, k_base_v4_pfx);
    batch_commit(bctxt);

    bctxt = batch_start();
    route_table_add_routes(bctxt, k_base_v4_pfx_2);
    batch_commit(bctxt);

    route_table_add_routes_verify();

    bctxt = batch_start();
    route_table_teardown(bctxt);
    batch_commit(bctxt);
}

/// do route table and individual route(s) add in same batch
TEST_F(route_test, route_add_2) {
    pds_batch_ctxt_t bctxt;

    bctxt = batch_start();
    route_table_setup(bctxt, k_base_v4_pfx);
    route_table_add_routes(bctxt, k_base_v4_pfx_2);
    batch_commit(bctxt);

    route_table_add_routes_verify();

    bctxt = batch_start();
    route_table_teardown(bctxt);
    batch_commit(bctxt);
}

/// do route table add and then individual route(s) add/del in same batch
TEST_F(route_test, route_del_1) {
    pds_batch_ctxt_t bctxt;

    // create route table
    bctxt = batch_start();
    route_table_setup(bctxt, k_base_v4_pfx);
    batch_commit(bctxt);

    bctxt = batch_start();
    route_table_delete_routes(bctxt, k_base_v4_pfx);
    batch_commit(bctxt);

    // verify delete
    route_table_delete_routes_verify(k_base_v4_pfx);

    bctxt = batch_start();
    route_table_teardown(bctxt);
    batch_commit(bctxt);
}

/// add route table and update individual route(s) in same batch
TEST_F(route_test, route_del_2) {
    pds_batch_ctxt_t bctxt;

    // create and update route table in one batch
    bctxt = batch_start();
    route_table_setup(bctxt, k_base_v4_pfx);
    route_table_delete_routes(bctxt, k_base_v4_pfx);
    batch_commit(bctxt);

    // verify delete
    route_table_delete_routes_verify(k_base_v4_pfx);

    bctxt = batch_start();
    route_table_teardown(bctxt);
    batch_commit(bctxt);
}

/// add route table and update individual route(s) in separate batches
TEST_F(route_test, route_upd_1) {
    pds_batch_ctxt_t bctxt;

    // create route table
    bctxt = batch_start();
    route_table_setup(bctxt, k_base_v4_pfx);
    batch_commit(bctxt);

    // update route
    bctxt = batch_start();
    route_table_update_route(bctxt, k_base_v4_pfx_3);
    batch_commit(bctxt);

    // verify update
    route_table_update_route_verify(k_base_v4_pfx_3);

    bctxt = batch_start();
    route_table_teardown(bctxt);
    batch_commit(bctxt);
}

/// add route table and update individual route(s) in same batch
TEST_F(route_test, route_upd_2) {
    pds_batch_ctxt_t bctxt;

    // create and update route table in one batch
    bctxt = batch_start();
    route_table_setup(bctxt, k_base_v4_pfx);
    route_table_update_route(bctxt, k_base_v4_pfx_3);
    batch_commit(bctxt);

    // verify update
    route_table_update_route_verify(k_base_v4_pfx_3);

    bctxt = batch_start();
    route_table_teardown(bctxt);
    batch_commit(bctxt);
}

/// @}

}    // namespace api
}    // namespace test

//----------------------------------------------------------------------------
// Entry point
//----------------------------------------------------------------------------

/// @private
int
main (int argc, char **argv)
{
    return api_test_program_run(argc, argv);
}
