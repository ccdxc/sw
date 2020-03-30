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

//----------------------------------------------------------------------------
// Route test cases implementation
//----------------------------------------------------------------------------

void
route_spec_fill (pds_route_spec_t *route_spec, uint32_t route_id,
                 uint32_t route_table_id, ip_prefix_t *ip_pfx)
{
    uint32_t base_tep_id = 2;

    route_spec->key = int2pdsobjkey(route_id);
    route_spec->route_table = int2pdsobjkey(route_table_id);
    route_spec->route.key = route_spec->key;
    route_spec->route.prefix = *ip_pfx;
    route_spec->route.nh_type = PDS_NH_TYPE_OVERLAY;
    route_spec->route.tep = int2pdsobjkey(base_tep_id);
}

/// \defgroup ROUTE_TESTS Route tests
/// @{

/// do route table and individual route(s) add in separate batches
TEST_F(route_test, route_add_1) {
    sdk_ret_t ret;
    pds_batch_ctxt_t bctxt;
    pds_route_spec_t route_spec;
    pds_route_table_info_t info;
    uint32_t num_routes = 1, add_route_count = 5;
    ip_prefix_t ip_pfx;
    ip_addr_t ip_addr;
    pds_obj_key_t key;

    bctxt = batch_start();
    sample_route_table_setup(bctxt, k_base_v4_pfx, IP_AF_IPV4,
                             num_routes, 1, 1);
    batch_commit(bctxt);

    bctxt = batch_start();
    test::extract_ip_pfx(k_base_v4_pfx_2.c_str(), &ip_pfx);
    for (uint32_t i = 0; i < add_route_count; i++) {
        ip_prefix_ip_next(&ip_pfx, &ip_addr);
        ip_pfx.addr = ip_addr;
        route_spec_fill(&route_spec, 100+i, 1, &ip_pfx);
        ret = pds_route_create(&route_spec, bctxt);
        ASSERT_TRUE(ret == SDK_RET_OK);
    }
    batch_commit(bctxt);

    memset(&info, 0, sizeof(pds_route_table_info_t));
    info.spec.route_info =
        (route_info_t *)SDK_CALLOC(PDS_MEM_ALLOC_ID_ROUTE_TABLE,
                                   ROUTE_INFO_SIZE(0));
    key = int2pdsobjkey(1);
    ret = pds_route_table_read(&key, &info);
    ASSERT_TRUE(ret == SDK_RET_OK);
    ASSERT_TRUE(info.spec.route_info->num_routes ==
                (num_routes + add_route_count));
    SDK_FREE(PDS_MEM_ALLOC_ID_ROUTE_TABLE, info.spec.route_info);

    bctxt = batch_start();
    sample_route_table_teardown(bctxt, 1, 1);
    batch_commit(bctxt);
}

#if 0
/// do route table and individual route(s) add in same batch
TEST_F(route_test, route_add_2) {
    sdk_ret_t ret;
    ip_addr_t ip_addr;
    pds_obj_key_t key;
    ip_prefix_t ip_pfx;
    pds_batch_ctxt_t bctxt;
    pds_route_spec_t route_spec;
    pds_route_table_info_t info;
    uint32_t num_routes = 1, add_route_count = 5;

    bctxt = batch_start();
    sample_route_table_setup(bctxt, k_base_v4_pfx, IP_AF_IPV4,
                             num_routes, 1, 1);
    test::extract_ip_pfx(k_base_v4_pfx_2.c_str(), &ip_pfx);
    for (uint32_t i = 0; i < add_route_count; i++) {
        ip_prefix_ip_next(&ip_pfx, &ip_addr);
        ip_pfx.addr = ip_addr;
        route_spec_fill(&route_spec, 100+i, 1, &ip_pfx);
        ret = pds_route_create(&route_spec, bctxt);
        ASSERT_TRUE(ret == SDK_RET_OK);
    }
    batch_commit(bctxt);

    memset(&info, 0, sizeof(pds_route_table_info_t));
    info.spec.route_info =
        (route_info_t *)SDK_CALLOC(PDS_MEM_ALLOC_ID_ROUTE_TABLE,
                                   ROUTE_INFO_SIZE(0));
    key = int2pdsobjkey(1);
    ret = pds_route_table_read(&key, &info);
    ASSERT_TRUE(ret == SDK_RET_OK);
    ASSERT_TRUE(info.spec.route_info->num_routes ==
                (num_routes + add_route_count));
    SDK_FREE(PDS_MEM_ALLOC_ID_ROUTE_TABLE, info.spec.route_info);

    bctxt = batch_start();
    sample_route_table_teardown(bctxt, 1, 1);
    batch_commit(bctxt);
}

