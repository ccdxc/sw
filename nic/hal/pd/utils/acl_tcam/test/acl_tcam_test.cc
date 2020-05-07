#include <gtest/gtest.h>
#include <stdio.h>
#include "nic/hal/pd/utils/acl_tcam/acl_tcam.hpp"
#include "nic/sdk/lib/p4/p4_api.hpp"
#include "gen/p4gen/p4/include/p4pd.h"

#define NUM_ENTRIES 512

using hal::pd::utils::acl_tcam;
using hal::pd::utils::priority_t;
using hal::pd::utils::acl_tcam_entry_handle_t;

typedef struct tcam_key_s {
    int a;
    int b;
} tcam_key_t;

typedef struct tcam_data_s {
    int p;
    int q;
} tcam_data_t;

class input_mapping_native_tcam_entry {

public:
    input_mapping_native_tcam_entry(input_mapping_native_swkey_t swkey, input_mapping_native_swkey_mask_t swkey_mask,
                          input_mapping_native_actiondata_t act_data) {
        memcpy(&key, &swkey, sizeof(key));
        memcpy(&key_mask, &swkey_mask, sizeof(key_mask));
        memcpy(&data, &act_data, sizeof(data));
    }
    ~input_mapping_native_tcam_entry() {}

    bool operator< (const input_mapping_native_tcam_entry &right) const {
        int i = memcmp(&(key), &(right.key), sizeof(input_mapping_native_swkey_t));
        if (i < 0) {
            return TRUE;
        } else {
            return FALSE;
        }
    }

    bool operator== (const input_mapping_native_tcam_entry &right) const {
        if (!memcmp(&(key), &(right.key), sizeof(input_mapping_native_swkey_t)) &&
            !memcmp(&(key_mask), &(right.key_mask), sizeof(input_mapping_native_swkey_mask_t)) &&
            !memcmp(&data, &(right.data), sizeof(input_mapping_native_actiondata_t))) {
            return TRUE;
        } else {
            return FALSE;
        }
    }


private:
    input_mapping_native_swkey_t key;
    input_mapping_native_swkey_mask_t key_mask;
    input_mapping_native_actiondata_t data;

};

class acl_tcam_test : public ::testing::Test {
protected:
    acl_tcam_test() {
    }

    virtual ~acl_tcam_test() {
    }

    // will be called immediately after the constructor before each test
    virtual void SetUp();

    // will be called immediately after each test before the destructor
    virtual void TearDown();


    hal_ret_t add_entry(priority_t priority, acl_tcam_entry_handle_t *handle_p = NULL);
    hal_ret_t update_entry(priority_t priority, acl_tcam_entry_handle_t handle);
    hal_ret_t remove_entry(acl_tcam_entry_handle_t handle);

    void verify_order(void);
    void print_order(void);

    std::string table_name_;
    acl_tcam *test_tcam_;
    std::map<priority_t, std::set<acl_tcam_entry_handle_t>, std::greater<priority_t>> entries;
    std::map<acl_tcam_entry_handle_t, priority_t> valid_handles;
    int seed_;
    int num_entries_;
};

void
acl_tcam_test::SetUp()
{
    table_name_ = "Input_Mapping_Native";
    test_tcam_ = acl_tcam::factory(table_name_, (uint32_t)P4TBL_ID_INPUT_MAPPING_NATIVE, NUM_ENTRIES,
                                   (uint32_t)sizeof(input_mapping_native_swkey_t),
                                   (uint32_t)sizeof(input_mapping_native_actiondata_t));
    seed_ = rand();
    seed_ = 1234;
    std::cout << "Random seed used " << seed_ << std::endl;
    srand(seed_);
    num_entries_ = 0;
}

void
acl_tcam_test::TearDown()
{
    delete test_tcam_;
    std::cout << "Random seed used " << seed_ << std::endl;
}

