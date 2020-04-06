#include <gtest/gtest.h>
#include <stdio.h>
#include "nic/sdk/lib/table/hbmhash/hbm_hash.hpp"
#include "nic/sdk/lib/table/hbmhash/hbm_hash_entry.hpp"
#include "nic/sdk/lib/p4/p4_api.hpp"
#include "gen/p4gen/p4/include/p4pd.h"
#include <boost/multiprecision/cpp_int.hpp>
#include <chrono>
#include <fstream>
#include "nic/hal/pd/utils/flow/test/jenkins_spooky/spooky.h"
#include "include/sdk/base.hpp"
#include "nic/include/hal_mem.hpp"

using sdk::table::HbmHash;
using sdk::table::HbmHashEntry;
using boost::multiprecision::uint512_t;
using boost::multiprecision::uint128_t;
using namespace std::chrono;

uint32_t
jenkins_one_at_a_time_hash(uint8_t* key, size_t length) {
  size_t i = 0;
  uint32_t hash = 0;
  while (i != length) {
    hash += key[i++];
    hash += hash << 10;
    hash ^= hash >> 6;
  }
  hash += hash << 3;
  hash ^= hash >> 11;
  hash += hash << 15;
  return hash;
}

uint32_t
hash_lsb_select(uint32_t hash_val, uint32_t num_lsb_bits)
{
    return (hash_val & ((1 << num_lsb_bits) - 1));
}

uint32_t
calc_jenkins_hash_(void *key, void *data, HbmHash *fl, bool spooky)
{
    sdk_ret_t                       rs = sdk::SDK_RET_OK;
    HbmHashEntry                       *entry = NULL;
    uint32_t                        hash_val = 0;
    void                            *hwkey = NULL;

    // create a flow entry
    entry = HbmHashEntry::factory(key, fl->get_key_len(), data, fl->get_data_len(),
                               fl->get_hwkey_len(), false, fl);

    // call P4 API to get hw key
    // hwkey = HAL_CALLOC(HAL_MEM_ALLOC_FLOW_HW_KEY, fl->get_hwkey_len());
    hwkey = HAL_CALLOC(0, fl->get_hwkey_len());

    rs = entry->form_hw_key(fl->get_table_id(), hwkey);
    if (rs != sdk::SDK_RET_OK) SDK_ASSERT(0);

    // cal. hash
    if (spooky) {
        hash_val = SpookyHash::Hash32((uint8_t *)hwkey, fl->get_hwkey_len(), 0);
    } else {
        hash_val = jenkins_one_at_a_time_hash((uint8_t *)hwkey, fl->get_hwkey_len());
    }
    HAL_FREE(0, hwkey);

    // delete entry;
    HbmHashEntry::destroy(entry, fl);

    return hash_val;
}

class hbmhash_test : public ::testing::Test {
protected:
  hbmhash_test() {
  }

  virtual ~hbmhash_test() {
  }

  // will be called immediately after the constructor before each test
  virtual void SetUp() {
  }

  // will be called immediately after each test before the destructor
  virtual void TearDown() {
  }

};


typedef struct flow_key_s {
    int a;
    int b;
} flow_key_t;

typedef struct flow_data_s {
    uint32_t p;
} flow_data_t;

// ----------------------------------------------------------------------------
// Test 1:
//      - Create flow table
//      - Insert a flow entry.
//      - Insert a flow entry.
//      - Insert a flow entry.
// ----------------------------------------------------------------------------
TEST_F(hbmhash_test, test1) {

    sdk_ret_t rs;
    uint32_t flow_idx = 0;

    flow_hash_swkey key;
    flow_hash_actiondata_t data;

    memset(&key, 0, sizeof(key));
    memset(&data, 0, sizeof(data));

    HbmHash *fl = HbmHash::factory("HbmHashTable", P4TBL_ID_FLOW_HASH, P4TBL_ID_FLOW_HASH_OVERFLOW,
                   2097152, 16384, sizeof(key), sizeof(data), 6, 8);

    key.flow_lkp_metadata_lkp_sport = 10;
    data.action_id = 1;

    rs = fl->insert((void *)&key, (void *)&data, &flow_idx);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);
    printf("flow idx: %d\n", flow_idx);

    key.flow_lkp_metadata_lkp_sport = 30;
    rs = fl->insert((void *)&key, (void *)&data, &flow_idx);
    printf("rs: %d\n", rs);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);
    printf("flow idx: %d\n", flow_idx);

    key.flow_lkp_metadata_lkp_sport = 20;
    key.flow_lkp_metadata_lkp_dport = 20;
    rs = fl->insert((void *)&key, (void *)&data, &flow_idx);
    printf("rs: %d\n", rs);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);
    printf("flow idx: %d\n", flow_idx);
}

// ----------------------------------------------------------------------------
// Test 2:
//      - Create flow table
//      - Insert a flow entry.
// ----------------------------------------------------------------------------
TEST_F(hbmhash_test, test2) {

    sdk_ret_t rs;
    flow_hash_swkey key;
    flow_hash_actiondata_t data;
    uint32_t flow_idx = 0;

    memset(&key, 0, sizeof(key));
    memset(&data, 0, sizeof(data));
    HbmHash *fl = HbmHash::factory("HbmHashTable", P4TBL_ID_FLOW_HASH, P4TBL_ID_FLOW_HASH_OVERFLOW,
            2097152, 16384, sizeof(key), sizeof(data));

    key.flow_lkp_metadata_lkp_sport = 10;
    data.action_id = 1;

    rs = fl->insert((void *)&key, (void *)&data, &flow_idx);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);
    printf("flow idx: %d\n", flow_idx);

    data.action_id = 2;
    rs = fl->update(flow_idx, (void *)&data);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);
    printf("flow idx: %d\n", flow_idx);
}

// ----------------------------------------------------------------------------
// Test 3:
//      - Create flow table
//      - Insert a flow entry.
// ----------------------------------------------------------------------------
TEST_F(hbmhash_test, test3) {

    sdk_ret_t rs;
    flow_hash_swkey key;
    flow_hash_actiondata_t data;
    uint32_t flow_idx = 0;

    memset(&key, 0, sizeof(key));
    memset(&data, 0, sizeof(data));
    HbmHash *fl = HbmHash::factory("HbmHashTable", P4TBL_ID_FLOW_HASH, P4TBL_ID_FLOW_HASH_OVERFLOW,
            2097152, 16384, sizeof(key), sizeof(data));

    key.flow_lkp_metadata_lkp_sport = 10;
    data.action_id = 1;

    for (int i = 10; i < 20; i++) {
        key.flow_lkp_metadata_lkp_sport++;
        data.action_id = 1;
        rs = fl->insert((void *)&key, (void *)&data,  &flow_idx);
        ASSERT_TRUE(rs == sdk::SDK_RET_OK);
        printf("flow idx: %d\n", flow_idx);
    }

}

// ----------------------------------------------------------------------------
// Test 4:
//      - Create flow table
//      - Insert a flow entry.
//      - Insert a flow entry.
// ----------------------------------------------------------------------------
TEST_F(hbmhash_test, test4) {

    sdk_ret_t rs;
    flow_hash_swkey key;
    flow_hash_actiondata_t data;
    uint32_t flow_idx = 0;

    memset(&key, 0, sizeof(key));
    memset(&data, 0, sizeof(data));
    HbmHash *fl = HbmHash::factory("HbmHashTable", P4TBL_ID_FLOW_HASH, P4TBL_ID_FLOW_HASH_OVERFLOW,
            2097152, 16384, sizeof(key), sizeof(data));

    key.flow_lkp_metadata_lkp_sport = 10;
    data.action_id = 1;

    rs = fl->insert((void *)&key, (void *)&data, &flow_idx);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);
    printf("flow idx: %d\n", flow_idx);

    rs = fl->insert((void *)&key, (void *)&data, &flow_idx);
    HAL_TRACE_DEBUG("rs: {}", rs);
    ASSERT_TRUE(rs == sdk::SDK_RET_ENTRY_EXISTS);
}

// ----------------------------------------------------------------------------
// Test 5:
//      - Create flow table
//      - Insert a flow entry.
// ----------------------------------------------------------------------------
TEST_F(hbmhash_test, test5) {

    sdk_ret_t rs;
    flow_hash_swkey key;
    flow_hash_actiondata_t data;
    uint32_t flow_idx = 0;

    memset(&key, 0, sizeof(key));
    memset(&data, 0, sizeof(data));
    HbmHash *fl = HbmHash::factory("HbmHashTable", P4TBL_ID_FLOW_HASH, P4TBL_ID_FLOW_HASH_OVERFLOW,
            2097152, 16384, sizeof(key), sizeof(data));

    key.flow_lkp_metadata_lkp_sport = 10;
    data.action_id = 1;

    for (int i = 0; i < 100; i++) {
        key.flow_lkp_metadata_lkp_sport = key.flow_lkp_metadata_lkp_sport + 10;
        data.action_id = 2;
        rs = fl->insert((void *)&key, (void *)&data,  &flow_idx);
        printf("rs: %d \n", rs);
        ASSERT_TRUE(rs == sdk::SDK_RET_OK);
        printf("flow idx: %d\n", flow_idx);
    }

}

