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

class pnso_chksum_test : public ::testing::Test {
public:

protected:

    pnso_chksum_test() {
    }

    virtual ~pnso_chksum_test() {
    }

    // will be called immediately after the constructor before each test
    virtual void SetUp() {
    }

    // will be called immediately after each test before the destructor
    virtual void TearDown() {
    }
};

TEST_F(pnso_chksum_test, ut_chksum_setup) {
	/* TODO-chksum_ut: ... */
}

TEST_F(pnso_chksum_test, ut_chksum_chain) {
	/* TODO-chksum_ut: ... */
}

TEST_F(pnso_chksum_test, ut_chksum_schedule) {
	/* TODO-chksum_ut: ... */
}

TEST_F(pnso_chksum_test, ut_chksum_poll) {
	/* TODO-chksum_ut: ... */
}

TEST_F(pnso_chksum_test, ut_chksum_read_status) {
	/* TODO-chksum_ut: ... */
}

TEST_F(pnso_chksum_test, ut_chksum_write_result) {
	/* TODO-chksum_ut: ... */
}

TEST_F(pnso_chksum_test, ut_chksum_teardown) {
	/* TODO-chksum_ut: ... */
}

int main(int argc, char **argv) {
    int ret;
    osal_log_init(OSAL_LOG_LEVEL_INFO);
    ::testing::InitGoogleTest(&argc, argv);
    ret = RUN_ALL_TESTS();
    osal_log_deinit();
    return ret;
}