void
acl_tcam_test::verify_order()
{
    hal_ret_t ret = HAL_RET_OK;
    // indexes tracks the tcam-indexes allocated
    std::set<uint32_t> indexes;
    // range_indexes tracks the range of tcam-indexes allocated for entries of
    // same priority
    std::set<uint32_t> range_indexes;
    uint32_t tcam_idx;
    uint32_t max_seen_idx;

    // Go from low priority to high priority and ensure the priority order
    // is maintained.
    // indexes set tracks the indexes allocated. So the current entries index
    // should be greater than the highest index in the indexes set
    for (auto mitr = entries.begin();
         mitr != entries.end();
         mitr++) {
        for (auto hitr = mitr->second.begin();
             hitr != mitr->second.end();
             hitr++) {
            test_tcam_->get_index(*hitr, &tcam_idx);
            ASSERT_EQ(ret, HAL_RET_OK);

            range_indexes.insert(tcam_idx);
            if (!indexes.empty()) {
                max_seen_idx = *std::max_element(indexes.begin(), indexes.end());
                if (tcam_idx <= max_seen_idx) {
                    std::cout << "tcam idx " << tcam_idx << " less than max seen " << max_seen_idx << std::endl;
                }
                ASSERT_GT(tcam_idx, *std::max_element(indexes.begin(), indexes.end()));
            }
        }

        indexes.insert(range_indexes.begin(), range_indexes.end());
        range_indexes.clear();
    }
}

void
acl_tcam_test::print_order()
{
    uint32_t tcam_idx;

    std::cout << "PRINTING ORDER" << std::endl;
    for (auto mitr = entries.begin();
         mitr != entries.end();
         mitr++) {
        for (auto hitr = mitr->second.begin();
             hitr != mitr->second.end();
             hitr++) {
            test_tcam_->get_index(*hitr, &tcam_idx);
            std::cout << mitr->first << "\t" << tcam_idx << std::endl;
        }
    }
    std::cout << "----------------------------------------------------" << std::endl;
}
hal_ret_t
acl_tcam_test::add_entry(priority_t priority, acl_tcam_entry_handle_t *handle_p)
{
    hal_ret_t ret = HAL_RET_OK;
    input_mapping_native_swkey_t key;
    input_mapping_native_swkey_mask_t key_mask;
    input_mapping_native_actiondata_t data;
    acl_tcam_entry_handle_t handle;

    memset(&key, 0, sizeof(key));
    memset(&key_mask, 0, sizeof(key_mask));
    memset(&data, 0, sizeof(data));

    key.tunnel_metadata_tunnel_type = 1;
    data.action_id = 1;
    memset(&key_mask, ~0, sizeof(tcam_key_t));

    ret = test_tcam_->insert((void *)&key, (void *)&key_mask,
                             (void *)&data, priority, &handle);

    if (ret != HAL_RET_OK) {
        return ret;
    }

    num_entries_++;

    entries[priority].insert(handle);
    valid_handles[handle] = priority;

    if (handle_p) { *handle_p = handle;}

    verify_order();
    return ret;
}

hal_ret_t
acl_tcam_test::update_entry(priority_t priority, acl_tcam_entry_handle_t handle)
{
    hal_ret_t ret = HAL_RET_OK;
    input_mapping_native_swkey_t key;
    input_mapping_native_swkey_mask_t key_mask;
    input_mapping_native_actiondata_t data;
    priority_t old_priority;
    auto hitr = valid_handles.find(handle);

    old_priority = hitr->second;

    memset(&key, 0, sizeof(key));
    memset(&key_mask, 0, sizeof(key_mask));
    memset(&data, 0, sizeof(data));

    key.tunnel_metadata_tunnel_type = 1;
    data.action_id = 1;
    memset(&key_mask, ~0, sizeof(tcam_key_t));

    std::cout << old_priority <<  " --> " << priority << std::endl;

    ret = test_tcam_->update(handle, (void *)&key, (void *)&key_mask,
                             (void *)&data, priority);

    if (ret != HAL_RET_OK) {
        return ret;
    }


    valid_handles[handle] = priority;
    entries[old_priority].erase(handle);
    entries[priority].insert(handle);

    verify_order();
    return ret;
}