// ----------------------------------------------------------------------------
// Test 6:
//      - Create flow table
//      - -ve update cases
// ----------------------------------------------------------------------------
TEST_F(hbmhash_test, test6) {

    sdk_ret_t rs;
    flow_hash_swkey key;
    flow_hash_actiondata_t data;
    uint32_t flow_idx[3] = { 0 };

    memset(&key, 0, sizeof(key));
    memset(&data, 0, sizeof(data));
    HbmHash *fl = HbmHash::factory("HbmHashTable", P4TBL_ID_FLOW_HASH, P4TBL_ID_FLOW_HASH_OVERFLOW,
            2097152, 16384, sizeof(key), sizeof(data));

    key.flow_lkp_metadata_lkp_sport = 10;
    data.action_id = 1;

    for (int i = 0; i < 3; i++) {
        key.flow_lkp_metadata_lkp_sport = key.flow_lkp_metadata_lkp_sport + 10;
        rs = fl->insert((void *)&key, (void *)&data, &flow_idx[i]);
        ASSERT_TRUE(rs == sdk::SDK_RET_OK);
    }

    data.action_id = 2;
    rs = fl->update(flow_idx[2] + 1, (void *)&data);
    ASSERT_TRUE(rs == sdk::SDK_RET_ENTRY_NOT_FOUND);

    for (int i = 0; i < 3; i++) {
        rs = fl->update(flow_idx[i], (void *)&data);
        ASSERT_TRUE(rs == sdk::SDK_RET_OK);
    }

}

// ----------------------------------------------------------------------------
// Test 7:
//      - Create flow table
// ----------------------------------------------------------------------------
TEST_F(hbmhash_test, DISABLED_test7) {

    // sdk_ret_t rs;
    flow_hash_swkey key = {0};
    flow_hash_actiondata_t data = {0};
    unsigned seed = std::time(0);
    std::srand (seed);
    // uint32_t flow_idx[1000000] = { 0 };
    uint32_t ft_bits = 0;

    memset(&key, 0, sizeof(key));
    memset(&data, 0, sizeof(data));
    HbmHash *fl = HbmHash::factory("HbmHashTable", P4TBL_ID_FLOW_HASH, P4TBL_ID_FLOW_HASH_OVERFLOW,
            262144, 16384, sizeof(key), sizeof(data));
            // 1048576, 16384, sizeof(key), sizeof(data));
            // 2097152, 16384, sizeof(key), sizeof(data));

    while (1) {
        for (int i = 0; i < 4; i++) {
            key.flow_lkp_metadata_lkp_src[i] = rand() % 256;
            key.flow_lkp_metadata_lkp_dst[i] = rand() % 256;
        }
        key.flow_lkp_metadata_lkp_vrf = rand() % 65536;
        key.flow_lkp_metadata_lkp_sport = rand() % 65536;
        key.flow_lkp_metadata_lkp_dport = rand() % 65536;
        key.flow_lkp_metadata_lkp_proto = rand() % 256;

        ft_bits = fl->calc_hash_(&key, &data);
        HAL_TRACE_DEBUG("Checking:{:#x}", ft_bits);

        if (ft_bits == 0x10001) {
            HAL_TRACE_DEBUG("MATCH::");
            for (int i = 0; i < 4; i++) {
                HAL_TRACE_DEBUG("Src[{}]: {:#x}", i, key.flow_lkp_metadata_lkp_src[i]);
                HAL_TRACE_DEBUG("Dst[{}]: {:#x}", i, key.flow_lkp_metadata_lkp_dst[i]);
            }
            HAL_TRACE_DEBUG("Vrf: {:#x}, sport: {:#x}, dport: {:#x}, proto: {:#x}",
                    key.flow_lkp_metadata_lkp_vrf,
                    key.flow_lkp_metadata_lkp_sport,
                    key.flow_lkp_metadata_lkp_dport,
                    key.flow_lkp_metadata_lkp_proto);
        }
    }


#if 0
    data.action_id = 0;

    uint32_t base = 0;
    for (int i = 0; i < 1000000; i++) {
        rs = fl.insert((void *)&key, (void *)&data, &flow_idx[i]);
        ASSERT_TRUE(rs == sdk::SDK_RET_OK || rs == sdk::SDK_RET_COLLISION);
        key.flow_lkp_metadata_lkp_dport++;
        if (key.flow_lkp_metadata_lkp_dport == 0) {
            base++;
            key.flow_lkp_metadata_lkp_sport = base;
        }
    }
#endif

#if 0
    data.action_id = 2;
    rs = fl.update(flow_idx[2] + 1, (void *)&data);
    ASSERT_TRUE(rs == sdk::SDK_RET_ENTRY_NOT_FOUND);

    for (int i = 0; i < 3; i++) {
        rs = fl.update(flow_idx[i], (void *)&data);
        ASSERT_TRUE(rs == sdk::SDK_RET_OK);
    }
#endif
}

// ----------------------------------------------------------------------------
// Test 8:
//      - Create flow with collisions
// ----------------------------------------------------------------------------
TEST_F(hbmhash_test, test8) {

    sdk_ret_t rs;
    flow_hash_swkey key = {0};
    flow_hash_actiondata_t data = {0};
    uint32_t flow_idx[9] = { 0 };

    memset(&key, 0, sizeof(key));
    memset(&data, 0, sizeof(data));
    HbmHash *fl = HbmHash::factory("HbmHashTable", P4TBL_ID_FLOW_HASH, P4TBL_ID_FLOW_HASH_OVERFLOW,
            262144, 16384, sizeof(key), sizeof(data));
            // 1048576, 16384, sizeof(key), sizeof(data));
            // 2097152, 16384, sizeof(key), sizeof(data));

    key.flow_lkp_metadata_lkp_src[0] = 0x01;
    key.flow_lkp_metadata_lkp_src[1] = 0x01;
    key.flow_lkp_metadata_lkp_src[2] = 0x00;
    key.flow_lkp_metadata_lkp_src[3] = 0x0a;
    key.flow_lkp_metadata_lkp_dst[0] = 0x01;
    key.flow_lkp_metadata_lkp_dst[1] = 0x00;
    key.flow_lkp_metadata_lkp_dst[2] = 0x00;
    key.flow_lkp_metadata_lkp_dst[3] = 0x0b;


    // Entry 1:
    key.flow_lkp_metadata_lkp_vrf = 3;
    key.flow_lkp_metadata_lkp_sport = 0xa277;
    key.flow_lkp_metadata_lkp_dport = 0x2;
    data.action_id = 0;
    rs = fl->insert((void *)&key, (void *)&data, &flow_idx[0]);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK || rs == sdk::SDK_RET_COLLISION);

    // Entry 2:
    key.flow_lkp_metadata_lkp_vrf = 3;
    key.flow_lkp_metadata_lkp_sport = 0x3b01;
    key.flow_lkp_metadata_lkp_dport = 0x3;
    data.action_id = 0;
    rs = fl->insert((void *)&key, (void *)&data, &flow_idx[1]);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK || rs == sdk::SDK_RET_COLLISION);

    // Remove Entry 1
    rs = fl->remove(flow_idx[0]);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);

    // Remove Entry 2
    rs = fl->remove(flow_idx[1]);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);
}

// ----------------------------------------------------------------------------
// Test 9:
//      - Create flow with collisions
// ----------------------------------------------------------------------------
TEST_F(hbmhash_test, test9) {

    sdk_ret_t rs;
    flow_hash_swkey key = {0};
    flow_hash_actiondata_t data = {0};
    uint32_t flow_idx[9] = { 0 };

    memset(&key, 0, sizeof(key));
    memset(&data, 0, sizeof(data));
    HbmHash *fl = HbmHash::factory("HbmHashTable", P4TBL_ID_FLOW_HASH, P4TBL_ID_FLOW_HASH_OVERFLOW,
            262144, 16384, sizeof(key), sizeof(data));
            // 1048576, 16384, sizeof(key), sizeof(data));
            // 2097152, 16384, sizeof(key), sizeof(data));

    key.flow_lkp_metadata_lkp_src[0] = 0x01;
    key.flow_lkp_metadata_lkp_src[1] = 0x01;
    key.flow_lkp_metadata_lkp_src[2] = 0x00;
    key.flow_lkp_metadata_lkp_src[3] = 0x0a;
    key.flow_lkp_metadata_lkp_dst[0] = 0x01;
    key.flow_lkp_metadata_lkp_dst[1] = 0x00;
    key.flow_lkp_metadata_lkp_dst[2] = 0x00;
    key.flow_lkp_metadata_lkp_dst[3] = 0x0b;


    // Entry 1:
    key.flow_lkp_metadata_lkp_vrf = 3;
    key.flow_lkp_metadata_lkp_sport = 0xa277;
    key.flow_lkp_metadata_lkp_dport = 0x2;
    data.action_id = 0;
    rs = fl->insert((void *)&key, (void *)&data, &flow_idx[0]);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK || rs == sdk::SDK_RET_COLLISION);

    // Entry 2:
    key.flow_lkp_metadata_lkp_vrf = 3;
    key.flow_lkp_metadata_lkp_sport = 0x3b01;
    key.flow_lkp_metadata_lkp_dport = 0x3;
    data.action_id = 0;
    rs = fl->insert((void *)&key, (void *)&data, &flow_idx[1]);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK || rs == sdk::SDK_RET_COLLISION);

    // Remove Entry 2
    rs = fl->remove(flow_idx[1]);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);

    // Remove Entry 1
    rs = fl->remove(flow_idx[0]);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);
}

