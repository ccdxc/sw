//------------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
// test cases to verify sldirectmap table management library
//------------------------------------------------------------------------------

#include <stdio.h>
#include "gtest/gtest.h"
#include "lib/table/sldirectmap/sldirectmap.hpp"
#include "lib/table/common/table.hpp"
#include "lib/p4/p4_api.hpp"

using namespace sdk;
using sdk::sdk_ret_t;
using sdk::table::sdk_table_factory_params_t;
using sdk::table::sdk_table_api_params_t;
using sdk::table::sdk_table_api_stats_t;

// class sldm_test : public hal_base_test {
class sldm_test : public ::testing::Test {
protected:
    sldm_test() {
  }

  virtual ~sldm_test() {
  }

  // will be called immediately after the constructor before each test
  virtual void SetUp() {
  }

  // will be called immediately after each test before the destructor
  virtual void TearDown() {
  }

  // Will be called at the beginning of all test cases in this class
  static void SetUpTestCase() {
      // hal_base_test::SetUpTestCase();
      // hal_test_utils_slab_disable_delete();
  }

};

typedef struct dm_data_s {
    int p;
    int q;
} dm_data_t;


#define P4TBL_ID_OUTPUT_MAPPING 33
#define P4TBL_ID_TWICE_NAT 17
typedef struct __attribute__((__packed__)) output_mapping_swkey {
    uint16_t control_metadata_dst_lport;/* Sourced from field union */
} output_mapping_swkey_t;


typedef enum output_mapping_actions_enum {
    OUTPUT_MAPPING_OUTPUT_MAPPING_DROP_ID = 0,
    OUTPUT_MAPPING_SET_TM_OPORT_ID = 1,
    OUTPUT_MAPPING_REDIRECT_TO_CPU_ID = 2,
    OUTPUT_MAPPING_REDIRECT_TO_REMOTE_ID = 3,
    OUTPUT_MAPPING_MAX_ID = 4
} output_mapping_actions_en;

typedef struct __attribute__((__packed__)) __output_mapping_set_tm_oport {
    uint8_t vlan_strip;
    uint8_t nports;
    uint8_t egress_mirror_en;
    uint8_t p4plus_app_id;
    uint8_t rdma_enabled;
    uint16_t dst_lif;
    uint32_t encap_vlan_id;
    uint8_t encap_vlan_id_valid;
    uint16_t access_vlan_id;
    uint8_t egress_port1;
    uint8_t egress_port2;
    uint8_t egress_port3;
    uint8_t egress_port4;
    uint8_t egress_port5;
    uint8_t egress_port6;
    uint8_t egress_port7;
    uint8_t egress_port8;
} output_mapping_set_tm_oport_t;

typedef struct __attribute__((__packed__)) __output_mapping_redirect_to_cpu {
    uint16_t dst_lif;
    uint8_t egress_mirror_en;
    uint8_t control_tm_oq;
    uint8_t cpu_copy_tm_oq;
} output_mapping_redirect_to_cpu_t;

typedef struct __attribute__((__packed__)) __output_mapping_redirect_to_remote {
    uint16_t tunnel_index;
    uint8_t tm_oport;
    uint8_t egress_mirror_en;
    uint8_t tm_oq;
} output_mapping_redirect_to_remote_t;

typedef union __output_mapping_action_union {
    output_mapping_set_tm_oport_t output_mapping_set_tm_oport;
    output_mapping_redirect_to_cpu_t output_mapping_redirect_to_cpu;
    output_mapping_redirect_to_remote_t output_mapping_redirect_to_remote;
} output_mapping_action_union_t;

typedef struct __attribute__((__packed__)) __output_mapping_actiondata {
    uint8_t actionid;
    output_mapping_action_union_t output_mapping_action_u;
} output_mapping_actiondata;

typedef struct __attribute__((__packed__)) twice_nat_swkey {
    uint16_t nat_metadata_twice_nat_idx;/* Sourced from field union */
} twice_nat_swkey_t;

