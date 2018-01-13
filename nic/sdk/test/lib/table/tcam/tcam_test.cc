//------------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//
// test cases to verify TCAM table management library
//------------------------------------------------------------------------------

#include "gtest/gtest.h"
#include <stdio.h>
#include "sdk/tcam.hpp"
#include "sdk/base.hpp"

using sdk::table::tcam;
using sdk::sdk_ret_t;
using namespace sdk;

typedef struct tcam_key_s {
    int a;
    int b;
} tcam_key_t;

typedef struct tcam_data_s {
    int p;
    int q;
} tcam_data_t;

class SampleTcamEntry {

public:
    SampleTcamEntry(tcam_key_t key, tcam_key_t key_mask, tcam_data_t data) {
        a = key.a;
        b = key.b;
        a_mask = key_mask.a;
        b_mask = key_mask.b;
        p = data.p;
        q = data.q;
    }
    ~SampleTcamEntry() {}

    bool operator< (const SampleTcamEntry &right) const {
        if (a != right.a) {
            return a < right.a;
        } else if (b != right.b) {
            return b < right.b;
        } else if (a_mask != right.a_mask) {
            return a_mask < right.a_mask;
        } else if (b_mask != right.b_mask) {
            return b_mask < right.b_mask;
        } else if (p != right.p) {
            return p < right.p;
        } else {
            return q < right.q;
        }
    }

    bool operator== (const SampleTcamEntry &right) const {
        if (a == right.a && b == right.b && a_mask == right.a_mask && 
                b_mask == right.b_mask && p == right.p && q == right.q) {
            return TRUE;
        } else {
            return FALSE;
        }
    }

private:
    int a;
    int b;
    int a_mask;
    int b_mask;

    int p;
    int q;

};

#define P4TBL_ID_INPUT_MAPPING_NATIVE 10
/* Software Key Structure for p4-table 'input_mapping_native' */
 /* P4-table 'input_mapping_native' ternary table.*/

 typedef union __input_mapping_native_union1_t { /* Sourced from field union */
     struct {
         uint8_t ipv6_dstAddr[16];
     };
     struct {
         uint8_t __fld_ipv4_dstAddr_un_pad[12];
         uint32_t ipv4_dstAddr;
     };
 } input_mapping_native_union1_t;


 typedef struct __attribute__((__packed__)) input_mapping_native_swkey {
     uint8_t inner_ipv4_valid;
     uint8_t inner_ipv6_valid;
     uint8_t mpls_0_valid;
     uint8_t ipv4_valid;
     uint8_t ipv6_valid;
     uint8_t entry_inactive_input_mapping;
     uint8_t tunnel_metadata_tunnel_type;
     input_mapping_native_union1_t input_mapping_native_u1;
 } input_mapping_native_swkey_t;


 typedef union __input_mapping_native_mask_union1_t { /* Sourced from field union */
     struct {
         uint8_t ipv6_dstAddr_mask[16];
     };
     struct {
         uint8_t __fld_ipv4_dstAddr_un_pad_mask[12];
         uint32_t ipv4_dstAddr_mask;
     };
 } input_mapping_native_mask_union1_t;

 typedef struct __attribute__((__packed__)) input_mapping_native_swkey_mask {
     uint8_t inner_ipv4_valid_mask;
     uint8_t inner_ipv6_valid_mask;
     uint8_t mpls_0_valid_mask;
     uint8_t ipv4_valid_mask;
     uint8_t ipv6_valid_mask;
     uint8_t entry_inactive_input_mapping_mask;
     uint8_t tunnel_metadata_tunnel_type_mask;
     input_mapping_native_mask_union1_t input_mapping_native_mask_u1;
 } input_mapping_native_swkey_mask_t;

 typedef enum input_mapping_native_actions_enum {
     INPUT_MAPPING_NATIVE_NATIVE_IPV4_PACKET_ID = 0,
     INPUT_MAPPING_NATIVE_NATIVE_IPV6_PACKET_ID = 1,
     INPUT_MAPPING_NATIVE_NATIVE_NON_IP_PACKET_ID = 2,
     INPUT_MAPPING_NATIVE_INPUT_MAPPING_MISS_ID = 3,
     INPUT_MAPPING_NATIVE_NOP_ID = 4,
     INPUT_MAPPING_NATIVE_MAX_ID = 5
 } input_mapping_native_actions_en;


 typedef union __input_mapping_native_action_union {
 } input_mapping_native_action_union_t;

 typedef struct __attribute__((__packed__)) __input_mapping_native_actiondata {
     uint8_t actionid;
     input_mapping_native_action_union_t input_mapping_native_action_u;
 } input_mapping_native_actiondata;

