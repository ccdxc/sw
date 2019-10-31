//------------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//------------------------------------------------------------------------------

#include <gtest/gtest.h>
#include <unistd.h>
#include "lib/event_thread/event_thread.hpp"
#include "lib/ipc/ipc.hpp"

using namespace sdk::lib;
namespace event = sdk::event_thread;

#define THREAD_T1 1
#define THREAD_T2 2
#define THREAD_T3 3
#define THREAD_T4 4
#define THREAD_T5 5
#define THREAD_T6 6
#define THREAD_T7 7

class event_thread_test : public ::testing::Test {
public:
    event::event_thread *t1;
    event::event_thread *t2;
    event::event_thread *t3;
    event::timer_t timer;
    event::timer_t timer2;
    event::io_t io;
    event::prepare_t prepare;
    int fd[2];
    bool got_ping = false;
    bool got_pong = false;
    bool got_ping_on_fd = false;
    bool got_pong_on_fd = false;
    bool t1_stopped = false;
    bool t2_stopped = false;
    uint32_t prepare_count = 0;

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
prepare_callback (event::prepare_t *prepare, void *ctx)
{
    event_thread_test *test = (event_thread_test *)ctx;
    test->prepare_count++;
    event::prepare_stop(prepare);
    printf("Prepare called and stopped\n");
}

void
timer_callback (event::timer_t *timer)
{
    event::message_send(THREAD_T2, (void *)"PING");
}

void
io_callback (event::io_t *io, int fd, int events)
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
    event::timer_init(&test->timer, timer_callback, 1., 0.);
    event::timer_start(&test->timer);
    event::prepare_init(&test->prepare, prepare_callback, test);
    event::prepare_start(&test->prepare);
}

void
init3 (void *ctx)
{
    event_thread_test *test = (event_thread_test *)ctx;
    test->io.ctx = ctx;
    event::io_init(&test->io, io_callback, test->fd[0], EVENT_READ);
    event::io_start(&test->io);
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
    event::message_send(THREAD_T1, (void *)"PONG");
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
    this->t1 = event::event_thread::factory(
        "t1", THREAD_T1, THREAD_ROLE_CONTROL, 0x0, init1, exit1, msg1, 0,
        SCHED_OTHER, true);
    this->t2 = event::event_thread::factory(
        "t2", THREAD_T2, THREAD_ROLE_CONTROL, 0x0, NULL, exit2, msg2, 0,
        SCHED_OTHER, true);
    this->t3 = event::event_thread::factory(
        "t3", THREAD_T3, THREAD_ROLE_CONTROL, 0x0, init3, NULL, NULL, 0,
        SCHED_OTHER, true);
    
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

    ASSERT_TRUE(this->prepare_count == 1);
}

void
ipc_cb (sdk::ipc::ipc_msg_ptr msg, void *ctx)
{
    printf("Server got: %s\n", (char *)msg->data());
    event::rpc_response(msg, msg->data(), msg->length());

    printf("Also publishing: broadcast\n");
    event::publish(0x123, (void *)"broadcast", 10);
}

void
timer2_callback (event::timer_t *timer)
{
    const char message[] = "echo from event";
    event::rpc_request(THREAD_T4, 0, (void *)message, sizeof(message),
                      (const void *)0x1234);
}

void
init_ipc_server (void *ctx)
{
    event::rpc_reg_request_handler(0, ipc_cb);
}

void
ipc_client_cb (uint32_t sender, sdk::ipc::ipc_msg_ptr msg, void *ctx,
               const void *cookie)
{
    printf("Client got: %s, cookie: %p\n", (char *)msg->data(),
           cookie);
}

void
subscribe_cb (void *data, size_t data_length, void *ctx)
{
    printf("Subscriber got: %s\n", (char *)data);
}

void
init_ipc_client (void *ctx)
{
    event_thread_test *test = (event_thread_test *)ctx;
    test->timer2.ctx = ctx;
    event::timer_init(&test->timer2, timer2_callback, 1., 0.);
    event::timer_start(&test->timer2);

    event::rpc_reg_request_handler(0, ipc_cb);
    event::rpc_reg_response_handler(0, ipc_client_cb);

    event::subscribe(0x123, subscribe_cb);
}


TEST_F (event_thread_test, ipc_functionality) {
    event::event_thread *t4 = event::event_thread::factory(
        "t4", THREAD_T4, THREAD_ROLE_CONTROL, 0x0, init_ipc_server, NULL, NULL,
        0, SCHED_OTHER, true);
    t4->start(NULL);

    //
    //
    // IF WE SEND MESSAGE BEFORE THE LISTENER BINDS THE MESSAGE WILL BE LOST
    //
    //
    sleep(2);

    //
    // async client
    //
    event::event_thread *t5 = event::event_thread::factory(
        "t5", THREAD_T5, THREAD_ROLE_CONTROL, 0x0, init_ipc_client, NULL, NULL,
        0, SCHED_OTHER, true);
    t5->start(this);
    sleep(2);
    
    //
    // sync client
    //
    sdk::ipc::ipc_msg_ptr msg;
    msg = sdk::ipc::request(THREAD_T4, 0, "ipc ping", 9);
    printf("%s\n", (char *)msg->data());
    msg = sdk::ipc::request(THREAD_T4, 0, "ipc ping", 9);
    printf("%s\n", (char *)msg->data());
    msg = sdk::ipc::request(THREAD_T4, 0, "ipc ping", 9);
    printf("%s\n", (char *)msg->data());
    msg = sdk::ipc::request(THREAD_T4, 0, "ipc ping", 9);
    printf("%s\n", (char *)msg->data());
    sleep(2);
}

int main (int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
