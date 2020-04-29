
//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This file contains the all dnat test cases for athena
///
//----------------------------------------------------------------------------
#include "nic/sdk/include/sdk/base.hpp"
#include "nic/apollo/api/include/athena/pds_dnat.h"
#include "nic/apollo/core/trace.hpp"
#include "nic/apollo/test/api/utils/base.hpp"
#include "nic/apollo/test/athena/api/include/trace.hpp"
#include "ftl_p4pd_mock.hpp"

extern "C" {
// Function prototypes
sdk_ret_t pds_dnat_map_create(void);
sdk_ret_t pds_dnat_map_delete(void);
void pds_dnat_map_set_core_id(uint32_t core_id);
}

sdk_logger::trace_cb_t g_trace_cb;

namespace test {
namespace api {

uint32_t v4_addr_1 = 0x14000000;
uint32_t v4_addr_2 = 0x1E000000;
uint8_t v6_addr_1[IP6_ADDR8_LEN] = { 0x88, 0x99, 0x77, 0, 0, 0, 0, 0,
                                     0, 0, 0, 0, 0, 0, 0, 0 }; 
uint8_t v6_addr_2[IP6_ADDR8_LEN] = { 0xFF, 0xEE, 0xDD, 0, 0, 0, 0, 0,
                                     0, 0, 0, 0, 0, 0xCC, 0xBB, 0xA0 };

//----------------------------------------------------------------------------
// DNAT test class
//----------------------------------------------------------------------------

class dnat_test : public pds_test_base {
protected:
    dnat_test() {}
    virtual ~dnat_test() {}
    
