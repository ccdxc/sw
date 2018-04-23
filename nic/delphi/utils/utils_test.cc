// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

#include "nic/delphi/utils/log.hpp"
#include "nic/delphi/utils/error.hpp"

#include "gtest/gtest.h"

namespace {

using namespace std;
using namespace delphi;

struct test_struct_ {
    int field1;
    string field2;
    template<typename OStream>
    friend OStream& operator<<(OStream& os, const struct test_struct_ &ts)
    {
        return os << "test_struct_{ field1=" << ts.field1 << ", field2=" << ts.field2 << " }";
    }
};

TEST(UtilsTest, BasicLoggerTest) {
    int i = 1;
    string test_str = "Test string";
    struct test_struct_ test_struct = { 1, "Test Field" };
    LogInfo("This is an info message {}", i++);
    LogWarn("This is an warn message {}", i++);
    LogError("This is an error message {}", i++);
    LogDebug("This is first debug message {}", i++);
    GetLogger()->set_level(spdlog::level::debug);
    LogDebug("This is second debug message {}", i++);
    LogInfo("This prints a string: {}", test_str);
    LogInfo("This prints a struct {}", test_struct);

}

error dummyFunc() {
    return error::New("Error message");
}

TEST(UtilsTest, BasicErrorTest) {
    // basic error object checks
    error err = error("New Error");
    ASSERT_FALSE(err.IsOK()) << "Error is returning OK";
    ASSERT_EQ(err.Error(), "New Error") << "Invalid error string";
    LogInfo("Error message is: {}", err.Error());
    error err2 = err;
    ASSERT_EQ(err2, err) << "copy of err object is not equal";
    error err3 = error::New("Test Error");
    ASSERT_FALSE(err3.IsOK()) << "Error is returning OK";
    ASSERT_EQ(err3.Error(), "Test Error") << "Invalid error string";
    LogInfo("Error message is: {}", err3.Error());
    ASSERT_NE(err, err3) << "Two different errors are considered equal";
    error err4 = error::New("New Error");
    ASSERT_EQ(err, err4) << "Two error objects with same error string are not equal";

    // no error checks
    error noerr = error::OK();
    ASSERT_TRUE(noerr.IsOK()) << "error::OK() is returning false";
    ASSERT_EQ(noerr.Error(), "") << "error::OK() is not returning empty string";
    ASSERT_EQ(noerr, error::OK()) << "error::OK() is not equal to error::OK()";

    // returning error object from function
    error retErr = dummyFunc();
    ASSERT_FALSE(retErr.IsOK()) << "Error is returning OK";
    ASSERT_NE(retErr.Error(), "") << "Returned error is empty";
    LogInfo("Returned error message is {}", retErr.Error());
    LogInfo("Returned error is {}", retErr);

}

} // namespace

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