class InputMappingNativeTcamEntry {

public:
    InputMappingNativeTcamEntry(input_mapping_native_swkey_t swkey, input_mapping_native_swkey_mask_t swkey_mask,
            input_mapping_native_actiondata act_data) {
        memcpy(&key, &swkey, sizeof(key)); 
        memcpy(&key_mask, &swkey_mask, sizeof(key_mask)); 
        memcpy(&data, &act_data, sizeof(data));
    }
    ~InputMappingNativeTcamEntry() {}

    bool operator< (const InputMappingNativeTcamEntry &right) const {
        int i = memcmp(&(key), &(right.key), sizeof(input_mapping_native_swkey_t));
        if (i < 0) {
            return TRUE;
        } else {
            return FALSE;
        }
    }

    bool operator== (const InputMappingNativeTcamEntry &right) const {
        if (!memcmp(&(key), &(right.key), sizeof(input_mapping_native_swkey_t)) &&
                !memcmp(&(key_mask), &(right.key_mask), sizeof(input_mapping_native_swkey_mask_t)) &&
                !memcmp(&data, &(right.data), sizeof(input_mapping_native_actiondata))) {
            return TRUE;
        } else {
            return FALSE;
        }
    }


private:
    input_mapping_native_swkey_t key;
    input_mapping_native_swkey_mask_t key_mask;
    input_mapping_native_actiondata data;

};

class tcam_test : public ::testing::Test {
protected:
    tcam_test() {
  }

  virtual ~tcam_test() {
  }

  // will be called immediately after the constructor before each test
  virtual void SetUp() {
  }

  // will be called immediately after each test before the destructor
  virtual void TearDown() {
  }

};


bool
populate_fn (const void *key, const void *key_mask,
             const void *data,
             uint32_t tcam_idx, const void *cb_data)
{
    std::set<SampleTcamEntry> *pop_set = (std::set<SampleTcamEntry> *)cb_data;
    tcam_key_t *key_p = (tcam_key_t *)key;
    tcam_key_t *key_mask_p = (tcam_key_t *)key_mask;
    tcam_data_t *data_p = (tcam_data_t *)data;

    pop_set->insert(SampleTcamEntry(*key_p, *key_mask_p, *data_p));

    return 0;
}

bool
populate_im_fn (const void *key, const void *key_mask,
             const void *data,
             uint32_t tcam_idx, const void *cb_data)
{
    std::set<InputMappingNativeTcamEntry> *pop_set = (std::set<InputMappingNativeTcamEntry> *)cb_data;
    input_mapping_native_swkey_t *key_p = (input_mapping_native_swkey_t*)key;
    input_mapping_native_swkey_mask_t *key_mask_p = (input_mapping_native_swkey_mask_t*)key_mask;
    input_mapping_native_actiondata *data_p = (input_mapping_native_actiondata *)data;

    pop_set->insert(InputMappingNativeTcamEntry(*key_p, *key_mask_p, *data_p));

    return 0;
}

/* ---------------------------------------------------------------------------
 *
 * Test Case 1:
 *      - Test Case to verify the update 
 * - Create Tcam table
 * - Insert Tcam entry
 * - Print Tcam Entries
 * - Update Tcam entry
 * - Print Tcam Entries
 */
TEST_F(tcam_test, test1) {

    std::string table_name = "Input_Mapping_Native";
    char * my_argument = const_cast<char*> (table_name.c_str());

    tcam *test_tcam = tcam::factory(my_argument, 
                                    (uint32_t)P4TBL_ID_INPUT_MAPPING_NATIVE, (uint32_t)100,
                                    (uint32_t)sizeof(input_mapping_native_swkey_t), 
                                    (uint32_t)sizeof(input_mapping_native_actiondata));

    std::set<InputMappingNativeTcamEntry> sent_set;
    std::set<InputMappingNativeTcamEntry> tcam_set;

    sdk_ret_t rs = sdk::SDK_RET_OK;
    input_mapping_native_swkey_t key;
    input_mapping_native_swkey_mask_t key_mask;
    input_mapping_native_actiondata data;
    uint32_t tcam_idx = 0;

    memset(&key, 0, sizeof(key));
    memset(&key_mask, 0, sizeof(key_mask));
    memset(&data, 0, sizeof(data));

    key.tunnel_metadata_tunnel_type = 1;
    data.actionid = 1;
    memset(&key_mask, ~0, sizeof(tcam_key_t));

    rs = test_tcam->insert((void *)&key, (void *)&key_mask,
            (void *)&data, &tcam_idx);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);

    data.actionid = 2;
    rs = test_tcam->update(tcam_idx, &data);
    ASSERT_TRUE(rs == SDK_RET_OK);
    sent_set.insert(InputMappingNativeTcamEntry(key, key_mask, data));

    rs = test_tcam->iterate(populate_im_fn, &tcam_set);
    ASSERT_TRUE(rs == SDK_RET_OK);

    ASSERT_TRUE(sent_set == tcam_set);

    rs = test_tcam->remove(tcam_idx);
    ASSERT_TRUE(rs == SDK_RET_OK);

}