typedef enum twice_nat_actions_enum {
    TWICE_NAT_NOP_ID = 0,
    TWICE_NAT_TWICE_NAT_REWRITE_INFO_ID = 1,
    TWICE_NAT_MAX_ID = 2
} twice_nat_actions_en;

typedef struct __attribute__((__packed__)) __twice_nat_twice_nat_rewrite_info {
    uint8_t ip[16];
    uint16_t l4_port;
} twice_nat_twice_nat_rewrite_info_t;

typedef union __twice_nat_action_union {
    twice_nat_twice_nat_rewrite_info_t twice_nat_twice_nat_rewrite_info;
} twice_nat_action_union_t;

typedef struct __attribute__((__packed__)) __twice_nat_actiondata {
    uint8_t actionid;
    twice_nat_action_union_t twice_nat_action_u;
} twice_nat_actiondata;


int p4pd_global_table_properties_get(uint32_t tableid,
                                     void *tbl_ctx)
{
    p4pd_table_properties_t *props = (p4pd_table_properties_t *)tbl_ctx;

    memset(props, 0, sizeof(p4pd_table_properties_t));
    props->tablename = (char *) "MockTable";
    props->key_struct_size = 32;
    props->actiondata_struct_size = 64;
    props->hash_type = 0;
    props->tabledepth = 100;
    props->hbm_layout.entry_width = 64;

    return 0;
}

static sldirectmap *
sldirectmap_init (bool sharing_en)
{
    sdk_table_factory_params_t fp = { 0 };
    fp.table_id = P4TBL_ID_OUTPUT_MAPPING;
    fp.entry_trace_en = true;
    return sldirectmap::factory(&fp);
}

/* -----------------------------------------------------------------------------
 *
 * Test Case 1:
 *      - Test Case to verify the insert
 * - Create DM table
 * - Insert DM Entry
 *
 * ---------------------------------------------------------------------------*/
TEST_F(sldm_test, test1) {
    sdk_table_api_params_t ap;
    sldirectmap *test_dm;
    output_mapping_actiondata dm;
    sdk_ret_t rt;

    dm.actionid = 1;
    ap.actiondata = &dm;

    test_dm = sldirectmap_init(false);

    ap.handle.pindex(0);
    rt = test_dm->reserve_index(&ap);
    ASSERT_TRUE(rt == SDK_RET_OK);

    ap.handle.pindex(1);
    rt = test_dm->insert_withid(&ap);
    ASSERT_TRUE(rt == SDK_RET_OK);

    // insert at next available index, return index
    rt = test_dm->insert(&ap);
    ASSERT_TRUE(rt == SDK_RET_OK);

    // insert at next available index, return index
    rt = test_dm->insert(&ap);
    ASSERT_TRUE(rt == SDK_RET_OK);

    // insert at next available index, return index
    rt = test_dm->insert(&ap);
    ASSERT_TRUE(rt == SDK_RET_OK);

    // insert at next available index, return index
    rt = test_dm->insert(&ap);
    ASSERT_TRUE(rt == SDK_RET_OK);

    // insert at next available index, return index
    rt = test_dm->reserve(&ap);
    ASSERT_TRUE(rt == SDK_RET_OK);

    ap.handle.pindex(2);
    ap.actiondata = &dm;
    rt = test_dm->insert_atid(&ap);
    ASSERT_TRUE(rt == SDK_RET_OK);

    ap.handle.pindex(8);
    ap.actiondata = &dm;
    rt = test_dm->insert_withid(&ap);
    ASSERT_TRUE(rt == SDK_RET_OK);
}

/* -----------------------------------------------------------------------------
 *
 * Test Case 2:
 *      - Test Case to verify the scale of inserts
 * - Create DM table for 100 entries
 * - Insert 101 DM Entry. Has to fail for 101st entry
 *
 * ---------------------------------------------------------------------------*/