    virtual void SetUp() {
        SDK_TRACE_INFO("============== SETUP : %s.%s ===============",
                       ::testing::UnitTest::GetInstance()->current_test_info()->test_case_name(),
                       ::testing::UnitTest::GetInstance()->current_test_info()->name());
        ftl_mock_init();
        pds_dnat_map_create();
        pds_dnat_map_set_core_id(2);
    }
    virtual void TearDown() {
        SDK_TRACE_INFO("============== TEARDOWN : %s.%s ===============",
                       ::testing::UnitTest::GetInstance()->current_test_info()->test_case_name(),
                       ::testing::UnitTest::GetInstance()->current_test_info()->name());
        pds_dnat_map_delete();
        ftl_mock_cleanup();
    }
    static void SetUpTestCase() {}
    static void TearDownTestCase() {}
};

//----------------------------------------------------------------------------
// DNAT test cases implementation
//----------------------------------------------------------------------------

/// \defgroup DNAT Dnat tests
/// @{

/// \brief Dnat tests
TEST_F(dnat_test, dnat_crud) {
    pds_dnat_mapping_spec_t spec = { 0 };
    pds_dnat_mapping_key_t key = { 0 };
    pds_dnat_mapping_info_t info = { 0 };

    memcpy(spec.key.addr, &v4_addr_1, IP4_ADDR8_LEN);
    spec.key.key_type = IP_AF_IPV4;
    spec.key.vnic_id = 2;
    memcpy(spec.data.addr, v6_addr_1, IP6_ADDR8_LEN);
    spec.data.addr_type = IP_AF_IPV6;
    SDK_ASSERT(pds_dnat_map_entry_create(&spec) == PDS_RET_OK);

    memcpy(key.addr, &v4_addr_1, IP4_ADDR8_LEN);
    key.key_type = IP_AF_IPV4;
    key.vnic_id = 2;
    SDK_ASSERT(pds_dnat_map_entry_read(&key, &info) == PDS_RET_OK);
    SDK_ASSERT(info.spec.data.addr_type == IP_AF_IPV6);
    SDK_ASSERT(memcmp(info.spec.data.addr, v6_addr_1, IP6_ADDR8_LEN) == 0);

    memset(&spec, 0, sizeof(spec)); 
    memcpy(spec.key.addr, v6_addr_2, IP6_ADDR8_LEN);
    spec.key.key_type = IP_AF_IPV6;
    spec.key.vnic_id = 12;
    memcpy(spec.data.addr, &v4_addr_2, IP4_ADDR8_LEN);
    spec.data.addr_type = IP_AF_IPV4;
    SDK_ASSERT(pds_dnat_map_entry_create(&spec) == PDS_RET_OK);

    memset(&key, 0, sizeof(key));
    memset(&info, 0, sizeof(info));
    memcpy(key.addr, v6_addr_2, IP6_ADDR8_LEN);
    key.key_type = IP_AF_IPV6;
    key.vnic_id = 12;
    SDK_ASSERT(pds_dnat_map_entry_read(&key, &info) == PDS_RET_OK);
    SDK_ASSERT(info.spec.data.addr_type == IP_AF_IPV4);
    SDK_ASSERT(memcmp(info.spec.data.addr, &v4_addr_2, IP4_ADDR8_LEN) == 0);

    SDK_ASSERT(pds_dnat_map_entry_create(NULL) == 
            PDS_RET_INVALID_ARG);

    memset(&spec, 0, sizeof(spec)); 
    memcpy(spec.key.addr, v6_addr_2, IP6_ADDR8_LEN);
    spec.key.key_type = IP_AF_NIL;
    spec.key.vnic_id = 413;
    memcpy(spec.data.addr, &v4_addr_2, IP4_ADDR8_LEN);
    SDK_ASSERT(pds_dnat_map_entry_create(&spec) == 
            PDS_RET_INVALID_ARG);

    memset(&spec, 0, sizeof(spec)); 
    memcpy(spec.key.addr, v6_addr_1, IP6_ADDR8_LEN);
    spec.key.key_type = IP_AF_IPV6;
    spec.key.vnic_id = 516;
    memcpy(spec.data.addr, &v4_addr_2, IP4_ADDR8_LEN);
    spec.data.addr_type = IP_AF_IPV4;
    SDK_ASSERT(pds_dnat_map_entry_create(&spec) == 
            PDS_RET_INVALID_ARG);

    memset(&spec, 0, sizeof(spec)); 
    memcpy(spec.key.addr, &v4_addr_1, IP4_ADDR8_LEN);
    spec.key.key_type = IP_AF_IPV4;
    spec.key.vnic_id = 0;
    memcpy(spec.data.addr, &v4_addr_2, IP4_ADDR8_LEN);
    spec.data.addr_type = IP_AF_IPV4;
    SDK_ASSERT(pds_dnat_map_entry_create(&spec) == 
            PDS_RET_INVALID_ARG);

    memset(&key, 0, sizeof(key)); 
    memcpy(key.addr, &v4_addr_1, IP4_ADDR8_LEN);
    key.key_type = IP_AF_IPV4;
    key.vnic_id = 2;
    SDK_ASSERT(pds_dnat_map_entry_delete(&key) == PDS_RET_OK);

    memset(&info, 0, sizeof(info));
    SDK_ASSERT(pds_dnat_map_entry_read(&key, &info) == 
            PDS_RET_ENTRY_NOT_FOUND);

    memset(&spec, 0, sizeof(spec)); 
    memcpy(spec.key.addr, v6_addr_2, IP6_ADDR8_LEN);
    spec.key.key_type = IP_AF_IPV6;
    spec.key.vnic_id = 12;
    memcpy(spec.data.addr, &v6_addr_1, IP6_ADDR8_LEN);
    spec.data.addr_type = IP_AF_IPV6;
    SDK_ASSERT(pds_dnat_map_entry_update(&spec) == PDS_RET_OK);

    memset(&key, 0, sizeof(key)); 
    memset(&info, 0, sizeof(info));
    memcpy(key.addr, v6_addr_2, IP6_ADDR8_LEN);
    key.key_type = IP_AF_IPV6;
    key.vnic_id = 12;
    SDK_ASSERT(pds_dnat_map_entry_read(&key, &info) == PDS_RET_OK);
    SDK_ASSERT(info.spec.data.addr_type == IP_AF_IPV6);
    SDK_ASSERT(memcmp(info.spec.data.addr, &v6_addr_1, IP6_ADDR8_LEN) == 0);

    SDK_ASSERT(pds_dnat_map_entry_create(NULL) == PDS_RET_INVALID_ARG);
}

/// @}

}    // namespace api
}    // namespace test

/// @private
int
main (int argc, char **argv)
{
    register_trace_cb(sdk_test_logger);
    return api_test_program_run(argc, argv);
}
