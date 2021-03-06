//------------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//
// test cases to verify hash table management library
//------------------------------------------------------------------------------

#include "gtest/gtest.h"
#include <stdio.h>
#include "lib/table/hash/hash.hpp"
#include "include/sdk/base.hpp"

using sdk::table::hash;
using sdk::sdk_ret_t;
// using namespace sdk;

typedef struct hash_key_s {
    int a;
    int b;
} hash_key_t;

typedef struct hash_data_s {
    int p;
    int q;
} hash_data_t;


#define P4TBL_ID_INPUT_PROPERTIES 3
#define P4TBL_ID_INPUT_PROPERTIES_OTCAM 15

typedef struct __attribute__((__packed__)) input_properties_swkey {
    uint16_t capri_intrinsic_lif;
    uint8_t entry_inactive_input_properties;
    uint16_t vlan_tag_vid;
    uint8_t vlan_tag_valid;
    uint8_t tunnel_metadata_tunnel_type;
    uint32_t tunnel_metadata_tunnel_vni;
} input_properties_swkey_t;


typedef enum input_properties_actions_enum {
    INPUT_PROPERTIES_INPUT_PROPERTIES_ID = 0,
    INPUT_PROPERTIES_MAX_ID = 1
} input_properties_actions_en;

typedef struct __attribute__((__packed__)) __input_properties_input_properties {
    uint16_t vrf;
    uint8_t dir;
    uint8_t flow_miss_action;
    uint8_t flow_miss_qos_class_id;
    uint16_t flow_miss_idx;
    uint8_t ipsg_enable;
    uint8_t l4_profile_idx;
    uint16_t dst_lport;
    uint16_t src_lport;
    uint8_t allow_flood;
    uint32_t bounce_vnid;
} input_properties_input_properties_t;

typedef union __input_properties_action_union {
    input_properties_input_properties_t input_properties_input_properties;
} input_properties_action_union_t;

typedef struct __attribute__((__packed__)) __input_properties_actiondata {
    uint8_t actionid;
    input_properties_action_union_t input_properties_action_u;
} input_properties_actiondata;


class SampleHashEntry {

public:
    SampleHashEntry(hash_key_t key, hash_data_t data) {
        a = key.a;
        b = key.b;
        p = data.p;
        q = data.q;
    }
    ~SampleHashEntry() {}

    bool operator< (const SampleHashEntry &right) const {
        if (a != right.a) {
            return a < right.a;
        } else if (b != right.b) {
            return b < right.b;
        } else if (p != right.p) {
            return p < right.p;
        } else {
            return q < right.q;
        }
    }

    bool operator== (const SampleHashEntry &right) const {
        if (a == right.a && b == right.b && p == right.p && q == right.q) {
            return TRUE;
        } else {
            return FALSE;
        }
    }


private:
    int a;
    int b;

    int p;
    int q;

};

class InputPropertiesHashEntry {

public:
    InputPropertiesHashEntry(input_properties_swkey_t swkey, input_properties_actiondata act_data) {
        memcpy(&key, &swkey, sizeof(key));
        memcpy(&data, &act_data, sizeof(data));
    }
    ~InputPropertiesHashEntry() {}

    bool operator< (const InputPropertiesHashEntry &right) const {
        int i = memcmp(&(key), &(right.key), sizeof(input_properties_swkey_t));
        if (i < 0) {
            return TRUE;
        } else {
            return FALSE;
        }
    }

    bool operator== (const InputPropertiesHashEntry &right) const {
        if (!memcmp(&(key), &(right.key), sizeof(input_properties_swkey_t)) &&
                !memcmp(&data, &(right.data), sizeof(input_properties_actiondata))) {
            return TRUE;
        } else {
            return FALSE;
        }
    }


private:
    input_properties_swkey_t key;
    input_properties_actiondata data;

};

class hash_test : public ::testing::Test {
protected:
  hash_test() {
  }

  virtual ~hash_test() {
  }

  // will be called immediately after the constructor before each test
  virtual void SetUp() {
  }

  // will be called immediately after each test before the destructor
  virtual void TearDown() {
  }

};


bool
populate_fn (const void *key,
             const void *data,
             uint32_t hash_idx, const void *cb_data)
{
    std::set<SampleHashEntry> *pop_set = (std::set<SampleHashEntry> *)cb_data;
    hash_key_t *key_p = (hash_key_t *)key;
    hash_data_t *data_p = (hash_data_t *)data;

    pop_set->insert(SampleHashEntry(*key_p,*data_p));

    return TRUE;
}