/* ---------------------------------------------------------------------------
 *
 * Test Case 2:
 *      - Test Case to verify the duplicate inserts
 * - Create Tcam table
 * - Insert Tcam entry
 * - Insert Tcam entry
 */
TEST_F(tcam_test, test2) {

    // std::string table_name = "Input_Mapping_Native";
    std::string table_name = "Input_Mapping_Native";
    char * table_str = const_cast<char*> (table_name.c_str());
    tcam *test_tcam = tcam::factory(table_str, (uint32_t)P4TBL_ID_INPUT_MAPPING_NATIVE, (uint32_t)100,
                          (uint32_t)sizeof(input_mapping_native_swkey_t), 
                          (uint32_t)sizeof(input_mapping_native_actiondata));

    sdk_ret_t rs = SDK_RET_OK;
    std::set<InputMappingNativeTcamEntry> sent_set;
    std::set<InputMappingNativeTcamEntry> tcam_set;
    input_mapping_native_swkey_t key;
    input_mapping_native_swkey_mask_t key_mask;
    input_mapping_native_actiondata data;
    uint32_t tcam_idx = 0, tcam_idx2 = 0;

    memset(&key, 0, sizeof(key));
    memset(&key_mask, 0, sizeof(key_mask));
    memset(&data, 0, sizeof(data));
    key.tunnel_metadata_tunnel_type = 1;
    data.actionid = 1;
    memset(&key_mask, ~0, sizeof(tcam_key_t));


    rs = test_tcam->insert((void *)&key, (void *)&key_mask,
            (void *)&data, &tcam_idx);
    ASSERT_TRUE(rs == SDK_RET_OK);


    rs = test_tcam->insert((void *)&key, (void *)&key_mask,
            (void *)&data, &tcam_idx2);
    ASSERT_TRUE(rs == SDK_RET_DUPLICATE_INS);

    rs = test_tcam->insert((void *)&key, (void *)&key_mask,
            (void *)&data, &tcam_idx2);
    ASSERT_TRUE(rs == SDK_RET_DUPLICATE_INS);


    rs = test_tcam->remove(tcam_idx);
    ASSERT_TRUE(rs == SDK_RET_OK);
}

/* ---------------------------------------------------------------------------
 *
 * Test Case 3:
 *      - Test Case to verify the removal
 * - Create Tcam table
 * - Insert Tcam entry
 * - Remove Tcam entry
 * - Remove Tcam entry - Should fail
 */
TEST_F(tcam_test, test3) {

    std::string table_name = "Input_Mapping_Native";
    char * table_str = const_cast<char*> (table_name.c_str());
    tcam *test_tcam = tcam::factory(table_str, (uint32_t)P4TBL_ID_INPUT_MAPPING_NATIVE, (uint32_t)100,
                          (uint32_t)sizeof(input_mapping_native_swkey_t), 
                          (uint32_t)sizeof(input_mapping_native_actiondata));

    sdk_ret_t rs = SDK_RET_OK;
    std::set<InputMappingNativeTcamEntry> sent_set;
    std::set<InputMappingNativeTcamEntry> tcam_set;
    input_mapping_native_swkey_t key;
    input_mapping_native_swkey_mask_t key_mask;
    input_mapping_native_actiondata data;
    uint32_t tcam_idx = 0;

    memset(&key, 0, sizeof(key));
    memset(&key_mask, 0, sizeof(key_mask));
    memset(&data, 0, sizeof(data));
    key.tunnel_metadata_tunnel_type = 1;
    data.actionid = 1;
    memset(&key_mask, ~0, sizeof(tcam_key_t));

    rs = test_tcam->insert((void *)&key, (void *)&key_mask,
            (void *)&data, &tcam_idx);
    ASSERT_TRUE(rs == SDK_RET_OK);

    sent_set.insert(InputMappingNativeTcamEntry(key, key_mask, data));

    rs = test_tcam->iterate(populate_im_fn, &tcam_set);
    ASSERT_TRUE(rs == SDK_RET_OK);

    ASSERT_TRUE(sent_set == tcam_set);

    rs = test_tcam->remove(tcam_idx);
    ASSERT_TRUE(rs == SDK_RET_OK);

    rs = test_tcam->remove(tcam_idx);
    ASSERT_TRUE(rs == SDK_RET_ENTRY_NOT_FOUND);


}

