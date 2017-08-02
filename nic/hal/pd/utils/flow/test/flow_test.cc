#include "flow.hpp"
#include <gtest/gtest.h>
#include <stdio.h>
#include <p4pd_api.hpp>
#include <p4pd.h>

using hal::pd::utils::Flow;

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

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
    p4pd_init();
  return RUN_ALL_TESTS();
}
