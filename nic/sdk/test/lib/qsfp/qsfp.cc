//------------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//
// Test Cases to verify qsfp in the pal library
//------------------------------------------------------------------------------

#include <gtest/gtest.h>
#include "nic/sdk/lib/pal/pal.hpp"
#include <inttypes.h>

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

static void
test_qsfp (int port)
{
    uint8_t qsfp_buffer[QSFP_READ_SIZE];
    sdk::lib::qsfp_page_t lowpage = sdk::lib::QSFP_PAGE_LOW;
    uint32_t readoffset = 0;
    int i;

    memset(qsfp_buffer, 0, QSFP_READ_SIZE);
    
    printf("Enter a offset to read:");
    scanf("%" SCNx8, &readoffset);
    printf("offset to read: 0x%x\n", readoffset);

    printf("Enter a page to read:");
    scanf("%" SCNx32, &lowpage);
    printf("page to read: %d\n", lowpage);

    ASSERT_EQ(sdk::lib::pal_qsfp_read(
        qsfp_buffer, QSFP_READ_SIZE, readoffset, lowpage, MAX_QSFP_RETRIES, port), 0);

    printf("\nAfter read\n");
    for (i = 0; i < QSFP_READ_SIZE; i++)
        printf("%x", qsfp_buffer[i]);
    
    printf("\nreading done for port: %d\n", port);
}

static void
test_qsfp_write (int port)
{
    uint8_t qsfp_buffer[QSFP_READ_SIZE];
    sdk::lib::qsfp_page_t lowpage = sdk::lib::QSFP_PAGE_LOW;
    uint32_t writeoffset = 0;
    uint8_t qsfp_wbuffer;
    int i;

    memset(qsfp_buffer, 0, QSFP_READ_SIZE);

    printf("Enter a offset to write:");
    scanf("%" SCNx8, &writeoffset);
    printf("offset to write: 0x%x\n", writeoffset);

    printf("Enter a page to read:");
    scanf("%" SCNx32, &lowpage);
    printf("page to read: %d\n", lowpage);

    ASSERT_EQ(sdk::lib::pal_qsfp_read(
        qsfp_buffer, QSFP_READ_SIZE, writeoffset, lowpage, MAX_QSFP_RETRIES, port), 0);

    printf("\nAfter read\n");
    for (i = 0; i < QSFP_READ_SIZE; i++)
        printf("%x", qsfp_buffer[i]);

    printf("Enter a value to write:");
    scanf("%" SCNx8, &qsfp_wbuffer);
    printf("value to write : %x\n", qsfp_wbuffer);

    ASSERT_EQ(sdk::lib::pal_qsfp_write(&qsfp_wbuffer, 1, writeoffset, lowpage, MAX_QSFP_RETRIES, port), 0);
    printf("writing done\n");

    memset(qsfp_buffer, 0, QSFP_READ_SIZE);
    ASSERT_EQ(sdk::lib::pal_qsfp_read(
        qsfp_buffer, QSFP_READ_SIZE, writeoffset, lowpage, MAX_QSFP_RETRIES, port), 0);

    printf("\nAfter read\n");
    for (i = 0; i < QSFP_READ_SIZE; i++)
        printf("%x", qsfp_buffer[i]);

    printf("\nwriting done for port: %d\n", port);
}

// test qsfp_write
TEST_F(qsfp_test, pal_hw_qsfp_write) {
    // initialize the pal
#ifdef __x86_64__
    assert(sdk::lib::pal_init(platform_type_t::PLATFORM_TYPE_SIM) == sdk::lib::PAL_RET_OK);
#elif __aarch64__
    assert(sdk::lib::pal_init(platform_type_t::PLATFORM_TYPE_HAPS) == sdk::lib::PAL_RET_OK);
#endif

    test_qsfp_write (1);
    test_qsfp_write (2);
}

// test qsfp_read
TEST_F(qsfp_test, pal_hw_qsfp_read) {
    // initialize the pal
#ifdef __x86_64__
    assert(sdk::lib::pal_init(platform_type_t::PLATFORM_TYPE_SIM) == sdk::lib::PAL_RET_OK);
#elif __aarch64__
    assert(sdk::lib::pal_init(platform_type_t::PLATFORM_TYPE_HAPS) == sdk::lib::PAL_RET_OK);
#endif
    test_qsfp (1);
    test_qsfp (2);
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