/* ---------------------------------------------------------------------------
 *
 * Test Case 4:
 *      - Test Case to verify the retrieval
 * - Create Tcam table
 * - Insert Tcam entry
 * - Print Tcam Entries
 * - Retrieve Tcam entry 
 */
TEST_F(tcam_test, test4) {

    std::string table_name = "Input_Mapping_Native";
    char * table_str = const_cast<char*> (table_name.c_str());
    tcam *test_tcam = tcam::factory(table_str, (uint32_t)P4TBL_ID_INPUT_MAPPING_NATIVE, (uint32_t)100,
                          (uint32_t)sizeof(input_mapping_native_swkey_t), 
                          (uint32_t)sizeof(input_mapping_native_actiondata));

    input_mapping_native_swkey_t key, retr_key;
    input_mapping_native_swkey_mask_t key_mask, retr_key_mask;
    input_mapping_native_actiondata data, retr_data;
    uint32_t tcam_idx = 0;

    sdk_ret_t rs = SDK_RET_OK;
    memset(&key, 0, sizeof(key));
    memset(&key_mask, 0, sizeof(key_mask));
    memset(&data, 0, sizeof(data));

    key.tunnel_metadata_tunnel_type = 1;
    data.actionid = 1;

    memset(&key_mask, ~0, sizeof(input_mapping_native_swkey_mask_t));


    rs = test_tcam->insert((void *)&key, (void *)&key_mask,
            (void *)&data, &tcam_idx);
    ASSERT_TRUE(rs == SDK_RET_OK);


    rs = test_tcam->retrieve(tcam_idx+1, (void *)&retr_key, (void *)&retr_key_mask, 
            (void *)&retr_data);
    ASSERT_TRUE(rs == SDK_RET_ENTRY_NOT_FOUND);

    rs = test_tcam->retrieve(tcam_idx+1000, (void *)&retr_key, (void *)&retr_key_mask, 
            (void *)&retr_data);
    ASSERT_TRUE(rs == SDK_RET_OOB);

    rs = test_tcam->retrieve(tcam_idx, (void *)&retr_key, (void *)&retr_key_mask, 
            (void *)&retr_data);
    ASSERT_TRUE(rs == SDK_RET_OK);

#if 0
    ASSERT_TRUE(key.a == retr_key.a);
    ASSERT_TRUE(key.b == retr_key.b);
    ASSERT_TRUE(key_mask.a == retr_key_mask.a);
    ASSERT_TRUE(key_mask.b == retr_key_mask.b);
    ASSERT_TRUE(data.p == retr_data.p);
    ASSERT_TRUE(data.q == retr_data.q);
#endif

    rs = test_tcam->remove(tcam_idx);
    ASSERT_TRUE(rs == SDK_RET_OK);
}



/* ---------------------------------------------------------------------------
 *
 * Test Case 5:
 *      - Testing -ve cases
 *      - Insert Failure
 *      - Remove Failure
 *      - Update Failure
 */
