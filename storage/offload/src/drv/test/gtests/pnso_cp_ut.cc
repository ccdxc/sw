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

class pnso_cp_test : public ::testing::Test {
public:

protected:

    pnso_cp_test() {
    }

    virtual ~pnso_cp_test() {
    }

    // will be called immediately after the constructor before each test
    virtual void SetUp() {
    }

    // will be called immediately after each test before the destructor
    virtual void TearDown() {
    }
};

TEST_F(pnso_cp_test, ut_cp_setup) {
	/* TODO-cp_ut: ... */
}

TEST_F(pnso_cp_test, ut_cp_chain) {
	/* TODO-cp_ut: ... */
}

TEST_F(pnso_cp_test, ut_cp_schedule) {
	/* TODO-cp_ut: ... */
}

TEST_F(pnso_cp_test, ut_cp_poll) {
	/* TODO-cp_ut: ... */
}

TEST_F(pnso_cp_test, ut_cp_read_status) {
	/* TODO-cp_ut: ... */
}

TEST_F(pnso_cp_test, ut_cp_write_result) {
	/* TODO-cp_ut: ... */
}

TEST_F(pnso_cp_test, ut_cp_teardown) {
	/* TODO-cp_ut: ... */
}

int main(int argc, char **argv) {
    int ret;
    osal_log_init(OSAL_LOG_LEVEL_INFO);
    ::testing::InitGoogleTest(&argc, argv);
    ret = RUN_ALL_TESTS();
    osal_log_deinit();
    return ret;
}