// ----------------------------------------------------------------------------
// Test 10:
//      - Create flow with collisions
//      - Insert 1
//      - Insert 2
//      - Insert 3
//      - Delete 1
//      - Delete 2
//      - Delete 3
// ----------------------------------------------------------------------------
TEST_F(hbmhash_test, test10) {

    sdk_ret_t rs;
    flow_hash_swkey key = {0};
    flow_hash_actiondata_t data = {0};
    uint32_t flow_idx[9] = { 0 };

    memset(&key, 0, sizeof(key));
    memset(&data, 0, sizeof(data));
    HbmHash *fl = HbmHash::factory("HbmHashTable", P4TBL_ID_FLOW_HASH, P4TBL_ID_FLOW_HASH_OVERFLOW,
            262144, 16384, sizeof(key), sizeof(data));
            // 1048576, 16384, sizeof(key), sizeof(data));
            // 2097152, 16384, sizeof(key), sizeof(data));

    key.flow_lkp_metadata_lkp_src[0] = 0x01;
    key.flow_lkp_metadata_lkp_src[1] = 0x01;
    key.flow_lkp_metadata_lkp_src[2] = 0x00;
    key.flow_lkp_metadata_lkp_src[3] = 0x0a;
    key.flow_lkp_metadata_lkp_dst[0] = 0x01;
    key.flow_lkp_metadata_lkp_dst[1] = 0x00;
    key.flow_lkp_metadata_lkp_dst[2] = 0x00;
    key.flow_lkp_metadata_lkp_dst[3] = 0x0b;


    // Entry 1:
    key.flow_lkp_metadata_lkp_vrf = 3;
    key.flow_lkp_metadata_lkp_sport = 0xa277;
    key.flow_lkp_metadata_lkp_dport = 0x2;
    data.action_id = 0;
    rs = fl->insert((void *)&key, (void *)&data, &flow_idx[0]);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK || rs == sdk::SDK_RET_COLLISION);

    // Entry 2:
    key.flow_lkp_metadata_lkp_vrf = 3;
    key.flow_lkp_metadata_lkp_sport = 0x3b01;
    key.flow_lkp_metadata_lkp_dport = 0x3;
    data.action_id = 0;
    rs = fl->insert((void *)&key, (void *)&data, &flow_idx[1]);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK || rs == sdk::SDK_RET_COLLISION);

    // Entry 3:
    key.flow_lkp_metadata_lkp_vrf = 3;
    key.flow_lkp_metadata_lkp_sport = 0xd238;
    key.flow_lkp_metadata_lkp_dport = 0x4;
    data.action_id = 0;
    rs = fl->insert((void *)&key, (void *)&data, &flow_idx[2]);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK || rs == sdk::SDK_RET_COLLISION);

    // Remove Entry 1
    rs = fl->remove(flow_idx[0]);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);

    // Remove Entry 1
    rs = fl->remove(flow_idx[1]);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);

    // Remove Entry 2
    rs = fl->remove(flow_idx[2]);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);
}

// ----------------------------------------------------------------------------
// Test 11:
//      - Create flow with collisions
//      - Insert 1
//      - Insert 2
//      - Insert 3
//      - Delete 1
//      - Delete 3
//      - Delete 2
// ----------------------------------------------------------------------------
TEST_F(hbmhash_test, test11) {

    sdk_ret_t rs;
    flow_hash_swkey key = {0};
    flow_hash_actiondata_t data = {0};
    uint32_t flow_idx[9] = { 0 };

    memset(&key, 0, sizeof(key));
    memset(&data, 0, sizeof(data));
    HbmHash *fl = HbmHash::factory("HbmHashTable", P4TBL_ID_FLOW_HASH, P4TBL_ID_FLOW_HASH_OVERFLOW,
            262144, 16384, sizeof(key), sizeof(data));
            // 1048576, 16384, sizeof(key), sizeof(data));
            // 2097152, 16384, sizeof(key), sizeof(data));

    key.flow_lkp_metadata_lkp_src[0] = 0x01;
    key.flow_lkp_metadata_lkp_src[1] = 0x01;
    key.flow_lkp_metadata_lkp_src[2] = 0x00;
    key.flow_lkp_metadata_lkp_src[3] = 0x0a;
    key.flow_lkp_metadata_lkp_dst[0] = 0x01;
    key.flow_lkp_metadata_lkp_dst[1] = 0x00;
    key.flow_lkp_metadata_lkp_dst[2] = 0x00;
    key.flow_lkp_metadata_lkp_dst[3] = 0x0b;


    // Entry 1:
    key.flow_lkp_metadata_lkp_vrf = 3;
    key.flow_lkp_metadata_lkp_sport = 0xa277;
    key.flow_lkp_metadata_lkp_dport = 0x2;
    data.action_id = 0;
    rs = fl->insert((void *)&key, (void *)&data, &flow_idx[0]);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK || rs == sdk::SDK_RET_COLLISION);

    // Entry 2:
    key.flow_lkp_metadata_lkp_vrf = 3;
    key.flow_lkp_metadata_lkp_sport = 0x3b01;
    key.flow_lkp_metadata_lkp_dport = 0x3;
    data.action_id = 0;
    rs = fl->insert((void *)&key, (void *)&data, &flow_idx[1]);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK || rs == sdk::SDK_RET_COLLISION);

    // Entry 3:
    key.flow_lkp_metadata_lkp_vrf = 3;
    key.flow_lkp_metadata_lkp_sport = 0xd238;
    key.flow_lkp_metadata_lkp_dport = 0x4;
    data.action_id = 0;
    rs = fl->insert((void *)&key, (void *)&data, &flow_idx[2]);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK || rs == sdk::SDK_RET_COLLISION);

    // Remove Entry 1
    rs = fl->remove(flow_idx[0]);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);

    // Remove Entry 1
    rs = fl->remove(flow_idx[2]);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);

    // Remove Entry 2
    rs = fl->remove(flow_idx[1]);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);
}
// ----------------------------------------------------------------------------
// Test 12:
//      - Create flow with collisions
//      - Insert 1
//      - Insert 2
//      - Insert 3
//      - Insert 4
//      - Delete 1
//      - Delete 2
//      - Delete 3
//      - Delete 4
// ----------------------------------------------------------------------------
TEST_F(hbmhash_test, test12) {

    sdk_ret_t rs;
    flow_hash_swkey key = {0};
    flow_hash_actiondata_t data = {0};
    uint32_t flow_idx[9] = { 0 };

    memset(&key, 0, sizeof(key));
    memset(&data, 0, sizeof(data));
    HbmHash *fl = HbmHash::factory("HbmHashTable", P4TBL_ID_FLOW_HASH, P4TBL_ID_FLOW_HASH_OVERFLOW,
            262144, 16384, sizeof(key), sizeof(data));
            // 1048576, 16384, sizeof(key), sizeof(data));
            // 2097152, 16384, sizeof(key), sizeof(data));

    key.flow_lkp_metadata_lkp_src[0] = 0x01;
    key.flow_lkp_metadata_lkp_src[1] = 0x01;
    key.flow_lkp_metadata_lkp_src[2] = 0x00;
    key.flow_lkp_metadata_lkp_src[3] = 0x0a;
    key.flow_lkp_metadata_lkp_dst[0] = 0x01;
    key.flow_lkp_metadata_lkp_dst[1] = 0x00;
    key.flow_lkp_metadata_lkp_dst[2] = 0x00;
    key.flow_lkp_metadata_lkp_dst[3] = 0x0b;


    // Entry 1:
    key.flow_lkp_metadata_lkp_vrf = 3;
    key.flow_lkp_metadata_lkp_sport = 0xa277;
    key.flow_lkp_metadata_lkp_dport = 0x2;
    data.action_id = 0;
    rs = fl->insert((void *)&key, (void *)&data, &flow_idx[0]);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK || rs == sdk::SDK_RET_COLLISION);

    // Entry 2:
    key.flow_lkp_metadata_lkp_vrf = 3;
    key.flow_lkp_metadata_lkp_sport = 0x3b01;
    key.flow_lkp_metadata_lkp_dport = 0x3;
    data.action_id = 0;
    rs = fl->insert((void *)&key, (void *)&data, &flow_idx[1]);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK || rs == sdk::SDK_RET_COLLISION);

    // Entry 3:
    key.flow_lkp_metadata_lkp_vrf = 3;
    key.flow_lkp_metadata_lkp_sport = 0xd238;
    key.flow_lkp_metadata_lkp_dport = 0x4;
    data.action_id = 0;
    rs = fl->insert((void *)&key, (void *)&data, &flow_idx[2]);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK || rs == sdk::SDK_RET_COLLISION);

    // Entry 4:
    key.flow_lkp_metadata_lkp_vrf = 3;
    key.flow_lkp_metadata_lkp_sport = 0x4b4e;
    key.flow_lkp_metadata_lkp_dport = 0x5;
    data.action_id = 0;
    rs = fl->insert((void *)&key, (void *)&data, &flow_idx[3]);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK || rs == sdk::SDK_RET_COLLISION);

    // Remove Entry 1
    rs = fl->remove(flow_idx[0]);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);

    // Remove Entry 2
    rs = fl->remove(flow_idx[1]);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);

    // Remove Entry 3
    rs = fl->remove(flow_idx[2]);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);

    // Remove Entry 4
    rs = fl->remove(flow_idx[3]);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);
}
// ----------------------------------------------------------------------------
// Test 13:
//      - Create flow with collisions
//      - Insert 1
//      - Insert 2
//      - Insert 3
//      - Insert 4
//      - Delete 4
//      - Delete 3
//      - Delete 2
//      - Delete 1
// ----------------------------------------------------------------------------
TEST_F(hbmhash_test, test13) {

    sdk_ret_t rs;
    flow_hash_swkey key = {0};
    flow_hash_actiondata_t data = {0};
    uint32_t flow_idx[9] = { 0 };

    memset(&key, 0, sizeof(key));
    memset(&data, 0, sizeof(data));
    HbmHash *fl = HbmHash::factory("HbmHashTable", P4TBL_ID_FLOW_HASH, P4TBL_ID_FLOW_HASH_OVERFLOW,
            262144, 16384, sizeof(key), sizeof(data));
            // 1048576, 16384, sizeof(key), sizeof(data));
            // 2097152, 16384, sizeof(key), sizeof(data));

    key.flow_lkp_metadata_lkp_src[0] = 0x01;
    key.flow_lkp_metadata_lkp_src[1] = 0x01;
    key.flow_lkp_metadata_lkp_src[2] = 0x00;
    key.flow_lkp_metadata_lkp_src[3] = 0x0a;
    key.flow_lkp_metadata_lkp_dst[0] = 0x01;
    key.flow_lkp_metadata_lkp_dst[1] = 0x00;
    key.flow_lkp_metadata_lkp_dst[2] = 0x00;
    key.flow_lkp_metadata_lkp_dst[3] = 0x0b;


    // Entry 1:
    key.flow_lkp_metadata_lkp_vrf = 3;
    key.flow_lkp_metadata_lkp_sport = 0xa277;
    key.flow_lkp_metadata_lkp_dport = 0x2;
    data.action_id = 0;
    rs = fl->insert((void *)&key, (void *)&data, &flow_idx[0]);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK || rs == sdk::SDK_RET_COLLISION);

    // Entry 2:
    key.flow_lkp_metadata_lkp_vrf = 3;
    key.flow_lkp_metadata_lkp_sport = 0x3b01;
    key.flow_lkp_metadata_lkp_dport = 0x3;
    data.action_id = 0;
    rs = fl->insert((void *)&key, (void *)&data, &flow_idx[1]);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK || rs == sdk::SDK_RET_COLLISION);

    // Entry 3:
    key.flow_lkp_metadata_lkp_vrf = 3;
    key.flow_lkp_metadata_lkp_sport = 0xd238;
    key.flow_lkp_metadata_lkp_dport = 0x4;
    data.action_id = 0;
    rs = fl->insert((void *)&key, (void *)&data, &flow_idx[2]);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK || rs == sdk::SDK_RET_COLLISION);

    // Entry 4:
    key.flow_lkp_metadata_lkp_vrf = 3;
    key.flow_lkp_metadata_lkp_sport = 0x4b4e;
    key.flow_lkp_metadata_lkp_dport = 0x5;
    data.action_id = 0;
    rs = fl->insert((void *)&key, (void *)&data, &flow_idx[3]);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK || rs == sdk::SDK_RET_COLLISION);

    // Remove Entry 1
    rs = fl->remove(flow_idx[3]);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);

    // Remove Entry 2
    rs = fl->remove(flow_idx[2]);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);

    // Remove Entry 3
    rs = fl->remove(flow_idx[1]);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);

    // Remove Entry 4
    rs = fl->remove(flow_idx[0]);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);
}