TEST_F(sldm_test, test2) {

    p4pd_table_properties_t props;
    sldirectmap *test_dm;
    output_mapping_actiondata dm;
    sdk_table_api_params_t ap;
    sdk_ret_t rt;

    test_dm = sldirectmap_init(false);
    dm.actionid = 1;
    ap.actiondata = &dm;

    p4pd_global_table_properties_get(P4TBL_ID_OUTPUT_MAPPING,  &props);

    for (uint32_t i = 0; i < props.tabledepth + 1; i++) {
        rt = test_dm->insert(&ap);
        if (i < props.tabledepth) {
            ASSERT_TRUE(rt == SDK_RET_OK);
        } else {
            ASSERT_TRUE(rt == SDK_RET_NO_RESOURCE);
            // Remove one entry and try to insert again
            ap.handle.pindex(props.tabledepth - 1);
            rt = test_dm->remove(&ap);
            ASSERT_TRUE(rt == SDK_RET_OK);

            rt = test_dm->insert(&ap);
            ASSERT_TRUE(rt == SDK_RET_OK);
        }
    }
}

/* -----------------------------------------------------------------------------
 *
 * Test Case 3:
 *      - Test Case to verify the insert
 * - Create DM table
 * - Update DM Entry
 * - -ve Update cases
 *
 * ---------------------------------------------------------------------------*/
TEST_F(sldm_test, test3) {

    p4pd_table_properties_t props;
    sldirectmap *test_dm;
    output_mapping_actiondata dm;
    sdk_table_api_params_t ap;
    sdk_ret_t rt;
    sdk_table_api_stats_t stats;

    test_dm = sldirectmap_init(false);
    dm.actionid = 1;
    ap.actiondata = &dm;

    p4pd_global_table_properties_get(P4TBL_ID_OUTPUT_MAPPING,  &props);

    rt = test_dm->insert(&ap);
    ASSERT_TRUE(rt == SDK_RET_OK);

    rt = test_dm->update(&ap);
    ASSERT_TRUE(rt == SDK_RET_OK);

    ap.handle.pindex(1);
    rt = test_dm->update(&ap);
    ASSERT_TRUE(rt == SDK_RET_ENTRY_NOT_FOUND);

    ap.handle.pindex(props.tabledepth + 1);
    rt = test_dm->update(&ap);
    ASSERT_TRUE(rt == SDK_RET_NO_RESOURCE);

    test_dm->stats_get(&stats);

    ASSERT_TRUE(stats.insert == 1);
    ASSERT_TRUE(stats.update == 1);
    ASSERT_TRUE(stats.update_fail == 2);
}

/* -----------------------------------------------------------------------------
 *
 * Test Case 4:
 *      - Test Case to verify the insert
 * - Create DM table
 * - Update DM Entry
 * - -ve Update cases
 *
 * ---------------------------------------------------------------------------*/
TEST_F(sldm_test, test4) {

    p4pd_table_properties_t props;
    sldirectmap *test_dm;
    output_mapping_actiondata dm;
    sdk_table_api_params_t ap;
    sdk_ret_t rt;
    uint32_t index;

    test_dm = sldirectmap_init(false);
    dm.actionid = 1;
    ap.actiondata = &dm;

    p4pd_global_table_properties_get(P4TBL_ID_OUTPUT_MAPPING,  &props);

    rt = test_dm->insert(&ap);
    ASSERT_TRUE(rt == SDK_RET_OK);
    index = ap.handle.pindex();

    rt = test_dm->update(&ap);
    ASSERT_TRUE(rt == SDK_RET_OK);

    ap.handle.pindex(10);
    rt = test_dm->remove(&ap);
    ASSERT_TRUE(rt == SDK_RET_ENTRY_NOT_FOUND);

    ap.handle.pindex(props.tabledepth + 1);
    rt = test_dm->remove(&ap);
    ASSERT_TRUE(rt == SDK_RET_NO_RESOURCE);

    ap.handle.pindex(index);
    rt = test_dm->remove(&ap);
    ASSERT_TRUE(rt == SDK_RET_OK);

    rt = test_dm->remove(&ap);
    ASSERT_TRUE(rt == SDK_RET_ENTRY_NOT_FOUND);
}

/* -----------------------------------------------------------------------------
 *
 * Test Case 5:
 *      - Test Case to verify the insert
 * - Create DM table
 * - Update DM Entry
 * - -ve Retrive cases
 *
 * ---------------------------------------------------------------------------*/