TEST_F(tcam_test, test5) {

    std::string table_name = "Input_Mapping_Native";
    char * table_str = const_cast<char*> (table_name.c_str());
    tcam *test_tcam = tcam::factory(table_str, (uint32_t)P4TBL_ID_INPUT_MAPPING_NATIVE, (uint32_t)100,
                          (uint32_t)sizeof(input_mapping_native_swkey_t), 
                          (uint32_t)sizeof(input_mapping_native_actiondata));

    sdk_ret_t rs = SDK_RET_OK;
    input_mapping_native_swkey_t key;
    input_mapping_native_swkey_mask_t key_mask;
    input_mapping_native_actiondata data;
    uint32_t tcam_idx = 0;

    memset(&key, 0, sizeof(key));
    memset(&key_mask, 0, sizeof(key_mask));
    memset(&data, 0, sizeof(data));

    key.tunnel_metadata_tunnel_type = 1;
    data.actionid = 1;
    memset(&key_mask, ~0, sizeof(input_mapping_native_swkey_t));


    rs = test_tcam->insert((void *)&key, (void *)&key_mask,
            (void *)&data, &tcam_idx);
    ASSERT_TRUE(rs == SDK_RET_OK);

    data.actionid = 2;
    rs = test_tcam->update(tcam_idx, &data);
    ASSERT_TRUE(rs == SDK_RET_OK);

    tcam_idx++;
    rs = test_tcam->remove(tcam_idx);
    ASSERT_TRUE(rs == SDK_RET_ENTRY_NOT_FOUND);

    rs = test_tcam->update(tcam_idx, &data);
    ASSERT_TRUE(rs == SDK_RET_ENTRY_NOT_FOUND);

    tcam_idx--;
    rs = test_tcam->remove(tcam_idx);
    ASSERT_TRUE(rs == SDK_RET_OK);
}


/* ---------------------------------------------------------------------------
 *
 * Test Case 6:
 *      - Scale Test to insert lots of entries
 */
TEST_F(tcam_test, test6) {

    std::string table_name = "Input_Mapping_Native";
    char * table_str = const_cast<char*> (table_name.c_str());
    tcam *test_tcam = tcam::factory(table_str, (uint32_t)P4TBL_ID_INPUT_MAPPING_NATIVE, (uint32_t)100,
                          (uint32_t)sizeof(input_mapping_native_swkey_t), 
                          (uint32_t)sizeof(input_mapping_native_actiondata));

    sdk_ret_t rs = SDK_RET_OK;
    std::set<InputMappingNativeTcamEntry> sent_set;
    std::set<InputMappingNativeTcamEntry> tcam_set;
    input_mapping_native_swkey_t key;
    input_mapping_native_swkey_mask_t key_mask;
    input_mapping_native_actiondata data;
    uint32_t tcam_idx[100] = {0};

    memset(&key, 0, sizeof(key));
    memset(&key_mask, 0, sizeof(key_mask));
    memset(&data, 0, sizeof(data));

    key.tunnel_metadata_tunnel_type = 1;
    data.actionid = 1;
    memset(&key_mask, ~0, sizeof(input_mapping_native_swkey_t));

    for (int i = 0; i < 50; i++) {
        rs = test_tcam->insert((void *)&key, (void *)&key_mask,
                (void *)&data, &tcam_idx[i]);
        ASSERT_TRUE(rs == SDK_RET_OK);
        sent_set.insert(InputMappingNativeTcamEntry(key, key_mask, data));
        key.tunnel_metadata_tunnel_type++;
    }

    rs = test_tcam->iterate(populate_im_fn, &tcam_set);
    ASSERT_TRUE(rs == SDK_RET_OK);

    ASSERT_TRUE(sent_set == tcam_set);

    for (int i = 0; i < 50; i++) {
        rs = test_tcam->remove(tcam_idx[i]);
        ASSERT_TRUE(rs == SDK_RET_OK);
    }
}

/* ---------------------------------------------------------------------------
 *
 * Test Case 7:
 *      - Test Case to verify the -ve insert cases.
 * - Create Tcam table
 * - -ve Insert cases
 *
 */
