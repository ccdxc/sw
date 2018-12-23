//------------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//
// Test Cases to verify qsfp in the pal library
//------------------------------------------------------------------------------

#include <gtest/gtest.h>
#include "nic/sdk/include/sdk/pal.hpp"

using namespace std;
#define QSFP_READ_SIZE  10

class qsfp_test : public ::testing::Test {
protected:
  qsfp_test() {
  }

  virtual ~qsfp_test() {
  }

  // will be called immediately after the constructor before each test
  virtual void SetUp() {
  }

  // will be called immediately after each test before the destructor
  virtual void TearDown() {
  }

};

// test qsfp_write
TEST_F(qsfp_test, pal_hw_qsfp_write) {
    uint8_t *qsfp_buffer = NULL;
    uint8_t qsfp_wbuffer;
    sdk::lib::qsfp_page_t lowpage = sdk::lib::QSFP_PAGE_LOW;
    uint8_t i;
    // initialize the pal
#ifdef __x86_64__
    assert(sdk::lib::pal_init(sdk::types::platform_type_t::PLATFORM_TYPE_SIM) == sdk::lib::PAL_RET_OK);
#elif __aarch64__
    assert(sdk::lib::pal_init(sdk::types::platform_type_t::PLATFORM_TYPE_HAPS) == sdk::lib::PAL_RET_OK);
#endif

    qsfp_buffer = (uint8_t*)malloc(QSFP_READ_SIZE);
    if (qsfp_buffer == NULL) {
        printf("failed to allocate memory.\n");
    }
    memset(qsfp_buffer, 0, QSFP_READ_SIZE);
    ASSERT_EQ(sdk::lib::pal_qsfp_read(qsfp_buffer, QSFP_READ_SIZE, 0x54, lowpage, MAX_QSFP_RETRIES, 2), 0);
    printf("after read\n");
    for (i = 0; i < QSFP_READ_SIZE; i++)
        printf("%x", qsfp_buffer[i]);
    printf("\nreading done\n");

    printf("Enter a value to write:");
    scanf("%c",&qsfp_wbuffer);
    ASSERT_EQ(sdk::lib::pal_qsfp_write(&qsfp_wbuffer, 1, 0x56, lowpage, MAX_QSFP_RETRIES, 2), 0);
    printf("writing done\n");

    memset(qsfp_buffer, 0, QSFP_READ_SIZE);
    ASSERT_EQ(sdk::lib::pal_qsfp_read(qsfp_buffer, QSFP_READ_SIZE, 0x54, lowpage, MAX_QSFP_RETRIES, 2), 0);
    printf("after read\n");
    for (i = 0; i < QSFP_READ_SIZE; i++)
        printf("%x", qsfp_buffer[i]);
    printf("\nreading done\n");



    qsfp_buffer = (uint8_t*)malloc(QSFP_READ_SIZE);
    if (qsfp_buffer == NULL) {
        printf("failed to allocate memory.\n");
    }
    memset(qsfp_buffer, 0, QSFP_READ_SIZE);
    ASSERT_EQ(sdk::lib::pal_qsfp_read(qsfp_buffer, QSFP_READ_SIZE, 0x54, lowpage, MAX_QSFP_RETRIES, 1), 0);
    printf("after read\n");
    for (i = 0; i < QSFP_READ_SIZE; i++)
        printf("%x", qsfp_buffer[i]);
    printf("\nreading done\n");

    ASSERT_EQ(sdk::lib::pal_qsfp_write(&qsfp_wbuffer, 1, 0x56, lowpage, MAX_QSFP_RETRIES, 1), 0);
    printf("writing done\n");

    memset(qsfp_buffer, 0, QSFP_READ_SIZE);
    ASSERT_EQ(sdk::lib::pal_qsfp_read(qsfp_buffer, QSFP_READ_SIZE, 0x54, lowpage, MAX_QSFP_RETRIES, 1), 0);
    printf("after read\n");
    for (i = 0; i < QSFP_READ_SIZE; i++)
        printf("%x", qsfp_buffer[i]);
    printf("\nreading done\n");

}

// test qsfp_read
TEST_F(qsfp_test, pal_hw_qsfp_read) {
    uint8_t *qsfp_buffer = NULL;
    sdk::lib::qsfp_page_t lowpage = sdk::lib::QSFP_PAGE_LOW;
    uint8_t i;
    // initialize the pal
#ifdef __x86_64__
    assert(sdk::lib::pal_init(sdk::types::platform_type_t::PLATFORM_TYPE_SIM) == sdk::lib::PAL_RET_OK);
#elif __aarch64__
    assert(sdk::lib::pal_init(sdk::types::platform_type_t::PLATFORM_TYPE_HAPS) == sdk::lib::PAL_RET_OK);
#endif
    qsfp_buffer = (uint8_t*)malloc(QSFP_READ_SIZE);
    if (qsfp_buffer == NULL) {
        printf("failed to allocate memory.\n");
    }
    memset(qsfp_buffer, 0, QSFP_READ_SIZE);
    ASSERT_EQ(sdk::lib::pal_qsfp_read(qsfp_buffer, QSFP_READ_SIZE, 0x54, lowpage, MAX_QSFP_RETRIES, 2), 0);
    printf("after read\n");		
    for (i = 0; i < QSFP_READ_SIZE; i++)		
        printf("%x", qsfp_buffer[i]);		
    printf("\nreading done\n");
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