TEST_F(sldm_test, test5) {

    p4pd_table_properties_t props;
    sldirectmap *test_dm;
    output_mapping_actiondata dm;
    sdk_table_api_params_t ap;
    sdk_ret_t rt;
    uint32_t index;

    test_dm = sldirectmap_init(false);
    dm.actionid = 1;
    ap.actiondata = &dm;

    p4pd_global_table_properties_get(P4TBL_ID_OUTPUT_MAPPING,  &props);

    rt = test_dm->insert(&ap);
    ASSERT_TRUE(rt == SDK_RET_OK);
    index = ap.handle.pindex();

    rt = test_dm->update(&ap);
    ASSERT_TRUE(rt == SDK_RET_OK);

    ap.handle.pindex(10);
    rt = test_dm->get(&ap);
    ASSERT_TRUE(rt == SDK_RET_ENTRY_NOT_FOUND);

    ap.handle.pindex(props.tabledepth + 1);
    rt = test_dm->get(&ap);
    ASSERT_TRUE(rt == SDK_RET_NO_RESOURCE);

    ap.handle.pindex(index);
    rt = test_dm->get(&ap);
    ASSERT_TRUE(rt == SDK_RET_OK);
}

/* -----------------------------------------------------------------------------
 *
 * Test Case 6:
 *      - Test Case to verify the insert
 * - Create DM table
 * - Iterate test
 *
 * ---------------------------------------------------------------------------*/
static void
print_fn(sdk_table_api_params_t *ap)
{
    SDK_TRACE_VERBOSE("Callback , index %u\n", ap->handle.pindex());
}

TEST_F(sldm_test, test6) {

    sldirectmap *test_dm;
    output_mapping_actiondata dm;
    sdk_table_api_params_t ap;
    sdk_ret_t rt;

    test_dm = sldirectmap_init(false);
    dm.actionid = 1;
    ap.actiondata = &dm;

    rt = test_dm->insert(&ap);
    ASSERT_TRUE(rt == SDK_RET_OK);

    rt = test_dm->insert(&ap);
    ASSERT_TRUE(rt == SDK_RET_OK);

    ap.itercb = print_fn;
    rt = test_dm->iterate(&ap);
    ASSERT_TRUE(rt == SDK_RET_OK);
}

/* -----------------------------------------------------------------------------
 *
 * Test Case 7:
 *      - Test Case to verify the scale of inserts
 * - Create DM table for 25 entries
 * - Insert 25
 * - Remove 25
 * - Insert 25
 *
 * ---------------------------------------------------------------------------*/
TEST_F(sldm_test, test7) {

    p4pd_table_properties_t props;
    sldirectmap *test_dm;
    output_mapping_actiondata dm;
    sdk_table_api_params_t ap;
    sdk_ret_t rt;
    uint32_t *index;

    test_dm = sldirectmap_init(false);
    dm.actionid = 1;
    ap.actiondata = &dm;

    p4pd_global_table_properties_get(P4TBL_ID_OUTPUT_MAPPING,  &props);

    index = (uint32_t *)malloc(props.tabledepth * sizeof(uint32_t));
    ASSERT_TRUE(index);

    for (uint32_t i = 0; i < 25; i++) {
        rt = test_dm->insert(&ap);
        index[i] = ap.handle.pindex();
	ASSERT_TRUE(rt == SDK_RET_OK);
    }
    for (uint32_t i = 0; i < 25; i++) {
        ap.handle.pindex(index[i]);
        rt = test_dm->remove(&ap);
        ASSERT_TRUE(rt == SDK_RET_OK);
    }
    for (uint32_t i = 0; i < 25; i++) {
        rt = test_dm->insert(&ap);
        index[i] = ap.handle.pindex();
	ASSERT_TRUE(rt == SDK_RET_OK);
    }
    for (uint32_t i = 0; i < 25; i++) {
        ap.handle.pindex(index[i]);
        rt = test_dm->remove(&ap);
	ASSERT_TRUE(rt == SDK_RET_OK);
    }
    free(index);
}