// ----------------------------------------------------------------------------
// Test 14:
//      - Create flow with collisions
//      - Insert 1
//      - Insert 2
//      - Insert 3
//      - Insert 4
//      - Randomly delete
// ----------------------------------------------------------------------------
TEST_F(hbmhash_test, test14) {

    sdk_ret_t rs;
    flow_hash_swkey key = {0};
    flow_hash_actiondata_t data = {0};
    uint32_t flow_idx[9] = { 0 };
    std::srand ( unsigned ( std::time(0) ) );
    std::vector<int> myvector;

    for (int i=0; i<4; ++i) myvector.push_back(i);

    std::random_shuffle ( myvector.begin(), myvector.end() );
    // print out content:
    std::cout << "myvector contains:";
    for (std::vector<int>::iterator it=myvector.begin(); it!=myvector.end(); ++it)
        std::cout << ' ' << *it;
    std::cout << std::endl;


    memset(&key, 0, sizeof(key));
    memset(&data, 0, sizeof(data));
    HbmHash *fl = HbmHash::factory("HbmHashTable", P4TBL_ID_FLOW_HASH, P4TBL_ID_FLOW_HASH_OVERFLOW,
            262144, 16384, sizeof(key), sizeof(data));
            // 1048576, 16384, sizeof(key), sizeof(data));
            // 2097152, 16384, sizeof(key), sizeof(data));

    key.flow_lkp_metadata_lkp_src[0] = 0x01;
    key.flow_lkp_metadata_lkp_src[1] = 0x01;
    key.flow_lkp_metadata_lkp_src[2] = 0x00;
    key.flow_lkp_metadata_lkp_src[3] = 0x0a;
    key.flow_lkp_metadata_lkp_dst[0] = 0x01;
    key.flow_lkp_metadata_lkp_dst[1] = 0x00;
    key.flow_lkp_metadata_lkp_dst[2] = 0x00;
    key.flow_lkp_metadata_lkp_dst[3] = 0x0b;


    // Entry 1:
    key.flow_lkp_metadata_lkp_vrf = 3;
    key.flow_lkp_metadata_lkp_sport = 0xa277;
    key.flow_lkp_metadata_lkp_dport = 0x2;
    data.action_id = 0;
    rs = fl->insert((void *)&key, (void *)&data, &flow_idx[0]);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK || rs == sdk::SDK_RET_COLLISION);

    // Entry 2:
    key.flow_lkp_metadata_lkp_vrf = 3;
    key.flow_lkp_metadata_lkp_sport = 0x3b01;
    key.flow_lkp_metadata_lkp_dport = 0x3;
    data.action_id = 0;
    rs = fl->insert((void *)&key, (void *)&data, &flow_idx[1]);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK || rs == sdk::SDK_RET_COLLISION);

    // Entry 3:
    key.flow_lkp_metadata_lkp_vrf = 3;
    key.flow_lkp_metadata_lkp_sport = 0xd238;
    key.flow_lkp_metadata_lkp_dport = 0x4;
    data.action_id = 0;
    rs = fl->insert((void *)&key, (void *)&data, &flow_idx[2]);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK || rs == sdk::SDK_RET_COLLISION);

    // Entry 4:
    key.flow_lkp_metadata_lkp_vrf = 3;
    key.flow_lkp_metadata_lkp_sport = 0x4b4e;
    key.flow_lkp_metadata_lkp_dport = 0x5;
    data.action_id = 0;
    rs = fl->insert((void *)&key, (void *)&data, &flow_idx[3]);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK || rs == sdk::SDK_RET_COLLISION);

    for (std::vector<int>::iterator it=myvector.begin(); it!=myvector.end(); ++it) {
        HAL_TRACE_DEBUG("Removing {}", *it);
        rs = fl->remove(flow_idx[*it]);
        ASSERT_TRUE(rs == sdk::SDK_RET_OK);
    }
}

// ----------------------------------------------------------------------------
// Test 15:
//      - Create flow with collisions
//      - Insert 1
//      - Insert 2
//      - Insert 3
//      - Insert 4
//      - Insert 5
//      - Randomly delete
// ----------------------------------------------------------------------------
TEST_F(hbmhash_test, test15) {

    sdk_ret_t rs;
    flow_hash_swkey key = {0};
    flow_hash_actiondata_t data = {0};
    uint32_t flow_idx[9] = { 0 };
    std::srand ( unsigned ( std::time(0) ) );
    std::vector<int> myvector;

    for (int i=0; i<5; ++i) myvector.push_back(i);

    std::random_shuffle ( myvector.begin(), myvector.end() );
    // print out content:
    std::cout << "myvector contains:";
    for (std::vector<int>::iterator it=myvector.begin(); it!=myvector.end(); ++it)
        std::cout << ' ' << *it;
    std::cout << std::endl;


    memset(&key, 0, sizeof(key));
    memset(&data, 0, sizeof(data));
    HbmHash *fl = HbmHash::factory("HbmHashTable", P4TBL_ID_FLOW_HASH, P4TBL_ID_FLOW_HASH_OVERFLOW,
            262144, 16384, sizeof(key), sizeof(data));
            // 1048576, 16384, sizeof(key), sizeof(data));
            // 2097152, 16384, sizeof(key), sizeof(data));

    key.flow_lkp_metadata_lkp_src[0] = 0x01;
    key.flow_lkp_metadata_lkp_src[1] = 0x01;
    key.flow_lkp_metadata_lkp_src[2] = 0x00;
    key.flow_lkp_metadata_lkp_src[3] = 0x0a;
    key.flow_lkp_metadata_lkp_dst[0] = 0x01;
    key.flow_lkp_metadata_lkp_dst[1] = 0x00;
    key.flow_lkp_metadata_lkp_dst[2] = 0x00;
    key.flow_lkp_metadata_lkp_dst[3] = 0x0b;


    // Entry 1:
    key.flow_lkp_metadata_lkp_vrf = 3;
    key.flow_lkp_metadata_lkp_sport = 0xa277;
    key.flow_lkp_metadata_lkp_dport = 0x2;
    data.action_id = 0;
    rs = fl->insert((void *)&key, (void *)&data, &flow_idx[0]);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK || rs == sdk::SDK_RET_COLLISION);

    // Entry 2:
    key.flow_lkp_metadata_lkp_vrf = 3;
    key.flow_lkp_metadata_lkp_sport = 0x3b01;
    key.flow_lkp_metadata_lkp_dport = 0x3;
    data.action_id = 0;
    rs = fl->insert((void *)&key, (void *)&data, &flow_idx[1]);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK || rs == sdk::SDK_RET_COLLISION);

    // Entry 3:
    key.flow_lkp_metadata_lkp_vrf = 3;
    key.flow_lkp_metadata_lkp_sport = 0xd238;
    key.flow_lkp_metadata_lkp_dport = 0x4;
    data.action_id = 0;
    rs = fl->insert((void *)&key, (void *)&data, &flow_idx[2]);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK || rs == sdk::SDK_RET_COLLISION);

    // Entry 4:
    key.flow_lkp_metadata_lkp_vrf = 3;
    key.flow_lkp_metadata_lkp_sport = 0x4b4e;
    key.flow_lkp_metadata_lkp_dport = 0x5;
    data.action_id = 0;
    rs = fl->insert((void *)&key, (void *)&data, &flow_idx[3]);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK || rs == sdk::SDK_RET_COLLISION);

    // Entry 5:
    key.flow_lkp_metadata_lkp_vrf = 3;
    key.flow_lkp_metadata_lkp_sport = 0;
    key.flow_lkp_metadata_lkp_dport = 0x36ad;
    data.action_id = 0;
    rs = fl->insert((void *)&key, (void *)&data, &flow_idx[4]);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK || rs == sdk::SDK_RET_COLLISION);

    for (std::vector<int>::iterator it=myvector.begin(); it!=myvector.end(); ++it) {
        HAL_TRACE_DEBUG("Removing {}", *it);
        rs = fl->remove(flow_idx[*it]);
        ASSERT_TRUE(rs == sdk::SDK_RET_OK);
    }
}