hal_ret_t
acl_tcam_test::remove_entry(acl_tcam_entry_handle_t handle)
{
    hal_ret_t ret = HAL_RET_OK;
    auto hitr = valid_handles.find(handle);

    ret = test_tcam_->remove(handle);

    if (hitr == valid_handles.end()) {
        return ret;
    }

    if (ret != HAL_RET_OK) {
        return ret;
    }

    num_entries_--;
    entries[hitr->second].erase(handle);
    valid_handles.erase(handle);

    verify_order();
    return ret;
}

/* ---------------------------------------------------------------------------
 *
 * Test Case 1:
 * - Create acl_tcam table
 * - Insert acl_tcam entry
 */
TEST_F(acl_tcam_test, test1)
{
    uint32_t tcam_idx;
    acl_tcam_entry_handle_t handle1, handle2;
    hal_ret_t ret = HAL_RET_OK;


    ASSERT_NO_FATAL_FAILURE(ret = add_entry(100, &handle1));
    ASSERT_EQ(ret, HAL_RET_OK);

    ret = test_tcam_->get_index(handle1, &tcam_idx);
    ASSERT_EQ(ret, HAL_RET_OK);
    ASSERT_EQ(tcam_idx, 0);

    ASSERT_NO_FATAL_FAILURE(ret = add_entry(100, &handle2));
    ASSERT_EQ(ret, HAL_RET_OK);

    ASSERT_NO_FATAL_FAILURE(ret = remove_entry(handle1));
    ASSERT_EQ(ret, HAL_RET_OK);

    ASSERT_NO_FATAL_FAILURE(ret = remove_entry(handle2));
    ASSERT_EQ(ret, HAL_RET_OK);
}

/* ---------------------------------------------------------------------------
 *
 * Test Case 2:
 * - Create acl_tcam table
 * - Insert acl_tcam entries with decreasing priority
 */
TEST_F(acl_tcam_test, test2)
{
    hal_ret_t ret = HAL_RET_OK;
    priority_t priority;
    acl_tcam_entry_handle_t handles[NUM_ENTRIES];

    for (int i = 0; i < NUM_ENTRIES; i++) {
        priority = NUM_ENTRIES-i;

        ASSERT_NO_FATAL_FAILURE(ret = add_entry(priority, &handles[i]));
        ASSERT_EQ(ret, HAL_RET_OK);
    }

    // table is full. Error out
    ASSERT_NO_FATAL_FAILURE(ret = add_entry(100));
    ASSERT_TRUE(ret != HAL_RET_OK);

    for (int i = 0; i < NUM_ENTRIES; i++) {
        ASSERT_NO_FATAL_FAILURE(ret = remove_entry(handles[i]));
        ASSERT_EQ(ret, HAL_RET_OK);
    }
    // table is empty error out
    ASSERT_NO_FATAL_FAILURE(ret = remove_entry(100));
    ASSERT_TRUE(ret != HAL_RET_OK);
}


/* ---------------------------------------------------------------------------
 *
 * Test Case 3:
 * - Create acl_tcam table
 * - Insert acl_tcam entries with increasing priority (involves moving of
 *   entries)
 *
 */
