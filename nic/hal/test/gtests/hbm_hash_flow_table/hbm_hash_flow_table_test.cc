//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#include <gtest/gtest.h>
#include <stdio.h>
#include "nic/sdk/include/sdk/base.hpp"
#include "nic/sdk/lib/pal/pal.hpp"
#include "nic/sdk/include/sdk/types.hpp"
#include "lib/table/hbmhash/hbm_hash.hpp"
#include "platform/capri/capri_tbl_rw.hpp"
#include "nic/sdk/lib/p4/p4_api.hpp"
#include "gen/p4gen/p4/include/p4pd.h"
#include <boost/multiprecision/cpp_int.hpp>
#include <chrono>

using sdk::table::HbmHash;
using boost::multiprecision::uint512_t;
using boost::multiprecision::uint128_t;
using namespace std::chrono;
using namespace sdk;

#define P4TBL_ID_FLOW_HASH              1
#define P4TBL_ID_FLOW_HASH_OVERFLOW     2

class hbm_hash_test : public ::testing::Test {
protected:
  hbm_hash_test() {
  }

  virtual ~hbm_hash_test() {
  }

  // will be called immediately after the constructor before each test
  virtual void SetUp() {
  }

  // will be called immediately after each test before the destructor
  virtual void TearDown() {
  }

};

typedef struct hbm_hash_key_s {
    int a;
    int b;
} hbm_hash_key_t;

typedef struct hbm_hash_data_s {
    uint32_t p;
} hbm_hash_data_t;

// ----------------------------------------------------------------------------
// Test 1:
//      - Create hbm_hash table
//      - Insert a hbm_hash entry.
//      - Insert a hbm_hash entry.
//      - Insert a hbm_hash entry.
// ----------------------------------------------------------------------------
TEST_F(hbm_hash_test, test1) {

    sdk_ret_t rs;
    uint32_t hbm_hash_idx = 0;

    flow_hash_swkey key;
    flow_hash_actiondata_t data;

    memset(&key, 0, sizeof(key));
    memset(&data, 0, sizeof(data));

    HbmHash *fl = HbmHash::factory("FlowTable", P4TBL_ID_FLOW_HASH,
                                   P4TBL_ID_FLOW_HASH_OVERFLOW,
                                   2097152, 16384,
                                   sizeof(key), sizeof(data));

    key.flow_lkp_metadata_lkp_sport = 10;
    data.action_id = 1;

    rs = fl->insert((void *)&key, (void *)&data, &hbm_hash_idx);
    ASSERT_TRUE(rs == SDK_RET_OK);
    printf("flow idx: %d\n", hbm_hash_idx);

    key.flow_lkp_metadata_lkp_sport = 30;
    rs = fl->insert((void *)&key, (void *)&data, &hbm_hash_idx);
    printf("rs: %d\n", rs);
    ASSERT_TRUE(rs == SDK_RET_OK);
    printf("flow idx: %d\n", hbm_hash_idx);

    key.flow_lkp_metadata_lkp_sport = 20;
    key.flow_lkp_metadata_lkp_dport = 20;
    rs = fl->insert((void *)&key, (void *)&data, &hbm_hash_idx);
    printf("rs: %d\n", rs);
    ASSERT_TRUE(rs == SDK_RET_OK);
    printf("flow idx: %d\n", hbm_hash_idx);
}

// ----------------------------------------------------------------------------
// Test 2:
//      - Create flow table
//      - Insert a flow entry.
// ----------------------------------------------------------------------------
TEST_F(hbm_hash_test, test2) {

    sdk_ret_t rs;
    flow_hash_swkey key;
    flow_hash_actiondata_t data;
    uint32_t hbm_hash_idx = 0;

    memset(&key, 0, sizeof(key));
    memset(&data, 0, sizeof(data));
    HbmHash *fl = HbmHash::factory("FlowTable", P4TBL_ID_FLOW_HASH,
                                   P4TBL_ID_FLOW_HASH_OVERFLOW,
                                   2097152, 16384,
                                   sizeof(key), sizeof(data));

    key.flow_lkp_metadata_lkp_sport = 10;
    data.action_id = 1;

    rs = fl->insert((void *)&key, (void *)&data, &hbm_hash_idx);
    ASSERT_TRUE(rs == SDK_RET_OK);
    printf("flow idx: %d\n", hbm_hash_idx);

    data.action_id = 2;
    rs = fl->update(hbm_hash_idx, (void *)&data);
    ASSERT_TRUE(rs == SDK_RET_OK);
    printf("flow idx: %d\n", hbm_hash_idx);
}

// ----------------------------------------------------------------------------
// Test 3:
//      - Create flow table
//      - Insert a flow entry.
// ----------------------------------------------------------------------------
TEST_F(hbm_hash_test, test3) {

    sdk_ret_t rs;
    flow_hash_swkey key;
    flow_hash_actiondata_t data;
    uint32_t hbm_hash_idx = 0;

    memset(&key, 0, sizeof(key));
    memset(&data, 0, sizeof(data));
    HbmHash *fl = HbmHash::factory("FlowTable", P4TBL_ID_FLOW_HASH,
                                   P4TBL_ID_FLOW_HASH_OVERFLOW,
                                   2097152, 16384,
                                   sizeof(key), sizeof(data));

    key.flow_lkp_metadata_lkp_sport = 10;
    data.action_id = 1;

    for (int i = 10; i < 20; i++) {
        key.flow_lkp_metadata_lkp_sport++;
        data.action_id = 1;
        rs = fl->insert((void *)&key, (void *)&data,  &hbm_hash_idx);
        ASSERT_TRUE(rs == SDK_RET_OK);
        printf("flow idx: %d\n", hbm_hash_idx);
    }

}