// ----------------------------------------------------------------------------
// Test 16:
//      - Create flow with collisions
//      - Insert 1
//      - Insert 2
//      - Insert 3
//      - Insert 4
//      - Insert 5
//      - Insert 6
//      - Insert 7
//      - Randomly delete
// ----------------------------------------------------------------------------
TEST_F(hbmhash_test, test16) {

    sdk_ret_t rs;
    flow_hash_swkey key = {0};
    flow_hash_actiondata_t data = {0};
    uint32_t flow_idx[9] = { 0 };
    unsigned seed = std::time(0);
    // unsigned seed = 1504501660;
    std::srand (seed);
    // std::srand ( unsigned ( std::time(0) ) );
    std::vector<int> myvector;

    for (int i=0; i<8; ++i) myvector.push_back(i);

    std::random_shuffle ( myvector.begin(), myvector.end() );
    // print out content:
    HAL_TRACE_DEBUG("seed: {}", seed);
    std::cout << "myvector contains:";
    for (std::vector<int>::iterator it=myvector.begin(); it!=myvector.end(); ++it)
        std::cout << ' ' << *it;
    std::cout << std::endl;


    memset(&key, 0, sizeof(key));
    memset(&data, 0, sizeof(data));
    HbmHash *fl = HbmHash::factory("HbmHashTable", P4TBL_ID_FLOW_HASH, P4TBL_ID_FLOW_HASH_OVERFLOW,
            262144, 16384, sizeof(key), sizeof(data));
            // 1048576, 16384, sizeof(key), sizeof(data));
            // 2097152, 16384, sizeof(key), sizeof(data));

    key.flow_lkp_metadata_lkp_src[0] = 0x01;
    key.flow_lkp_metadata_lkp_src[1] = 0x01;
    key.flow_lkp_metadata_lkp_src[2] = 0x00;
    key.flow_lkp_metadata_lkp_src[3] = 0x0a;
    key.flow_lkp_metadata_lkp_dst[0] = 0x01;
    key.flow_lkp_metadata_lkp_dst[1] = 0x00;
    key.flow_lkp_metadata_lkp_dst[2] = 0x00;
    key.flow_lkp_metadata_lkp_dst[3] = 0x0b;


    // Entry 1:
    key.flow_lkp_metadata_lkp_vrf = 3;
    key.flow_lkp_metadata_lkp_sport = 0xa277;
    key.flow_lkp_metadata_lkp_dport = 0x2;
    data.action_id = 0;
    rs = fl->insert((void *)&key, (void *)&data, &flow_idx[0]);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK || rs == sdk::SDK_RET_COLLISION);

    // Entry 2:
    key.flow_lkp_metadata_lkp_vrf = 3;
    key.flow_lkp_metadata_lkp_sport = 0x3b01;
    key.flow_lkp_metadata_lkp_dport = 0x3;
    data.action_id = 0;
    rs = fl->insert((void *)&key, (void *)&data, &flow_idx[1]);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK || rs == sdk::SDK_RET_COLLISION);

    // Entry 3:
    key.flow_lkp_metadata_lkp_vrf = 3;
    key.flow_lkp_metadata_lkp_sport = 0xd238;
    key.flow_lkp_metadata_lkp_dport = 0x4;
    data.action_id = 0;
    rs = fl->insert((void *)&key, (void *)&data, &flow_idx[2]);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK || rs == sdk::SDK_RET_COLLISION);

    // Entry 4:
    key.flow_lkp_metadata_lkp_vrf = 3;
    key.flow_lkp_metadata_lkp_sport = 0x4b4e;
    key.flow_lkp_metadata_lkp_dport = 0x5;
    data.action_id = 0;
    rs = fl->insert((void *)&key, (void *)&data, &flow_idx[3]);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK || rs == sdk::SDK_RET_COLLISION);

    // Entry 5:
    key.flow_lkp_metadata_lkp_vrf = 3;
    key.flow_lkp_metadata_lkp_sport = 0;
    key.flow_lkp_metadata_lkp_dport = 0x36ad;
    data.action_id = 0;
    rs = fl->insert((void *)&key, (void *)&data, &flow_idx[4]);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK || rs == sdk::SDK_RET_COLLISION);

    // Entry 6:
    key.flow_lkp_metadata_lkp_vrf = 3;
    key.flow_lkp_metadata_lkp_sport = 3;
    key.flow_lkp_metadata_lkp_dport = 0xd125;
    data.action_id = 0;
    rs = fl->insert((void *)&key, (void *)&data, &flow_idx[5]);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK || rs == sdk::SDK_RET_COLLISION);

    // Entry 7:
    key.flow_lkp_metadata_lkp_vrf = 0;
    key.flow_lkp_metadata_lkp_sport = 0;
    key.flow_lkp_metadata_lkp_dport = 0xc430;
    data.action_id = 0;
    rs = fl->insert((void *)&key, (void *)&data, &flow_idx[6]);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK || rs == sdk::SDK_RET_COLLISION);

    // Entry 8:
    key.flow_lkp_metadata_lkp_vrf = 1;
    key.flow_lkp_metadata_lkp_sport = 1;
    key.flow_lkp_metadata_lkp_dport = 0xc8c3;
    data.action_id = 0;
    rs = fl->insert((void *)&key, (void *)&data, &flow_idx[7]);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK || rs == sdk::SDK_RET_COLLISION);

    for (std::vector<int>::iterator it=myvector.begin(); it!=myvector.end(); ++it) {
        HAL_TRACE_DEBUG("Removing {}", *it);
        rs = fl->remove(flow_idx[*it]);
        ASSERT_TRUE(rs == sdk::SDK_RET_OK);
    }
}
// ----------------------------------------------------------------------------
// Test 17:
//      - Create flow with collisions
//      - Insert 1 - 9
//      - Randomly delete
// ----------------------------------------------------------------------------
TEST_F(hbmhash_test, test17) {

    sdk_ret_t rs;
    flow_hash_swkey key = {0};
    flow_hash_actiondata_t data = {0};
    uint32_t max = 11;
    uint32_t flow_idx[max] = { 0 };
    unsigned seed = std::time(0);
    // unsigned seed = 1504501660;
    std::srand (seed);
    // std::srand ( unsigned ( std::time(0) ) );
    std::vector<int> myvector;

    for (uint i=0; i < max; ++i) myvector.push_back(i);

    std::random_shuffle ( myvector.begin(), myvector.end() );
    // print out content:
    HAL_TRACE_DEBUG("seed: {}", seed);
    std::cout << "myvector contains:";
    for (std::vector<int>::iterator it=myvector.begin(); it!=myvector.end(); ++it)
        std::cout << ' ' << *it;
    std::cout << std::endl;


    memset(&key, 0, sizeof(key));
    memset(&data, 0, sizeof(data));
    HbmHash *fl = HbmHash::factory("HbmHashTable", P4TBL_ID_FLOW_HASH, P4TBL_ID_FLOW_HASH_OVERFLOW,
            262144, 16384, sizeof(key), sizeof(data));
            // 1048576, 16384, sizeof(key), sizeof(data));
            // 2097152, 16384, sizeof(key), sizeof(data));

    key.flow_lkp_metadata_lkp_src[0] = 0x01;
    key.flow_lkp_metadata_lkp_src[1] = 0x01;
    key.flow_lkp_metadata_lkp_src[2] = 0x00;
    key.flow_lkp_metadata_lkp_src[3] = 0x0a;
    key.flow_lkp_metadata_lkp_dst[0] = 0x01;
    key.flow_lkp_metadata_lkp_dst[1] = 0x00;
    key.flow_lkp_metadata_lkp_dst[2] = 0x00;
    key.flow_lkp_metadata_lkp_dst[3] = 0x0b;


    // Entry 1:
    key.flow_lkp_metadata_lkp_vrf = 3;
    key.flow_lkp_metadata_lkp_sport = 0xa277;
    key.flow_lkp_metadata_lkp_dport = 0x2;
    data.action_id = 0;
    rs = fl->insert((void *)&key, (void *)&data, &flow_idx[0]);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK || rs == sdk::SDK_RET_COLLISION);

    // Entry 2:
    key.flow_lkp_metadata_lkp_vrf = 3;
    key.flow_lkp_metadata_lkp_sport = 0x3b01;
    key.flow_lkp_metadata_lkp_dport = 0x3;
    data.action_id = 0;
    rs = fl->insert((void *)&key, (void *)&data, &flow_idx[1]);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK || rs == sdk::SDK_RET_COLLISION);

    // Entry 3:
    key.flow_lkp_metadata_lkp_vrf = 3;
    key.flow_lkp_metadata_lkp_sport = 0xd238;
    key.flow_lkp_metadata_lkp_dport = 0x4;
    data.action_id = 0;
    rs = fl->insert((void *)&key, (void *)&data, &flow_idx[2]);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK || rs == sdk::SDK_RET_COLLISION);

    // Entry 4:
    key.flow_lkp_metadata_lkp_vrf = 3;
    key.flow_lkp_metadata_lkp_sport = 0x4b4e;
    key.flow_lkp_metadata_lkp_dport = 0x5;
    data.action_id = 0;
    rs = fl->insert((void *)&key, (void *)&data, &flow_idx[3]);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK || rs == sdk::SDK_RET_COLLISION);

    // Entry 5:
    key.flow_lkp_metadata_lkp_vrf = 3;
    key.flow_lkp_metadata_lkp_sport = 0;
    key.flow_lkp_metadata_lkp_dport = 0x36ad;
    data.action_id = 0;
    rs = fl->insert((void *)&key, (void *)&data, &flow_idx[4]);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK || rs == sdk::SDK_RET_COLLISION);

    // Entry 6:
    key.flow_lkp_metadata_lkp_vrf = 3;
    key.flow_lkp_metadata_lkp_sport = 3;
    key.flow_lkp_metadata_lkp_dport = 0xd125;
    data.action_id = 0;
    rs = fl->insert((void *)&key, (void *)&data, &flow_idx[5]);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK || rs == sdk::SDK_RET_COLLISION);

    // Entry 7:
    key.flow_lkp_metadata_lkp_vrf = 0;
    key.flow_lkp_metadata_lkp_sport = 0;
    key.flow_lkp_metadata_lkp_dport = 0xc430;
    data.action_id = 0;
    rs = fl->insert((void *)&key, (void *)&data, &flow_idx[6]);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK || rs == sdk::SDK_RET_COLLISION);

    // Entry 8:
    key.flow_lkp_metadata_lkp_vrf = 1;
    key.flow_lkp_metadata_lkp_sport = 1;
    key.flow_lkp_metadata_lkp_dport = 0xc8c3;
    data.action_id = 0;
    rs = fl->insert((void *)&key, (void *)&data, &flow_idx[7]);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK || rs == sdk::SDK_RET_COLLISION);

    // Entry 9:
    key.flow_lkp_metadata_lkp_vrf = 1;
    key.flow_lkp_metadata_lkp_sport = 2;
    key.flow_lkp_metadata_lkp_dport = 0x2f4b;
    data.action_id = 0;
    rs = fl->insert((void *)&key, (void *)&data, &flow_idx[8]);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK || rs == sdk::SDK_RET_COLLISION);


    // Entry 10:
    key.flow_lkp_metadata_lkp_vrf = 0x244f;
    key.flow_lkp_metadata_lkp_sport = 0xd17e;
    key.flow_lkp_metadata_lkp_dport = 0x804e;
    key.flow_lkp_metadata_lkp_src[0] = 0xca;
    key.flow_lkp_metadata_lkp_src[1] = 0xd8;
    key.flow_lkp_metadata_lkp_src[2] = 0xc7;
    key.flow_lkp_metadata_lkp_src[3] = 0xf3;
    key.flow_lkp_metadata_lkp_dst[0] = 0x8f;
    key.flow_lkp_metadata_lkp_dst[1] = 0x3f;
    key.flow_lkp_metadata_lkp_dst[2] = 0x7e;
    key.flow_lkp_metadata_lkp_dst[3] = 0x3b;
    key.flow_lkp_metadata_lkp_proto = 0xbe;
    rs = fl->insert((void *)&key, (void *)&data, &flow_idx[9]);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK || rs == sdk::SDK_RET_COLLISION);

    // Entry 11:
    key.flow_lkp_metadata_lkp_vrf = 0xd20d;
    key.flow_lkp_metadata_lkp_sport = 0xe909;
    key.flow_lkp_metadata_lkp_dport = 0x4d6a;
    key.flow_lkp_metadata_lkp_proto = 0x38;
    key.flow_lkp_metadata_lkp_src[0] = 0x94;
    key.flow_lkp_metadata_lkp_src[1] = 0x4;
    key.flow_lkp_metadata_lkp_src[2] = 0x26;
    key.flow_lkp_metadata_lkp_src[3] = 0xa8;
    key.flow_lkp_metadata_lkp_dst[0] = 0x52;
    key.flow_lkp_metadata_lkp_dst[1] = 0x85;
    key.flow_lkp_metadata_lkp_dst[2] = 0x13;
    key.flow_lkp_metadata_lkp_dst[3] = 0x11;
    rs = fl->insert((void *)&key, (void *)&data, &flow_idx[10]);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK || rs == sdk::SDK_RET_COLLISION);

    for (std::vector<int>::iterator it=myvector.begin(); it!=myvector.end(); ++it) {
        HAL_TRACE_DEBUG("Removing {}", *it);
        rs = fl->remove(flow_idx[*it]);
        ASSERT_TRUE(rs == sdk::SDK_RET_OK);
    }

    HAL_TRACE_DEBUG("tableid:{}, table_name:{}, capacity:{}, oflow_capac:{}, "
                    "num_in_use:{}, oflow_num_in_use:{}"
                    "num_inserts:{}, num_insert_errors:{}, "
                    "num_deletes:{}, num_delete_errors:{}",
                    fl->table_id(), fl->table_name(),
                    fl->table_capacity(), fl->coll_table_capacity(),
                    fl->table_num_entries_in_use(), fl->coll_table_num_entries_in_use(),
                    fl->table_num_inserts(), fl->table_num_insert_errors(),
                    fl->table_num_deletes(), fl->table_num_delete_errors());
}