/* -----------------------------------------------------------------------------
 *
 * Test Case 8:
 *      - Test Case to verify the insert withid
 * - Create DM table
 * - Insert DM Entry
 * - Insert DM Entry withid
 *
 * ---------------------------------------------------------------------------*/
TEST_F(sldm_test, test8) {

    p4pd_table_properties_t props;
    sldirectmap *test_dm;
    output_mapping_actiondata dm;
    sdk_table_api_params_t ap;
    sdk_ret_t rt;
    uint32_t index;

    test_dm = sldirectmap_init(false);
    dm.actionid = 1;
    ap.actiondata = &dm;

    p4pd_global_table_properties_get(P4TBL_ID_OUTPUT_MAPPING,  &props);

    rt = test_dm->insert(&ap);
    ASSERT_TRUE(rt == SDK_RET_OK);
    index = ap.handle.pindex();

    ap.handle.pindex(1);
    rt = test_dm->insert_withid(&ap);
    ASSERT_TRUE(rt == SDK_RET_OK);

    rt = test_dm->insert_withid(&ap);
    ASSERT_TRUE(rt == SDK_RET_ENTRY_EXISTS);

    ap.handle.pindex(index);
    rt = test_dm->remove(&ap);
    ASSERT_TRUE(rt == SDK_RET_OK);

    ap.handle.pindex(1);
    rt = test_dm->remove(&ap);
    ASSERT_TRUE(rt == SDK_RET_OK);

    ap.handle.pindex(props.tabledepth+1);
    rt = test_dm->insert_withid(&ap);
    ASSERT_TRUE(rt == SDK_RET_NO_RESOURCE);
}

/* -----------------------------------------------------------------------------
 *
 * Test Case 9:
 *      - Test Case to verify stats
 *
 * ---------------------------------------------------------------------------*/
TEST_F(sldm_test, test9) {

    p4pd_table_properties_t props;
    sldirectmap *test_dm;
    output_mapping_actiondata dm;
    sdk_table_api_params_t ap;
    sdk_ret_t rt;
    sdk_table_api_stats_t stats;

    test_dm = sldirectmap_init(false);
    dm.actionid = 1;
    ap.actiondata = &dm;

    p4pd_global_table_properties_get(P4TBL_ID_OUTPUT_MAPPING,  &props);

    for (uint32_t i = 0; i < props.tabledepth + 2; i++) {
        ap.handle.pindex(i);
        rt = test_dm->insert(&ap);
        if (i < props.tabledepth) {
            ASSERT_TRUE(rt == SDK_RET_OK);
        } else {
            ASSERT_TRUE(rt == SDK_RET_NO_RESOURCE);
        }
    }
    test_dm->stats_get(&stats);

    ASSERT_TRUE(stats.insert == 100);
    ASSERT_TRUE(stats.insert_fail == 2);


    ap.handle.pindex(0);
    rt = test_dm->remove(&ap);
    ASSERT_TRUE(rt == SDK_RET_OK);

    rt = test_dm->insert_withid(&ap);
    ASSERT_TRUE(rt == SDK_RET_OK);

    rt = test_dm->insert_withid(&ap);
    ASSERT_TRUE(rt == SDK_RET_ENTRY_EXISTS);

    ap.handle.pindex(props.tabledepth + 1000);
    rt = test_dm->insert_withid(&ap);
    ASSERT_TRUE(rt == SDK_RET_NO_RESOURCE);

    ap.handle.pindex(0);
    rt = test_dm->update(&ap);
    ASSERT_TRUE(rt == SDK_RET_OK);

    rt = test_dm->remove(&ap);
    ASSERT_TRUE(rt == SDK_RET_OK);

    ap.handle.pindex(props.tabledepth + 1000);
    rt = test_dm->update(&ap);
    ASSERT_TRUE(rt == SDK_RET_NO_RESOURCE);

    ap.handle.pindex(0);
    rt = test_dm->update(&ap);
    ASSERT_TRUE(rt == SDK_RET_ENTRY_NOT_FOUND);

    ap.handle.pindex(props.tabledepth + 1000);
    rt = test_dm->remove(&ap);
    ASSERT_TRUE(rt == SDK_RET_NO_RESOURCE);

    ap.handle.pindex(0);
    rt = test_dm->remove(&ap);
    ASSERT_TRUE(rt == SDK_RET_ENTRY_NOT_FOUND);

    test_dm->stats_get(&stats);
    ASSERT_TRUE(stats.insert == 101);
    ASSERT_TRUE(stats.insert_duplicate == 1);
    ASSERT_TRUE(stats.insert_fail == 3);
    ASSERT_TRUE(stats.remove == 2);
    ASSERT_TRUE(stats.remove_fail == 1);
    ASSERT_TRUE(stats.remove_not_found == 1);
    ASSERT_TRUE(stats.update  == 1);
    ASSERT_TRUE(stats.update_fail= 2);
}

