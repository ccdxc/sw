#include <met.hpp>
#include <gtest/gtest.h>
#include <stdio.h>

using hal::pd::utils::Met;

class met_test : public ::testing::Test {
protected:
    met_test() {
  }

  virtual ~met_test() {
  }

  // will be called immediately after the constructor before each test
  virtual void SetUp() {
  }

  // will be called immediately after each test before the destructor
  virtual void TearDown() {
  }

};


typedef struct hal_pd_replication_tbl_mbr_entry_s {
    uint32_t lif : 11;
    uint32_t encap_id : 24;
    uint32_t tunnel_rewrite_idx : 11;
    uint32_t l3_rewrite_index : 12;
    uint32_t pad : 6;
} hal_pd_replication_tbl_mbr_entry_t;


/* ---------------------------------------------------------------------------
 *
 * Test Case 1:
 *      - Test Case to verify the update 
 *   - Create Met
 *   - Create Repl list
 *   - Add repl 
 *   - Rem repl
 *   - Delete Repl list
 */
TEST_F(met_test, test1) {

    hal_ret_t rs = HAL_RET_OK;
    uint32_t repl_list_idx = 0;
    std::string table_name = "Test_Table";
	hal_pd_replication_tbl_mbr_entry_t entry;

    Met test_met = Met(table_name, (uint32_t)1, 64000, 6, 
                       sizeof(hal_pd_replication_tbl_mbr_entry_t));

    memset(&entry, 0, sizeof(entry));
    rs = test_met.create_repl_list(&repl_list_idx);
    ASSERT_TRUE(rs == HAL_RET_OK);

	entry.lif = 10;
	entry.encap_id = 100;
    rs = test_met.add_replication(repl_list_idx, &entry);
    ASSERT_TRUE(rs == HAL_RET_OK);

    rs = test_met.del_replication(repl_list_idx, &entry);
    ASSERT_TRUE(rs == HAL_RET_OK);

	rs = test_met.delete_repl_list(repl_list_idx);
    ASSERT_TRUE(rs == HAL_RET_OK);
}

/* ---------------------------------------------------------------------------
 *
 * Test Case 2:
 *      - Test Case to verify -ve create
 *   - Create Met
 */
TEST_F(met_test, test2) {

    hal_ret_t rs = HAL_RET_OK;
    uint32_t repl_list_idx[100], tmp_idx = 0;
    std::string table_name = "Test_Table";
	hal_pd_replication_tbl_mbr_entry_t entry;

    memset(&entry, 0, sizeof(entry));
    Met test_met = Met(table_name, (uint32_t)1, 100, 6, 
                       sizeof(entry));

    for (int i = 0; i < 100; i++) {
        rs = test_met.create_repl_list(&repl_list_idx[i]);
        ASSERT_TRUE(rs == HAL_RET_OK);
    }

    rs = test_met.create_repl_list(&tmp_idx);
    ASSERT_TRUE(rs == HAL_RET_NO_RESOURCE);

    for (int i = 0; i < 100; i++) {
        rs = test_met.delete_repl_list(repl_list_idx[i]);
        ASSERT_TRUE(rs == HAL_RET_OK);
    }

    for (int i = 0; i < 100; i++) {
        rs = test_met.create_repl_list(&repl_list_idx[i]);
        ASSERT_TRUE(rs == HAL_RET_OK);
    }

    for (int i = 0; i < 100; i++) {
        rs = test_met.delete_repl_list(repl_list_idx[i]);
        ASSERT_TRUE(rs == HAL_RET_OK);
    }

}

/* ---------------------------------------------------------------------------
 *
 * Test Case 3:
 *      - Test Case to verify -ve delete
 *   - Create Met
 */
TEST_F(met_test, test3) {

    hal_ret_t rs = HAL_RET_OK;
    uint32_t tmp_idx = 0;
    std::string table_name = "Test_Table";
	hal_pd_replication_tbl_mbr_entry_t entry;

    memset(&entry, 0, sizeof(entry));
    Met test_met = Met(table_name, (uint32_t)1, 100, 6, 
                       sizeof(entry));

    rs = test_met.create_repl_list(&tmp_idx);
    ASSERT_TRUE(rs == HAL_RET_OK);

    rs = test_met.delete_repl_list(tmp_idx);
    ASSERT_TRUE(rs == HAL_RET_OK);

    rs = test_met.delete_repl_list(tmp_idx);
    ASSERT_TRUE(rs == HAL_RET_ENTRY_NOT_FOUND);
}

