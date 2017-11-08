#include <gtest/gtest.h>
#include "nic/hal/pd/utils/directmap/directmap.hpp"
#include "nic/hal/pd/p4pd_api.hpp"
#include "nic/gen/iris/include/p4pd.h"
#include "nic/hal/pd/hal_pd.hpp"
#include "nic/hal/test/utils/hal_test_utils.hpp"
#include "nic/hal/test/utils/hal_base_test.hpp"
#include "nic/include/pd_api.hpp"

using hal::pd::utils::DirectMap;

// class dm_test : public ::testing::Test {
class dm_test : public hal_base_test {
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
      hal_base_test::SetUpTestCase();
      hal_test_utils_slab_disable_delete();
  }

};

typedef struct dm_data_s {
    int p;
    int q;
} dm_data_t;


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
    DirectMap test_dm = DirectMap(table_name, P4TBL_ID_OUTPUT_MAPPING, 100,
            sizeof(output_mapping_actiondata));

    output_mapping_actiondata dm;
    uint32_t index;
    dm.actionid = 1;

    hal_ret_t rt;
    rt = test_dm.insert(&dm, &index); 
    ASSERT_TRUE(rt == HAL_RET_OK);

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
    DirectMap test_dm = DirectMap(table_name, P4TBL_ID_OUTPUT_MAPPING, 100,
            sizeof(output_mapping_actiondata));

    output_mapping_actiondata dm;
    uint32_t index;
    dm.actionid = 1;

    hal_ret_t rt;
    for (uint32_t i = 0; i < 101; i++) {
        rt = test_dm.insert(&dm, &index); 
        if (i < 100) {
            ASSERT_TRUE(rt == HAL_RET_OK);
        } else {
            ASSERT_TRUE(rt == HAL_RET_NO_RESOURCE);
            // Remove one entry and try to insert again
            rt = test_dm.remove(99);
            ASSERT_TRUE(rt == HAL_RET_OK);

            rt = test_dm.insert(&dm, &index); 
            ASSERT_TRUE(rt == HAL_RET_OK);
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
    DirectMap test_dm = DirectMap(table_name, P4TBL_ID_OUTPUT_MAPPING, 100,
            sizeof(output_mapping_actiondata));

    output_mapping_actiondata dm;
    uint32_t index;
    const uint64_t *stats;
    dm.actionid = 1;

    hal_ret_t rt;
    rt = test_dm.insert(&dm, &index); 
    ASSERT_TRUE(rt == HAL_RET_OK);

    rt = test_dm.update(index, &dm); 
    ASSERT_TRUE(rt == HAL_RET_OK);

    rt = test_dm.update(index + 1, &dm); 
    ASSERT_TRUE(rt == HAL_RET_ENTRY_NOT_FOUND);
	
    rt = test_dm.update(index + 1000, &dm); 
    ASSERT_TRUE(rt == HAL_RET_INVALID_ARG);

    rt = test_dm.fetch_stats(&stats);
    ASSERT_TRUE(rt == HAL_RET_OK);

#if 0
    printf("Insert SUCCESS: %lu, UPD SUCC: %lu, FAIL_ENF: %lu, FAIL_INV: %lu\n",
            stats[DirectMap::STATS_INS_SUCCESS],
            stats[DirectMap::STATS_UPD_SUCCESS],
            stats[DirectMap::STATS_UPD_FAIL_ENTRY_NOT_FOUND],
            stats[DirectMap::STATS_UPD_FAIL_INV_ARG]);
#endif
    ASSERT_TRUE(stats[DirectMap::STATS_INS_SUCCESS] == 1);
    ASSERT_TRUE(stats[DirectMap::STATS_UPD_SUCCESS] == 1);
    ASSERT_TRUE(stats[DirectMap::STATS_UPD_FAIL_ENTRY_NOT_FOUND] == 1);
    ASSERT_TRUE(stats[DirectMap::STATS_UPD_FAIL_INV_ARG] == 1);
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
    DirectMap test_dm = DirectMap(table_name, P4TBL_ID_OUTPUT_MAPPING, 100,
            sizeof(output_mapping_actiondata));

    output_mapping_actiondata dm;
    uint32_t index;
    dm.actionid = 1;

    hal_ret_t rt;
    rt = test_dm.insert(&dm, &index); 
    ASSERT_TRUE(rt == HAL_RET_OK);

    rt = test_dm.update(index, &dm); 
    ASSERT_TRUE(rt == HAL_RET_OK);

    rt = test_dm.remove(index + 1);
    ASSERT_TRUE(rt == HAL_RET_ENTRY_NOT_FOUND);
	
    rt = test_dm.remove(index + 1000);
    ASSERT_TRUE(rt == HAL_RET_INVALID_ARG);

    rt = test_dm.remove(index);
    ASSERT_TRUE(rt == HAL_RET_OK);

    rt = test_dm.remove(index);
    ASSERT_TRUE(rt == HAL_RET_ENTRY_NOT_FOUND);
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
    DirectMap test_dm = DirectMap(table_name, P4TBL_ID_OUTPUT_MAPPING, 100,
            sizeof(output_mapping_actiondata));

    output_mapping_actiondata dm, retr_dm;
    uint32_t index;
    dm.actionid = 1;

    hal_ret_t rt;
    rt = test_dm.insert(&dm, &index); 
    ASSERT_TRUE(rt == HAL_RET_OK);

    rt = test_dm.update(index, &dm); 
    ASSERT_TRUE(rt == HAL_RET_OK);

    rt = test_dm.retrieve(index + 1, &retr_dm);
    ASSERT_TRUE(rt == HAL_RET_ENTRY_NOT_FOUND);
	
    rt = test_dm.retrieve(index + 1000, &retr_dm);
    ASSERT_TRUE(rt == HAL_RET_INVALID_ARG);

    rt = test_dm.retrieve(index, &retr_dm);
    ASSERT_TRUE(rt == HAL_RET_OK);
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
    DirectMap test_dm = DirectMap(table_name, P4TBL_ID_OUTPUT_MAPPING, 100,
            sizeof(output_mapping_actiondata));

    output_mapping_actiondata dm;
    uint32_t index;
    dm.actionid = 1;

    hal_ret_t rt;
    rt = test_dm.insert(&dm, &index); 
    ASSERT_TRUE(rt == HAL_RET_OK);

	rt = test_dm.iterate(print_fn, NULL);
    ASSERT_TRUE(rt == HAL_RET_OK);
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
    DirectMap test_dm = DirectMap(table_name, P4TBL_ID_OUTPUT_MAPPING, 25,
            sizeof(output_mapping_actiondata));

    output_mapping_actiondata dm;
    uint32_t index[25];
    dm.actionid = 1;

    hal_ret_t rt;
    for (uint32_t i = 0; i < 25; i++) {
        rt = test_dm.insert(&dm, &index[i]); 
		ASSERT_TRUE(rt == HAL_RET_OK);
    }
    for (uint32_t i = 0; i < 25; i++) {
        rt = test_dm.remove(index[i]); 
		ASSERT_TRUE(rt == HAL_RET_OK);
    }
    for (uint32_t i = 0; i < 25; i++) {
        rt = test_dm.insert(&dm, &index[i]); 
		ASSERT_TRUE(rt == HAL_RET_OK);
    }
    for (uint32_t i = 0; i < 25; i++) {
        rt = test_dm.remove(index[i]); 
		ASSERT_TRUE(rt == HAL_RET_OK);
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
    DirectMap test_dm = DirectMap(table_name, P4TBL_ID_OUTPUT_MAPPING, 100,
            sizeof(output_mapping_actiondata));

    output_mapping_actiondata dm;
    uint32_t index;
    dm.actionid = 1;

    hal_ret_t rt;
    rt = test_dm.insert(&dm, &index); 
    ASSERT_TRUE(rt == HAL_RET_OK);

    rt = test_dm.insert_withid(&dm, index+1); 
    ASSERT_TRUE(rt == HAL_RET_OK);

    rt = test_dm.insert_withid(&dm, index+1); 
    ASSERT_TRUE(rt == HAL_RET_DUP_INS_FAIL);

    rt = test_dm.remove(index); 
    ASSERT_TRUE(rt == HAL_RET_OK);

    rt = test_dm.remove(index+1); 
    ASSERT_TRUE(rt == HAL_RET_OK);

    rt = test_dm.insert_withid(&dm, 100); 
    ASSERT_TRUE(rt == HAL_RET_OOB);
}

/* -----------------------------------------------------------------------------
 *
 * Test Case 9:
 *      - Test Case to verify stats
 *
 * ---------------------------------------------------------------------------*/
TEST_F(dm_test, test9) {

    std::string table_name = "Output_Mapping";
    DirectMap test_dm = DirectMap(table_name, P4TBL_ID_OUTPUT_MAPPING, 100,
            sizeof(output_mapping_actiondata));

    output_mapping_actiondata dm;
    uint32_t index;
    const uint64_t *stats;
    dm.actionid = 1;

    hal_ret_t rt;
    for (uint32_t i = 0; i < 101; i++) {
        rt = test_dm.insert(&dm, &index); 
        if (i < 100) {
            ASSERT_TRUE(rt == HAL_RET_OK);
        } else {
            ASSERT_TRUE(rt == HAL_RET_NO_RESOURCE);
        }
    }
    rt = test_dm.fetch_stats(&stats);
    ASSERT_TRUE(rt == HAL_RET_OK);

    ASSERT_TRUE(stats[DirectMap::STATS_INS_SUCCESS] == 100);
    ASSERT_TRUE(stats[DirectMap::STATS_INS_FAIL_NO_RES] == 1);


    rt = test_dm.remove(index); 
    ASSERT_TRUE(rt == HAL_RET_OK);

    rt = test_dm.insert_withid(&dm, index); 
    ASSERT_TRUE(rt == HAL_RET_OK);

    rt = test_dm.insert_withid(&dm, index); 
    ASSERT_TRUE(rt == HAL_RET_DUP_INS_FAIL);

    rt = test_dm.insert_withid(&dm, 1000); 
    ASSERT_TRUE(rt == HAL_RET_OOB);


    rt = test_dm.update(index, &dm); 
    ASSERT_TRUE(rt == HAL_RET_OK);
    
    rt = test_dm.remove(index); 
    ASSERT_TRUE(rt == HAL_RET_OK);

    rt = test_dm.update(1000, &dm); 
    ASSERT_TRUE(rt == HAL_RET_INVALID_ARG);

    rt = test_dm.update(index, &dm); 
    ASSERT_TRUE(rt == HAL_RET_ENTRY_NOT_FOUND);


    rt = test_dm.remove(1000); 
    ASSERT_TRUE(rt == HAL_RET_INVALID_ARG);

    rt = test_dm.remove(index); 
    ASSERT_TRUE(rt == HAL_RET_ENTRY_NOT_FOUND);

    ASSERT_TRUE(stats[DirectMap::STATS_INS_SUCCESS] == 100);
    ASSERT_TRUE(stats[DirectMap::STATS_INS_FAIL_NO_RES] == 1);
    ASSERT_TRUE(stats[DirectMap::STATS_REM_SUCCESS] == 2);
    ASSERT_TRUE(stats[DirectMap::STATS_INS_WITHID_SUCCESS] == 1);
    ASSERT_TRUE(stats[DirectMap::STATS_INS_WITHID_FAIL_DUP_INS] == 1);
    ASSERT_TRUE(stats[DirectMap::STATS_INS_WITHID_FAIL_OOB] == 1);
    ASSERT_TRUE(stats[DirectMap::STATS_UPD_SUCCESS] == 1);
    ASSERT_TRUE(stats[DirectMap::STATS_UPD_FAIL_INV_ARG] == 1);
    ASSERT_TRUE(stats[DirectMap::STATS_UPD_FAIL_ENTRY_NOT_FOUND] == 1);
    ASSERT_TRUE(stats[DirectMap::STATS_REM_FAIL_INV_ARG] == 1);
    ASSERT_TRUE(stats[DirectMap::STATS_REM_FAIL_ENTRY_NOT_FOUND] == 1);

    HAL_TRACE_DEBUG("tableid:{}, table_name:{}, capacity:{}, num_in_use:{}, "
                    "num_inserts:{}, num_insert_errors:{}, "
                    "num_deletes:{}, num_delete_errors:{}",
                    test_dm.table_id(), test_dm.table_name(),
                    test_dm.table_capacity(), test_dm.table_num_entries_in_use(),
                    test_dm.table_num_inserts(), test_dm.table_num_insert_errors(),
                    test_dm.table_num_deletes(), test_dm.table_num_delete_errors());
}

/*
 * Test Case 10;
 *  - Testing shared direct map
 */
TEST_F(dm_test, test10) {
    std::string table_name = "Twice_Nat";
    DirectMap test_dm = DirectMap(table_name, P4TBL_ID_TWICE_NAT, 100,
            sizeof(twice_nat_actiondata), true, true);

    twice_nat_actiondata    data = { 0 };
    uint32_t    index = 0;
    hal_ret_t   ret;

    // hal::pd::hal_pd_mem_init();

    data.actionid = TWICE_NAT_TWICE_NAT_REWRITE_INFO_ID;
    data.twice_nat_action_u.twice_nat_twice_nat_rewrite_info.l4_port = 10;

    // Insert an entry
    ret = test_dm.insert(&data, &index);
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Delete the entry
    ret = test_dm.remove(0, &data);
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Insert an entry
    ret = test_dm.insert(&data, &index);
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Insert an entry
    ret = test_dm.insert(&data, &index);
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Delete an entry - Should not delete from HW & SW
    ret = test_dm.remove(0, &data);
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Delete an entry - Should delete from HW
    ret = test_dm.remove(0, &data);
    HAL_TRACE_DEBUG("ret:{}", ret);
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Insert with id
    ret = test_dm.insert_withid(&data, 10);
    ASSERT_TRUE(ret == HAL_RET_OK);
    // Insert with id
    ret = test_dm.insert_withid(&data, 10);
    ASSERT_TRUE(ret == HAL_RET_OK);
    // Delete
    ret = test_dm.remove(0, &data);
    ASSERT_TRUE(ret == HAL_RET_OK);
    // Delete
    ret = test_dm.remove(0, &data);
    HAL_TRACE_DEBUG("ret:{}", ret);
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Insert
    ret = test_dm.insert(&data, &index);
    ASSERT_TRUE(ret == HAL_RET_OK);
    // Insert with id
    data.actionid = 0;
    ret = test_dm.insert_withid(&data, 20);
    HAL_TRACE_DEBUG("ret:{}", ret);
    ASSERT_TRUE(ret == HAL_RET_OK);
    // Delete
    ret = test_dm.remove(0, &data);
    ASSERT_TRUE(ret == HAL_RET_OK);
    // Delete
    data.actionid = TWICE_NAT_TWICE_NAT_REWRITE_INFO_ID;
    ret = test_dm.remove(0, &data);
    ASSERT_TRUE(ret == HAL_RET_OK);
}

/*
 * Test Case 11;
 *  - Testing shared direct map
 */
TEST_F(dm_test, test11) {

    hal_ret_t                   ret;
    hal::pd::pd_twice_nat_entry_args_t   args;
    uint32_t                    nat_idx = 0;

    args.nat_l4_port = 100;
    args.nat_ip.af = IP_AF_IPV4;
    args.nat_ip.addr.v4_addr = 0x0a000003;
    args.twice_nat_act = TWICE_NAT_TWICE_NAT_REWRITE_INFO_ID;

    ret = pd_twice_nat_add(&args, &nat_idx);
    ASSERT_TRUE(ret == HAL_RET_OK);

    ret = pd_twice_nat_add(&args, &nat_idx);
    ASSERT_TRUE(ret == HAL_RET_OK);

    ret = pd_twice_nat_del(&args);
    ASSERT_TRUE(ret == HAL_RET_OK);

    ret = pd_twice_nat_del(&args);
    ASSERT_TRUE(ret == HAL_RET_OK);

}
int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  p4pd_init();
  return RUN_ALL_TESTS();
}
