#include "flow.hpp"
#include <gtest/gtest.h>
#include <stdio.h>
#include <p4pd_api.hpp>
#include <p4pd.h>
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

static bool
get_key (uint512_t x, flow_hash_swkey* key)
{
    key->flow_lkp_metadata_lkp_dir = 1;

    // ipv4
    key->flow_lkp_metadata_lkp_type = 1;

    key->flow_lkp_metadata_lkp_vrf = 3;

    uint128_t sip = 0x40020001;
    uint128_t dip = 0x40020002;

    for (int i = 0; i < 16; i++) {
        key->flow_lkp_metadata_lkp_src[i] = sip.convert_to<uint8_t>() & 0xff;
        sip >>= 8;
    }
    for (int i = 0; i < 16; i++) {
        key->flow_lkp_metadata_lkp_dst[i] = dip.convert_to<uint8_t>() & 0xff;
        dip >>= 8;
    }

    key->flow_lkp_metadata_lkp_sport = x.convert_to<uint16_t>() & 0xffff;
    if (!key->flow_lkp_metadata_lkp_sport) {
        return false;
    }
    x >>= 16;

    key->flow_lkp_metadata_lkp_dport = x.convert_to<uint16_t>() & 0xffff;
    if (!key->flow_lkp_metadata_lkp_dport) {
        return false;
    }
    x >>= 16;

    // TCP
    key->flow_lkp_metadata_lkp_proto = 6;

    return true;

#if 0
    key->flow_lkp_metadata_lkp_dir = x.convert_to<uint8_t>() & 0x1;
    x >>= 1;
    for (int i = 0; i < 16; i++) {
        key->flow_lkp_metadata_lkp_src[i] = x.convert_to<uint8_t>() & 0xff;
        x >>= 8;
    }
    for (int i = 0; i < 16; i++) {
        key->flow_lkp_metadata_lkp_dst[i] = x.convert_to<uint8_t>() & 0xff;
        x >>= 8;
    }

    key->flow_lkp_metadata_lkp_sport = x.convert_to<uint16_t>() & 0xffff;
    x >>= 16;

    key->flow_lkp_metadata_lkp_dport = x.convert_to<uint16_t>() & 0xffff;
    x >>= 16;

    key->flow_lkp_metadata_lkp_vrf = x.convert_to<uint16_t>() & 0xffff;
    x >>= 16;

    key->flow_lkp_metadata_lkp_type = x.convert_to<uint8_t>() & 0x3;
    x >>= 2;

    key->flow_lkp_metadata_lkp_proto = x.convert_to<uint8_t>() & 0xff;
#endif
}

// ----------------------------------------------------------------------------
// Test to generate hash collissions:
// ----------------------------------------------------------------------------
TEST_F(flow_test, gen_hash_collision) {

    flow_hash_swkey key;
    flow_hash_actiondata data;

    memset(&data, 0, sizeof(data));

    Flow fl = Flow("FlowTable", P4TBL_ID_FLOW_HASH, P4TBL_ID_FLOW_HASH_OVERFLOW,
                   2097152, 16384, sizeof(key), sizeof(data));


    uint32_t hash_val;
    std::map<uint32_t, uint512_t> hm;
    std::map<uint32_t, uint512_t>::iterator itr;


    uint512_t i;
    for (i = 0; i < (1ull<<32); i++) {
        if (!get_key(i, &key)) {
            continue;
        }

        hash_val = fl.calc_hash_((void *)&key, (void *)&data);
        itr = hm.find(hash_val);
        if (itr != hm.end()) {
            std::cout << "HASH collission for hash_val " << hash_val;
            std::cout << std::endl;
            std::cout << "flow1 key " << i;
            std::cout << std::endl;
//            std::cout << key;
            std::cout << std::endl;
            std::cout << "flow2 key " << itr->second;
//            get_key(itr->second, &key);
            std::cout << std::endl;
//            std::cout << key;
            std::cout << std::endl;
            std::cout << std::endl;
            break;
        }
        hm[hash_val] = i;

        if (!(i%10000)) {
            std::cout << "Running " << i << std::endl;
        }
    }

}


int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
    p4pd_init();
  return RUN_ALL_TESTS();
}