bool
populate_ip_fn (void *key, void *key_mask,
                void *data,
                uint32_t hash_idx, const void *cb_data)
{
    std::set<InputPropertiesHashEntry> *pop_set = (std::set<InputPropertiesHashEntry> *)cb_data;
    input_properties_swkey_t *key_p = (input_properties_swkey_t*)key;
    input_properties_actiondata *data_p = (input_properties_actiondata *)data;

    pop_set->insert(InputPropertiesHashEntry(*key_p,*data_p));

    return TRUE;
}

bool
print_fn (const void *key,
          const void *data,
          uint32_t hash_idx, const void *cb_data)
{
    hash_key_t *key1 = (hash_key_t *)key;
    hash_data_t *data1 = (hash_data_t *)data;

    printf("Hash Idx: %d %s Table_idx: %d ", hash_idx,
            sdk::table::hash::is_dleft(hash_idx) ? "DLEFT" : "OTCAM",
            sdk::table::hash::is_dleft(hash_idx) ?
            sdk::table::hash::get_dleft_id_from_hash_idx_(hash_idx) :
            sdk::table::hash::get_otcam_id_from_hash_idx_(hash_idx));
    printf("Key: %d %d ", key1->a, key1->b);
    printf("Data: %d %d\n", data1->p, data1->q);

    return TRUE;
}

/* ---------------------------------------------------------------------------
 *
 * Test Case 1:
 *      - Test Case to verify the update
 * - Create Hash table
 * - Insert Hash entry
 * - Print Hash Entries
 * - Update Hash entry
 * - Print Hash Entries
 */
TEST_F(hash_test, test1) {

    std::string table_name = "Input_Properties";
    char * table_str = const_cast<char*> (table_name.c_str());
    sdk::table::hash *test_hash = sdk::table::hash::factory(table_str, (uint32_t)P4TBL_ID_INPUT_PROPERTIES,
            (uint32_t)P4TBL_ID_INPUT_PROPERTIES_OTCAM,
            (uint32_t)100,
            (uint32_t)10, (uint32_t)sizeof(input_properties_swkey_t),
            (uint32_t)sizeof(input_properties_actiondata));

    // std::set<SampleHashEntry> sent_set;
    // std::set<SampleHashEntry> hash_set;

    std::set<InputPropertiesHashEntry> sent_set;
    std::set<InputPropertiesHashEntry> hash_set;

    sdk_ret_t rs = sdk::SDK_RET_OK;
    // hash_key_t key;
    // hash_data_t data;
    input_properties_swkey_t key;
    input_properties_actiondata data;
    uint32_t hash_idx = 0;

    memset(&key, 0, sizeof(key));
    memset(&data, 0, sizeof(data));

    key.capri_intrinsic_lif = 10;
    data.actionid = 1;

    rs = test_hash->insert((void *)&key, (void *)&data,  &hash_idx, FALSE);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);

    data.actionid = 2;
    rs = test_hash->update(hash_idx, &data);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);
    sent_set.insert(InputPropertiesHashEntry(key,data));

    rs = test_hash->iterate(populate_ip_fn, &hash_set, sdk::table::hash::BOTH);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);

    ASSERT_TRUE(sent_set == hash_set);

    rs = test_hash->remove(hash_idx);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);
}

/* ---------------------------------------------------------------------------
 *
 * Test Case 2:
 *      - Test Case to verify the duplicate insert failure
 * - Create Hash table
 * - Insert Hash entry
 * - Print Hash Entries
 * - Update Hash entry
 * - Print Hash Entries
 */
