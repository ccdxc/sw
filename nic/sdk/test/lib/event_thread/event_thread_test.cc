//------------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//------------------------------------------------------------------------------

#include <gtest/gtest.h>
#include <unistd.h>
#include "lib/event_thread/event_thread.hpp"

using namespace sdk::lib;

#define THREAD_T1 0
#define THREAD_T2 1
#define THREAD_T3 2

class event_thread_test : public ::testing::Test {
public:
    event_thread *t1;
    event_thread *t2;
    event_thread *t3;
    event_timer timer;
    event_io    io;
    int         fd[2];
    bool got_ping = false;
    bool got_pong = false;
    bool got_ping_on_fd = false;
    bool got_pong_on_fd = false;
    bool t1_stopped = false;
    bool t2_stopped = false;

protected:
  event_thread_test() {
      pipe(this->fd);
  }

  virtual ~event_thread_test() {
  }

  // will be called immediately after the constructor before each test
  virtual void SetUp() {
  }

  // will be called immediately after each test before the destructor
  virtual void TearDown() {
  }

};

void
timer_callback (event_timer_t *timer)
{
    event_message_send(THREAD_T2, (void *)"PING");
}

void
io_callback (event_io_t *io, int fd, int events)
{
    char buf[5];
    int n;

    event_thread_test *test = (event_thread_test *)io->ctx;
    ASSERT_EQ(events, EVENT_READ);

    n = read(fd, buf, 5);
    ASSERT_EQ(n, 5);

    if (strcmp(buf, "PING") == 0) {
        test->got_ping_on_fd = true;
    } else if (strcmp(buf, "PONG") == 0) {
        test->got_pong_on_fd = true;
    } else {
        ASSERT_TRUE(false);
    }
}

void
init1 (void *ctx)
{
    event_thread_test *test = (event_thread_test *)ctx;
    test->timer.ctx = ctx;
    event_timer_init(&test->timer, timer_callback, 1., 0.);
    event_timer_start(&test->timer);
}

void
init3 (void *ctx)
{
    event_thread_test *test = (event_thread_test *)ctx;
    test->io.ctx = ctx;
    event_io_init(&test->io, io_callback, test->fd[0], EVENT_READ);
    event_io_start(&test->io);
}

void
msg1 (void *message, void *ctx)
{
    event_thread_test *test = (event_thread_test *)ctx;
    test->got_pong = (strcmp((char *)message, "PONG") == 0);
    printf("%s\n", (char *)message);
    write(test->fd[1], "PONG", 5);
}

void
msg2 (void *message, void *ctx)
{
    event_thread_test *test = (event_thread_test *)ctx;
    test->got_ping = (strcmp((char *)message, "PING") == 0);
    printf("%s\n", (char *)message);
    write(test->fd[1], "PING", 5);
    event_message_send(THREAD_T1, (void *)"PONG");
}

void
exit1 (void *ctx)
{
    event_thread_test *test = (event_thread_test *)ctx;
    test->t1_stopped = true;
}

void
exit2 (void *ctx)
{
    event_thread_test *test = (event_thread_test *)ctx;
    test->t2_stopped = true;
}

TEST_F (event_thread_test, basic_functionality) {
    this->t1 = event_thread::factory("t1", THREAD_T1, THREAD_ROLE_CONTROL,
        0x0, init1, exit1, msg1, 0, SCHED_OTHER, true);
    this->t2 = event_thread::factory("t2", THREAD_T2, THREAD_ROLE_CONTROL,
        0x0, NULL, exit2, msg2, 0, SCHED_OTHER, true);
    this->t3 = event_thread::factory("t3", THREAD_T3, THREAD_ROLE_CONTROL,
        0x0, init3, NULL, NULL, 0, SCHED_OTHER, true);
    
    this->t1->start(this);
    this->t2->start(this);
    this->t3->start(this);

    sleep(2);

    ASSERT_TRUE(this->got_ping);
    ASSERT_TRUE(this->got_pong);

    ASSERT_TRUE(this->got_ping_on_fd);
    ASSERT_TRUE(this->got_pong_on_fd);

    this->t1->stop();
    this->t2->stop();
    this->t3->stop();

    sleep(2);

    ASSERT_TRUE(this->t1_stopped);
    ASSERT_TRUE(this->t2_stopped);
}

int main (int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
