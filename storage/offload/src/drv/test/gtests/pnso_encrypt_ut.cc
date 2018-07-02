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

class pnso_encrypt_test : public ::testing::Test {
public:

protected:

    pnso_encrypt_test() {
    }

    virtual ~pnso_encrypt_test() {
    }

    // will be called immediately after the constructor before each test
    virtual void SetUp() {
    }

    // will be called immediately after each test before the destructor
    virtual void TearDown() {
    }
};

TEST_F(pnso_encrypt_test, ut_encrypt_setup) {
	/* TODO-encrypt_ut: ... */
}

TEST_F(pnso_encrypt_test, ut_encrypt_chain) {
	/* TODO-encrypt_ut: ... */
}

TEST_F(pnso_encrypt_test, ut_encrypt_schedule) {
	/* TODO-encrypt_ut: ... */
}

TEST_F(pnso_encrypt_test, ut_encrypt_poll) {
	/* TODO-encrypt_ut: ... */
}

TEST_F(pnso_encrypt_test, ut_encrypt_read_status) {
	/* TODO-encrypt_ut: ... */
}

TEST_F(pnso_encrypt_test, ut_encrypt_write_result) {
	/* TODO-encrypt_ut: ... */
}

TEST_F(pnso_encrypt_test, ut_encrypt_teardown) {
	/* TODO-encrypt_ut: ... */
}

int main(int argc, char **argv) {
    int ret;
    osal_log_init(OSAL_LOG_LEVEL_INFO);
    ::testing::InitGoogleTest(&argc, argv);
    ret = RUN_ALL_TESTS();
    osal_log_deinit();
    return ret;
}
