//------------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//
// test cases to verify TCAM table management library
//------------------------------------------------------------------------------

#include "gtest/gtest.h"
#include <stdio.h>
#include "sdk/directmap.hpp"
#include "sdk/base.hpp"


using sdk::table::directmap;
using sdk::sdk_ret_t;
using namespace sdk;

// class dm_test : public hal_base_test {
class dm_test : public ::testing::Test {
protected:
    dm_test() {
  }

  virtual ~dm_test() {
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


/* -----------------------------------------------------------------------------
 *
 * Test Case 1:
 *      - Test Case to verify the insert
 * - Create DM table
 * - Insert DM Entry
 *
 * ---------------------------------------------------------------------------*/
TEST_F(dm_test, test1) {

    std::string table_name = "Output_Mapping";
    char * table_str = const_cast<char*> (table_name.c_str());
    directmap *test_dm = directmap::factory(table_str, P4TBL_ID_OUTPUT_MAPPING, 100,
            sizeof(output_mapping_actiondata));

    output_mapping_actiondata dm;
    uint32_t index = 0;
    dm.actionid = 1;

    sdk_ret_t rt;
    rt = test_dm->insert(&dm, &index); 
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
TEST_F(dm_test, test2) {

    std::string table_name = "Output_Mapping";
    char * table_str = const_cast<char*> (table_name.c_str());
    directmap *test_dm = directmap::factory(table_str, P4TBL_ID_OUTPUT_MAPPING, 100,
            sizeof(output_mapping_actiondata));

    output_mapping_actiondata dm;
    uint32_t index = 0;
    dm.actionid = 1;

    sdk_ret_t rt;
    for (uint32_t i = 0; i < 101; i++) {
        rt = test_dm->insert(&dm, &index); 
        if (i < 100) {
            ASSERT_TRUE(rt == SDK_RET_OK);
        } else {
            ASSERT_TRUE(rt == SDK_RET_NO_RESOURCE);
            // Remove one entry and try to insert again
            rt = test_dm->remove(99);
            ASSERT_TRUE(rt == SDK_RET_OK);

            rt = test_dm->insert(&dm, &index); 
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
TEST_F(dm_test, test3) {

    std::string table_name = "Output_Mapping";
    char * table_str = const_cast<char*> (table_name.c_str());
    directmap *test_dm = directmap::factory(table_str, P4TBL_ID_OUTPUT_MAPPING, 100,
            sizeof(output_mapping_actiondata));

    output_mapping_actiondata dm;
    uint32_t index = 0;
    const uint64_t *stats;
    dm.actionid = 1;

    sdk_ret_t rt;
    rt = test_dm->insert(&dm, &index); 
    ASSERT_TRUE(rt == SDK_RET_OK);

    rt = test_dm->update(index, &dm); 
    ASSERT_TRUE(rt == SDK_RET_OK);

    rt = test_dm->update(index + 1, &dm); 
    ASSERT_TRUE(rt == SDK_RET_ENTRY_NOT_FOUND);
	
    rt = test_dm->update(index + 1000, &dm); 
    ASSERT_TRUE(rt == SDK_RET_INVALID_ARG);

    rt = test_dm->fetch_stats(&stats);
    ASSERT_TRUE(rt == SDK_RET_OK);

#if 0
    printf("Insert SUCCESS: %lu, UPD SUCC: %lu, FAIL_ENF: %lu, FAIL_INV: %lu\n",
            stats[directmap::STATS_INS_SUCCESS],
            stats[directmap::STATS_UPD_SUCCESS],
            stats[directmap::STATS_UPD_FAIL_ENTRY_NOT_FOUND],
            stats[directmap::STATS_UPD_FAIL_INV_ARG]);
#endif
    ASSERT_TRUE(stats[directmap::STATS_INS_SUCCESS] == 1);
    ASSERT_TRUE(stats[directmap::STATS_UPD_SUCCESS] == 1);
    ASSERT_TRUE(stats[directmap::STATS_UPD_FAIL_ENTRY_NOT_FOUND] == 1);
    ASSERT_TRUE(stats[directmap::STATS_UPD_FAIL_INV_ARG] == 1);
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
TEST_F(dm_test, test4) {

    std::string table_name = "Output_Mapping";
    char * table_str = const_cast<char*> (table_name.c_str());
    directmap *test_dm = directmap::factory(table_str, P4TBL_ID_OUTPUT_MAPPING, 100,
            sizeof(output_mapping_actiondata));

    output_mapping_actiondata dm;
    uint32_t index = 0;
    dm.actionid = 1;

    sdk_ret_t rt;
    rt = test_dm->insert(&dm, &index); 
    ASSERT_TRUE(rt == SDK_RET_OK);

    rt = test_dm->update(index, &dm); 
    ASSERT_TRUE(rt == SDK_RET_OK);

    rt = test_dm->remove(index + 1);
    ASSERT_TRUE(rt == SDK_RET_ENTRY_NOT_FOUND);
	
    rt = test_dm->remove(index + 1000);
    ASSERT_TRUE(rt == SDK_RET_INVALID_ARG);

    rt = test_dm->remove(index);
    ASSERT_TRUE(rt == SDK_RET_OK);

    rt = test_dm->remove(index);
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
TEST_F(dm_test, test5) {

    std::string table_name = "Output_Mapping";
    char * table_str = const_cast<char*> (table_name.c_str());
    directmap *test_dm = directmap::factory(table_str, P4TBL_ID_OUTPUT_MAPPING, 100,
            sizeof(output_mapping_actiondata));

    output_mapping_actiondata dm, retr_dm;
    uint32_t index = 0;
    dm.actionid = 1;

    sdk_ret_t rt;
    rt = test_dm->insert(&dm, &index); 
    ASSERT_TRUE(rt == SDK_RET_OK);

    rt = test_dm->update(index, &dm); 
    ASSERT_TRUE(rt == SDK_RET_OK);

    rt = test_dm->retrieve(index + 1, &retr_dm);
    ASSERT_TRUE(rt == SDK_RET_ENTRY_NOT_FOUND);
	
    rt = test_dm->retrieve(index + 1000, &retr_dm);
    ASSERT_TRUE(rt == SDK_RET_INVALID_ARG);

    rt = test_dm->retrieve(index, &retr_dm);
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
bool print_fn(uint32_t index, void *data, const void *cb_data)
{
	return TRUE;
}
TEST_F(dm_test, test6) {

    std::string table_name = "Output_Mapping";
    char * table_str = const_cast<char*> (table_name.c_str());
    directmap *test_dm = directmap::factory(table_str, P4TBL_ID_OUTPUT_MAPPING, 100,
            sizeof(output_mapping_actiondata));

    output_mapping_actiondata dm;
    uint32_t index = 0;
    dm.actionid = 1;

    sdk_ret_t rt;
    rt = test_dm->insert(&dm, &index); 
    ASSERT_TRUE(rt == SDK_RET_OK);

	rt = test_dm->iterate(print_fn, NULL);
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
TEST_F(dm_test, test7) {

    std::string table_name = "Output_Mapping";
    char * table_str = const_cast<char*> (table_name.c_str());
    directmap *test_dm = directmap::factory(table_str, P4TBL_ID_OUTPUT_MAPPING, 25,
            sizeof(output_mapping_actiondata));

    output_mapping_actiondata dm;
    uint32_t index[25] = {0};
    dm.actionid = 1;

    sdk_ret_t rt;
    for (uint32_t i = 0; i < 25; i++) {
        rt = test_dm->insert(&dm, &index[i]); 
		ASSERT_TRUE(rt == SDK_RET_OK);
    }
    for (uint32_t i = 0; i < 25; i++) {
        rt = test_dm->remove(index[i]); 
		ASSERT_TRUE(rt == SDK_RET_OK);
    }
    for (uint32_t i = 0; i < 25; i++) {
        rt = test_dm->insert(&dm, &index[i]); 
		ASSERT_TRUE(rt == SDK_RET_OK);
    }
    for (uint32_t i = 0; i < 25; i++) {
        rt = test_dm->remove(index[i]); 
		ASSERT_TRUE(rt == SDK_RET_OK);
    }

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
TEST_F(dm_test, test8) {

    std::string table_name = "Output_Mapping";
    char * table_str = const_cast<char*> (table_name.c_str());
    directmap *test_dm = directmap::factory(table_str, P4TBL_ID_OUTPUT_MAPPING, 100,
            sizeof(output_mapping_actiondata));

    output_mapping_actiondata dm;
    uint32_t index = 0;
    dm.actionid = 1;

    sdk_ret_t rt;
    rt = test_dm->insert(&dm, &index); 
    ASSERT_TRUE(rt == SDK_RET_OK);

    rt = test_dm->insert_withid(&dm, index+1); 
    ASSERT_TRUE(rt == SDK_RET_OK);

    rt = test_dm->insert_withid(&dm, index+1); 
    ASSERT_TRUE(rt == SDK_RET_DUPLICATE_INS);

    rt = test_dm->remove(index); 
    ASSERT_TRUE(rt == SDK_RET_OK);

    rt = test_dm->remove(index+1); 
    ASSERT_TRUE(rt == SDK_RET_OK);

    rt = test_dm->insert_withid(&dm, 100); 
    ASSERT_TRUE(rt == SDK_RET_OOB);
}

/* -----------------------------------------------------------------------------
 *
 * Test Case 9:
 *      - Test Case to verify stats
 *
 * ---------------------------------------------------------------------------*/
TEST_F(dm_test, test9) {

    std::string table_name = "Output_Mapping";
    char * table_str = const_cast<char*> (table_name.c_str());
    directmap *test_dm = directmap::factory(table_str, P4TBL_ID_OUTPUT_MAPPING, 100,
            sizeof(output_mapping_actiondata));

    output_mapping_actiondata dm;
    uint32_t index = 0;
    const uint64_t *stats;
    dm.actionid = 1;

    sdk_ret_t rt;
    for (uint32_t i = 0; i < 101; i++) {
        rt = test_dm->insert(&dm, &index); 
        if (i < 100) {
            ASSERT_TRUE(rt == SDK_RET_OK);
        } else {
            ASSERT_TRUE(rt == SDK_RET_NO_RESOURCE);
        }
    }
    rt = test_dm->fetch_stats(&stats);
    ASSERT_TRUE(rt == SDK_RET_OK);

    ASSERT_TRUE(stats[directmap::STATS_INS_SUCCESS] == 100);
    ASSERT_TRUE(stats[directmap::STATS_INS_FAIL_NO_RES] == 1);


    rt = test_dm->remove(index); 
    ASSERT_TRUE(rt == SDK_RET_OK);

    rt = test_dm->insert_withid(&dm, index); 
    ASSERT_TRUE(rt == SDK_RET_OK);

    rt = test_dm->insert_withid(&dm, index); 
    ASSERT_TRUE(rt == SDK_RET_DUPLICATE_INS);

    rt = test_dm->insert_withid(&dm, 1000); 
    ASSERT_TRUE(rt == SDK_RET_OOB);


    rt = test_dm->update(index, &dm); 
    ASSERT_TRUE(rt == SDK_RET_OK);
    
    rt = test_dm->remove(index); 
    ASSERT_TRUE(rt == SDK_RET_OK);

    rt = test_dm->update(1000, &dm); 
    ASSERT_TRUE(rt == SDK_RET_INVALID_ARG);

    rt = test_dm->update(index, &dm); 
    ASSERT_TRUE(rt == SDK_RET_ENTRY_NOT_FOUND);


    rt = test_dm->remove(1000); 
    ASSERT_TRUE(rt == SDK_RET_INVALID_ARG);

    rt = test_dm->remove(index); 
    ASSERT_TRUE(rt == SDK_RET_ENTRY_NOT_FOUND);

    ASSERT_TRUE(stats[directmap::STATS_INS_SUCCESS] == 100);
    ASSERT_TRUE(stats[directmap::STATS_INS_FAIL_NO_RES] == 1);
    ASSERT_TRUE(stats[directmap::STATS_REM_SUCCESS] == 2);
    ASSERT_TRUE(stats[directmap::STATS_INS_WITHID_SUCCESS] == 1);
    ASSERT_TRUE(stats[directmap::STATS_INS_WITHID_FAIL_DUP_INS] == 1);
    ASSERT_TRUE(stats[directmap::STATS_INS_WITHID_FAIL_OOB] == 1);
    ASSERT_TRUE(stats[directmap::STATS_UPD_SUCCESS] == 1);
    ASSERT_TRUE(stats[directmap::STATS_UPD_FAIL_INV_ARG] == 1);
    ASSERT_TRUE(stats[directmap::STATS_UPD_FAIL_ENTRY_NOT_FOUND] == 1);
    ASSERT_TRUE(stats[directmap::STATS_REM_FAIL_INV_ARG] == 1);
    ASSERT_TRUE(stats[directmap::STATS_REM_FAIL_ENTRY_NOT_FOUND] == 1);

    SDK_TRACE_DEBUG("tableid:%d, table_name:%s, capacity:%d, num_in_use:%d, "
                    "num_inserts:%d, num_insert_errors:%d, "
                    "num_deletes:%d, num_delete_errors:%d\n",
                    test_dm->id(), test_dm->name(),
                    test_dm->capacity(), test_dm->num_entries_in_use(),
                    test_dm->num_inserts(), test_dm->num_insert_errors(),
                    test_dm->num_deletes(), test_dm->num_delete_errors());
}

/*
 * Test Case 10;
 *  - Testing shared direct map
 */
TEST_F(dm_test, test10) {
    std::string table_name = "Twice_Nat";
    char * table_str = const_cast<char*> (table_name.c_str());
    directmap *test_dm = directmap::factory(table_str, P4TBL_ID_TWICE_NAT, 100,
            sizeof(twice_nat_actiondata), true);

    twice_nat_actiondata    data = { 0 };
    uint32_t    index = 0;
    sdk_ret_t   ret;

    // hal::pd::hal_pd_mem_init();

    data.actionid = TWICE_NAT_TWICE_NAT_REWRITE_INFO_ID;
    data.twice_nat_action_u.twice_nat_twice_nat_rewrite_info.l4_port = 10;

    // Insert an entry
    ret = test_dm->insert(&data, &index);
    ASSERT_TRUE(ret == SDK_RET_OK);

    // Delete the entry
    ret = test_dm->remove(0, &data);
    ASSERT_TRUE(ret == SDK_RET_OK);

    // Insert an entry
    ret = test_dm->insert(&data, &index);
    ASSERT_TRUE(ret == SDK_RET_OK);

    // Insert an entry
    ret = test_dm->insert(&data, &index);
    ASSERT_TRUE(ret == SDK_RET_OK);

    // Delete an entry - Should not delete from HW & SW
    ret = test_dm->remove(0, &data);
    ASSERT_TRUE(ret == SDK_RET_OK);

    // Delete an entry - Should delete from HW
    ret = test_dm->remove(0, &data);
    SDK_TRACE_DEBUG("ret:%d", ret);
    ASSERT_TRUE(ret == SDK_RET_OK);

    // Insert with id
    ret = test_dm->insert_withid(&data, 10);
    ASSERT_TRUE(ret == SDK_RET_OK);
    // Insert with id
    ret = test_dm->insert_withid(&data, 10);
    ASSERT_TRUE(ret == SDK_RET_OK);
    // Delete
    ret = test_dm->remove(0, &data);
    ASSERT_TRUE(ret == SDK_RET_OK);
    // Delete
    ret = test_dm->remove(0, &data);
    SDK_TRACE_DEBUG("ret:{}", ret);
    ASSERT_TRUE(ret == SDK_RET_OK);

    // Insert
    ret = test_dm->insert(&data, &index);
    ASSERT_TRUE(ret == SDK_RET_OK);
    // Insert with id
    data.actionid = 0;
    ret = test_dm->insert_withid(&data, 20);
    SDK_TRACE_DEBUG("ret:{}", ret);
    ASSERT_TRUE(ret == SDK_RET_OK);
    // Delete
    ret = test_dm->remove(0, &data);
    ASSERT_TRUE(ret == SDK_RET_OK);
    // Delete
    data.actionid = TWICE_NAT_TWICE_NAT_REWRITE_INFO_ID;
    ret = test_dm->remove(0, &data);
    ASSERT_TRUE(ret == SDK_RET_OK);
}

// TODO: Bharat: Uncomment this once directmap is moved to SDK
#if 0
/*
 * Test Case 11;
 *  - Testing shared direct map
 */
TEST_F(dm_test, test11) {

    sdk_ret_t                   ret;
    hal::pd::pd_twice_nat_entry_args_t   args;
    uint32_t                    nat_idx = 0;

    args.nat_l4_port = 100;
    args.nat_ip.af = IP_AF_IPV4;
    args.nat_ip.addr.v4_addr = 0x0a000003;
    args.twice_nat_act = TWICE_NAT_TWICE_NAT_REWRITE_INFO_ID;

    ret = pd_twice_nat_add(&args, &nat_idx);
    ASSERT_TRUE(ret == SDK_RET_OK);

    ret = pd_twice_nat_add(&args, &nat_idx);
    ASSERT_TRUE(ret == SDK_RET_OK);

    ret = pd_twice_nat_del(&args);
    ASSERT_TRUE(ret == SDK_RET_OK);

    ret = pd_twice_nat_del(&args);
    ASSERT_TRUE(ret == SDK_RET_OK);

}
#endif
int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
