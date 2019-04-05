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

    ASSERT_EQ(sdk::platform::readFruKey(MANUFACTURERDATE_KEY,value), 0);
    cout << MANUFACTURERDATE_KEY << " is " << value << endl;
    value = "";

    ASSERT_EQ(sdk::platform::readFruKey(MANUFACTURER_KEY,value), 0);
    cout << MANUFACTURER_KEY << " is " << value << endl;
    value = "";

    ASSERT_EQ(sdk::platform::readFruKey(PRODUCTNAME_KEY,value), 0);
    cout << PRODUCTNAME_KEY << " is " << value << endl;
    value = "";

    ASSERT_EQ(sdk::platform::readFruKey(SERIALNUMBER_KEY,value), 0);
    cout << SERIALNUMBER_KEY << " is " << value << endl;
    value = "";

    ASSERT_EQ(sdk::platform::readFruKey(PARTNUM_KEY,value), 0);
    cout << PARTNUM_KEY << " is " << value << endl;
    value = "";

    ASSERT_EQ(sdk::platform::readFruKey(BOARDID_KEY,value), 0);
    cout << BOARDID_KEY << " is " << value << endl;
    value = "";

    ASSERT_EQ(sdk::platform::readFruKey(ENGCHANGELEVEL_KEY,value), 0);
    cout << ENGCHANGELEVEL_KEY << " is " << value << endl;
    value = "";

    ASSERT_EQ(sdk::platform::readFruKey(MACADDRESS_KEY,value), 0);
    cout << MACADDRESS_KEY << " is " << value << endl;
    value = "";

    ASSERT_EQ(sdk::platform::readFruKey(NUMMACADDR_KEY,value), 0);
    cout << NUMMACADDR_KEY << " is " << value << endl;
    value = "";

    ASSERT_EQ(sdk::platform::readFruKey("UKNOWN",value), -1);

}

TEST_F(sdkfru_test, open_file) {
    string key;
    string value = "";

    ASSERT_EQ(sdk::platform::readFruKey(MANUFACTURERDATE_KEY,value), 0);
    cout << MANUFACTURERDATE_KEY << " is " << value << endl;
    value = "";

    ASSERT_EQ(sdk::platform::readFruKey(MANUFACTURER_KEY,value), 0);
    cout << MANUFACTURER_KEY << " is " << value << endl;
    value = "";

    ASSERT_EQ(sdk::platform::readFruKey(PRODUCTNAME_KEY,value), 0);
    cout << PRODUCTNAME_KEY << " is " << value << endl;
    value = "";

    ASSERT_EQ(sdk::platform::readFruKey(SERIALNUMBER_KEY,value), 0);
    cout << SERIALNUMBER_KEY << " is " << value << endl;
    value = "";

    ASSERT_EQ(sdk::platform::readFruKey(PARTNUM_KEY,value), 0);
    cout << PARTNUM_KEY << " is " << value << endl;
    value = "";

    ASSERT_EQ(sdk::platform::readFruKey(BOARDID_KEY,value), 0);
    cout << BOARDID_KEY << " is " << value << endl;
    value = "";

    ASSERT_EQ(sdk::platform::readFruKey(ENGCHANGELEVEL_KEY,value), 0);
    cout << ENGCHANGELEVEL_KEY << " is " << value << endl;
    value = "";
    
    ASSERT_EQ(sdk::platform::readFruKey(MACADDRESS_KEY,value), 0);
    cout << MACADDRESS_KEY << " is " << value << endl;
    value = "";

    ASSERT_EQ(sdk::platform::readFruKey(NUMMACADDR_KEY,value), 0);
    cout << NUMMACADDR_KEY << " is " << value << endl;
    value = "";

    ASSERT_EQ(sdk::platform::readFruKey("UKNOWN",value), -1);

}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