TEST_F(hash_test, test2) {

    std::string table_name = "Input_Properties";
    char * table_str = const_cast<char*> (table_name.c_str());
    sdk::table::hash *test_hash = sdk::table::hash::factory(table_str, (uint32_t)P4TBL_ID_INPUT_PROPERTIES,
            (uint32_t)P4TBL_ID_INPUT_PROPERTIES_OTCAM,
            (uint32_t)100,
            (uint32_t)10, (uint32_t)sizeof(input_properties_swkey_t),
            (uint32_t)sizeof(input_properties_actiondata));

    sdk_ret_t rs = sdk::SDK_RET_OK;
    std::set<InputPropertiesHashEntry> sent_set;
    std::set<InputPropertiesHashEntry> hash_set;
    input_properties_swkey_t key;
    input_properties_actiondata data;
    uint32_t hash_idx = -1;

    memset(&key, 0, sizeof(key));
    memset(&data, 0, sizeof(data));

    key.capri_intrinsic_lif = 10;
    data.actionid = 1;

    rs = test_hash->insert((void *)&key, (void *)&data,  &hash_idx, FALSE);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);
    sent_set.insert(InputPropertiesHashEntry(key,data));

    rs = test_hash->insert((void *)&key, (void *)&data,  &hash_idx, FALSE);
    ASSERT_TRUE(rs == sdk::SDK_RET_ENTRY_EXISTS);

    rs = test_hash->iterate(populate_ip_fn, &hash_set, sdk::table::hash::BOTH);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);

    ASSERT_TRUE(sent_set == hash_set);

    rs = test_hash->remove(hash_idx);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);
}

/* ---------------------------------------------------------------------------
 *
 * Test Case 3:
 *      - Test Case to verify the removal
 * - Create Hash table
 * - Insert Hash entry
 * - Print Hash Entries
 * - Remove Hash entry
 * - Remove Hash entry - Should fail
 */
TEST_F(hash_test, test3) {

    std::string table_name = "Input_Properties";
    char * table_str = const_cast<char*> (table_name.c_str());
    sdk::table::hash *test_hash = sdk::table::hash::factory(table_str, (uint32_t)P4TBL_ID_INPUT_PROPERTIES,
            (uint32_t)P4TBL_ID_INPUT_PROPERTIES_OTCAM,
            (uint32_t)100,
            (uint32_t)10, (uint32_t)sizeof(input_properties_swkey_t),
            (uint32_t)sizeof(input_properties_actiondata));

    sdk_ret_t rs = sdk::SDK_RET_OK;
    std::set<InputPropertiesHashEntry> sent_set;
    std::set<InputPropertiesHashEntry> hash_set;
    input_properties_swkey_t key;
    input_properties_actiondata data;
    uint32_t hash_idx = -1;

    memset(&key, 0, sizeof(key));
    memset(&data, 0, sizeof(data));

    key.capri_intrinsic_lif = 10;
    data.actionid = 1;

    rs = test_hash->insert((void *)&key, (void *)&data,  &hash_idx, FALSE);
    sent_set.insert(InputPropertiesHashEntry(key,data));
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);

    rs = test_hash->iterate(populate_ip_fn, &hash_set, sdk::table::hash::BOTH);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);

    ASSERT_TRUE(sent_set == hash_set);

    rs = test_hash->remove(hash_idx);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);


    rs = test_hash->remove(hash_idx);
    ASSERT_TRUE(rs == sdk::SDK_RET_ENTRY_NOT_FOUND);
}

/* ---------------------------------------------------------------------------
 *
 * Test Case 4:
 *      - Test Case to verify the retrieval
 * - Create Hash table
 * - Insert Hash entry
 * - Print Hash Entries
 * - Retrieve Hash entry
 */
TEST_F(hash_test, test4) {

    std::string table_name = "Input_Properties";
    char * table_str = const_cast<char*> (table_name.c_str());
    sdk::table::hash *test_hash = sdk::table::hash::factory(table_str, (uint32_t)P4TBL_ID_INPUT_PROPERTIES,
            (uint32_t)P4TBL_ID_INPUT_PROPERTIES_OTCAM,
            (uint32_t)100,
            (uint32_t)10, (uint32_t)sizeof(input_properties_swkey_t),
            (uint32_t)sizeof(input_properties_actiondata));

    sdk_ret_t rs = sdk::SDK_RET_OK;
    input_properties_swkey_t key, retr_key;
    input_properties_actiondata data, retr_data;
    uint32_t hash_idx = 0;

    memset(&key, 0, sizeof(key));
    memset(&data, 0, sizeof(data));


    key.capri_intrinsic_lif = 10;
    data.actionid = 1;

    rs = test_hash->insert((void *)&key, (void *)&data,  &hash_idx, FALSE);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);

    rs = test_hash->retrieve(hash_idx + 1000, (void *)&retr_key, (void *)&retr_data);
    ASSERT_TRUE(rs == sdk::SDK_RET_OOB);

    rs = test_hash->retrieve(hash_idx + 1, (void *)&retr_key, (void *)&retr_data);
    ASSERT_TRUE(rs == sdk::SDK_RET_ENTRY_NOT_FOUND);


    // TODO: Bharat: Uncomment this once we move hash to SDK
    rs = test_hash->retrieve(1 << 28 /*otcam_bit_ in Hash*/, (void *)&retr_key, (void *)&retr_data);
    ASSERT_TRUE(rs == sdk::SDK_RET_ENTRY_NOT_FOUND);

    rs = test_hash->retrieve(hash_idx, (void *)&retr_key, (void *)&retr_data);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);

    ASSERT_TRUE(key.capri_intrinsic_lif == retr_key.capri_intrinsic_lif);
    ASSERT_TRUE(data.actionid == retr_data.actionid);

    rs = test_hash->remove(hash_idx);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);
}