// ----------------------------------------------------------------------------
// Test 4:
//      - Create flow table
//      - Insert a flow entry.
//      - Insert a flow entry.
// ----------------------------------------------------------------------------
TEST_F(hbm_hash_test, test4) {

    sdk_ret_t rs;
    flow_hash_swkey key;
    flow_hash_actiondata_t data;
    uint32_t hbm_hash_idx = 0;

    memset(&key, 0, sizeof(key));
    memset(&data, 0, sizeof(data));
    HbmHash *fl = HbmHash::factory("FlowTable", P4TBL_ID_FLOW_HASH,
                                   P4TBL_ID_FLOW_HASH_OVERFLOW,
                                   2097152, 16384,
                                   sizeof(key), sizeof(data));

    key.flow_lkp_metadata_lkp_sport = 10;
    data.action_id = 1;

    rs = fl->insert((void *)&key, (void *)&data, &hbm_hash_idx);
    ASSERT_TRUE(rs == SDK_RET_OK);
    printf("flow idx: %d\n", hbm_hash_idx);

    rs = fl->insert((void *)&key, (void *)&data, &hbm_hash_idx);
    SDK_TRACE_DEBUG("rs: %d\n", rs);
    ASSERT_TRUE(rs == SDK_RET_ENTRY_EXISTS);
}

// ----------------------------------------------------------------------------
// Test 5:
//      - Create flow table
//      - Insert a flow entry.
// ----------------------------------------------------------------------------
TEST_F(hbm_hash_test, test5) {

    sdk_ret_t rs;
    flow_hash_swkey key;
    flow_hash_actiondata_t data;
    uint32_t hbm_hash_idx = 0;

    memset(&key, 0, sizeof(key));
    memset(&data, 0, sizeof(data));
    HbmHash *fl = HbmHash::factory("FlowTable", P4TBL_ID_FLOW_HASH,
                                   P4TBL_ID_FLOW_HASH_OVERFLOW,
                                   2097152, 16384,
                                   sizeof(key), sizeof(data));

    key.flow_lkp_metadata_lkp_sport = 10;
    data.action_id = 1;

    for (int i = 0; i < 100; i++) {
        key.flow_lkp_metadata_lkp_sport = key.flow_lkp_metadata_lkp_sport + 10;
        data.action_id = 2;
        rs = fl->insert((void *)&key, (void *)&data,  &hbm_hash_idx);
        printf("rs: %d \n", rs);
        ASSERT_TRUE(rs == SDK_RET_OK);
        printf("flow idx: %d\n", hbm_hash_idx);
    }

}

// ----------------------------------------------------------------------------
// Test 6:
//      - Create flow table
//      - -ve update cases
// ----------------------------------------------------------------------------
TEST_F(hbm_hash_test, test6) {

    sdk_ret_t rs;
    flow_hash_swkey key;
    flow_hash_actiondata_t data;
    uint32_t hbm_hash_idx[3] = { 0 };

    memset(&key, 0, sizeof(key));
    memset(&data, 0, sizeof(data));
    HbmHash *fl = HbmHash::factory("FlowTable", P4TBL_ID_FLOW_HASH,
                                   P4TBL_ID_FLOW_HASH_OVERFLOW,
                                   2097152, 16384,
                                   sizeof(key), sizeof(data));

    key.flow_lkp_metadata_lkp_sport = 10;
    data.action_id = 1;

    for (int i = 0; i < 3; i++) {
        key.flow_lkp_metadata_lkp_sport = key.flow_lkp_metadata_lkp_sport + 10;
        rs = fl->insert((void *)&key, (void *)&data, &hbm_hash_idx[i]);
        ASSERT_TRUE(rs == SDK_RET_OK);
    }

    data.action_id = 2;
    rs = fl->update(hbm_hash_idx[2] + 1, (void *)&data);
    ASSERT_TRUE(rs == SDK_RET_ENTRY_NOT_FOUND);

    for (int i = 0; i < 3; i++) {
        rs = fl->update(hbm_hash_idx[i], (void *)&data);
        ASSERT_TRUE(rs == SDK_RET_OK);
    }

}

