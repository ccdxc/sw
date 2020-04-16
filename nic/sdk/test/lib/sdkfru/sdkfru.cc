//------------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//
// Test Cases to verify fru library
//------------------------------------------------------------------------------

#include <gtest/gtest.h>
#include "platform/fru/fru.hpp"

using namespace std;

class sdkfru_test : public ::testing::Test {
protected:
  sdkfru_test() {
  }

  virtual ~sdkfru_test() {
  }

  // will be called immediately after the constructor before each test
  virtual void SetUp() {
  }

  // will be called immediately after each test before the destructor
  virtual void TearDown() {
  }

};

TEST_F(sdkfru_test, read_key) {
    string key;
    string value = "";

    ASSERT_EQ(sdk::platform::readfrukey(BOARD_MANUFACTURERDATE_KEY,value), 0);
    cout << BOARD_MANUFACTURERDATE_KEY << " is " << value << endl;
    value = "";

    ASSERT_EQ(sdk::platform::readfrukey(BOARD_MANUFACTURER_KEY,value), 0);
    cout << BOARD_MANUFACTURER_KEY << " is " << value << endl;
    value = "";

    ASSERT_EQ(sdk::platform::readfrukey(BOARD_PRODUCTNAME_KEY,value), 0);
    cout << BOARD_PRODUCTNAME_KEY << " is " << value << endl;
    value = "";

    ASSERT_EQ(sdk::platform::readfrukey(BOARD_SERIALNUMBER_KEY,value), 0);
    cout << BOARD_SERIALNUMBER_KEY << " is " << value << endl;
    value = "";

    ASSERT_EQ(sdk::platform::readfrukey(BOARD_PARTNUM_KEY,value), 0);
    cout << BOARD_PARTNUM_KEY << " is " << value << endl;
    value = "";

    ASSERT_EQ(sdk::platform::readfrukey(BOARD_ID_KEY,value), 0);
    cout << BOARD_ID_KEY << " is " << value << endl;
    value = "";

    ASSERT_EQ(sdk::platform::readfrukey(BOARD_ENGCHANGELEVEL_KEY,value), 0);
    cout << BOARD_ENGCHANGELEVEL_KEY << " is " << value << endl;
    value = "";

    ASSERT_EQ(sdk::platform::readfrukey(BOARD_MACADDRESS_KEY,value), 0);
    cout << BOARD_MACADDRESS_KEY << " is " << value << endl;
    value = "";

    ASSERT_EQ(sdk::platform::readfrukey(BOARD_NUMMACADDR_KEY,value), 0);
    cout << BOARD_NUMMACADDR_KEY << " is " << value << endl;
    value = "";

    ASSERT_EQ(sdk::platform::readfrukey("UKNOWN",value), -1);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
