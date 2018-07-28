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

class pnso_dc_test : public ::testing::Test {
public:

protected:

    pnso_dc_test() {
    }

    virtual ~pnso_dc_test() {
    }

    // will be called immediately after the constructor before each test
    virtual void SetUp() {
    }

    // will be called immediately after each test before the destructor
    virtual void TearDown() {
    }
};

TEST_F(pnso_dc_test, ut_dc_setup) {
	/* TODO-dc_ut: ... */
}

TEST_F(pnso_dc_test, ut_dc_chain) {
	/* TODO-dc_ut: ... */
}

TEST_F(pnso_dc_test, ut_dc_schedule) {
	/* TODO-dc_ut: ... */
}

TEST_F(pnso_dc_test, ut_dc_poll) {
	/* TODO-dc_ut: ... */
}

TEST_F(pnso_dc_test, ut_dc_read_status) {
	/* TODO-dc_ut: ... */
}

TEST_F(pnso_dc_test, ut_dc_write_result) {
	/* TODO-dc_ut: ... */
}

TEST_F(pnso_dc_test, ut_dc_teardown) {
	/* TODO-dc_ut: ... */
}

int main(int argc, char **argv) {
    int ret;
    osal_log_init(OSAL_LOG_LEVEL_INFO);
    ::testing::InitGoogleTest(&argc, argv);
    ret = RUN_ALL_TESTS();
    osal_log_deinit();
    return ret;
}