TEST_F(tcam_test, test7) {

    std::string table_name = "Input_Mapping_Native";
    char * table_str = const_cast<char*> (table_name.c_str());
    tcam *test_tcam = tcam::factory(table_str, (uint32_t)P4TBL_ID_INPUT_MAPPING_NATIVE, (uint32_t)100,
                          (uint32_t)sizeof(input_mapping_native_swkey_t), 
                          (uint32_t)sizeof(input_mapping_native_actiondata));

    sdk_ret_t rs = SDK_RET_OK;
    std::set<InputMappingNativeTcamEntry> sent_set;
    std::set<InputMappingNativeTcamEntry> tcam_set;
    input_mapping_native_swkey_t key;
    input_mapping_native_swkey_mask_t key_mask;
    input_mapping_native_actiondata data;
    uint32_t tcam_idx = 0;
    uint32_t idx[100] = {0};

    memset(&key, 0, sizeof(key));
    memset(&key_mask, 0, sizeof(key_mask));
    memset(&data, 0, sizeof(data));
    key.tunnel_metadata_tunnel_type = 1;
    data.actionid = 1;

    memset(&key_mask, ~0, sizeof(tcam_key_t));

    // insert
    rs = test_tcam->insert((void *)&key, (void *)&key_mask,
                          (void *)&data, &tcam_idx);
    ASSERT_TRUE(rs == SDK_RET_OK);

    // insert again
    rs = test_tcam->insert((void *)&key, (void *)&key_mask,
                          (void *)&data, &tcam_idx);
    ASSERT_TRUE(rs == SDK_RET_DUPLICATE_INS);

    // remove
    rs = test_tcam->remove(tcam_idx);
    ASSERT_TRUE(rs == SDK_RET_OK);

    // 100 inserts
    for (int i = 0; i < 100; i++) {
        rs = test_tcam->insert((void *)&key, (void *)&key_mask,
                              (void *)&data, &idx[i]);
        key.tunnel_metadata_tunnel_type++;
        ASSERT_TRUE(rs == SDK_RET_OK);
    }

    // insert
    rs = test_tcam->insert((void *)&key, (void *)&key_mask,
                          (void *)&data, &tcam_idx);
    ASSERT_TRUE(rs == SDK_RET_NO_RESOURCE);

    // 100 removes
    for (int i = 0; i < 100; i++) {
        rs = test_tcam->remove(idx[i]);
        ASSERT_TRUE(rs == SDK_RET_OK);
    }

    SDK_TRACE_DEBUG("tableid:{}, table_name:{}, capacity:{}, num_in_use:{}, "
                    "num_inserts:{}, num_insert_errors:{}, "
                    "num_deletes:{}, num_delete_errors:{}",
                    test_tcam->id(), test_tcam->name(),
                    test_tcam->capacity(), test_tcam->num_entries_in_use(),
                    test_tcam->num_inserts(), test_tcam->num_insert_errors(),
                    test_tcam->num_deletes(), test_tcam->num_delete_errors());
}

/* ---------------------------------------------------------------------------
 *
 * Test Case 8:
 *      - Test Case to verify the -ve update cases.
 * - Create Tcam table
 * - -ve update cases
 *
 */
TEST_F(tcam_test, test8) {

    std::string table_name = "Input_Mapping_Native";
    char * table_str = const_cast<char*> (table_name.c_str());
    tcam *test_tcam = tcam::factory(table_str, (uint32_t)P4TBL_ID_INPUT_MAPPING_NATIVE, (uint32_t)100,
                          (uint32_t)sizeof(input_mapping_native_swkey_t), 
                          (uint32_t)sizeof(input_mapping_native_actiondata));

    sdk_ret_t rs = SDK_RET_OK;
    std::set<InputMappingNativeTcamEntry> sent_set;
    std::set<InputMappingNativeTcamEntry> tcam_set;
    input_mapping_native_swkey_t key;
    input_mapping_native_swkey_mask_t key_mask;
    input_mapping_native_actiondata data;
    uint32_t tcam_idx = 0;

    memset(&key, 0, sizeof(key));
    memset(&key_mask, 0, sizeof(key_mask));
    memset(&data, 0, sizeof(data));

    key.tunnel_metadata_tunnel_type = 1;
    data.actionid = 1;
    memset(&key_mask, ~0, sizeof(input_mapping_native_swkey_mask_t));


    // insert
    rs = test_tcam->insert((void *)&key, (void *)&key_mask,
                          (void *)&data, &tcam_idx);
    ASSERT_TRUE(rs == SDK_RET_OK);

    // update
    rs = test_tcam->update(tcam_idx, (void *)&data);
    ASSERT_TRUE(rs == SDK_RET_OK);

    // update
    rs = test_tcam->update(tcam_idx + 1000, (void *)&data);
    ASSERT_TRUE(rs == SDK_RET_OOB);

    // update
    rs = test_tcam->update(tcam_idx + 1, (void *)&data);
    ASSERT_TRUE(rs == SDK_RET_ENTRY_NOT_FOUND);

    // remove
    rs = test_tcam->remove(tcam_idx);
    ASSERT_TRUE(rs == SDK_RET_OK);
}

/* ---------------------------------------------------------------------------
 *
 * Test Case 9:
 *      - Test Case to verify the -ve remove cases.
 * - Create Tcam table
 * - -ve remove cases
 *
 */