/// do route table add and then individual route(s) add/del in same batch
TEST_F(route_test, route_add_del_1) {
    sdk_ret_t ret;
    ip_addr_t ip_addr;
    pds_obj_key_t key;
    ip_prefix_t ip_pfx;
    pds_batch_ctxt_t bctxt;
    pds_route_spec_t route_spec;
    pds_route_table_info_t info;
    uint32_t num_routes = 1, add_route_count = 5;

    bctxt = batch_start();
    sample_route_table_setup(bctxt, k_base_v4_pfx, IP_AF_IPV4,
                             num_routes, 1, 1);
    test::extract_ip_pfx(k_base_v4_pfx_2.c_str(), &ip_pfx);
    for (uint32_t i = 0; i < add_route_count; i++) {
        ip_prefix_ip_next(&ip_pfx, &ip_addr);
        ip_pfx.addr = ip_addr;
        route_spec_fill(&route_spec, 100+i, 1, &ip_pfx);
        ret = pds_route_create(&route_spec, bctxt);
        ASSERT_TRUE(ret == SDK_RET_OK);
    }
    // TODO: need an API for del here @rsrikanth
    batch_commit(bctxt);

    memset(&info, 0, sizeof(pds_route_table_info_t));
    info.spec.route_info =
        (route_info_t *)SDK_CALLOC(PDS_MEM_ALLOC_ID_ROUTE_TABLE,
                                   ROUTE_INFO_SIZE(0));
    key = int2pdsobjkey(1);
    ret = pds_route_table_read(&key, &info);
    ASSERT_TRUE(ret == SDK_RET_OK);
    ASSERT_TRUE(info.spec.route_info->num_routes ==
                (num_routes + add_route_count));
    SDK_FREE(PDS_MEM_ALLOC_ID_ROUTE_TABLE, info.spec.route_info);

    bctxt = batch_start();
    sample_route_table_teardown(bctxt, 1, 1);
    batch_commit(bctxt);
}
#endif

/// add route table and update individual route(s) in separate batches
TEST_F(route_test, route_upd_1) {
    sdk_ret_t ret;
    ip_prefix_t ip_pfx;
    uint32_t num_routes = 1;
    pds_route_spec_t route_spec;
    pds_route_table_info_t info;
    pds_obj_key_t key;
    pds_batch_ctxt_t bctxt;

    // create route table
    bctxt = batch_start();
    sample_route_table_setup(bctxt, k_base_v4_pfx, IP_AF_IPV4,
                             num_routes, 1, 2);
    batch_commit(bctxt);

    // update 1st route
    bctxt = batch_start();
    test::extract_ip_pfx(k_base_v4_pfx_3.c_str(), &ip_pfx);
    route_spec_fill(&route_spec, 1, 2, &ip_pfx);
    ret = pds_route_update(&route_spec, bctxt);
    ASSERT_TRUE(ret == SDK_RET_OK);
    batch_commit(bctxt);

    // read 1st route from route table
    memset(&info, 0, sizeof(pds_route_table_info_t));
    info.spec.route_info =
        (route_info_t *)SDK_CALLOC(PDS_MEM_ALLOC_ID_ROUTE_TABLE,
                                   ROUTE_INFO_SIZE(1));
    info.spec.route_info->num_routes = 1;
    key = int2pdsobjkey(2);
    ret = pds_route_table_read(&key, &info);
    ASSERT_TRUE(ret == SDK_RET_OK);
    ASSERT_TRUE(memcmp(&info.spec.route_info->routes[0].prefix,
                       &ip_pfx, sizeof(ip_prefix_t)) == 0);
    SDK_FREE(PDS_MEM_ALLOC_ID_ROUTE_TABLE, info.spec.route_info);

    bctxt = batch_start();
    sample_route_table_teardown(bctxt, 2, 1);
    batch_commit(bctxt);
}

#if 0
/// add route table and update individual route(s) in same batch
TEST_F(route_test, route_upd_2) {
    sdk_ret_t ret;
    ip_prefix_t ip_pfx;
    uint32_t num_routes = 1;
    pds_route_spec_t route_spec;
    pds_route_table_info_t info;
    pds_obj_key_t key;
    pds_batch_ctxt_t bctxt;

    // create route table
    bctxt = batch_start();
    sample_route_table_setup(bctxt, k_base_v4_pfx, IP_AF_IPV4,
                             num_routes, 1, 2);
    // update 1st route
    test::extract_ip_pfx(k_base_v4_pfx_3.c_str(), &ip_pfx);
    route_spec_fill(&route_spec, 1, 2, &ip_pfx);
    ret = pds_route_update(&route_spec, bctxt);
    ASSERT_TRUE(ret == SDK_RET_OK);
    batch_commit(bctxt);

    // read 1st route from route table
    memset(&info, 0, sizeof(pds_route_table_info_t));
    info.spec.route_info =
        (route_info_t *)SDK_CALLOC(PDS_MEM_ALLOC_ID_ROUTE_TABLE,
                                   ROUTE_INFO_SIZE(1));
    info.spec.route_info->num_routes = 1;
    key = int2pdsobjkey(2);
    ret = pds_route_table_read(&key, &info);
    ASSERT_TRUE(ret == SDK_RET_OK);
    ASSERT_TRUE(memcmp(&info.spec.route_info->routes[0].prefix,
                       &ip_pfx, sizeof(ip_prefix_t)) == 0);
    SDK_FREE(PDS_MEM_ALLOC_ID_ROUTE_TABLE, info.spec.route_info);

    bctxt = batch_start();
    sample_route_table_teardown(bctxt, 2, 1);
    batch_commit(bctxt);
}
#endif

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