/*
 * Test Case 10;
 *  - Testing shared direct map
 */
TEST_F(sldm_test, DISABLED_test10) {

    p4pd_table_properties_t props;
    sldirectmap *test_dm;
    twice_nat_actiondata    data = { 0 };
    sdk_table_api_params_t ap;
    sdk_ret_t ret;
    uint32_t index;

    test_dm = sldirectmap_init(true);
    data.actionid = TWICE_NAT_TWICE_NAT_REWRITE_INFO_ID;
    data.twice_nat_action_u.twice_nat_twice_nat_rewrite_info.l4_port = 10;
    ap.actiondata = &data;
    ap.handle.pindex(0);

    p4pd_global_table_properties_get(P4TBL_ID_OUTPUT_MAPPING,  &props);

    // Insert an entry
    ret = test_dm->insert(&ap);
    ASSERT_TRUE(ret == SDK_RET_OK);

    // Delete the entry
    ret = test_dm->remove(&ap);
    ASSERT_TRUE(ret == SDK_RET_OK);

    // Insert an entry
    ret = test_dm->insert(&ap);
    ASSERT_TRUE(ret == SDK_RET_OK);

    // Insert an entry
    ret = test_dm->insert(&ap);
    ASSERT_TRUE(ret == SDK_RET_OK);

    // Delete an entry - Should not delete from HW & SW
    ret = test_dm->remove(&ap);
    ASSERT_TRUE(ret == SDK_RET_OK);

    // Delete an entry - Should delete from HW
    ret = test_dm->remove(&ap);
    SDK_TRACE_DEBUG("ret:%d", ret);
    ASSERT_TRUE(ret == SDK_RET_OK);

    // Insert with id
    ap.handle.pindex(10);
    ret = test_dm->insert_withid(&ap);
    ASSERT_TRUE(ret == SDK_RET_OK);
    // Insert with id
    ret = test_dm->insert_withid(&ap);
    ASSERT_TRUE(ret == SDK_RET_OK);
    // Delete
    ret = test_dm->remove(&ap);
    SDK_TRACE_DEBUG("ret:%d", ret);
    ASSERT_TRUE(ret == SDK_RET_OK);
    // Delete
    ret = test_dm->remove(&ap);
    SDK_TRACE_DEBUG("ret:%d", ret);
    ASSERT_TRUE(ret == SDK_RET_OK);

    // Insert
    ret = test_dm->insert(&ap);
    index = ap.handle.pindex();
    ASSERT_TRUE(ret == SDK_RET_OK);
    // Insert with id
    ap.handle.pindex(20);
    data.actionid = 0;
    ret = test_dm->insert_withid(&ap);
    SDK_TRACE_DEBUG("ret:{}", ret);
    ASSERT_TRUE(ret == SDK_RET_OK);
    // Delete
    ap.handle.pindex(index);
    ret = test_dm->remove(&ap);
    ASSERT_TRUE(ret == SDK_RET_OK);
    // Delete
    data.actionid = TWICE_NAT_TWICE_NAT_REWRITE_INFO_ID;
    ret = test_dm->remove(&ap);
    ASSERT_TRUE(ret == SDK_RET_OK);
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