// ----------------------------------------------------------------------------
// Test 7:
//      - Create flow table
// ----------------------------------------------------------------------------
TEST_F(hbm_hash_test, DISABLED_test7) {

    // sdk_ret_t rs;
    flow_hash_swkey key = {0};
    flow_hash_actiondata_t data = {0};
    unsigned seed = std::time(0);
    std::srand (seed);
    // uint32_t hbm_hash_idx[1000000] = { 0 };
    uint32_t ft_bits = 0;

    memset(&key, 0, sizeof(key));
    memset(&data, 0, sizeof(data));
    HbmHash *fl = HbmHash::factory("FlowTable", P4TBL_ID_FLOW_HASH,
                                   P4TBL_ID_FLOW_HASH_OVERFLOW,
                                   262144, 16384,
                                   sizeof(key), sizeof(data));
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
        SDK_TRACE_DEBUG("Checking:%#x\n", ft_bits);

        if (ft_bits == 0x10001) {
            SDK_TRACE_DEBUG("MATCH::");
            for (int i = 0; i < 4; i++) {
                SDK_TRACE_DEBUG("Src[%d]: %#x\n", i, key.flow_lkp_metadata_lkp_src[i]);
                SDK_TRACE_DEBUG("Dst[%d]: %#x\n", i, key.flow_lkp_metadata_lkp_dst[i]);
            }
            SDK_TRACE_DEBUG("Vrf:%d, sport:%d, dport:%d, proto:%d\n",
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
        rs = fl.insert((void *)&key, (void *)&data, &hbm_hash_idx[i]);
        ASSERT_TRUE(rs == SDK_RET_OK || rs == SDK_RET_COLLISION);
        key.flow_lkp_metadata_lkp_dport++;
        if (key.flow_lkp_metadata_lkp_dport == 0) {
            base++;
            key.flow_lkp_metadata_lkp_sport = base;
        }
    }
#endif

#if 0
    data.action_id = 2;
    rs = fl.update(hbm_hash_idx[2] + 1, (void *)&data);
    ASSERT_TRUE(rs == SDK_RET_ENTRY_NOT_FOUND);

    for (int i = 0; i < 3; i++) {
        rs = fl.update(hbm_hash_idx[i], (void *)&data);
        ASSERT_TRUE(rs == SDK_RET_OK);
    }
#endif
}

// ----------------------------------------------------------------------------
// Test 8:
//      - Create flow with collisions
// ----------------------------------------------------------------------------
TEST_F(hbm_hash_test, test8) {

    sdk_ret_t rs;
    flow_hash_swkey key = {0};
    flow_hash_actiondata_t data = {0};
    uint32_t hbm_hash_idx[9] = { 0 };

    memset(&key, 0, sizeof(key));
    memset(&data, 0, sizeof(data));
    HbmHash *fl = HbmHash::factory("FlowTable", P4TBL_ID_FLOW_HASH,
                                   P4TBL_ID_FLOW_HASH_OVERFLOW,
                                   262144, 16384,
                                   sizeof(key), sizeof(data));
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
    rs = fl->insert((void *)&key, (void *)&data, &hbm_hash_idx[0]);
    ASSERT_TRUE(rs == SDK_RET_OK || rs == SDK_RET_COLLISION);

    // Entry 2:
    key.flow_lkp_metadata_lkp_vrf = 3;
    key.flow_lkp_metadata_lkp_sport = 0x3b01;
    key.flow_lkp_metadata_lkp_dport = 0x3;
    data.action_id = 0;
    rs = fl->insert((void *)&key, (void *)&data, &hbm_hash_idx[1]);
    ASSERT_TRUE(rs == SDK_RET_OK || rs == SDK_RET_COLLISION);

    // Remove Entry 1
    rs = fl->remove(hbm_hash_idx[0]);
    ASSERT_TRUE(rs == SDK_RET_OK);

    // Remove Entry 2
    rs = fl->remove(hbm_hash_idx[1]);
    ASSERT_TRUE(rs == SDK_RET_OK);
}

// ----------------------------------------------------------------------------
// Test 9:
//      - Create flow with collisions
// ----------------------------------------------------------------------------
TEST_F(hbm_hash_test, test9) {

    sdk_ret_t rs;
    flow_hash_swkey key = {0};
    flow_hash_actiondata_t data = {0};
    uint32_t hbm_hash_idx[9] = { 0 };

    memset(&key, 0, sizeof(key));
    memset(&data, 0, sizeof(data));
    HbmHash *fl = HbmHash::factory("FlowTable", P4TBL_ID_FLOW_HASH,
                                   P4TBL_ID_FLOW_HASH_OVERFLOW,
                                   262144, 16384,
                                   sizeof(key), sizeof(data));
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
    rs = fl->insert((void *)&key, (void *)&data, &hbm_hash_idx[0]);
    ASSERT_TRUE(rs == SDK_RET_OK || rs == SDK_RET_COLLISION);

    // Entry 2:
    key.flow_lkp_metadata_lkp_vrf = 3;
    key.flow_lkp_metadata_lkp_sport = 0x3b01;
    key.flow_lkp_metadata_lkp_dport = 0x3;
    data.action_id = 0;
    rs = fl->insert((void *)&key, (void *)&data, &hbm_hash_idx[1]);
    ASSERT_TRUE(rs == SDK_RET_OK || rs == SDK_RET_COLLISION);

    // Remove Entry 2
    rs = fl->remove(hbm_hash_idx[1]);
    ASSERT_TRUE(rs == SDK_RET_OK);

    // Remove Entry 1
    rs = fl->remove(hbm_hash_idx[0]);
    ASSERT_TRUE(rs == SDK_RET_OK);
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
TEST_F(hbm_hash_test, test10) {

    sdk_ret_t rs;
    flow_hash_swkey key = {0};
    flow_hash_actiondata_t data = {0};
    uint32_t hbm_hash_idx[9] = { 0 };

    memset(&key, 0, sizeof(key));
    memset(&data, 0, sizeof(data));
    HbmHash *fl = HbmHash::factory("FlowTable", P4TBL_ID_FLOW_HASH,
                                   P4TBL_ID_FLOW_HASH_OVERFLOW,
                                   262144, 16384,
                                   sizeof(key), sizeof(data));
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
    rs = fl->insert((void *)&key, (void *)&data, &hbm_hash_idx[0]);
    ASSERT_TRUE(rs == SDK_RET_OK || rs == SDK_RET_COLLISION);

    // Entry 2:
    key.flow_lkp_metadata_lkp_vrf = 3;
    key.flow_lkp_metadata_lkp_sport = 0x3b01;
    key.flow_lkp_metadata_lkp_dport = 0x3;
    data.action_id = 0;
    rs = fl->insert((void *)&key, (void *)&data, &hbm_hash_idx[1]);
    ASSERT_TRUE(rs == SDK_RET_OK || rs == SDK_RET_COLLISION);

    // Entry 3:
    key.flow_lkp_metadata_lkp_vrf = 3;
    key.flow_lkp_metadata_lkp_sport = 0xd238;
    key.flow_lkp_metadata_lkp_dport = 0x4;
    data.action_id = 0;
    rs = fl->insert((void *)&key, (void *)&data, &hbm_hash_idx[2]);
    ASSERT_TRUE(rs == SDK_RET_OK || rs == SDK_RET_COLLISION);

    // Remove Entry 1
    rs = fl->remove(hbm_hash_idx[0]);
    ASSERT_TRUE(rs == SDK_RET_OK);

    // Remove Entry 1
    rs = fl->remove(hbm_hash_idx[1]);
    ASSERT_TRUE(rs == SDK_RET_OK);

    // Remove Entry 2
    rs = fl->remove(hbm_hash_idx[2]);
    ASSERT_TRUE(rs == SDK_RET_OK);
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
TEST_F(hbm_hash_test, test11) {

    sdk_ret_t rs;
    flow_hash_swkey key = {0};
    flow_hash_actiondata_t data = {0};
    uint32_t hbm_hash_idx[9] = { 0 };

    memset(&key, 0, sizeof(key));
    memset(&data, 0, sizeof(data));
    HbmHash *fl = HbmHash::factory("FlowTable", P4TBL_ID_FLOW_HASH,
                                   P4TBL_ID_FLOW_HASH_OVERFLOW,
                                   262144, 16384,
                                   sizeof(key), sizeof(data));
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
    rs = fl->insert((void *)&key, (void *)&data, &hbm_hash_idx[0]);
    ASSERT_TRUE(rs == SDK_RET_OK || rs == SDK_RET_COLLISION);

    // Entry 2:
    key.flow_lkp_metadata_lkp_vrf = 3;
    key.flow_lkp_metadata_lkp_sport = 0x3b01;
    key.flow_lkp_metadata_lkp_dport = 0x3;
    data.action_id = 0;
    rs = fl->insert((void *)&key, (void *)&data, &hbm_hash_idx[1]);
    ASSERT_TRUE(rs == SDK_RET_OK || rs == SDK_RET_COLLISION);

    // Entry 3:
    key.flow_lkp_metadata_lkp_vrf = 3;
    key.flow_lkp_metadata_lkp_sport = 0xd238;
    key.flow_lkp_metadata_lkp_dport = 0x4;
    data.action_id = 0;
    rs = fl->insert((void *)&key, (void *)&data, &hbm_hash_idx[2]);
    ASSERT_TRUE(rs == SDK_RET_OK || rs == SDK_RET_COLLISION);

    // Remove Entry 1
    rs = fl->remove(hbm_hash_idx[0]);
    ASSERT_TRUE(rs == SDK_RET_OK);

    // Remove Entry 1
    rs = fl->remove(hbm_hash_idx[2]);
    ASSERT_TRUE(rs == SDK_RET_OK);

    // Remove Entry 2
    rs = fl->remove(hbm_hash_idx[1]);
    ASSERT_TRUE(rs == SDK_RET_OK);
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
TEST_F(hbm_hash_test, test12) {

    sdk_ret_t rs;
    flow_hash_swkey key = {0};
    flow_hash_actiondata_t data = {0};
    uint32_t hbm_hash_idx[9] = { 0 };

    memset(&key, 0, sizeof(key));
    memset(&data, 0, sizeof(data));
    HbmHash *fl = HbmHash::factory("FlowTable", P4TBL_ID_FLOW_HASH,
                                   P4TBL_ID_FLOW_HASH_OVERFLOW,
                                   262144, 16384,
                                   sizeof(key), sizeof(data));
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
    rs = fl->insert((void *)&key, (void *)&data, &hbm_hash_idx[0]);
    ASSERT_TRUE(rs == SDK_RET_OK || rs == SDK_RET_COLLISION);

    // Entry 2:
    key.flow_lkp_metadata_lkp_vrf = 3;
    key.flow_lkp_metadata_lkp_sport = 0x3b01;
    key.flow_lkp_metadata_lkp_dport = 0x3;
    data.action_id = 0;
    rs = fl->insert((void *)&key, (void *)&data, &hbm_hash_idx[1]);
    ASSERT_TRUE(rs == SDK_RET_OK || rs == SDK_RET_COLLISION);

    // Entry 3:
    key.flow_lkp_metadata_lkp_vrf = 3;
    key.flow_lkp_metadata_lkp_sport = 0xd238;
    key.flow_lkp_metadata_lkp_dport = 0x4;
    data.action_id = 0;
    rs = fl->insert((void *)&key, (void *)&data, &hbm_hash_idx[2]);
    ASSERT_TRUE(rs == SDK_RET_OK || rs == SDK_RET_COLLISION);

    // Entry 4:
    key.flow_lkp_metadata_lkp_vrf = 3;
    key.flow_lkp_metadata_lkp_sport = 0x4b4e;
    key.flow_lkp_metadata_lkp_dport = 0x5;
    data.action_id = 0;
    rs = fl->insert((void *)&key, (void *)&data, &hbm_hash_idx[3]);
    ASSERT_TRUE(rs == SDK_RET_OK || rs == SDK_RET_COLLISION);

    // Remove Entry 1
    rs = fl->remove(hbm_hash_idx[0]);
    ASSERT_TRUE(rs == SDK_RET_OK);

    // Remove Entry 2
    rs = fl->remove(hbm_hash_idx[1]);
    ASSERT_TRUE(rs == SDK_RET_OK);

    // Remove Entry 3
    rs = fl->remove(hbm_hash_idx[2]);
    ASSERT_TRUE(rs == SDK_RET_OK);

    // Remove Entry 4
    rs = fl->remove(hbm_hash_idx[3]);
    ASSERT_TRUE(rs == SDK_RET_OK);
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
TEST_F(hbm_hash_test, test13) {

    sdk_ret_t rs;
    flow_hash_swkey key = {0};
    flow_hash_actiondata_t data = {0};
    uint32_t hbm_hash_idx[9] = { 0 };

    memset(&key, 0, sizeof(key));
    memset(&data, 0, sizeof(data));
    HbmHash *fl = HbmHash::factory("FlowTable", P4TBL_ID_FLOW_HASH,
                                   P4TBL_ID_FLOW_HASH_OVERFLOW,
                                   262144, 16384,
                                   sizeof(key), sizeof(data));
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
    rs = fl->insert((void *)&key, (void *)&data, &hbm_hash_idx[0]);
    ASSERT_TRUE(rs == SDK_RET_OK || rs == SDK_RET_COLLISION);

    // Entry 2:
    key.flow_lkp_metadata_lkp_vrf = 3;
    key.flow_lkp_metadata_lkp_sport = 0x3b01;
    key.flow_lkp_metadata_lkp_dport = 0x3;
    data.action_id = 0;
    rs = fl->insert((void *)&key, (void *)&data, &hbm_hash_idx[1]);
    ASSERT_TRUE(rs == SDK_RET_OK || rs == SDK_RET_COLLISION);

    // Entry 3:
    key.flow_lkp_metadata_lkp_vrf = 3;
    key.flow_lkp_metadata_lkp_sport = 0xd238;
    key.flow_lkp_metadata_lkp_dport = 0x4;
    data.action_id = 0;
    rs = fl->insert((void *)&key, (void *)&data, &hbm_hash_idx[2]);
    ASSERT_TRUE(rs == SDK_RET_OK || rs == SDK_RET_COLLISION);

    // Entry 4:
    key.flow_lkp_metadata_lkp_vrf = 3;
    key.flow_lkp_metadata_lkp_sport = 0x4b4e;
    key.flow_lkp_metadata_lkp_dport = 0x5;
    data.action_id = 0;
    rs = fl->insert((void *)&key, (void *)&data, &hbm_hash_idx[3]);
    ASSERT_TRUE(rs == SDK_RET_OK || rs == SDK_RET_COLLISION);

    // Remove Entry 1
    rs = fl->remove(hbm_hash_idx[3]);
    ASSERT_TRUE(rs == SDK_RET_OK);

    // Remove Entry 2
    rs = fl->remove(hbm_hash_idx[2]);
    ASSERT_TRUE(rs == SDK_RET_OK);

    // Remove Entry 3
    rs = fl->remove(hbm_hash_idx[1]);
    ASSERT_TRUE(rs == SDK_RET_OK);

    // Remove Entry 4
    rs = fl->remove(hbm_hash_idx[0]);
    ASSERT_TRUE(rs == SDK_RET_OK);
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
TEST_F(hbm_hash_test, test14) {

    sdk_ret_t rs;
    flow_hash_swkey key = {0};
    flow_hash_actiondata_t data = {0};
    uint32_t hbm_hash_idx[9] = { 0 };
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
    HbmHash *fl = HbmHash::factory("FlowTable", P4TBL_ID_FLOW_HASH,
                                   P4TBL_ID_FLOW_HASH_OVERFLOW,
                                   262144, 16384,
                                   sizeof(key), sizeof(data));
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
    rs = fl->insert((void *)&key, (void *)&data, &hbm_hash_idx[0]);
    ASSERT_TRUE(rs == SDK_RET_OK || rs == SDK_RET_COLLISION);

    // Entry 2:
    key.flow_lkp_metadata_lkp_vrf = 3;
    key.flow_lkp_metadata_lkp_sport = 0x3b01;
    key.flow_lkp_metadata_lkp_dport = 0x3;
    data.action_id = 0;
    rs = fl->insert((void *)&key, (void *)&data, &hbm_hash_idx[1]);
    ASSERT_TRUE(rs == SDK_RET_OK || rs == SDK_RET_COLLISION);

    // Entry 3:
    key.flow_lkp_metadata_lkp_vrf = 3;
    key.flow_lkp_metadata_lkp_sport = 0xd238;
    key.flow_lkp_metadata_lkp_dport = 0x4;
    data.action_id = 0;
    rs = fl->insert((void *)&key, (void *)&data, &hbm_hash_idx[2]);
    ASSERT_TRUE(rs == SDK_RET_OK || rs == SDK_RET_COLLISION);

    // Entry 4:
    key.flow_lkp_metadata_lkp_vrf = 3;
    key.flow_lkp_metadata_lkp_sport = 0x4b4e;
    key.flow_lkp_metadata_lkp_dport = 0x5;
    data.action_id = 0;
    rs = fl->insert((void *)&key, (void *)&data, &hbm_hash_idx[3]);
    ASSERT_TRUE(rs == SDK_RET_OK || rs == SDK_RET_COLLISION);

    for (std::vector<int>::iterator it=myvector.begin(); it!=myvector.end(); ++it) {
        SDK_TRACE_DEBUG("Removing %d\n", *it);
        rs = fl->remove(hbm_hash_idx[*it]);
        ASSERT_TRUE(rs == SDK_RET_OK);
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
TEST_F(hbm_hash_test, test15) {

    sdk_ret_t rs;
    flow_hash_swkey key = {0};
    flow_hash_actiondata_t data = {0};
    uint32_t hbm_hash_idx[9] = { 0 };
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
    HbmHash *fl = HbmHash::factory("FlowTable", P4TBL_ID_FLOW_HASH,
                                   P4TBL_ID_FLOW_HASH_OVERFLOW,
                                   262144, 16384,
                                   sizeof(key), sizeof(data));
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
    rs = fl->insert((void *)&key, (void *)&data, &hbm_hash_idx[0]);
    ASSERT_TRUE(rs == SDK_RET_OK || rs == SDK_RET_COLLISION);

    // Entry 2:
    key.flow_lkp_metadata_lkp_vrf = 3;
    key.flow_lkp_metadata_lkp_sport = 0x3b01;
    key.flow_lkp_metadata_lkp_dport = 0x3;
    data.action_id = 0;
    rs = fl->insert((void *)&key, (void *)&data, &hbm_hash_idx[1]);
    ASSERT_TRUE(rs == SDK_RET_OK || rs == SDK_RET_COLLISION);

    // Entry 3:
    key.flow_lkp_metadata_lkp_vrf = 3;
    key.flow_lkp_metadata_lkp_sport = 0xd238;
    key.flow_lkp_metadata_lkp_dport = 0x4;
    data.action_id = 0;
    rs = fl->insert((void *)&key, (void *)&data, &hbm_hash_idx[2]);
    ASSERT_TRUE(rs == SDK_RET_OK || rs == SDK_RET_COLLISION);

    // Entry 4:
    key.flow_lkp_metadata_lkp_vrf = 3;
    key.flow_lkp_metadata_lkp_sport = 0x4b4e;
    key.flow_lkp_metadata_lkp_dport = 0x5;
    data.action_id = 0;
    rs = fl->insert((void *)&key, (void *)&data, &hbm_hash_idx[3]);
    ASSERT_TRUE(rs == SDK_RET_OK || rs == SDK_RET_COLLISION);

    // Entry 5:
    key.flow_lkp_metadata_lkp_vrf = 3;
    key.flow_lkp_metadata_lkp_sport = 0;
    key.flow_lkp_metadata_lkp_dport = 0x36ad;
    data.action_id = 0;
    rs = fl->insert((void *)&key, (void *)&data, &hbm_hash_idx[4]);
    ASSERT_TRUE(rs == SDK_RET_OK || rs == SDK_RET_COLLISION);

    for (std::vector<int>::iterator it=myvector.begin(); it!=myvector.end(); ++it) {
        SDK_TRACE_DEBUG("Removing %d\n", *it);
        rs = fl->remove(hbm_hash_idx[*it]);
        ASSERT_TRUE(rs == SDK_RET_OK);
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
TEST_F(hbm_hash_test, test16) {

    sdk_ret_t rs;
    flow_hash_swkey key = {0};
    flow_hash_actiondata_t data = {0};
    uint32_t hbm_hash_idx[9] = { 0 };
    unsigned seed = std::time(0);
    // unsigned seed = 1504501660;
    std::srand (seed);
    // std::srand ( unsigned ( std::time(0) ) );
    std::vector<int> myvector;

    for (int i=0; i<8; ++i) myvector.push_back(i);

    std::random_shuffle ( myvector.begin(), myvector.end() );
    // print out content:
    SDK_TRACE_DEBUG("seed:%d\n", seed);
    std::cout << "myvector contains:";
    for (std::vector<int>::iterator it=myvector.begin(); it!=myvector.end(); ++it)
        std::cout << ' ' << *it;
    std::cout << std::endl;


    memset(&key, 0, sizeof(key));
    memset(&data, 0, sizeof(data));
    HbmHash *fl = HbmHash::factory("FlowTable", P4TBL_ID_FLOW_HASH,
                                   P4TBL_ID_FLOW_HASH_OVERFLOW,
                                   262144, 16384,
                                   sizeof(key), sizeof(data));
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
    rs = fl->insert((void *)&key, (void *)&data, &hbm_hash_idx[0]);
    ASSERT_TRUE(rs == SDK_RET_OK || rs == SDK_RET_COLLISION);

    // Entry 2:
    key.flow_lkp_metadata_lkp_vrf = 3;
    key.flow_lkp_metadata_lkp_sport = 0x3b01;
    key.flow_lkp_metadata_lkp_dport = 0x3;
    data.action_id = 0;
    rs = fl->insert((void *)&key, (void *)&data, &hbm_hash_idx[1]);
    ASSERT_TRUE(rs == SDK_RET_OK || rs == SDK_RET_COLLISION);

    // Entry 3:
    key.flow_lkp_metadata_lkp_vrf = 3;
    key.flow_lkp_metadata_lkp_sport = 0xd238;
    key.flow_lkp_metadata_lkp_dport = 0x4;
    data.action_id = 0;
    rs = fl->insert((void *)&key, (void *)&data, &hbm_hash_idx[2]);
    ASSERT_TRUE(rs == SDK_RET_OK || rs == SDK_RET_COLLISION);

    // Entry 4:
    key.flow_lkp_metadata_lkp_vrf = 3;
    key.flow_lkp_metadata_lkp_sport = 0x4b4e;
    key.flow_lkp_metadata_lkp_dport = 0x5;
    data.action_id = 0;
    rs = fl->insert((void *)&key, (void *)&data, &hbm_hash_idx[3]);
    ASSERT_TRUE(rs == SDK_RET_OK || rs == SDK_RET_COLLISION);

    // Entry 5:
    key.flow_lkp_metadata_lkp_vrf = 3;
    key.flow_lkp_metadata_lkp_sport = 0;
    key.flow_lkp_metadata_lkp_dport = 0x36ad;
    data.action_id = 0;
    rs = fl->insert((void *)&key, (void *)&data, &hbm_hash_idx[4]);
    ASSERT_TRUE(rs == SDK_RET_OK || rs == SDK_RET_COLLISION);

    // Entry 6:
    key.flow_lkp_metadata_lkp_vrf = 3;
    key.flow_lkp_metadata_lkp_sport = 3;
    key.flow_lkp_metadata_lkp_dport = 0xd125;
    data.action_id = 0;
    rs = fl->insert((void *)&key, (void *)&data, &hbm_hash_idx[5]);
    ASSERT_TRUE(rs == SDK_RET_OK || rs == SDK_RET_COLLISION);

    // Entry 7:
    key.flow_lkp_metadata_lkp_vrf = 0;
    key.flow_lkp_metadata_lkp_sport = 0;
    key.flow_lkp_metadata_lkp_dport = 0xc430;
    data.action_id = 0;
    rs = fl->insert((void *)&key, (void *)&data, &hbm_hash_idx[6]);
    ASSERT_TRUE(rs == SDK_RET_OK || rs == SDK_RET_COLLISION);

    // Entry 8:
    key.flow_lkp_metadata_lkp_vrf = 1;
    key.flow_lkp_metadata_lkp_sport = 1;
    key.flow_lkp_metadata_lkp_dport = 0xc8c3;
    data.action_id = 0;
    rs = fl->insert((void *)&key, (void *)&data, &hbm_hash_idx[7]);
    ASSERT_TRUE(rs == SDK_RET_OK || rs == SDK_RET_COLLISION);

    for (std::vector<int>::iterator it=myvector.begin(); it!=myvector.end(); ++it) {
        SDK_TRACE_DEBUG("Removing %d\n", *it);
        rs = fl->remove(hbm_hash_idx[*it]);
        ASSERT_TRUE(rs == SDK_RET_OK);
    }
}
// ----------------------------------------------------------------------------
// Test 17:
//      - Create flow with collisions
//      - Insert 1 - 9
//      - Randomly delete
// ----------------------------------------------------------------------------
TEST_F(hbm_hash_test, test17) {

    sdk_ret_t rs;
    flow_hash_swkey key = {0};
    flow_hash_actiondata_t data = {0};
    uint32_t max = 11;
    uint32_t hbm_hash_idx[max] = { 0 };
    unsigned seed = std::time(0);
    // unsigned seed = 1504501660;
    std::srand (seed);
    // std::srand ( unsigned ( std::time(0) ) );
    std::vector<int> myvector;

    for (uint i=0; i < max; ++i) myvector.push_back(i);

    std::random_shuffle ( myvector.begin(), myvector.end() );
    // print out content:
    SDK_TRACE_DEBUG("seed: %d\n", seed);
    std::cout << "myvector contains:";
    for (std::vector<int>::iterator it=myvector.begin(); it!=myvector.end(); ++it)
        std::cout << ' ' << *it;
    std::cout << std::endl;


    memset(&key, 0, sizeof(key));
    memset(&data, 0, sizeof(data));
    HbmHash *fl = HbmHash::factory("FlowTable", P4TBL_ID_FLOW_HASH,
                                   P4TBL_ID_FLOW_HASH_OVERFLOW,
                                   262144, 16384,
                                   sizeof(key), sizeof(data));
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
    rs = fl->insert((void *)&key, (void *)&data, &hbm_hash_idx[0]);
    ASSERT_TRUE(rs == SDK_RET_OK || rs == SDK_RET_COLLISION);

    // Entry 2:
    key.flow_lkp_metadata_lkp_vrf = 3;
    key.flow_lkp_metadata_lkp_sport = 0x3b01;
    key.flow_lkp_metadata_lkp_dport = 0x3;
    data.action_id = 0;
    rs = fl->insert((void *)&key, (void *)&data, &hbm_hash_idx[1]);
    ASSERT_TRUE(rs == SDK_RET_OK || rs == SDK_RET_COLLISION);

    // Entry 3:
    key.flow_lkp_metadata_lkp_vrf = 3;
    key.flow_lkp_metadata_lkp_sport = 0xd238;
    key.flow_lkp_metadata_lkp_dport = 0x4;
    data.action_id = 0;
    rs = fl->insert((void *)&key, (void *)&data, &hbm_hash_idx[2]);
    ASSERT_TRUE(rs == SDK_RET_OK || rs == SDK_RET_COLLISION);

    // Entry 4:
    key.flow_lkp_metadata_lkp_vrf = 3;
    key.flow_lkp_metadata_lkp_sport = 0x4b4e;
    key.flow_lkp_metadata_lkp_dport = 0x5;
    data.action_id = 0;
    rs = fl->insert((void *)&key, (void *)&data, &hbm_hash_idx[3]);
    ASSERT_TRUE(rs == SDK_RET_OK || rs == SDK_RET_COLLISION);

    // Entry 5:
    key.flow_lkp_metadata_lkp_vrf = 3;
    key.flow_lkp_metadata_lkp_sport = 0;
    key.flow_lkp_metadata_lkp_dport = 0x36ad;
    data.action_id = 0;
    rs = fl->insert((void *)&key, (void *)&data, &hbm_hash_idx[4]);
    ASSERT_TRUE(rs == SDK_RET_OK || rs == SDK_RET_COLLISION);

    // Entry 6:
    key.flow_lkp_metadata_lkp_vrf = 3;
    key.flow_lkp_metadata_lkp_sport = 3;
    key.flow_lkp_metadata_lkp_dport = 0xd125;
    data.action_id = 0;
    rs = fl->insert((void *)&key, (void *)&data, &hbm_hash_idx[5]);
    ASSERT_TRUE(rs == SDK_RET_OK || rs == SDK_RET_COLLISION);

    // Entry 7:
    key.flow_lkp_metadata_lkp_vrf = 0;
    key.flow_lkp_metadata_lkp_sport = 0;
    key.flow_lkp_metadata_lkp_dport = 0xc430;
    data.action_id = 0;
    rs = fl->insert((void *)&key, (void *)&data, &hbm_hash_idx[6]);
    ASSERT_TRUE(rs == SDK_RET_OK || rs == SDK_RET_COLLISION);

    // Entry 8:
    key.flow_lkp_metadata_lkp_vrf = 1;
    key.flow_lkp_metadata_lkp_sport = 1;
    key.flow_lkp_metadata_lkp_dport = 0xc8c3;
    data.action_id = 0;
    rs = fl->insert((void *)&key, (void *)&data, &hbm_hash_idx[7]);
    ASSERT_TRUE(rs == SDK_RET_OK || rs == SDK_RET_COLLISION);

    // Entry 9:
    key.flow_lkp_metadata_lkp_vrf = 1;
    key.flow_lkp_metadata_lkp_sport = 2;
    key.flow_lkp_metadata_lkp_dport = 0x2f4b;
    data.action_id = 0;
    rs = fl->insert((void *)&key, (void *)&data, &hbm_hash_idx[8]);
    ASSERT_TRUE(rs == SDK_RET_OK || rs == SDK_RET_COLLISION);


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
    rs = fl->insert((void *)&key, (void *)&data, &hbm_hash_idx[9]);
    ASSERT_TRUE(rs == SDK_RET_OK || rs == SDK_RET_COLLISION);

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
    rs = fl->insert((void *)&key, (void *)&data, &hbm_hash_idx[10]);
    ASSERT_TRUE(rs == SDK_RET_OK || rs == SDK_RET_COLLISION);

    for (std::vector<int>::iterator it=myvector.begin(); it!=myvector.end(); ++it) {
        SDK_TRACE_DEBUG("Removing %d\n", *it);
        rs = fl->remove(hbm_hash_idx[*it]);
        ASSERT_TRUE(rs == SDK_RET_OK);
    }

    SDK_TRACE_DEBUG("tableid:%d, table_name:%s, capacity:%d, coll_capac:%d, "
                    "num_in_use:%d, coll_num_in_use:%d"
                    "num_inserts:%d, num_insert_errors:%d, "
                    "num_deletes:%d, num_delete_errors:%d\n",
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
TEST_F(hbm_hash_test, DISABLED_test18) {

    sdk_ret_t rs;
    flow_hash_swkey key = {0};
    flow_hash_actiondata_t data = {0};
    uint32_t max = 10, count = 0;
    uint32_t hbm_hash_idx[max] = { 0 };
    unsigned seed = std::time(0);
    std::srand (seed);
    // uint32_t hbm_hash_idx[1000000] = { 0 };
    uint32_t ft_bits = 0;
    std::vector<int> myvector;

    for (uint i=0; i < max; ++i) myvector.push_back(i);

    std::random_shuffle ( myvector.begin(), myvector.end() );
    // print out content:
    SDK_TRACE_DEBUG("seed: %d\n", seed);
    std::cout << "myvector contains:";
    for (std::vector<int>::iterator it=myvector.begin(); it!=myvector.end(); ++it)
        std::cout << ' ' << *it;
    std::cout << std::endl;

    memset(&key, 0, sizeof(key));
    memset(&data, 0, sizeof(data));
    HbmHash *fl = HbmHash::factory("FlowTable", P4TBL_ID_FLOW_HASH,
                                   P4TBL_ID_FLOW_HASH_OVERFLOW,
                                   262144, 16384,
                                   sizeof(key), sizeof(data));
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
        SDK_TRACE_DEBUG("Checking:%#x\n", ft_bits);

        if (ft_bits == 0x10001) {
            SDK_TRACE_DEBUG("MATCH::");
            for (int i = 0; i < 4; i++) {
                SDK_TRACE_DEBUG("Src[%d]: %#x\n", i, key.flow_lkp_metadata_lkp_src[i]);
                SDK_TRACE_DEBUG("Dst[%d]: %#x\n", i, key.flow_lkp_metadata_lkp_dst[i]);
            }
            SDK_TRACE_DEBUG("Vrf:%d, sport:%d, dport:%d, proto:%d\n",
                    key.flow_lkp_metadata_lkp_vrf,
                    key.flow_lkp_metadata_lkp_sport,
                    key.flow_lkp_metadata_lkp_dport,
                    key.flow_lkp_metadata_lkp_proto);
            rs = fl->insert((void *)&key, (void *)&data, &hbm_hash_idx[count]);
            ASSERT_TRUE(rs == SDK_RET_OK || rs == SDK_RET_COLLISION);
            count++;
            if (count == max) {
                break;
            }
        }
    }
    // Remvoe the entries
    for (std::vector<int>::iterator it=myvector.begin(); it!=myvector.end(); ++it) {
        SDK_TRACE_DEBUG("Removing %d\n", *it);
        rs = fl->remove(hbm_hash_idx[*it]);
        ASSERT_TRUE(rs == SDK_RET_OK);
    }

    SDK_TRACE_DEBUG("tableid:%d, table_name:%s, capacity:%d, coll_capac:%d, "
                    "num_in_use:%d, coll_num_in_use:%d"
                    "num_inserts:%d, num_insert_errors:%d, "
                    "num_deletes:{}, num_delete_errors:%d\n",
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
TEST_F(hbm_hash_test, test19) {

    sdk_ret_t rs;
    flow_hash_swkey key = {0};
    flow_hash_actiondata_t data = {0};
    uint32_t max = 64000, count = 0;
    uint32_t hbm_hash_idx[max] = { 0 };
    uint32_t total_time = 0;
    // uint32_t hbm_hash_idx[1000000] = { 0 };
    // uint32_t ft_bits = 0;

    // unsigned seed = std::time(0);
    unsigned seed = 1529534551;
    std::srand (seed);
    SDK_TRACE_DEBUG("seed: %d\n", seed);

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
    HbmHash *fl = HbmHash::factory("FlowTable", P4TBL_ID_FLOW_HASH,
                                   P4TBL_ID_FLOW_HASH_OVERFLOW,
                                   262144, 16384,
                                   sizeof(key), sizeof(data));
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
        // SDK_TRACE_DEBUG("Checking:%#x\n", ft_bits);

        for (int i = 0; i < 4; i++) {
            SDK_TRACE_DEBUG("Src[%d]: %#x\n", i, key.flow_lkp_metadata_lkp_src[i]);
            SDK_TRACE_DEBUG("Dst[%d]: %#x\n", i, key.flow_lkp_metadata_lkp_dst[i]);
        }
        SDK_TRACE_DEBUG("Vrf:%#x, sport:%#x, dport:%#x, proto:%#x\n",
                        key.flow_lkp_metadata_lkp_vrf,
                        key.flow_lkp_metadata_lkp_sport,
                        key.flow_lkp_metadata_lkp_dport,
                        key.flow_lkp_metadata_lkp_proto);
        high_resolution_clock::time_point t1 = high_resolution_clock::now();
        rs = fl->insert((void *)&key, (void *)&data, &hbm_hash_idx[count]);
        high_resolution_clock::time_point t2 = high_resolution_clock::now();
        auto duration = duration_cast<microseconds>( t2 - t1 ).count();
        total_time += duration;
        ASSERT_TRUE(rs == SDK_RET_OK || rs == SDK_RET_COLLISION);
        count++;
    }

#if 0
    // Remvoe the entries
    for (std::vector<int>::iterator it=myvector.begin(); it!=myvector.end(); ++it) {
        SDK_TRACE_DEBUG("Removing %d\n", *it);
        rs = fl->remove(hbm_hash_idx[*it]);
        ASSERT_TRUE(rs == SDK_RET_OK);
    }
#endif

    printf("Total Time:%u, num_in_use:%u, coll_num_in_use:%u, num_inserts:%u, "
           "num_insert_errors:%u\n", total_time, fl->table_num_entries_in_use(),
           fl->coll_table_num_entries_in_use(), fl->table_num_inserts(), fl->table_num_insert_errors());

    SDK_TRACE_DEBUG("Took:%d tableid:%d, table_name:%s, capacity:%d, coll_capac:%d, "
                    "num_in_use:%d, coll_num_in_use:%d"
                    "num_inserts:%d, num_insert_errors:%d, "
                    "num_deletes:%d, num_delete_errors:%d\n", total_time,
                    fl->table_id(), fl->table_name(),
                    fl->table_capacity(), fl->coll_table_capacity(),
                    fl->table_num_entries_in_use(), fl->coll_table_num_entries_in_use(),
                    fl->table_num_inserts(), fl->table_num_insert_errors(),
                    fl->table_num_deletes(), fl->table_num_delete_errors());
}

int
main(int argc, char **argv)
{
    sdk_ret_t rs;
    std::string logfile;

    ::testing::InitGoogleTest(&argc, argv);

    SDK_TRACE_DEBUG("Starting Main ... \n");
    sdk::lib::pal_init(platform_type_t::PLATFORM_TYPE_MOCK);

    p4pd_cfg_t                 p4pd_cfg = {
        .table_map_cfg_file  = "iris/p4_table_map.json",
        .p4pd_pgm_name       = "iris",
        .p4pd_rxdma_pgm_name = "p4plus",
        .p4pd_txdma_pgm_name = "p4plus",
        .cfg_path            = std::getenv("HAL_CONFIG_PATH"),
    };

    p4pd_init(&p4pd_cfg);
    rs = sdk::asic::pd::asicpd_table_rw_init(NULL);
    assert(rs == SDK_RET_OK);

    // testing::GTEST_FLAG(filter) = "-*test18*";
    int res = RUN_ALL_TESTS();
    //p4pd_cleanup();
    return res;
}