/* ---------------------------------------------------------------------------
 *
 * Test Case 4:
 *      - Test Case to verify -ve add repl
 *   - Create Met
 */
TEST_F(met_test, test4) {

    hal_ret_t rs = HAL_RET_OK;
    uint32_t repl_list_idx = 0;
    std::string table_name = "Test_Table";
	hal_pd_replication_tbl_mbr_entry_t entry;

    memset(&entry, 0, sizeof(entry));
    Met test_met = Met(table_name, (uint32_t)1, 64000, 6, 
                       sizeof(hal_pd_replication_tbl_mbr_entry_t));

    test_met.create_repl_list(&repl_list_idx);
    ASSERT_TRUE(rs == HAL_RET_OK);

	entry.lif = 10;
	entry.encap_id = 100;
    rs = test_met.add_replication(repl_list_idx, &entry);
    ASSERT_TRUE(rs == HAL_RET_OK);

    rs = test_met.add_replication(repl_list_idx, &entry);
    ASSERT_TRUE(rs == HAL_RET_OK);

    rs = test_met.del_replication(repl_list_idx, &entry);
    ASSERT_TRUE(rs == HAL_RET_OK);

    rs = test_met.del_replication(repl_list_idx, &entry);
    ASSERT_TRUE(rs == HAL_RET_OK);

    rs = test_met.add_replication(repl_list_idx + 1, &entry);
    ASSERT_TRUE(rs == HAL_RET_ENTRY_NOT_FOUND);

	rs = test_met.delete_repl_list(repl_list_idx);
    ASSERT_TRUE(rs == HAL_RET_OK);
}

/* ---------------------------------------------------------------------------
 *
 * Test Case 5:
 *      - Test Case to verify more than 6 replications
 *   - Create Met
 */
TEST_F(met_test, test5) {

    hal_ret_t rs = HAL_RET_OK;
    uint32_t repl_list_idx;
    std::string table_name = "Test_Table";
	hal_pd_replication_tbl_mbr_entry_t entry;

    memset(&entry, 0, sizeof(entry));
    Met test_met = Met(table_name, (uint32_t)1, 64000, 6, 
                       sizeof(hal_pd_replication_tbl_mbr_entry_t));

    test_met.create_repl_list(&repl_list_idx);
    ASSERT_TRUE(rs == HAL_RET_OK);

	entry.lif = 10;
	entry.encap_id = 100;
    for (int i = 0; i < 100; i++) {
        rs = test_met.add_replication(repl_list_idx, &entry);
        ASSERT_TRUE(rs == HAL_RET_OK);
    }

    for (int i = 0; i < 100; i++) {
        rs = test_met.del_replication(repl_list_idx, &entry);
        ASSERT_TRUE(rs == HAL_RET_OK);
    }

	rs = test_met.delete_repl_list(repl_list_idx);
    ASSERT_TRUE(rs == HAL_RET_OK);
}

/* ---------------------------------------------------------------------------
 *
 * Test Case 6:
 *      - Test Case to verify -ve del repl
 *   - Create Met
 */
TEST_F(met_test, test6) {

    hal_ret_t rs = HAL_RET_OK;
    uint32_t repl_list_idx = 0;
    std::string table_name = "Test_Table";
	hal_pd_replication_tbl_mbr_entry_t entry;

    memset(&entry, 0, sizeof(entry));
    Met test_met = Met(table_name, (uint32_t)1, 64000, 6, 
                       sizeof(hal_pd_replication_tbl_mbr_entry_t));

    test_met.create_repl_list(&repl_list_idx);
    ASSERT_TRUE(rs == HAL_RET_OK);

	entry.lif = 10;
	entry.encap_id = 100;
    rs = test_met.add_replication(repl_list_idx, &entry);
    ASSERT_TRUE(rs == HAL_RET_OK);

    rs = test_met.add_replication(repl_list_idx, &entry);
    ASSERT_TRUE(rs == HAL_RET_OK);

    rs = test_met.del_replication(repl_list_idx, &entry);
    ASSERT_TRUE(rs == HAL_RET_OK);

    rs = test_met.del_replication(repl_list_idx, &entry);
    ASSERT_TRUE(rs == HAL_RET_OK);

    rs = test_met.del_replication(repl_list_idx + 1, &entry);
    ASSERT_TRUE(rs == HAL_RET_ENTRY_NOT_FOUND);

	rs = test_met.delete_repl_list(repl_list_idx);
    ASSERT_TRUE(rs == HAL_RET_OK);
}
int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

