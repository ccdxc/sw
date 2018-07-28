/*
 * {C} Copyright 2018 Pensando Systems Inc.
 * All rights reserved.
 *
 */
#include <gtest/gtest.h>
#include <vector>
#include <unistd.h>

#include "osal_logger.h"
#include "storage/offload/src/drv/pnso_chain.h"

using namespace std;

class pnso_hash_test : public ::testing::Test {
public:

protected:

    pnso_hash_test() {
    }

    virtual ~pnso_hash_test() {
    }

    // will be called immediately after the constructor before each test
    virtual void SetUp() {
    }

    // will be called immediately after each test before the destructor
    virtual void TearDown() {
    }
};

TEST_F(pnso_hash_test, ut_hash_setup) {
	/* TODO-hash_ut: ... */
}

TEST_F(pnso_hash_test, ut_hash_chain) {
	/* TODO-hash_ut: ... */
}

TEST_F(pnso_hash_test, ut_hash_schedule) {
	/* TODO-hash_ut: ... */
}

TEST_F(pnso_hash_test, ut_hash_poll) {
	/* TODO-hash_ut: ... */
}

TEST_F(pnso_hash_test, ut_hash_read_status) {
	/* TODO-hash_ut: ... */
}

TEST_F(pnso_hash_test, ut_hash_write_result) {
	/* TODO-hash_ut: ... */
}

TEST_F(pnso_hash_test, ut_hash_teardown) {
	/* TODO-hash_ut: ... */
}

int main(int argc, char **argv) {
    int ret;
    osal_log_init(OSAL_LOG_LEVEL_INFO);
    ::testing::InitGoogleTest(&argc, argv);
    ret = RUN_ALL_TESTS();
    osal_log_deinit();
    return ret;
}
