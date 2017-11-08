#include <gtest/gtest.h>
#include <stdio.h>
#include "nic/hal/pd/utils/flow/flow.hpp"
#include "nic/hal/pd/p4pd_api.hpp"
#include "nic/gen/iris/include/p4pd.h"
#include <boost/multiprecision/cpp_int.hpp>

using hal::pd::utils::Flow;
using boost::multiprecision::uint512_t;
using boost::multiprecision::uint128_t;

class flow_test : public ::testing::Test {
protected:
  flow_test() {
  }

  virtual ~flow_test() {
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
TEST_F(flow_test, test1) {

    hal_ret_t rs;
    uint32_t flow_idx = 0;

    flow_hash_swkey key;
    flow_hash_actiondata data;

    memset(&key, 0, sizeof(key));
    memset(&data, 0, sizeof(data));

    Flow fl = Flow("FlowTable", P4TBL_ID_FLOW_HASH, P4TBL_ID_FLOW_HASH_OVERFLOW,
                   2097152, 16384, sizeof(key), sizeof(data));

    key.flow_lkp_metadata_lkp_sport = 10;
    data.actionid = 1;

    rs = fl.insert((void *)&key, (void *)&data, &flow_idx);
    ASSERT_TRUE(rs == HAL_RET_OK);
    printf("flow idx: %d\n", flow_idx);

    key.flow_lkp_metadata_lkp_sport = 30;
    rs = fl.insert((void *)&key, (void *)&data, &flow_idx);
    printf("rs: %d\n", rs);
    ASSERT_TRUE(rs == HAL_RET_OK);
    printf("flow idx: %d\n", flow_idx);

    key.flow_lkp_metadata_lkp_sport = 20;
    key.flow_lkp_metadata_lkp_dport = 20;
    rs = fl.insert((void *)&key, (void *)&data, &flow_idx);
    printf("rs: %d\n", rs);
    ASSERT_TRUE(rs == HAL_RET_OK);
    printf("flow idx: %d\n", flow_idx);
}

// ----------------------------------------------------------------------------
// Test 2:
//      - Create flow table
//      - Insert a flow entry.
// ----------------------------------------------------------------------------
TEST_F(flow_test, test2) {

    hal_ret_t rs;
    flow_hash_swkey key;
    flow_hash_actiondata data;
    uint32_t flow_idx = 0;

    memset(&key, 0, sizeof(key));
    memset(&data, 0, sizeof(data));
    Flow fl = Flow("FlowTable", P4TBL_ID_FLOW_HASH, P4TBL_ID_FLOW_HASH_OVERFLOW, 
            2097152, 16384, sizeof(key), sizeof(data));

    key.flow_lkp_metadata_lkp_sport = 10;
    data.actionid = 1;

    rs = fl.insert((void *)&key, (void *)&data, &flow_idx);
    ASSERT_TRUE(rs == HAL_RET_OK);
    printf("flow idx: %d\n", flow_idx);

    data.actionid = 2;
    rs = fl.update(flow_idx, (void *)&data);
    ASSERT_TRUE(rs == HAL_RET_OK);
    printf("flow idx: %d\n", flow_idx);
}

// ----------------------------------------------------------------------------
// Test 3:
//      - Create flow table
//      - Insert a flow entry.
// ----------------------------------------------------------------------------
TEST_F(flow_test, test3) {

    hal_ret_t rs;
    flow_hash_swkey key;
    flow_hash_actiondata data;
    uint32_t flow_idx = 0;

    memset(&key, 0, sizeof(key));
    memset(&data, 0, sizeof(data));
    Flow fl = Flow("FlowTable", P4TBL_ID_FLOW_HASH, P4TBL_ID_FLOW_HASH_OVERFLOW, 
            2097152, 16384, sizeof(key), sizeof(data));

    key.flow_lkp_metadata_lkp_sport = 10;
    data.actionid = 1;

    for (int i = 10; i < 20; i++) {
        key.flow_lkp_metadata_lkp_sport++;
        data.actionid = 1;
        rs = fl.insert((void *)&key, (void *)&data,  &flow_idx);
        ASSERT_TRUE(rs == HAL_RET_OK);
        printf("flow idx: %d\n", flow_idx);
    }

}

// ----------------------------------------------------------------------------
// Test 4:
//      - Create flow table
//      - Insert a flow entry.
//      - Insert a flow entry.
// ----------------------------------------------------------------------------
TEST_F(flow_test, test4) {

    hal_ret_t rs;
    flow_hash_swkey key;
    flow_hash_actiondata data;
    uint32_t flow_idx = 0;

    memset(&key, 0, sizeof(key));
    memset(&data, 0, sizeof(data));
    Flow fl = Flow("FlowTable", P4TBL_ID_FLOW_HASH, P4TBL_ID_FLOW_HASH_OVERFLOW, 
            2097152, 16384, sizeof(key), sizeof(data));

    key.flow_lkp_metadata_lkp_sport = 10;
    data.actionid = 1;

    rs = fl.insert((void *)&key, (void *)&data, &flow_idx);
    ASSERT_TRUE(rs == HAL_RET_OK);
    printf("flow idx: %d\n", flow_idx);

    rs = fl.insert((void *)&key, (void *)&data, &flow_idx);
    ASSERT_TRUE(rs == HAL_RET_DUP_INS_FAIL);
}

// ----------------------------------------------------------------------------
// Test 5:
//      - Create flow table
//      - Insert a flow entry.
// ----------------------------------------------------------------------------
TEST_F(flow_test, test5) {

    hal_ret_t rs;
    flow_hash_swkey key;
    flow_hash_actiondata data;
    uint32_t flow_idx = 0;

    memset(&key, 0, sizeof(key));
    memset(&data, 0, sizeof(data));
    Flow fl = Flow("FlowTable", P4TBL_ID_FLOW_HASH, P4TBL_ID_FLOW_HASH_OVERFLOW, 
            2097152, 16384, sizeof(key), sizeof(data));

    key.flow_lkp_metadata_lkp_sport = 10;
    data.actionid = 1;

    for (int i = 0; i < 100; i++) {
        key.flow_lkp_metadata_lkp_sport = key.flow_lkp_metadata_lkp_sport + 10;
        data.actionid = 2;
        rs = fl.insert((void *)&key, (void *)&data,  &flow_idx);
        printf("rs: %d \n", rs);
        ASSERT_TRUE(rs == HAL_RET_OK);
        printf("flow idx: %d\n", flow_idx);
    }

}

// ----------------------------------------------------------------------------
// Test 6:
//      - Create flow table
//      - -ve update cases
// ----------------------------------------------------------------------------
TEST_F(flow_test, test6) {

    hal_ret_t rs;
    flow_hash_swkey key;
    flow_hash_actiondata data;
    uint32_t flow_idx[3] = { 0 };

    memset(&key, 0, sizeof(key));
    memset(&data, 0, sizeof(data));
    Flow fl = Flow("FlowTable", P4TBL_ID_FLOW_HASH, P4TBL_ID_FLOW_HASH_OVERFLOW, 
            2097152, 16384, sizeof(key), sizeof(data));

    key.flow_lkp_metadata_lkp_sport = 10;
    data.actionid = 1;

    for (int i = 0; i < 3; i++) {
        key.flow_lkp_metadata_lkp_sport = key.flow_lkp_metadata_lkp_sport + 10;
        rs = fl.insert((void *)&key, (void *)&data, &flow_idx[i]);
        ASSERT_TRUE(rs == HAL_RET_OK);
    }

    data.actionid = 2;
    rs = fl.update(flow_idx[2] + 1, (void *)&data);
    ASSERT_TRUE(rs == HAL_RET_ENTRY_NOT_FOUND);

    for (int i = 0; i < 3; i++) {
        rs = fl.update(flow_idx[i], (void *)&data);
        ASSERT_TRUE(rs == HAL_RET_OK);
    }

}

// ----------------------------------------------------------------------------
// Test 7:
//      - Create flow table
// ----------------------------------------------------------------------------
TEST_F(flow_test, DISABLED_test7) {

    // hal_ret_t rs;
    flow_hash_swkey key = {0};
    flow_hash_actiondata data = {0};
	unsigned seed = std::time(0);
    std::srand (seed);
    // uint32_t flow_idx[1000000] = { 0 };
    uint32_t ft_bits = 0;

    memset(&key, 0, sizeof(key));
    memset(&data, 0, sizeof(data));
    Flow fl = Flow("FlowTable", P4TBL_ID_FLOW_HASH, P4TBL_ID_FLOW_HASH_OVERFLOW, 
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

        ft_bits = fl.calc_hash_(&key, &data);
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
    data.actionid = 0;

    uint32_t base = 0;
    for (int i = 0; i < 1000000; i++) {
        rs = fl.insert((void *)&key, (void *)&data, &flow_idx[i]);
        ASSERT_TRUE(rs == HAL_RET_OK || rs == HAL_RET_FLOW_COLL);
        key.flow_lkp_metadata_lkp_dport++;
        if (key.flow_lkp_metadata_lkp_dport == 0) {
            base++;
            key.flow_lkp_metadata_lkp_sport = base;
        }
    }
#endif

#if 0
    data.actionid = 2;
    rs = fl.update(flow_idx[2] + 1, (void *)&data);
    ASSERT_TRUE(rs == HAL_RET_ENTRY_NOT_FOUND);

    for (int i = 0; i < 3; i++) {
        rs = fl.update(flow_idx[i], (void *)&data);
        ASSERT_TRUE(rs == HAL_RET_OK);
    }
#endif
}

// ----------------------------------------------------------------------------
// Test 8:
//      - Create flow with collisions
// ----------------------------------------------------------------------------
TEST_F(flow_test, test8) {

    hal_ret_t rs;
    flow_hash_swkey key = {0};
    flow_hash_actiondata data = {0};
    uint32_t flow_idx[9] = { 0 };

    memset(&key, 0, sizeof(key));
    memset(&data, 0, sizeof(data));
    Flow fl = Flow("FlowTable", P4TBL_ID_FLOW_HASH, P4TBL_ID_FLOW_HASH_OVERFLOW, 
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
    data.actionid = 0;
    rs = fl.insert((void *)&key, (void *)&data, &flow_idx[0]);
    ASSERT_TRUE(rs == HAL_RET_OK || rs == HAL_RET_FLOW_COLL);

    // Entry 2:
    key.flow_lkp_metadata_lkp_vrf = 3;
    key.flow_lkp_metadata_lkp_sport = 0x3b01;
    key.flow_lkp_metadata_lkp_dport = 0x3;
    data.actionid = 0;
    rs = fl.insert((void *)&key, (void *)&data, &flow_idx[1]);
    ASSERT_TRUE(rs == HAL_RET_OK || rs == HAL_RET_FLOW_COLL);

    // Remove Entry 1
    rs = fl.remove(flow_idx[0]);
    ASSERT_TRUE(rs == HAL_RET_OK);

    // Remove Entry 2
    rs = fl.remove(flow_idx[1]);
    ASSERT_TRUE(rs == HAL_RET_OK);
}

// ----------------------------------------------------------------------------
// Test 9:
//      - Create flow with collisions
// ----------------------------------------------------------------------------
TEST_F(flow_test, test9) {

    hal_ret_t rs;
    flow_hash_swkey key = {0};
    flow_hash_actiondata data = {0};
    uint32_t flow_idx[9] = { 0 };

    memset(&key, 0, sizeof(key));
    memset(&data, 0, sizeof(data));
    Flow fl = Flow("FlowTable", P4TBL_ID_FLOW_HASH, P4TBL_ID_FLOW_HASH_OVERFLOW, 
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
    data.actionid = 0;
    rs = fl.insert((void *)&key, (void *)&data, &flow_idx[0]);
    ASSERT_TRUE(rs == HAL_RET_OK || rs == HAL_RET_FLOW_COLL);

    // Entry 2:
    key.flow_lkp_metadata_lkp_vrf = 3;
    key.flow_lkp_metadata_lkp_sport = 0x3b01;
    key.flow_lkp_metadata_lkp_dport = 0x3;
    data.actionid = 0;
    rs = fl.insert((void *)&key, (void *)&data, &flow_idx[1]);
    ASSERT_TRUE(rs == HAL_RET_OK || rs == HAL_RET_FLOW_COLL);

    // Remove Entry 2
    rs = fl.remove(flow_idx[1]);
    ASSERT_TRUE(rs == HAL_RET_OK);

    // Remove Entry 1
    rs = fl.remove(flow_idx[0]);
    ASSERT_TRUE(rs == HAL_RET_OK);
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
TEST_F(flow_test, test10) {

    hal_ret_t rs;
    flow_hash_swkey key = {0};
    flow_hash_actiondata data = {0};
    uint32_t flow_idx[9] = { 0 };

    memset(&key, 0, sizeof(key));
    memset(&data, 0, sizeof(data));
    Flow fl = Flow("FlowTable", P4TBL_ID_FLOW_HASH, P4TBL_ID_FLOW_HASH_OVERFLOW, 
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
    data.actionid = 0;
    rs = fl.insert((void *)&key, (void *)&data, &flow_idx[0]);
    ASSERT_TRUE(rs == HAL_RET_OK || rs == HAL_RET_FLOW_COLL);

    // Entry 2:
    key.flow_lkp_metadata_lkp_vrf = 3;
    key.flow_lkp_metadata_lkp_sport = 0x3b01;
    key.flow_lkp_metadata_lkp_dport = 0x3;
    data.actionid = 0;
    rs = fl.insert((void *)&key, (void *)&data, &flow_idx[1]);
    ASSERT_TRUE(rs == HAL_RET_OK || rs == HAL_RET_FLOW_COLL);

    // Entry 3:
    key.flow_lkp_metadata_lkp_vrf = 3;
    key.flow_lkp_metadata_lkp_sport = 0xd238;
    key.flow_lkp_metadata_lkp_dport = 0x4;
    data.actionid = 0;
    rs = fl.insert((void *)&key, (void *)&data, &flow_idx[2]);
    ASSERT_TRUE(rs == HAL_RET_OK || rs == HAL_RET_FLOW_COLL);

    // Remove Entry 1
    rs = fl.remove(flow_idx[0]);
    ASSERT_TRUE(rs == HAL_RET_OK);

    // Remove Entry 1
    rs = fl.remove(flow_idx[1]);
    ASSERT_TRUE(rs == HAL_RET_OK);

    // Remove Entry 2
    rs = fl.remove(flow_idx[2]);
    ASSERT_TRUE(rs == HAL_RET_OK);
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
TEST_F(flow_test, test11) {

    hal_ret_t rs;
    flow_hash_swkey key = {0};
    flow_hash_actiondata data = {0};
    uint32_t flow_idx[9] = { 0 };

    memset(&key, 0, sizeof(key));
    memset(&data, 0, sizeof(data));
    Flow fl = Flow("FlowTable", P4TBL_ID_FLOW_HASH, P4TBL_ID_FLOW_HASH_OVERFLOW, 
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
    data.actionid = 0;
    rs = fl.insert((void *)&key, (void *)&data, &flow_idx[0]);
    ASSERT_TRUE(rs == HAL_RET_OK || rs == HAL_RET_FLOW_COLL);

    // Entry 2:
    key.flow_lkp_metadata_lkp_vrf = 3;
    key.flow_lkp_metadata_lkp_sport = 0x3b01;
    key.flow_lkp_metadata_lkp_dport = 0x3;
    data.actionid = 0;
    rs = fl.insert((void *)&key, (void *)&data, &flow_idx[1]);
    ASSERT_TRUE(rs == HAL_RET_OK || rs == HAL_RET_FLOW_COLL);

    // Entry 3:
    key.flow_lkp_metadata_lkp_vrf = 3;
    key.flow_lkp_metadata_lkp_sport = 0xd238;
    key.flow_lkp_metadata_lkp_dport = 0x4;
    data.actionid = 0;
    rs = fl.insert((void *)&key, (void *)&data, &flow_idx[2]);
    ASSERT_TRUE(rs == HAL_RET_OK || rs == HAL_RET_FLOW_COLL);

    // Remove Entry 1
    rs = fl.remove(flow_idx[0]);
    ASSERT_TRUE(rs == HAL_RET_OK);

    // Remove Entry 1
    rs = fl.remove(flow_idx[2]);
    ASSERT_TRUE(rs == HAL_RET_OK);

    // Remove Entry 2
    rs = fl.remove(flow_idx[1]);
    ASSERT_TRUE(rs == HAL_RET_OK);
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
TEST_F(flow_test, test12) {

    hal_ret_t rs;
    flow_hash_swkey key = {0};
    flow_hash_actiondata data = {0};
    uint32_t flow_idx[9] = { 0 };

    memset(&key, 0, sizeof(key));
    memset(&data, 0, sizeof(data));
    Flow fl = Flow("FlowTable", P4TBL_ID_FLOW_HASH, P4TBL_ID_FLOW_HASH_OVERFLOW, 
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
    data.actionid = 0;
    rs = fl.insert((void *)&key, (void *)&data, &flow_idx[0]);
    ASSERT_TRUE(rs == HAL_RET_OK || rs == HAL_RET_FLOW_COLL);

    // Entry 2:
    key.flow_lkp_metadata_lkp_vrf = 3;
    key.flow_lkp_metadata_lkp_sport = 0x3b01;
    key.flow_lkp_metadata_lkp_dport = 0x3;
    data.actionid = 0;
    rs = fl.insert((void *)&key, (void *)&data, &flow_idx[1]);
    ASSERT_TRUE(rs == HAL_RET_OK || rs == HAL_RET_FLOW_COLL);

    // Entry 3:
    key.flow_lkp_metadata_lkp_vrf = 3;
    key.flow_lkp_metadata_lkp_sport = 0xd238;
    key.flow_lkp_metadata_lkp_dport = 0x4;
    data.actionid = 0;
    rs = fl.insert((void *)&key, (void *)&data, &flow_idx[2]);
    ASSERT_TRUE(rs == HAL_RET_OK || rs == HAL_RET_FLOW_COLL);

    // Entry 4:
    key.flow_lkp_metadata_lkp_vrf = 3;
    key.flow_lkp_metadata_lkp_sport = 0x4b4e;
    key.flow_lkp_metadata_lkp_dport = 0x5;
    data.actionid = 0;
    rs = fl.insert((void *)&key, (void *)&data, &flow_idx[3]);
    ASSERT_TRUE(rs == HAL_RET_OK || rs == HAL_RET_FLOW_COLL);

    // Remove Entry 1
    rs = fl.remove(flow_idx[0]);
    ASSERT_TRUE(rs == HAL_RET_OK);

    // Remove Entry 2
    rs = fl.remove(flow_idx[1]);
    ASSERT_TRUE(rs == HAL_RET_OK);

    // Remove Entry 3
    rs = fl.remove(flow_idx[2]);
    ASSERT_TRUE(rs == HAL_RET_OK);

    // Remove Entry 4
    rs = fl.remove(flow_idx[3]);
    ASSERT_TRUE(rs == HAL_RET_OK);
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
TEST_F(flow_test, test13) {

    hal_ret_t rs;
    flow_hash_swkey key = {0};
    flow_hash_actiondata data = {0};
    uint32_t flow_idx[9] = { 0 };

    memset(&key, 0, sizeof(key));
    memset(&data, 0, sizeof(data));
    Flow fl = Flow("FlowTable", P4TBL_ID_FLOW_HASH, P4TBL_ID_FLOW_HASH_OVERFLOW, 
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
    data.actionid = 0;
    rs = fl.insert((void *)&key, (void *)&data, &flow_idx[0]);
    ASSERT_TRUE(rs == HAL_RET_OK || rs == HAL_RET_FLOW_COLL);

    // Entry 2:
    key.flow_lkp_metadata_lkp_vrf = 3;
    key.flow_lkp_metadata_lkp_sport = 0x3b01;
    key.flow_lkp_metadata_lkp_dport = 0x3;
    data.actionid = 0;
    rs = fl.insert((void *)&key, (void *)&data, &flow_idx[1]);
    ASSERT_TRUE(rs == HAL_RET_OK || rs == HAL_RET_FLOW_COLL);

    // Entry 3:
    key.flow_lkp_metadata_lkp_vrf = 3;
    key.flow_lkp_metadata_lkp_sport = 0xd238;
    key.flow_lkp_metadata_lkp_dport = 0x4;
    data.actionid = 0;
    rs = fl.insert((void *)&key, (void *)&data, &flow_idx[2]);
    ASSERT_TRUE(rs == HAL_RET_OK || rs == HAL_RET_FLOW_COLL);

    // Entry 4:
    key.flow_lkp_metadata_lkp_vrf = 3;
    key.flow_lkp_metadata_lkp_sport = 0x4b4e;
    key.flow_lkp_metadata_lkp_dport = 0x5;
    data.actionid = 0;
    rs = fl.insert((void *)&key, (void *)&data, &flow_idx[3]);
    ASSERT_TRUE(rs == HAL_RET_OK || rs == HAL_RET_FLOW_COLL);

    // Remove Entry 1
    rs = fl.remove(flow_idx[4]);
    ASSERT_TRUE(rs == HAL_RET_OK);

    // Remove Entry 2
    rs = fl.remove(flow_idx[3]);
    ASSERT_TRUE(rs == HAL_RET_OK);

    // Remove Entry 3
    rs = fl.remove(flow_idx[2]);
    ASSERT_TRUE(rs == HAL_RET_OK);

    // Remove Entry 4
    rs = fl.remove(flow_idx[1]);
    ASSERT_TRUE(rs == HAL_RET_OK);
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
TEST_F(flow_test, test14) {

    hal_ret_t rs;
    flow_hash_swkey key = {0};
    flow_hash_actiondata data = {0};
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
    Flow fl = Flow("FlowTable", P4TBL_ID_FLOW_HASH, P4TBL_ID_FLOW_HASH_OVERFLOW, 
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
    data.actionid = 0;
    rs = fl.insert((void *)&key, (void *)&data, &flow_idx[0]);
    ASSERT_TRUE(rs == HAL_RET_OK || rs == HAL_RET_FLOW_COLL);

    // Entry 2:
    key.flow_lkp_metadata_lkp_vrf = 3;
    key.flow_lkp_metadata_lkp_sport = 0x3b01;
    key.flow_lkp_metadata_lkp_dport = 0x3;
    data.actionid = 0;
    rs = fl.insert((void *)&key, (void *)&data, &flow_idx[1]);
    ASSERT_TRUE(rs == HAL_RET_OK || rs == HAL_RET_FLOW_COLL);

    // Entry 3:
    key.flow_lkp_metadata_lkp_vrf = 3;
    key.flow_lkp_metadata_lkp_sport = 0xd238;
    key.flow_lkp_metadata_lkp_dport = 0x4;
    data.actionid = 0;
    rs = fl.insert((void *)&key, (void *)&data, &flow_idx[2]);
    ASSERT_TRUE(rs == HAL_RET_OK || rs == HAL_RET_FLOW_COLL);

    // Entry 4:
    key.flow_lkp_metadata_lkp_vrf = 3;
    key.flow_lkp_metadata_lkp_sport = 0x4b4e;
    key.flow_lkp_metadata_lkp_dport = 0x5;
    data.actionid = 0;
    rs = fl.insert((void *)&key, (void *)&data, &flow_idx[3]);
    ASSERT_TRUE(rs == HAL_RET_OK || rs == HAL_RET_FLOW_COLL);

	for (std::vector<int>::iterator it=myvector.begin(); it!=myvector.end(); ++it) {
		HAL_TRACE_DEBUG("Removing {}", *it);
		rs = fl.remove(flow_idx[*it]);
		ASSERT_TRUE(rs == HAL_RET_OK);
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
TEST_F(flow_test, test15) {

    hal_ret_t rs;
    flow_hash_swkey key = {0};
    flow_hash_actiondata data = {0};
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
    Flow fl = Flow("FlowTable", P4TBL_ID_FLOW_HASH, P4TBL_ID_FLOW_HASH_OVERFLOW, 
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
    data.actionid = 0;
    rs = fl.insert((void *)&key, (void *)&data, &flow_idx[0]);
    ASSERT_TRUE(rs == HAL_RET_OK || rs == HAL_RET_FLOW_COLL);

    // Entry 2:
    key.flow_lkp_metadata_lkp_vrf = 3;
    key.flow_lkp_metadata_lkp_sport = 0x3b01;
    key.flow_lkp_metadata_lkp_dport = 0x3;
    data.actionid = 0;
    rs = fl.insert((void *)&key, (void *)&data, &flow_idx[1]);
    ASSERT_TRUE(rs == HAL_RET_OK || rs == HAL_RET_FLOW_COLL);

    // Entry 3:
    key.flow_lkp_metadata_lkp_vrf = 3;
    key.flow_lkp_metadata_lkp_sport = 0xd238;
    key.flow_lkp_metadata_lkp_dport = 0x4;
    data.actionid = 0;
    rs = fl.insert((void *)&key, (void *)&data, &flow_idx[2]);
    ASSERT_TRUE(rs == HAL_RET_OK || rs == HAL_RET_FLOW_COLL);

    // Entry 4:
    key.flow_lkp_metadata_lkp_vrf = 3;
    key.flow_lkp_metadata_lkp_sport = 0x4b4e;
    key.flow_lkp_metadata_lkp_dport = 0x5;
    data.actionid = 0;
    rs = fl.insert((void *)&key, (void *)&data, &flow_idx[3]);
    ASSERT_TRUE(rs == HAL_RET_OK || rs == HAL_RET_FLOW_COLL);

    // Entry 5:
    key.flow_lkp_metadata_lkp_vrf = 3;
    key.flow_lkp_metadata_lkp_sport = 0;
    key.flow_lkp_metadata_lkp_dport = 0x36ad;
    data.actionid = 0;
    rs = fl.insert((void *)&key, (void *)&data, &flow_idx[4]);
    ASSERT_TRUE(rs == HAL_RET_OK || rs == HAL_RET_FLOW_COLL);

	for (std::vector<int>::iterator it=myvector.begin(); it!=myvector.end(); ++it) {
		HAL_TRACE_DEBUG("Removing {}", *it);
		rs = fl.remove(flow_idx[*it]);
		ASSERT_TRUE(rs == HAL_RET_OK);
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
TEST_F(flow_test, test16) {

    hal_ret_t rs;
    flow_hash_swkey key = {0};
    flow_hash_actiondata data = {0};
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
    Flow fl = Flow("FlowTable", P4TBL_ID_FLOW_HASH, P4TBL_ID_FLOW_HASH_OVERFLOW, 
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
    data.actionid = 0;
    rs = fl.insert((void *)&key, (void *)&data, &flow_idx[0]);
    ASSERT_TRUE(rs == HAL_RET_OK || rs == HAL_RET_FLOW_COLL);

    // Entry 2:
    key.flow_lkp_metadata_lkp_vrf = 3;
    key.flow_lkp_metadata_lkp_sport = 0x3b01;
    key.flow_lkp_metadata_lkp_dport = 0x3;
    data.actionid = 0;
    rs = fl.insert((void *)&key, (void *)&data, &flow_idx[1]);
    ASSERT_TRUE(rs == HAL_RET_OK || rs == HAL_RET_FLOW_COLL);

    // Entry 3:
    key.flow_lkp_metadata_lkp_vrf = 3;
    key.flow_lkp_metadata_lkp_sport = 0xd238;
    key.flow_lkp_metadata_lkp_dport = 0x4;
    data.actionid = 0;
    rs = fl.insert((void *)&key, (void *)&data, &flow_idx[2]);
    ASSERT_TRUE(rs == HAL_RET_OK || rs == HAL_RET_FLOW_COLL);

    // Entry 4:
    key.flow_lkp_metadata_lkp_vrf = 3;
    key.flow_lkp_metadata_lkp_sport = 0x4b4e;
    key.flow_lkp_metadata_lkp_dport = 0x5;
    data.actionid = 0;
    rs = fl.insert((void *)&key, (void *)&data, &flow_idx[3]);
    ASSERT_TRUE(rs == HAL_RET_OK || rs == HAL_RET_FLOW_COLL);

    // Entry 5:
    key.flow_lkp_metadata_lkp_vrf = 3;
    key.flow_lkp_metadata_lkp_sport = 0;
    key.flow_lkp_metadata_lkp_dport = 0x36ad;
    data.actionid = 0;
    rs = fl.insert((void *)&key, (void *)&data, &flow_idx[4]);
    ASSERT_TRUE(rs == HAL_RET_OK || rs == HAL_RET_FLOW_COLL);

    // Entry 6:
    key.flow_lkp_metadata_lkp_vrf = 3;
    key.flow_lkp_metadata_lkp_sport = 3;
    key.flow_lkp_metadata_lkp_dport = 0xd125;
    data.actionid = 0;
    rs = fl.insert((void *)&key, (void *)&data, &flow_idx[5]);
    ASSERT_TRUE(rs == HAL_RET_OK || rs == HAL_RET_FLOW_COLL);

    // Entry 7:
    key.flow_lkp_metadata_lkp_vrf = 0;
    key.flow_lkp_metadata_lkp_sport = 0;
    key.flow_lkp_metadata_lkp_dport = 0xc430;
    data.actionid = 0;
    rs = fl.insert((void *)&key, (void *)&data, &flow_idx[6]);
    ASSERT_TRUE(rs == HAL_RET_OK || rs == HAL_RET_FLOW_COLL);

    // Entry 8:
    key.flow_lkp_metadata_lkp_vrf = 1;
    key.flow_lkp_metadata_lkp_sport = 1;
    key.flow_lkp_metadata_lkp_dport = 0xc8c3;
    data.actionid = 0;
    rs = fl.insert((void *)&key, (void *)&data, &flow_idx[7]);
    ASSERT_TRUE(rs == HAL_RET_OK || rs == HAL_RET_FLOW_COLL);

	for (std::vector<int>::iterator it=myvector.begin(); it!=myvector.end(); ++it) {
		HAL_TRACE_DEBUG("Removing {}", *it);
		rs = fl.remove(flow_idx[*it]);
		ASSERT_TRUE(rs == HAL_RET_OK);
	}
}
// ----------------------------------------------------------------------------
// Test 17:
//      - Create flow with collisions
//      - Insert 1 - 9
//      - Randomly delete
// ----------------------------------------------------------------------------
TEST_F(flow_test, test17) {

    hal_ret_t rs;
    flow_hash_swkey key = {0};
    flow_hash_actiondata data = {0};
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
    Flow fl = Flow("FlowTable", P4TBL_ID_FLOW_HASH, P4TBL_ID_FLOW_HASH_OVERFLOW, 
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
    data.actionid = 0;
    rs = fl.insert((void *)&key, (void *)&data, &flow_idx[0]);
    ASSERT_TRUE(rs == HAL_RET_OK || rs == HAL_RET_FLOW_COLL);

    // Entry 2:
    key.flow_lkp_metadata_lkp_vrf = 3;
    key.flow_lkp_metadata_lkp_sport = 0x3b01;
    key.flow_lkp_metadata_lkp_dport = 0x3;
    data.actionid = 0;
    rs = fl.insert((void *)&key, (void *)&data, &flow_idx[1]);
    ASSERT_TRUE(rs == HAL_RET_OK || rs == HAL_RET_FLOW_COLL);

    // Entry 3:
    key.flow_lkp_metadata_lkp_vrf = 3;
    key.flow_lkp_metadata_lkp_sport = 0xd238;
    key.flow_lkp_metadata_lkp_dport = 0x4;
    data.actionid = 0;
    rs = fl.insert((void *)&key, (void *)&data, &flow_idx[2]);
    ASSERT_TRUE(rs == HAL_RET_OK || rs == HAL_RET_FLOW_COLL);

    // Entry 4:
    key.flow_lkp_metadata_lkp_vrf = 3;
    key.flow_lkp_metadata_lkp_sport = 0x4b4e;
    key.flow_lkp_metadata_lkp_dport = 0x5;
    data.actionid = 0;
    rs = fl.insert((void *)&key, (void *)&data, &flow_idx[3]);
    ASSERT_TRUE(rs == HAL_RET_OK || rs == HAL_RET_FLOW_COLL);

    // Entry 5:
    key.flow_lkp_metadata_lkp_vrf = 3;
    key.flow_lkp_metadata_lkp_sport = 0;
    key.flow_lkp_metadata_lkp_dport = 0x36ad;
    data.actionid = 0;
    rs = fl.insert((void *)&key, (void *)&data, &flow_idx[4]);
    ASSERT_TRUE(rs == HAL_RET_OK || rs == HAL_RET_FLOW_COLL);

    // Entry 6:
    key.flow_lkp_metadata_lkp_vrf = 3;
    key.flow_lkp_metadata_lkp_sport = 3;
    key.flow_lkp_metadata_lkp_dport = 0xd125;
    data.actionid = 0;
    rs = fl.insert((void *)&key, (void *)&data, &flow_idx[5]);
    ASSERT_TRUE(rs == HAL_RET_OK || rs == HAL_RET_FLOW_COLL);

    // Entry 7:
    key.flow_lkp_metadata_lkp_vrf = 0;
    key.flow_lkp_metadata_lkp_sport = 0;
    key.flow_lkp_metadata_lkp_dport = 0xc430;
    data.actionid = 0;
    rs = fl.insert((void *)&key, (void *)&data, &flow_idx[6]);
    ASSERT_TRUE(rs == HAL_RET_OK || rs == HAL_RET_FLOW_COLL);

    // Entry 8:
    key.flow_lkp_metadata_lkp_vrf = 1;
    key.flow_lkp_metadata_lkp_sport = 1;
    key.flow_lkp_metadata_lkp_dport = 0xc8c3;
    data.actionid = 0;
    rs = fl.insert((void *)&key, (void *)&data, &flow_idx[7]);
    ASSERT_TRUE(rs == HAL_RET_OK || rs == HAL_RET_FLOW_COLL);

    // Entry 9:
    key.flow_lkp_metadata_lkp_vrf = 1;
    key.flow_lkp_metadata_lkp_sport = 2;
    key.flow_lkp_metadata_lkp_dport = 0x2f4b;
    data.actionid = 0;
    rs = fl.insert((void *)&key, (void *)&data, &flow_idx[8]);
    ASSERT_TRUE(rs == HAL_RET_OK || rs == HAL_RET_FLOW_COLL);


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
    rs = fl.insert((void *)&key, (void *)&data, &flow_idx[9]);
    ASSERT_TRUE(rs == HAL_RET_OK || rs == HAL_RET_FLOW_COLL);

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
    rs = fl.insert((void *)&key, (void *)&data, &flow_idx[10]);
    ASSERT_TRUE(rs == HAL_RET_OK || rs == HAL_RET_FLOW_COLL);

	for (std::vector<int>::iterator it=myvector.begin(); it!=myvector.end(); ++it) {
		HAL_TRACE_DEBUG("Removing {}", *it);
		rs = fl.remove(flow_idx[*it]);
		ASSERT_TRUE(rs == HAL_RET_OK);
	}

    HAL_TRACE_DEBUG("tableid:{}, table_name:{}, capacity:{}, oflow_capac:{}, "
                    "num_in_use:{}, oflow_num_in_use:{}"
                    "num_inserts:{}, num_insert_errors:{}, "
                    "num_deletes:{}, num_delete_errors:{}",
                    fl.table_id(), fl.table_name(),
                    fl.table_capacity(), fl.oflow_table_capacity(),
                    fl.table_num_entries_in_use(), fl.oflow_table_num_entries_in_use(),
                    fl.table_num_inserts(), fl.table_num_insert_errors(),
                    fl.table_num_deletes(), fl.table_num_delete_errors());
}

// ----------------------------------------------------------------------------
// Test 18:
//      - Create flow table
// ----------------------------------------------------------------------------
TEST_F(flow_test, DISABLED_test18) {

    hal_ret_t rs;
    flow_hash_swkey key = {0};
    flow_hash_actiondata data = {0};
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
    Flow fl = Flow("FlowTable", P4TBL_ID_FLOW_HASH, P4TBL_ID_FLOW_HASH_OVERFLOW, 
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

        ft_bits = fl.calc_hash_(&key, &data);
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
            rs = fl.insert((void *)&key, (void *)&data, &flow_idx[count]);
            ASSERT_TRUE(rs == HAL_RET_OK || rs == HAL_RET_FLOW_COLL);
            count++;
            if (count == max) {
                break;
            }
        }
    }
    // Remvoe the entries
	for (std::vector<int>::iterator it=myvector.begin(); it!=myvector.end(); ++it) {
		HAL_TRACE_DEBUG("Removing {}", *it);
		rs = fl.remove(flow_idx[*it]);
		ASSERT_TRUE(rs == HAL_RET_OK);
	}

    HAL_TRACE_DEBUG("tableid:{}, table_name:{}, capacity:{}, oflow_capac:{}, "
                    "num_in_use:{}, oflow_num_in_use:{}"
                    "num_inserts:{}, num_insert_errors:{}, "
                    "num_deletes:{}, num_delete_errors:{}",
                    fl.table_id(), fl.table_name(),
                    fl.table_capacity(), fl.oflow_table_capacity(),
                    fl.table_num_entries_in_use(), fl.oflow_table_num_entries_in_use(),
                    fl.table_num_inserts(), fl.table_num_insert_errors(),
                    fl.table_num_deletes(), fl.table_num_delete_errors());
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
    p4pd_init();
    // testing::GTEST_FLAG(filter) = "-*test18*";
    int res = RUN_ALL_TESTS();
    p4pd_cleanup();
    return res;
}