/* ---------------------------------------------------------------------------
 *
 * Test Case 5:
 *      - Testing -ve cases
 *      - Insert Failure
 *      - Remove Failure
 *      - Update Failure
 */
TEST_F(hash_test, test5) {

    std::string table_name = "Input_Properties";
    char * table_str = const_cast<char*> (table_name.c_str());
    sdk::table::hash *test_hash = sdk::table::hash::factory(table_str, (uint32_t)P4TBL_ID_INPUT_PROPERTIES,
            (uint32_t)P4TBL_ID_INPUT_PROPERTIES_OTCAM,
            (uint32_t)100,
            (uint32_t)10, (uint32_t)sizeof(input_properties_swkey_t),
            (uint32_t)sizeof(input_properties_actiondata));

    sdk_ret_t rs = sdk::SDK_RET_OK;
    input_properties_swkey_t key;
    input_properties_actiondata data;
    uint32_t hash_idx = -1;

    memset(&key, 0, sizeof(key));
    memset(&data, 0, sizeof(data));

    key.capri_intrinsic_lif = 10;
    data.actionid = 1;

    rs = test_hash->insert((void *)&key, (void *)&data,  &hash_idx, FALSE);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);

    rs = test_hash->insert((void *)&key, (void *)&data,  &hash_idx, FALSE);
    ASSERT_TRUE(rs == sdk::SDK_RET_ENTRY_EXISTS);

    data.actionid = 3;
    rs = test_hash->update(hash_idx, &data);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);

    hash_idx++;
    rs = test_hash->remove(hash_idx);
    ASSERT_TRUE(rs == sdk::SDK_RET_ENTRY_NOT_FOUND);


    rs = test_hash->update(hash_idx, &data);
    ASSERT_TRUE(rs == sdk::SDK_RET_ENTRY_NOT_FOUND);

    hash_idx--;
    rs = test_hash->remove(hash_idx);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);
}


/* ---------------------------------------------------------------------------
 *
 * Test Case 6:
 *      - Scale Test to insert lots of entries
 *      - Insert Failure
 *      - Remove Failure
 *      - Update Failure
 */
TEST_F(hash_test, test6) {

    std::string table_name = "Input_Properties";
    char * table_str = const_cast<char*> (table_name.c_str());
    sdk::table::hash *test_hash = sdk::table::hash::factory(table_str, (uint32_t)P4TBL_ID_INPUT_PROPERTIES,
            (uint32_t)P4TBL_ID_INPUT_PROPERTIES_OTCAM,
            (uint32_t)100,
            (uint32_t)60, (uint32_t)sizeof(input_properties_swkey_t),
            (uint32_t)sizeof(input_properties_actiondata));

    sdk_ret_t rs = sdk::SDK_RET_OK;
    std::set<InputPropertiesHashEntry> sent_set;
    std::set<InputPropertiesHashEntry> hash_set;
    input_properties_swkey_t key;
    input_properties_actiondata data;
    uint32_t hash_idx[100];

    memset(&key, 0, sizeof(key));
    memset(&data, 0, sizeof(data));

    key.capri_intrinsic_lif = 10;
    data.actionid = 1;

    for (int i = 0; i < 50; i++) {
        rs = test_hash->insert((void *)&key, (void *)&data,  &hash_idx[i], FALSE);
        SDK_TRACE_DEBUG("rs: %d\n", rs);
        ASSERT_TRUE(rs == sdk::SDK_RET_OK);
        sent_set.insert(InputPropertiesHashEntry(key,data));
        key.capri_intrinsic_lif++;
    }

    printf("Done with inserts \n");

    rs = test_hash->iterate(populate_ip_fn, &hash_set, sdk::table::hash::BOTH);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);

    ASSERT_TRUE(sent_set == hash_set);

    for (int i = 0; i < 50; i++) {
        rs = test_hash->remove(hash_idx[i]);
        ASSERT_TRUE(rs == sdk::SDK_RET_OK);
    }

    SDK_TRACE_DEBUG("tableid:%d, table_name:%s, capacity:%d, oflow_capac:%d, "
                    "num_in_use:%d, oflow_num_in_use:%d"
                    "num_inserts:%d, num_insert_errors:%d, "
                    "num_deletes:%d, num_delete_errors:%d",
                    test_hash->id(), test_hash->name(),
                    test_hash->capacity(), test_hash->oflow_capacity(),
                    test_hash->num_entries_in_use(), test_hash->oflow_num_entries_in_use(),
                    test_hash->num_inserts(), test_hash->num_insert_errors(),
                    test_hash->num_deletes(), test_hash->num_delete_errors());
}

