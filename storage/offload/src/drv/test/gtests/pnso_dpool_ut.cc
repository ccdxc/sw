/*
 * {C} Copyright 2018 Pensando Systems Inc.
 * All rights reserved.
 *
 */
#include <gtest/gtest.h>
#include <vector>
#include <unistd.h>

#include "osal_logger.h"
#include "storage/offload/src/drv/pnso_dpool.h"

using namespace std;

class pnso_dpool_test : public ::testing::Test {
public:

protected:

    pnso_dpool_test() {
    }

    virtual ~pnso_dpool_test() {
    }

    // will be called immediately after the constructor before each test
    virtual void SetUp() {
    }

    // will be called immediately after each test before the destructor
    virtual void TearDown() {
    }
};

TEST_F(pnso_dpool_test, ut_dpool_create) {
	/* TODO-dpool: ... */
}

TEST_F(pnso_dpool_test, ut_dpool_destroy) {
	/* TODO-dpool: ... */
}

TEST_F(pnso_dpool_test, ut_dpool_get_desc) {
	/* TODO-dpool: ... */
}

TEST_F(pnso_dpool_test, ut_dpool_put_desc) {
	/* TODO-dpool: ... */
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