// ----------------------------------------------------------------------------
// Test 18:
//      - Create flow table
// ----------------------------------------------------------------------------
TEST_F(hbmhash_test, DISABLED_test18) {

    sdk_ret_t rs;
    flow_hash_swkey key = {0};
    flow_hash_actiondata_t data = {0};
    uint32_t max = 10, count = 0;
    uint32_t flow_idx[max] = { 0 };
    unsigned seed = std::time(0);
    std::srand (seed);
    // uint32_t flow_idx[1000000] = { 0 };
    uint32_t ft_bits = 0;
    std::vector<int> myvector;

    for (uint i=0; i < max; ++i) myvector.push_back(i);

    std::random_shuffle ( myvector.begin(), myvector.end() );
    // print out content:
    HAL_TRACE_DEBUG("seed: {}", seed);
    std::cout << "myvector contains:";
    for (std::vector<int>::iterator it=myvector.begin(); it!=myvector.end(); ++it)
        std::cout << ' ' << *it;
    std::cout << std::endl;

    memset(&key, 0, sizeof(key));
    memset(&data, 0, sizeof(data));
    HbmHash *fl = HbmHash::factory("HbmHashTable", P4TBL_ID_FLOW_HASH, P4TBL_ID_FLOW_HASH_OVERFLOW,
            262144, 16384, sizeof(key), sizeof(data));
            // 1048576, 16384, sizeof(key), sizeof(data));
            // 2097152, 16384, sizeof(key), sizeof(data));

    while (1) {
        for (int i = 0; i < 4; i++) {
            key.flow_lkp_metadata_lkp_src[i] = rand() % 256;
            key.flow_lkp_metadata_lkp_dst[i] = rand() % 256;
        }
        key.flow_lkp_metadata_lkp_vrf = rand() % 65536;
        key.flow_lkp_metadata_lkp_sport = rand() % 65536;
        key.flow_lkp_metadata_lkp_dport = rand() % 65536;
        key.flow_lkp_metadata_lkp_proto = rand() % 256;

        ft_bits = fl->calc_hash_(&key, &data);
        HAL_TRACE_DEBUG("Checking:{:#x}", ft_bits);

        if (ft_bits == 0x10001) {
            HAL_TRACE_DEBUG("MATCH::");
            for (int i = 0; i < 4; i++) {
                HAL_TRACE_DEBUG("Src[{}]: {:#x}", i, key.flow_lkp_metadata_lkp_src[i]);
                HAL_TRACE_DEBUG("Dst[{}]: {:#x}", i, key.flow_lkp_metadata_lkp_dst[i]);
            }
            HAL_TRACE_DEBUG("Vrf: {:#x}, sport: {:#x}, dport: {:#x}, proto: {:#x}",
                    key.flow_lkp_metadata_lkp_vrf,
                    key.flow_lkp_metadata_lkp_sport,
                    key.flow_lkp_metadata_lkp_dport,
                    key.flow_lkp_metadata_lkp_proto);
            rs = fl->insert((void *)&key, (void *)&data, &flow_idx[count]);
            ASSERT_TRUE(rs == sdk::SDK_RET_OK || rs == sdk::SDK_RET_COLLISION);
            count++;
            if (count == max) {
                break;
            }
        }
    }
    // Remvoe the entries
    for (std::vector<int>::iterator it=myvector.begin(); it!=myvector.end(); ++it) {
        HAL_TRACE_DEBUG("Removing {}", *it);
        rs = fl->remove(flow_idx[*it]);
        ASSERT_TRUE(rs == sdk::SDK_RET_OK);
    }

    HAL_TRACE_DEBUG("tableid:{}, table_name:{}, capacity:{}, oflow_capac:{}, "
                    "num_in_use:{}, oflow_num_in_use:{}"
                    "num_inserts:{}, num_insert_errors:{}, "
                    "num_deletes:{}, num_delete_errors:{}",
                    fl->table_id(), fl->table_name(),
                    fl->table_capacity(), fl->coll_table_capacity(),
                    fl->table_num_entries_in_use(), fl->coll_table_num_entries_in_use(),
                    fl->table_num_inserts(), fl->table_num_insert_errors(),
                    fl->table_num_deletes(), fl->table_num_delete_errors());
}