TEST_F(acl_tcam_test, test3)
{

    hal_ret_t ret = HAL_RET_OK;
    priority_t priority;
    acl_tcam_entry_handle_t handles[NUM_ENTRIES];

    for (int i = 0; i < NUM_ENTRIES; i++) {
        priority = i;

        ASSERT_NO_FATAL_FAILURE(ret = add_entry(priority, &handles[i]));
        ASSERT_EQ(ret, HAL_RET_OK);
    }

    // table is full. Error out
    ASSERT_NO_FATAL_FAILURE(ret = add_entry(100));
    ASSERT_TRUE(ret != HAL_RET_OK);

    for (int i = 0; i < NUM_ENTRIES; i++) {
        ASSERT_NO_FATAL_FAILURE(ret = remove_entry(handles[i]));
        ASSERT_EQ(ret, HAL_RET_OK);
    }
    // table is empty error out
    ASSERT_NO_FATAL_FAILURE(ret = remove_entry(100));
    ASSERT_TRUE(ret != HAL_RET_OK);
}

/* ---------------------------------------------------------------------------
 *
 * Test Case 4:
 * - Create acl_tcam table
 * - Insert acl_tcam entries with random priority (involves moving of
 *   entries)
 *
 */
TEST_F(acl_tcam_test, test4)
{

    hal_ret_t ret = HAL_RET_OK;
    priority_t priority;
    acl_tcam_entry_handle_t handles[NUM_ENTRIES];

    for (int i = 0; i < NUM_ENTRIES; i++) {
        priority = rand();

        ASSERT_NO_FATAL_FAILURE(ret = add_entry(priority, &handles[i]));
        ASSERT_EQ(ret, HAL_RET_OK);
    }

    // table is full. Error out
    ASSERT_NO_FATAL_FAILURE(ret = add_entry(100));
    ASSERT_TRUE(ret != HAL_RET_OK);

    for (int i = 0; i < NUM_ENTRIES; i++) {
        ASSERT_NO_FATAL_FAILURE(ret = remove_entry(handles[i]));
        ASSERT_EQ(ret, HAL_RET_OK);
    }
    // table is empty error out
    ASSERT_NO_FATAL_FAILURE(ret = remove_entry(100));
    ASSERT_TRUE(ret != HAL_RET_OK);
}

/* ---------------------------------------------------------------------------
 *
 * Test Case 5:
 * - Create acl_tcam table
 * - Insert acl_tcam entries with random priority (involves moving of
 *   entries)
 * - Update acl_tcam entries with random priority
 *
 */
TEST_F(acl_tcam_test, test5)
{

    hal_ret_t ret = HAL_RET_OK;
    priority_t priority;
    acl_tcam_entry_handle_t handles[NUM_ENTRIES];

    // Keep 1 space free for updates where priority changes (make before break
    // needs 1 free space)
    for (int i = 0; i < NUM_ENTRIES - 1; i++) {
        priority = rand();

        ASSERT_NO_FATAL_FAILURE(ret = add_entry(priority, &handles[i]));
        ASSERT_EQ(ret, HAL_RET_OK);
    }

    std::cout << "INITIAL ORDER" << std::endl;

    // Update
    for (int i = 0; i < NUM_ENTRIES - 1; i++) {
        priority = rand();

        ASSERT_NO_FATAL_FAILURE(ret = update_entry(priority, handles[i]));
        ASSERT_EQ(ret, HAL_RET_OK);
    }


    for (int i = 0; i < NUM_ENTRIES - 1; i++) {
        ASSERT_NO_FATAL_FAILURE(ret = remove_entry(handles[i]));
        ASSERT_EQ(ret, HAL_RET_OK);
    }
    // table is empty error out
    ASSERT_NO_FATAL_FAILURE(ret = remove_entry(100));
    ASSERT_TRUE(ret != HAL_RET_OK);
}

int main(int argc, char **argv)
{
    p4pd_cfg_t    p4pd_cfg = {
        .table_map_cfg_file  = "iris/p4_table_map.json",
        .p4pd_pgm_name       = "iris",
        .p4pd_rxdma_pgm_name = "p4plus",
        .p4pd_txdma_pgm_name = "p4plus",
        .cfg_path            = std::getenv("HAL_CONFIG_PATH")
    };

    ::testing::InitGoogleTest(&argc, argv);
    p4pd_init(&p4pd_cfg);
    return RUN_ALL_TESTS();
}