/* ---------------------------------------------------------------------------
 *
 * Test Case 7:
 *      - Test Case to verify the -ve inserts
 * - Create Hash table
 * - Insert Hash entry
 * - Test -ve insert cases
 */
TEST_F(hash_test, test7) {

    std::string table_name = "Input_Properties";
    char * table_str = const_cast<char*> (table_name.c_str());
    sdk::table::hash *test_hash = sdk::table::hash::factory(table_str, (uint32_t)P4TBL_ID_INPUT_PROPERTIES,
            (uint32_t)P4TBL_ID_INPUT_PROPERTIES_OTCAM,
            (uint32_t)100,
            (uint32_t)10, (uint32_t)sizeof(input_properties_swkey_t),
            (uint32_t)sizeof(input_properties_actiondata));

    std::set<InputPropertiesHashEntry> sent_set;
    std::set<InputPropertiesHashEntry> hash_set;

    sdk_ret_t rs = sdk::SDK_RET_OK;
    input_properties_swkey_t key;
    input_properties_actiondata data;
    uint32_t hash_idx = 0;

    memset(&key, 0, sizeof(key));
    memset(&data, 0, sizeof(data));

    key.capri_intrinsic_lif = 10;
    data.actionid = 1;

    rs = test_hash->insert((void *)&key, (void *)&data,  &hash_idx, FALSE);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);

    rs = test_hash->insert((void *)&key, (void *)&data,  &hash_idx, FALSE);
    ASSERT_TRUE(rs == sdk::SDK_RET_ENTRY_EXISTS);

    rs = test_hash->remove(hash_idx);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);
}

/* ---------------------------------------------------------------------------
 *
 * Test Case 8:
 *      - Scale Test to insert lots of entries
 */
TEST_F(hash_test, test8) {

    std::string table_name = "Input_Properties";
    char * table_str = const_cast<char*> (table_name.c_str());
    sdk::table::hash *test_hash = sdk::table::hash::factory(table_str, (uint32_t)P4TBL_ID_INPUT_PROPERTIES,
            (uint32_t)P4TBL_ID_INPUT_PROPERTIES_OTCAM,
            (uint32_t)100,
            (uint32_t)10, (uint32_t)sizeof(input_properties_swkey_t),
            (uint32_t)sizeof(input_properties_actiondata));
    sdk_ret_t rs = sdk::SDK_RET_OK;
    input_properties_swkey_t key;
    input_properties_actiondata data;
    uint32_t hash_idx[100] = {0};

    memset(&key, 0, sizeof(key));
    memset(&data, 0, sizeof(data));

    key.capri_intrinsic_lif = 10;
    data.actionid = 1;

    memset(hash_idx, ~0, sizeof(uint32_t) * 100);
    for (int i = 0; i < 100; i++) {
        rs = test_hash->insert((void *)&key, (void *)&data,  &hash_idx[i], FALSE);
        ASSERT_TRUE(rs == sdk::SDK_RET_OK || rs == sdk::SDK_RET_NO_RESOURCE);
        key.capri_intrinsic_lif++;
    }

    for (int i = 0; i < 100; i++) {
        if (hash_idx[i] != 0xFFFFFFFF) {
            rs = test_hash->remove(hash_idx[i]);
            ASSERT_TRUE(rs == sdk::SDK_RET_OK);
        }
    }
}