// ----------------------------------------------------------------------------
// Test 19:
//      - Tests the scale
// ----------------------------------------------------------------------------
TEST_F(hbmhash_test, test19) {

    sdk_ret_t rs;
    flow_hash_swkey key = {0};
    flow_hash_actiondata_t data = {0};
    uint32_t max = 64000, count = 0;
    uint32_t flow_idx[max] = { 0 };
    uint32_t total_time = 0;
    // uint32_t flow_idx[1000000] = { 0 };
    // uint32_t ft_bits = 0;

    // unsigned seed = std::time(0);
    unsigned seed = 1529534551;
    std::srand (seed);
    HAL_TRACE_DEBUG("seed: {}", seed);

#if 0
    std::vector<int> myvector;
    for (uint i=0; i < max; ++i) myvector.push_back(i);
    // For random deletes
    std::random_shuffle ( myvector.begin(), myvector.end() );
    // print out content:
    std::cout << "myvector contains:";
    for (std::vector<int>::iterator it=myvector.begin(); it!=myvector.end(); ++it)
        std::cout << ' ' << *it;
    std::cout << std::endl;
#endif



    memset(&key, 0, sizeof(key));
    memset(&data, 0, sizeof(data));
    HbmHash *fl = HbmHash::factory("HbmHashTable", P4TBL_ID_FLOW_HASH, P4TBL_ID_FLOW_HASH_OVERFLOW,
            262144, 16384, sizeof(key), sizeof(data));
            // 1048576, 16384, sizeof(key), sizeof(data));
            // 2097152, 16384, sizeof(key), sizeof(data));

    while (count < max) {
        for (int i = 0; i < 4; i++) {
            key.flow_lkp_metadata_lkp_src[i] = rand() % 256;
            key.flow_lkp_metadata_lkp_dst[i] = rand() % 256;
        }
        key.flow_lkp_metadata_lkp_vrf = rand() % 65536;
        key.flow_lkp_metadata_lkp_sport = rand() % 65536;
        key.flow_lkp_metadata_lkp_dport = rand() % 65536;
        key.flow_lkp_metadata_lkp_proto = rand() % 256;

        // ft_bits = fl->calc_hash_(&key, &data);
        // HAL_TRACE_DEBUG("Checking:{:#x}", ft_bits);

        for (int i = 0; i < 4; i++) {
            HAL_TRACE_DEBUG("Src[{}]: {:#x}", i, key.flow_lkp_metadata_lkp_src[i]);
            HAL_TRACE_DEBUG("Dst[{}]: {:#x}", i, key.flow_lkp_metadata_lkp_dst[i]);
        }
        HAL_TRACE_DEBUG("Vrf: {:#x}, sport: {:#x}, dport: {:#x}, proto: {:#x}",
                        key.flow_lkp_metadata_lkp_vrf,
                        key.flow_lkp_metadata_lkp_sport,
                        key.flow_lkp_metadata_lkp_dport,
                        key.flow_lkp_metadata_lkp_proto);
        high_resolution_clock::time_point t1 = high_resolution_clock::now();
        rs = fl->insert((void *)&key, (void *)&data, &flow_idx[count]);
        high_resolution_clock::time_point t2 = high_resolution_clock::now();
        auto duration = duration_cast<microseconds>( t2 - t1 ).count();
        total_time += duration;
        ASSERT_TRUE(rs == sdk::SDK_RET_OK || rs == sdk::SDK_RET_COLLISION);
        count++;
    }



#if 0
    // Remvoe the entries
    for (std::vector<int>::iterator it=myvector.begin(); it!=myvector.end(); ++it) {
        HAL_TRACE_DEBUG("Removing {}", *it);
        rs = fl->remove(flow_idx[*it]);
        ASSERT_TRUE(rs == sdk::SDK_RET_OK);
    }
#endif

    printf("Total Time: %u, num_in_use: %u, oflow_num_in_use:%u, num_inserts:%u, "
           "num_insert_errors:%u\n", total_time, fl->table_num_entries_in_use(),
           fl->coll_table_num_entries_in_use(), fl->table_num_inserts(), fl->table_num_insert_errors());

    HAL_TRACE_DEBUG("Took : {} tableid:{}, table_name:{}, capacity:{}, oflow_capac:{}, "
                    "num_in_use:{}, oflow_num_in_use:{}"
                    "num_inserts:{}, num_insert_errors:{}, "
                    "num_deletes:{}, num_delete_errors:{}", total_time,
                    fl->table_id(), fl->table_name(),
                    fl->table_capacity(), fl->coll_table_capacity(),
                    fl->table_num_entries_in_use(), fl->coll_table_num_entries_in_use(),
                    fl->table_num_inserts(), fl->table_num_insert_errors(),
                    fl->table_num_deletes(), fl->table_num_delete_errors());
}

// ----------------------------------------------------------------------------
// Test 20:
//      - Measuing Hash performance
// ----------------------------------------------------------------------------
TEST_F(hbmhash_test, test20) {

    flow_hash_swkey key = {0};
    flow_hash_actiondata_t data = {0};
    unsigned seed;
    // unsigned seed = std::time(0);
    // unsigned seed = 1531851844;
    // std::srand (seed);
    // uint32_t flow_idx[1000000] = { 0 };
	std::map<uint32_t, uint32_t> crc32_colls_map;
	std::map<uint32_t, uint32_t> jenkins_colls_map;
	std::map<uint32_t, uint32_t> jenkins_spooky_colls_map;
    uint32_t crc32_colls_count = 0, jenkins_colls_count = 0, jenkins_spooky_colls_count = 0;
    uint32_t crc32_hash_val, jenkins_hash_val, jenkins_spooky_hash_val;

	std::map<uint32_t, uint32_t> crc32_colls_map_lsb;
	std::map<uint32_t, uint32_t> jenkins_colls_map_lsb;
	std::map<uint32_t, uint32_t> jenkins_spooky_colls_map_lsb;
    uint32_t crc32_colls_count_lsb = 0, jenkins_colls_count_lsb = 0, jenkins_spooky_colls_count_lsb = 0;
    uint32_t crc32_hash_val_lsb, jenkins_hash_val_lsb, jenkins_spooky_hash_val_lsb;
    uint32_t flow_num, num_flows = 1024 * 1024;
    uint32_t num_runs = 10;


    memset(&key, 0, sizeof(key));
    memset(&data, 0, sizeof(data));
    HbmHash *fl = HbmHash::factory("HbmHashTable", P4TBL_ID_FLOW_HASH, P4TBL_ID_FLOW_HASH_OVERFLOW,
            262144, 16384, sizeof(key), sizeof(data));
            // 1048576, 16384, sizeof(key), sizeof(data));
            // 2097152, 16384, sizeof(key), sizeof(data));

    ofstream hash_coll_f;
    hash_coll_f.open("hash_coll.csv");
    hash_coll_f << "Seed, CRC32,, Jenkins,, Jenkins Spooky ,,\n";
    hash_coll_f << ", 21 Bit , 32 Bit , 21 Bit, 32 Bit, 21 Bit, 32 Bit,\n";
    for (uint32_t i = 0; i < num_runs; i++) {
        crc32_colls_count = 0;
        jenkins_colls_count = 0;
        jenkins_spooky_colls_count = 0;
        crc32_colls_count_lsb = 0;
        jenkins_colls_count_lsb = 0;
        jenkins_spooky_colls_count_lsb = 0;
        crc32_colls_map.clear();
        jenkins_colls_map.clear();
        jenkins_spooky_colls_map.clear();
        crc32_colls_map_lsb.clear();
        jenkins_colls_map_lsb.clear();
        jenkins_spooky_colls_map_lsb.clear();
        flow_num = num_flows;
        seed = std::time(0);
        std::srand (seed);



        while (flow_num--) {
            for (uint32_t i = 0; i < 4; i++) {
                key.flow_lkp_metadata_lkp_src[i] = rand() % 256;
                key.flow_lkp_metadata_lkp_dst[i] = rand() % 256;
            }
            key.flow_lkp_metadata_lkp_vrf = rand() % 65536;
            key.flow_lkp_metadata_lkp_sport = rand() % 65536;
            key.flow_lkp_metadata_lkp_dport = rand() % 65536;
            key.flow_lkp_metadata_lkp_proto = rand() % 256;

            crc32_hash_val = fl->calc_hash_(&key, &data);
            if (crc32_colls_map.count(crc32_hash_val)) {
                crc32_colls_map[crc32_hash_val]++;
                crc32_colls_count++;
            } else {
                crc32_colls_map[crc32_hash_val] = 0;
            }

            jenkins_hash_val = calc_jenkins_hash_(&key, &data, fl, false);
            if (jenkins_colls_map.count(jenkins_hash_val)) {
                jenkins_colls_map[jenkins_hash_val]++;
                jenkins_colls_count++;
            } else {
                jenkins_colls_map[jenkins_hash_val] = 0;
            }

            jenkins_spooky_hash_val = calc_jenkins_hash_(&key, &data, fl, true);
            if (jenkins_spooky_colls_map.count(jenkins_spooky_hash_val)) {
                jenkins_spooky_colls_map[jenkins_spooky_hash_val]++;
                jenkins_spooky_colls_count++;
            } else {
                jenkins_spooky_colls_map[jenkins_spooky_hash_val] = 0;
            }

            crc32_hash_val_lsb = hash_lsb_select(crc32_hash_val, 21);
            if (crc32_colls_map_lsb.count(crc32_hash_val_lsb)) {
                crc32_colls_map_lsb[crc32_hash_val_lsb]++;
                crc32_colls_count_lsb++;
            } else {
                crc32_colls_map_lsb[crc32_hash_val_lsb] = 0;
            }

            jenkins_hash_val_lsb = hash_lsb_select(jenkins_hash_val, 21);
            if (jenkins_colls_map_lsb.count(jenkins_hash_val_lsb)) {
                jenkins_colls_map_lsb[jenkins_hash_val_lsb]++;
                jenkins_colls_count_lsb++;
            } else {
                jenkins_colls_map_lsb[jenkins_hash_val_lsb] = 0;
            }

            jenkins_spooky_hash_val_lsb = hash_lsb_select(jenkins_spooky_hash_val, 21);
            if (jenkins_spooky_colls_map_lsb.count(jenkins_spooky_hash_val_lsb)) {
                jenkins_spooky_colls_map_lsb[jenkins_spooky_hash_val_lsb]++;
                jenkins_spooky_colls_count_lsb++;
            } else {
                jenkins_spooky_colls_map_lsb[jenkins_spooky_hash_val_lsb] = 0;
            }
        }

        HAL_TRACE_DEBUG("Seed: {}, Collisions: CRC32: {}, Jenkins: {}, Jenkins_spooky: {} LSB: CRC32: {}, Jenkins: {}, Jenkins_spooky: {}",
                        seed, crc32_colls_count, jenkins_colls_count, jenkins_spooky_colls_count,
                        crc32_colls_count_lsb, jenkins_colls_count_lsb, jenkins_spooky_colls_count_lsb);

        hash_coll_f << seed << "," << crc32_colls_count_lsb << "," << crc32_colls_count << ","
            << jenkins_colls_count_lsb << "," << jenkins_colls_count << ","
            << jenkins_spooky_colls_count_lsb << "," << jenkins_spooky_colls_count << "\n";
    }

#if 0
        HAL_TRACE_DEBUG("Checking:{:#x}", ft_bits);

        if (ft_bits == 0x10001) {
            HAL_TRACE_DEBUG("MATCH::");
            for (int i = 0; i < 4; i++) {
                HAL_TRACE_DEBUG("Src[{}]: {:#x}", i, key.flow_lkp_metadata_lkp_src[i]);
                HAL_TRACE_DEBUG("Dst[{}]: {:#x}", i, key.flow_lkp_metadata_lkp_dst[i]);
            }
            HAL_TRACE_DEBUG("Vrf: {:#x}, sport: {:#x}, dport: {:#x}, proto: {:#x}",
                    key.flow_lkp_metadata_lkp_vrf,
                    key.flow_lkp_metadata_lkp_sport,
                    key.flow_lkp_metadata_lkp_dport,
                    key.flow_lkp_metadata_lkp_proto);
        }
#endif
}

