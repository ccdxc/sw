#include "nic/utils/events/recorder/recorder.hpp"
#include "nic/utils/ipc/constants.h"
#include "nic/utils/events/recorder/constants.h"
#include "gen/proto/kh.pb.h"
#include "gen/proto/events.pb.h"
#include "gen/proto/example_event_types.pb.h"
#include <gtest/gtest.h>
#include <sys/mman.h>
#include <sys/stat.h>        /* For mode constants */
#include <fcntl.h>           /* For O_* constants */
#include <string>

class events_recorder_test : public ::testing::Test {
protected:
    events_recorder_test() {
    }

    virtual ~events_recorder_test() {
    }

    // will be called immediately after the constructor before each test
    virtual void SetUp() {
    }

    // will be called immediately after each test before the destructor
    virtual void TearDown() {
    }
};

// basic events recorder test
// - test recording events using the recorder library.
TEST_F(events_recorder_test, basic) {
    const char* component  = "test-c";
    const char* shm_name   = "/events_recorder_test_basic";
    int shm_size           = 512; // 512 bytes
    int max_events_allowed = ((shm_size - IPC_OVH_SIZE)/SHM_BUF_SIZE) - 1;

    // initialize events recorder
    events_recorder* recorder = events_recorder::init(shm_name, shm_size, component, example_event_types::EventTypes_descriptor());
    ASSERT_NE(recorder, nullptr);

    // invalid event type (event type does not exist)
    ASSERT_EQ(recorder->event(events::INFO, 5, "Vrf", kh::VrfKeyHandle(), "test event message"), -1);

    // record some events
    for (int index = 1; index <= max_events_allowed; index++) {
        ASSERT_EQ(recorder->event(events::INFO, example_event_types::EVENT_TYPE1, "Vrf",  kh::VrfKeyHandle(), "test event message"), 0);
        usleep(1000 * 10); // 10ms
    }

    // any further writes should fail
    for (int index = 1; index <= 5; index++) {
	    kh::VrfKeyHandle key;
	    key.set_vrf_id(index);
        ASSERT_EQ(recorder->event(events::INFO, example_event_types::EVENT_TYPE1, "Vrf", key, "test event message"), -1);
    }

    recorder->deinit();
}