/* ---------------------------------------------------------------------------
 *
 * Test Case 9:
 *      - Test Case to verify the -ve update
 */
TEST_F(hash_test, test9) {

    std::string table_name = "Input_Properties";
    char * table_str = const_cast<char*> (table_name.c_str());
    sdk::table::hash *test_hash = sdk::table::hash::factory(table_str, (uint32_t)P4TBL_ID_INPUT_PROPERTIES,
            (uint32_t)P4TBL_ID_INPUT_PROPERTIES_OTCAM,
            (uint32_t)100,
            (uint32_t)10, (uint32_t)sizeof(input_properties_swkey_t),
            (uint32_t)sizeof(input_properties_actiondata));

    sdk_ret_t rs = sdk::SDK_RET_OK;
    input_properties_swkey_t key;
    input_properties_actiondata data;
    uint32_t hash_idx = 0;

    memset(&key, 0, sizeof(key));
    memset(&data, 0, sizeof(data));

    key.capri_intrinsic_lif = 10;
    data.actionid = 1;

    rs = test_hash->insert((void *)&key, (void *)&data,  &hash_idx, FALSE);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);

    rs = test_hash->update(hash_idx+1000, &data);
    ASSERT_TRUE(rs == sdk::SDK_RET_OOB);

    rs = test_hash->update(hash_idx+1, &data);
    ASSERT_TRUE(rs == sdk::SDK_RET_ENTRY_NOT_FOUND);

    rs = test_hash->update(1 << 28 /*otcam_bit_ in Hash*/, &data);
    printf("rs: %d\n", rs);
    ASSERT_TRUE(rs == sdk::SDK_RET_ENTRY_NOT_FOUND);
    // ASSERT_TRUE(rs == SDK_RET_INVALID_ARG);

    rs = test_hash->remove(hash_idx);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);
}

/* ---------------------------------------------------------------------------
 *
 * Test Case 10:
 *      - Test Case to verify the -ve remove
 */
TEST_F(hash_test, test10) {

    std::string table_name = "Input_Properties";
    char * table_str = const_cast<char*> (table_name.c_str());
    sdk::table::hash *test_hash = sdk::table::hash::factory(table_str, (uint32_t)P4TBL_ID_INPUT_PROPERTIES,
            (uint32_t)P4TBL_ID_INPUT_PROPERTIES_OTCAM,
            (uint32_t)100,
            (uint32_t)10, (uint32_t)sizeof(input_properties_swkey_t),
            (uint32_t)sizeof(input_properties_actiondata));
    sdk_ret_t rs = sdk::SDK_RET_OK;
    input_properties_swkey_t key;
    input_properties_actiondata data;
    uint32_t hash_idx = 0;

    memset(&key, 0, sizeof(key));
    memset(&data, 0, sizeof(data));

    key.capri_intrinsic_lif = 10;
    data.actionid = 1;

    rs = test_hash->insert((void *)&key, (void *)&data,  &hash_idx, FALSE);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);

    rs = test_hash->remove(hash_idx+1000);
    ASSERT_TRUE(rs == sdk::SDK_RET_OOB);

    rs = test_hash->remove(hash_idx+1);
    ASSERT_TRUE(rs == sdk::SDK_RET_ENTRY_NOT_FOUND);

    rs = test_hash->remove(1 << 28 /*otcam_bit_ in Hash*/);
    ASSERT_TRUE(rs == sdk::SDK_RET_ENTRY_NOT_FOUND);
    // ASSERT_TRUE(rs == SDK_RET_INVALID_ARG);

    rs = test_hash->remove(hash_idx);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);
}

