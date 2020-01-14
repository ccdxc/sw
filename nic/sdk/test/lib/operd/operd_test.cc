//------------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//------------------------------------------------------------------------------

#include <gtest/gtest.h>
#include <poll.h>
#include <pthread.h> 
#include <unistd.h>

#include "lib/operd/operd.hpp"

class operd_test : public ::testing::Test {
public:
protected:
    operd_test() {
    }
    
    virtual ~operd_test() {
    }
    
    virtual void SetUp() {
    }

    virtual void TearDown() {
    }
};

TEST_F (operd_test, basic) {
    char buffer[312];

    memset(buffer, 'a', sizeof(buffer));
    buffer[311] = '\0';

    sdk::operd::producer_ptr prod = sdk::operd::producer::create("test");
    sdk::operd::consumer_ptr cons = sdk::operd::consumer::create("test");

    prod->printf(1, "test %s", "log line");

    prod->write(0, 1, buffer, sizeof(buffer));
   
    sdk::operd::log_ptr l = nullptr;

    l = cons->read();
    ASSERT_STREQ(l->data(), "test log line");
    
    l = cons->read();
    ASSERT_TRUE(memcmp(l->data(), buffer, sizeof(buffer)) == 0);
}

int main (int argc, char **argv) {
    unlink("/dev/shm/test");
    setenv("OPERD_REGIONS",
           "/usr/src/github.com/pensando/sw/nic/sdk/test/"
           "lib/operd/operd-regions-test.json", 1);
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
