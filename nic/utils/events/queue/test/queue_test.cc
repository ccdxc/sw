#include "nic/utils/ipc/constants.h"
#include "nic/utils/events/queue/queue.hpp"
#include <gtest/gtest.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <cstdint>

#define TEST_SHM_SIZE 300 // in bytes
#define TEST_BUF_SIZE 64

class events_queue_test : public ::testing::Test {
protected:
    Logger logger;
    const char* test_case_name;

    events_queue_test() {
    }

    virtual ~events_queue_test() {
    }

    // will be called immediately after the constructor before each test
    virtual void SetUp() {
        test_case_name = ::testing::UnitTest::GetInstance()->current_test_info()->name();
        logger = spdlog::stdout_color_mt(test_case_name);
        logger->set_pattern("%L [%Y-%m-%d %H:%M:%S.%f] %P/%n: %v");
    }

    // will be called immediately after each test before the destructor
    virtual void TearDown() {
        spdlog::drop(test_case_name);
    }
};


// basic events queue test
// - tests init/factory functions.
// - verify buffer allocation and writes.
TEST_F(events_queue_test, single_thread) {
    const char* shm_name = "/events_shm_test_single_thread";

    // buf size > shm size
    ASSERT_EQ(events_queue::init(shm_name, TEST_SHM_SIZE, TEST_SHM_SIZE+1, logger), nullptr);

    int buf_size = TEST_BUF_SIZE;
    int index = 0;

    // 1. test init/factory
    events_queue *evts_queue = events_queue::init(shm_name, TEST_SHM_SIZE, buf_size, logger);
    ASSERT_NE(evts_queue, nullptr);

    int total_num_bufs = ((TEST_SHM_SIZE - IPC_OVH_SIZE)/buf_size)-1;
    printf("total number of buffers available in the memory: %d\n", total_num_bufs);

    // 2. test get/put buffer
    uint8_t *buf;
    int wr_err;
    for (; index < total_num_bufs; index++) {
        buf = evts_queue->get_buffer(60);
        ASSERT_NE(buf, nullptr);
        wr_err = evts_queue->write_msg_size(buf, 60);
        ASSERT_EQ(wr_err, 0);
    }

    // any further get buffer should fail as the shared memory is full
    ASSERT_EQ(evts_queue->get_buffer(60), nullptr);

    // write with invalid size
    ASSERT_EQ(evts_queue->write_msg_size(buf, -60), -1);

    // write on a empty buffer
    ASSERT_EQ(evts_queue->write_msg_size(nullptr, 60), -1);

    evts_queue->deinit();
}

// TODO: try getting more buffers by adjusting the reader/consumer index.