// test messages/events written and read were intact
// - write event `evt`.
// - read event and verify it matches `evt`.
TEST_F(events_recorder_test, verify_rw) {
    const char* component  = "test-c";
    const char* shm_name   = "/events_recorder_test_verify_rw";
    int shm_size           = 512; // 512 bytes

    // initialize events recorder
    events_recorder* recorder = events_recorder::init(shm_name, shm_size, component, example_event_types::EventTypes_descriptor());
    ASSERT_NE(recorder, nullptr);

    // WRITE to shared mem.
    ASSERT_EQ(recorder->event(events::INFO, example_event_types::EVENT_TYPE1, "Vrf",  kh::VrfKeyHandle(), "test event message"), 0);

    // OPEN & READ from shared mem.
    int fd = shm_open(shm_name, O_RDWR, 0666);
    ASSERT_EQ(fd > 0, true);
    void *mmap_addr = mmap(0, shm_size, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
    ASSERT_NE(mmap_addr, MAP_FAILED);
    uint8_t *shm = (uint8_t *)mmap_addr;

    volatile uint32_t *ri = (volatile uint32_t *)&shm[IPC_READ_OFFSET];
    int offset = *ri * SHM_BUF_SIZE + IPC_OVH_SIZE;
    int msgSize = *((uint32_t *) &shm[offset]);
    volatile uint8_t *dataStAddr = &shm[offset + IPC_HDR_SIZE];
    events::Event evt;
    bool ok = evt.ParseFromArray((const void *)dataStAddr, msgSize);
    ASSERT_EQ(ok, true);

    // ensure read event matches what was written
    ASSERT_EQ(evt.severity(), 0);
    recorder->deinit();
}

struct arg_struct {
    const char* shm_name;
    int shm_size;
    int total_messages_to_be_read;
} args;


// helper function to read events from shared memory
void* read_from_shm(void *args) {
    unsigned int thread_id = (unsigned int)pthread_self();
    printf("reader thread: %u\n", thread_id);

    struct arg_struct arg = *((arg_struct*)args);
    int total_events = arg.total_messages_to_be_read;

    int fd = shm_open(arg.shm_name, O_RDWR, 0666);
    if (fd < 0) {
        printf("err: failed to open shared memory\n");
        return (void *)-1;
    }

    // map shared memory to a virtual address of this process
    void *mmap_addr = mmap(0, arg.shm_size, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
    if (mmap_addr == MAP_FAILED) {
        printf("err: failed to mmap the shared memory\n");
        return (void *)-1;
    }

    uint8_t *shm = (uint8_t *)mmap_addr;

    int numBuffers = (arg.shm_size - IPC_OVH_SIZE)/SHM_BUF_SIZE;

    volatile uint32_t *ri = (volatile uint32_t *)&shm[IPC_READ_OFFSET];
    volatile uint32_t *wi = (volatile uint32_t *)&shm[IPC_WRITE_OFFSET];
    volatile uint32_t *tw = (volatile uint32_t *)&shm[IPC_TOTAL_WRITES];

    printf("+++ reading events from shared memory +++ \n");
    for (;;) {
        int msgCount = (*wi + numBuffers - *ri) % numBuffers;
        if (msgCount>0) {
            int offset = *ri * SHM_BUF_SIZE + IPC_OVH_SIZE;
            int msgSize = *((uint32_t *) &shm[offset]);
            volatile uint8_t *dataStAddr = &shm[offset + IPC_HDR_SIZE];

            events::Event evt;
            bool ok = evt.ParseFromArray((const void *)dataStAddr, msgSize);
            if (!ok) {
                printf("err: failed to read event from shared memory\n");
                return (void *)-1;
            }

            // ensure the event data is read right
            if (evt.type().empty() || evt.component().empty() || evt.message().empty()) {
                return (void *)-1;
            }
            printf("{severity : %d, type: %s, message: %s, component: %s, time: %lu, object_kind: %s, obj_type: %s}\n",
                evt.severity(),
                evt.type().c_str(),
                evt.message().c_str(),
                evt.component().c_str(),
                evt.time(),
                evt.object_kind().c_str(),
                evt.object_key().type_url().c_str());

            *ri = (*ri + 1) % numBuffers;  // update read index
            arg.total_messages_to_be_read--;
            if  (arg.total_messages_to_be_read == 0) {
                // make sure the shared memory is empty now as all the messages are read
                if (*ri  != *wi) { // *ri == *wi - empty
                    printf("err: mismatched reader(%d) and writer(%d) indices\n", *ri, *wi);
                    return (void *)-1;
                } else if(total_events != *tw) {
                    printf("err: mismatched total writes(%d) and total events/messages(%d) sent to shared mem.\n", total_events, *tw);
                    return (void *)-1;
                }

                return (void *)0;
            }
        }
    }

    return (void *)0;
}

// test recorder by sending multiple events while simultaneously reading it off from another thread
// - write multiple events.
// - spin off a pthread and let it read events from shared memory.
// - ensure all the events sent were read by the reader.
TEST_F(events_recorder_test, multiple_events) {
    const char* component = "test-c";
    const char* shm_name  = "/events_recorder_test_multiple_events";
    int shm_size          = 1024; // 1024 bytes
    int total_events      = 100;

    // initialize events recorder
    events_recorder* recorder = events_recorder::init(shm_name, shm_size, component, example_event_types::EventTypes_descriptor());
    ASSERT_NE(recorder, nullptr);

    args.shm_name = shm_name;
    args.shm_size = shm_size;
    args.total_messages_to_be_read = total_events;

    pthread_t reader;
    void *r_status=0;
    pthread_create(&reader, NULL, read_from_shm, (void *)&args);

    // record some events
    for (int index=1; index <= total_events; index++) {
        if (index%2 == 0) { // VrfKeyHandle as object key
            kh::VrfKeyHandle key;
            ASSERT_EQ(recorder->event(events::INFO, example_event_types::EVENT_TYPE1, "Vrf",  key, "test msg - %d", index), 0);
        } else {            // SecurityPolicyKey as object key
            kh::SecurityPolicyKey key;
            key.set_security_policy_id(index);
            ASSERT_EQ(recorder->event(events::INFO, example_event_types::EVENT_TYPE2, "SecurityGroup", key, "test msg - %d", index), 0);
        }
        usleep(1000 * 10); // 10ms
    }

    pthread_join(reader, &r_status);

    long int reader_ret_val = (intptr_t)r_status;
    ASSERT_NE(reader_ret_val, -1);

    recorder->deinit();
}