TEST_F(hash_test, test11) {

    std::string table_name = "Input_Properties";
    char * table_str = const_cast<char*> (table_name.c_str());
    sdk::table::hash *test_hash = sdk::table::hash::factory(table_str, (uint32_t)P4TBL_ID_INPUT_PROPERTIES,
            (uint32_t)P4TBL_ID_INPUT_PROPERTIES_OTCAM,
            (uint32_t)100,
            (uint32_t)10, (uint32_t)sizeof(input_properties_swkey_t),
            (uint32_t)sizeof(input_properties_actiondata), sdk::table::hash::HASH_POLY1);

    // std::set<SampleHashEntry> sent_set;
    // std::set<SampleHashEntry> hash_set;

    std::set<InputPropertiesHashEntry> sent_set;
    std::set<InputPropertiesHashEntry> hash_set;

    sdk_ret_t rs = sdk::SDK_RET_OK;
    // hash_key_t key;
    // hash_data_t data;
    input_properties_swkey_t key;
    input_properties_actiondata data;
    uint32_t hash_idx = 0;

    memset(&key, 0, sizeof(key));
    memset(&data, 0, sizeof(data));

    key.capri_intrinsic_lif = 10;
    data.actionid = 1;

    rs = test_hash->insert((void *)&key, (void *)&data,  &hash_idx, FALSE);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);

    data.actionid = 2;
    rs = test_hash->update(hash_idx, &data);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);
    sent_set.insert(InputPropertiesHashEntry(key,data));

    rs = test_hash->iterate(populate_ip_fn, &hash_set, sdk::table::hash::BOTH);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);

    ASSERT_TRUE(sent_set == hash_set);

    rs = test_hash->remove(hash_idx);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);
}

TEST_F(hash_test, test12) {

    std::string table_name = "Input_Properties";
    char * table_str = const_cast<char*> (table_name.c_str());
    sdk::table::hash *test_hash = sdk::table::hash::factory(table_str, (uint32_t)P4TBL_ID_INPUT_PROPERTIES,
            (uint32_t)P4TBL_ID_INPUT_PROPERTIES_OTCAM,
            (uint32_t)100,
            (uint32_t)10, (uint32_t)sizeof(input_properties_swkey_t),
            (uint32_t)sizeof(input_properties_actiondata), sdk::table::hash::HASH_POLY2);

    // std::set<SampleHashEntry> sent_set;
    // std::set<SampleHashEntry> hash_set;

    std::set<InputPropertiesHashEntry> sent_set;
    std::set<InputPropertiesHashEntry> hash_set;

    sdk_ret_t rs = sdk::SDK_RET_OK;
    // hash_key_t key;
    // hash_data_t data;
    input_properties_swkey_t key;
    input_properties_actiondata data;
    uint32_t hash_idx = 0;

    memset(&key, 0, sizeof(key));
    memset(&data, 0, sizeof(data));

    key.capri_intrinsic_lif = 10;
    data.actionid = 1;

    rs = test_hash->insert((void *)&key, (void *)&data,  &hash_idx, FALSE);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);

    data.actionid = 2;
    rs = test_hash->update(hash_idx, &data);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);
    sent_set.insert(InputPropertiesHashEntry(key,data));

    rs = test_hash->iterate(populate_ip_fn, &hash_set, sdk::table::hash::BOTH);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);

    ASSERT_TRUE(sent_set == hash_set);

    rs = test_hash->remove(hash_idx);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);
}

TEST_F(hash_test, test13) {

    std::string table_name = "Input_Properties";
    char * table_str = const_cast<char*> (table_name.c_str());
    sdk::table::hash *test_hash = sdk::table::hash::factory(table_str, (uint32_t)P4TBL_ID_INPUT_PROPERTIES,
            (uint32_t)P4TBL_ID_INPUT_PROPERTIES_OTCAM,
            (uint32_t)100,
            (uint32_t)10, (uint32_t)sizeof(input_properties_swkey_t),
            (uint32_t)sizeof(input_properties_actiondata), sdk::table::hash::HASH_POLY3);

    // std::set<SampleHashEntry> sent_set;
    // std::set<SampleHashEntry> hash_set;

    std::set<InputPropertiesHashEntry> sent_set;
    std::set<InputPropertiesHashEntry> hash_set;

    sdk_ret_t rs = sdk::SDK_RET_OK;
    // hash_key_t key;
    // hash_data_t data;
    input_properties_swkey_t key;
    input_properties_actiondata data;
    uint32_t hash_idx = 0;

    memset(&key, 0, sizeof(key));
    memset(&data, 0, sizeof(data));

    key.capri_intrinsic_lif = 10;
    data.actionid = 1;

    rs = test_hash->insert((void *)&key, (void *)&data,  &hash_idx, FALSE);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);

    data.actionid = 2;
    rs = test_hash->update(hash_idx, &data);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);
    sent_set.insert(InputPropertiesHashEntry(key,data));

    rs = test_hash->iterate(populate_ip_fn, &hash_set, sdk::table::hash::BOTH);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);

    ASSERT_TRUE(sent_set == hash_set);

    rs = test_hash->remove(hash_idx);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);
}


int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