TEST_F(tcam_test, test9) {

    std::string table_name = "Input_Mapping_Native";
    char * table_str = const_cast<char*> (table_name.c_str());
    tcam *test_tcam = tcam::factory(table_str, (uint32_t)P4TBL_ID_INPUT_MAPPING_NATIVE, (uint32_t)100,
                          (uint32_t)sizeof(input_mapping_native_swkey_t), 
                          (uint32_t)sizeof(input_mapping_native_actiondata));

    sdk_ret_t rs = SDK_RET_OK;
    std::set<InputMappingNativeTcamEntry> sent_set;
    std::set<InputMappingNativeTcamEntry> tcam_set;
    input_mapping_native_swkey_t key;
    input_mapping_native_swkey_mask_t key_mask;
    input_mapping_native_actiondata data;
    uint32_t tcam_idx = 0;

    memset(&key, 0, sizeof(key));
    memset(&key_mask, 0, sizeof(key_mask));
    memset(&data, 0, sizeof(data));

    key.tunnel_metadata_tunnel_type = 1;
    data.actionid = 1;
    memset(&key_mask, ~0, sizeof(input_mapping_native_swkey_mask_t));


    // insert
    rs = test_tcam->insert((void *)&key, (void *)&key_mask,
                          (void *)&data, &tcam_idx);
    ASSERT_TRUE(rs == SDK_RET_OK);

    // update
    rs = test_tcam->remove(tcam_idx + 1000);
    ASSERT_TRUE(rs == SDK_RET_OOB);

    // update
    rs = test_tcam->remove(tcam_idx + 1);
    ASSERT_TRUE(rs == SDK_RET_ENTRY_NOT_FOUND);

    // remove
    rs = test_tcam->remove(tcam_idx);
    ASSERT_TRUE(rs == SDK_RET_OK);
}

/* ---------------------------------------------------------------------------
 *
 * Test Case 10:
 *      - Test Case to verify the scale
 * - Create Tcam table
 *
 */
TEST_F(tcam_test, test10) {

    std::string table_name = "Input_Mapping_Native";
    char * table_str = const_cast<char*> (table_name.c_str());
    tcam *test_tcam = tcam::factory(table_str, (uint32_t)P4TBL_ID_INPUT_MAPPING_NATIVE, (uint32_t)100,
                          (uint32_t)sizeof(input_mapping_native_swkey_t), 
                          (uint32_t)sizeof(input_mapping_native_actiondata));

    sdk_ret_t rs = SDK_RET_OK;
    std::set<InputMappingNativeTcamEntry> sent_set;
    std::set<InputMappingNativeTcamEntry> tcam_set;
    input_mapping_native_swkey_t key;
    input_mapping_native_swkey_mask_t key_mask;
    input_mapping_native_actiondata data;
    uint32_t idx[100] = {0};

    memset(&key, 0, sizeof(key));
    memset(&key_mask, 0, sizeof(key_mask));
    memset(&data, 0, sizeof(data));

    key.tunnel_metadata_tunnel_type = 1;
    data.actionid = 1;
    memset(&key_mask, ~0, sizeof(input_mapping_native_swkey_mask_t));


    // 100 inserts
    for (int i = 0; i < 100; i++) {
        rs = test_tcam->insert((void *)&key, (void *)&key_mask,
                              (void *)&data, &idx[i]);
        key.tunnel_metadata_tunnel_type++;
        ASSERT_TRUE(rs == SDK_RET_OK);
    }

    // 100 removes
    for (int i = 0; i < 100; i++) {
        rs = test_tcam->remove(idx[i]);
        ASSERT_TRUE(rs == SDK_RET_OK);
    }

    // 100 inserts
    for (int i = 0; i < 100; i++) {
        rs = test_tcam->insert((void *)&key, (void *)&key_mask,
                              (void *)&data, &idx[i]);
        key.tunnel_metadata_tunnel_type++;
        ASSERT_TRUE(rs == SDK_RET_OK);
    }

    // 100 removes
    for (int i = 0; i < 100; i++) {
        rs = test_tcam->remove(idx[i]);
        ASSERT_TRUE(rs == SDK_RET_OK);
    }
}

/* ---------------------------------------------------------------------------
 *
 * Test Case 11:
 *      - Test Case to verify the scale
 * - Create Tcam table
 *
 */