// ----------------------------------------------------------------------------
// Test 21:
//      - Create flow table
// ----------------------------------------------------------------------------
TEST_F(hbmhash_test, DISABLED_test21) {

    sdk_ret_t rs;
    flow_hash_swkey key = {0};
    flow_hash_actiondata_t data = {0};
    uint32_t max = 2 * 1024 * 1024, count = 0;
    // uint32_t flow_idx[max] = { 0 };
    unsigned seed = std::time(0);
    std::srand (seed);
    // uint32_t flow_idx[1000000] = { 0 };
    uint32_t flow_idx = 0;
    uint32_t ft_bits = 0;
    // std::vector<int> myvector;
    int ip_sa = 10, ip_da = 20;
    uint32_t sport = 1000, dport = 1000;

#if 0
    for (uint i=0; i < max; ++i) myvector.push_back(i);

    std::random_shuffle ( myvector.begin(), myvector.end() );
    // print out content:
    HAL_TRACE_DEBUG("seed: {}", seed);
    std::cout << "myvector contains:";
    for (std::vector<int>::iterator it=myvector.begin(); it!=myvector.end(); ++it)
        std::cout << ' ' << *it;
    std::cout << std::endl;
#endif

    memset(&key, 0, sizeof(key));
    memset(&data, 0, sizeof(data));
    HbmHash *fl = HbmHash::factory("HbmHashTable", P4TBL_ID_FLOW_HASH, P4TBL_ID_FLOW_HASH_OVERFLOW,
            2097152, 262144, sizeof(key), sizeof(data));
            // 1048576, 16384, sizeof(key), sizeof(data));
            // 2097152, 16384, sizeof(key), sizeof(data));

    while (1) {
        ip_sa = 10;
        ip_da = 20;

        for (int i = 0; i < 4; i++) {
            key.flow_lkp_metadata_lkp_src[i] = ip_sa % 256;
            key.flow_lkp_metadata_lkp_dst[i] = ip_da % 256;
            ip_sa++;
            ip_da++;
        }
        key.flow_lkp_metadata_lkp_vrf = 4;
        key.flow_lkp_metadata_lkp_sport = sport;
        key.flow_lkp_metadata_lkp_dport = dport;
        key.flow_lkp_metadata_lkp_proto = 17;

        ft_bits = fl->calc_hash_(&key, &data);
        HAL_TRACE_DEBUG("Checking:{:#x}", ft_bits);

        if (true || ft_bits == 0x10001) {
            // HAL_TRACE_DEBUG("MATCH::");
            for (int i = 0; i < 4; i++) {
                HAL_TRACE_DEBUG("Src[{}]: {:#x}", i, key.flow_lkp_metadata_lkp_src[i]);
                HAL_TRACE_DEBUG("Dst[{}]: {:#x}", i, key.flow_lkp_metadata_lkp_dst[i]);
            }
            HAL_TRACE_DEBUG("Vrf: {:#x}, sport: {:#x}, dport: {:#x}, proto: {:#x}",
                    key.flow_lkp_metadata_lkp_vrf,
                    key.flow_lkp_metadata_lkp_sport,
                    key.flow_lkp_metadata_lkp_dport,
                    key.flow_lkp_metadata_lkp_proto);
            rs = fl->insert((void *)&key, (void *)&data, &flow_idx);
            // rs = fl->insert((void *)&key, (void *)&data, &flow_idx[count]);
            ASSERT_TRUE(rs == sdk::SDK_RET_OK || rs == sdk::SDK_RET_COLLISION || rs == sdk::SDK_RET_ENTRY_EXISTS);
            if (rs != sdk::SDK_RET_ENTRY_EXISTS) {
                count++;
            }
            if (count == max) {
                break;
            }
        }

        dport++;
        if (dport == 3200) {
            sport++;
            dport = 1000;
        }
    }

#if 0
    // Remvoe the entries
    for (std::vector<int>::iterator it=myvector.begin(); it!=myvector.end(); ++it) {
        HAL_TRACE_DEBUG("Removing {}", *it);
        rs = fl->remove(flow_idx[*it]);
        ASSERT_TRUE(rs == sdk::SDK_RET_OK);
    }
#endif

    HAL_TRACE_DEBUG("tableid:{}, table_name:{}, capacity:{}, oflow_capac:{}, "
                    "num_in_use:{}, oflow_num_in_use:{}"
                    "num_inserts:{}, num_insert_errors:{}, "
                    "num_deletes:{}, num_delete_errors:{}",
                    fl->table_id(), fl->table_name(),
                    fl->table_capacity(), fl->coll_table_capacity(),
                    fl->table_num_entries_in_use(), fl->coll_table_num_entries_in_use(),
                    fl->table_num_inserts(), fl->table_num_insert_errors(),
                    fl->table_num_deletes(), fl->table_num_delete_errors());
}

static int
hal_sdk_logger (uint32_t mod_id, sdk_trace_level_e trace_level,
                const char *format, ...)
{
    char       logbuf[1024];
    va_list    args;

    va_start(args, format);
    vsnprintf(logbuf, sizeof(logbuf), format, args);
    HAL_TRACE_ERR_NO_META("{}", logbuf);
    va_end(args);

    return 0;
}

 //-----------------------------------------------------------------------------
 // SDK initiaization
 //-----------------------------------------------------------------------------
 hal_ret_t
 hal_sdk_init (void)
 {
     sdk::lib::logger::init(hal_sdk_logger);
     return HAL_RET_OK;
 }

int main(int argc, char **argv) {


    ::testing::InitGoogleTest(&argc, argv);

    std::string logfile;
    logfile = std::string("./hal.log");
    hal::utils::trace_init("hal", 0x3, true,
                           logfile.c_str(),
                           NULL,
                           TRACE_FILE_SIZE_DEFAULT,
                           TRACE_NUM_FILES_DEFAULT,
                           ::utils::trace_debug,
                           ::utils::trace_none);

    hal_sdk_init();

    HAL_TRACE_DEBUG("Starting Main ... ");

    p4pd_cfg_t                 p4pd_cfg = {
        .table_map_cfg_file  = "iris/capri_p4_table_map.json",
        .p4pd_pgm_name       = "iris",
        .p4pd_rxdma_pgm_name = "p4plus",
        .p4pd_txdma_pgm_name = "p4plus",
        .cfg_path            = std::getenv("HAL_CONFIG_PATH"),
    };
    p4pd_init(&p4pd_cfg);

    // testing::GTEST_FLAG(filter) = "-*test18*";
    int res = RUN_ALL_TESTS();
    p4pd_cleanup();
    return res;
}
