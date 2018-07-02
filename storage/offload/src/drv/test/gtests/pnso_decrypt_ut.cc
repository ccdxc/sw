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

class pnso_decrypt_test : public ::testing::Test {
public:

protected:

    pnso_decrypt_test() {
    }

    virtual ~pnso_decrypt_test() {
    }

    // will be called immediately after the constructor before each test
    virtual void SetUp() {
    }

    // will be called immediately after each test before the destructor
    virtual void TearDown() {
    }
};

TEST_F(pnso_decrypt_test, ut_decrypt_setup) {
	/* TODO-decrypt_ut: ... */
}

TEST_F(pnso_decrypt_test, ut_decrypt_chain) {
	/* TODO-decrypt_ut: ... */
}

TEST_F(pnso_decrypt_test, ut_decrypt_schedule) {
	/* TODO-decrypt_ut: ... */
}

TEST_F(pnso_decrypt_test, ut_decrypt_poll) {
	/* TODO-decrypt_ut: ... */
}

TEST_F(pnso_decrypt_test, ut_decrypt_read_status) {
	/* TODO-decrypt_ut: ... */
}

TEST_F(pnso_decrypt_test, ut_decrypt_write_result) {
	/* TODO-decrypt_ut: ... */
}

TEST_F(pnso_decrypt_test, ut_decrypt_teardown) {
	/* TODO-decrypt_ut: ... */
}

int main(int argc, char **argv) {
    int ret;
    osal_log_init(OSAL_LOG_LEVEL_INFO);
    ::testing::InitGoogleTest(&argc, argv);
    ret = RUN_ALL_TESTS();
    osal_log_deinit();
    return ret;
}