TEST_F(tcam_test, test11) {

    std::string table_name = "Input_Mapping_Native";
    char * table_str = const_cast<char*> (table_name.c_str());
    tcam *test_tcam = tcam::factory(table_str, (uint32_t)P4TBL_ID_INPUT_MAPPING_NATIVE, (uint32_t)100,
                          (uint32_t)sizeof(input_mapping_native_swkey_t), 
                          (uint32_t)sizeof(input_mapping_native_actiondata));

    sdk_ret_t rs = SDK_RET_OK;
    std::set<InputMappingNativeTcamEntry> sent_set;
    std::set<InputMappingNativeTcamEntry> tcam_set;
    input_mapping_native_swkey_t key;
    input_mapping_native_swkey_mask_t key_mask;
    input_mapping_native_actiondata data;
    uint32_t idx[100] = {0};

    memset(&key, 0, sizeof(key));
    memset(&key_mask, 0, sizeof(key_mask));
    memset(&data, 0, sizeof(data));

    key.tunnel_metadata_tunnel_type = 1;
    data.actionid = 1;
    memset(&key_mask, ~0, sizeof(input_mapping_native_swkey_mask_t));


    // 100 inserts
    for (int i = 0; i < 100; i++) {
        rs = test_tcam->insert((void *)&key, (void *)&key_mask,
                              (void *)&data, &idx[i]);
        key.tunnel_metadata_tunnel_type++;
        ASSERT_TRUE(rs == SDK_RET_OK);
    }

    // 50 removes
    for (int i = 0; i < 50; i++) {
        rs = test_tcam->remove(idx[i]);
        ASSERT_TRUE(rs == SDK_RET_OK);
    }

    // 50 inserts
    for (int i = 0; i < 50; i++) {
        rs = test_tcam->insert((void *)&key, (void *)&key_mask,
                              (void *)&data, &idx[i]);
        key.tunnel_metadata_tunnel_type++;
        ASSERT_TRUE(rs == SDK_RET_OK);
    }

    // 100 removes
    for (int i = 0; i < 100; i++) {
        rs = test_tcam->remove(idx[i]);
        ASSERT_TRUE(rs == SDK_RET_OK);
    }
}

/* ---------------------------------------------------------------------------
 *
 * Test Case 12:
 *      - Test Case to verify the withid cases.
 * - Create Tcam table
 *
 */
TEST_F(tcam_test, test12) {

    std::string table_name = "Input_Mapping_Native";
    char * table_str = const_cast<char*> (table_name.c_str());
    tcam *test_tcam = tcam::factory(table_str, (uint32_t)P4TBL_ID_INPUT_MAPPING_NATIVE, (uint32_t)100,
                          (uint32_t)sizeof(input_mapping_native_swkey_t), 
                          (uint32_t)sizeof(input_mapping_native_actiondata));

    sdk_ret_t rs = SDK_RET_OK;
    std::set<InputMappingNativeTcamEntry> sent_set;
    std::set<InputMappingNativeTcamEntry> tcam_set;
    input_mapping_native_swkey_t key;
    input_mapping_native_swkey_mask_t key_mask;
    input_mapping_native_actiondata data;
    uint32_t tcam_idx = 0;

    memset(&key, 0, sizeof(key));
    memset(&key_mask, 0, sizeof(key_mask));
    memset(&data, 0, sizeof(data));

    key.tunnel_metadata_tunnel_type = 1;
    data.actionid = 1;
    memset(&key_mask, ~0, sizeof(input_mapping_native_swkey_mask_t));


    // insert 0th entry
    rs = test_tcam->insert((void *)&key, (void *)&key_mask,
                          (void *)&data, &tcam_idx);
    ASSERT_TRUE(rs == SDK_RET_OK);

    // insert withid 1st entry
    key.tunnel_metadata_tunnel_type++;
    rs = test_tcam->insert_withid((void *)&key, (void *)&key_mask,
                          (void *)&data, tcam_idx+1);
    ASSERT_TRUE(rs == SDK_RET_OK);

    // insert with same key
    rs = test_tcam->insert((void *)&key, (void *)&key_mask,
                          (void *)&data, &tcam_idx);
    ASSERT_TRUE(rs == SDK_RET_DUPLICATE_INS);

    // insert with same key
    rs = test_tcam->insert_withid((void *)&key, (void *)&key_mask,
                          (void *)&data, tcam_idx);
    ASSERT_TRUE(rs == SDK_RET_DUPLICATE_INS);

    // remove 0th entry
    rs = test_tcam->remove(tcam_idx);
    ASSERT_TRUE(rs == SDK_RET_OK);

    // remove 1st entry
    rs = test_tcam->remove(tcam_idx + 1);
    ASSERT_TRUE(rs == SDK_RET_OK);
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

