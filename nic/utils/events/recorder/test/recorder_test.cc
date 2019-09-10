#include "nic/utils/events/recorder/recorder.hpp"
#include "nic/utils/ipc/constants.h"
#include "gen/proto/kh.pb.h"
#include "gen/proto/events.pb.h"
#include "gen/proto/eventtypes.pb.h"
#include <gtest/gtest.h>
#include <sys/mman.h>
#include <sys/stat.h>        /* For mode constants */
#include <fcntl.h>           /* For O_* constants */
#include <string>


class events_recorder_test : public ::testing::Test {
protected:
    Logger logger;
    const char* test_case_name;

    events_recorder_test() {
    }

    virtual ~events_recorder_test() {
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

// basic events recorder test
// - test recording events using the recorder library.
TEST_F(events_recorder_test, test_basic) {
    const char* component  = ::testing::UnitTest::GetInstance()->current_test_info()->name();
    int shm_size           = 800; // 800 bytes (holds 2 events)
    int max_events_allowed = ((shm_size - IPC_OVH_SIZE)/SHM_BUF_SIZE) - 1;

    // initialize events recorder
    events_recorder* recorder = events_recorder::init(component, logger, shm_size);
    ASSERT_NE(recorder, nullptr);

    // record some events
    int index = 1;
    for (; index <= max_events_allowed-1; index++) {
        kh::VrfKeyHandle key;
        key.set_vrf_id(index);
        ASSERT_EQ(recorder->event_with_ref(eventtypes::SERVICE_STARTED, "Vrf",  key, "test event message - %d ", index), 0);
        usleep(1000 * 10); // 10ms
    }
    // last event
    if (max_events_allowed != 0) {
        ASSERT_EQ(recorder->event(eventtypes::SERVICE_STARTED, "test event message - %d ", index), 0);
    }

    // any further writes should fail
    for (int index = 1; index <= 5; index++) {
	    kh::VrfKeyHandle key;
	    key.set_vrf_id(index);
        ASSERT_EQ(recorder->event_with_ref(eventtypes::SERVICE_STARTED, "Vrf", key, "test event message"), -1);
    }

    recorder->deinit();
}


// test messages/events written and read were intact
// - write event `evt`.
// - read event and verify it matches `evt`.
TEST_F(events_recorder_test, test_verify_rw) {
    const char* component  = ::testing::UnitTest::GetInstance()->current_test_info()->name();
    int shm_size           = 576; // 576 bytes

    // initialize events recorder
    events_recorder* recorder = events_recorder::init(component, logger, shm_size);
    ASSERT_NE(recorder, nullptr);

    // WRITE to shared mem.
    ASSERT_EQ(recorder->event_with_ref(eventtypes::SERVICE_STARTED, "Vrf",  kh::VrfKeyHandle(), "test event message"), 0);

    // OPEN & READ from shared mem.
    std::string shm_name  = std::string("/") + component + std::string(".events");
    int fd = shm_open(shm_name.c_str(), O_RDWR, 0666);
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
    ASSERT_EQ(evt.message(), "test event message");
    ASSERT_EQ(evt.type(), eventtypes::SERVICE_STARTED);
    ASSERT_EQ(evt.component(), component);
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
            if (evt.component().empty() || evt.message().empty()) {
                return (void *)-1;
            }
            printf("{type: %d, message: %s, component: %s, time: %lu, object_kind: %s, obj_type: %s}\n",
                evt.type(),
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
TEST_F(events_recorder_test, test_multiple_rws) {
    const char* component  = ::testing::UnitTest::GetInstance()->current_test_info()->name();
    int shm_size          = 1024; // 1024 bytes
    int total_events      = 100;

    // initialize events recorder
    events_recorder* recorder = events_recorder::init(component, logger, shm_size);
    ASSERT_NE(recorder, nullptr);

    std::string shm_name  = std::string("/") + component + std::string(".events");
    args.shm_name = shm_name.c_str();
    args.shm_size = shm_size;
    args.total_messages_to_be_read = total_events;

    pthread_t reader;
    void *r_status=0;
    pthread_create(&reader, NULL, read_from_shm, (void *)&args);

    // record some events
    for (int index=1; index <= total_events; index++) {
        if (index%2 == 0) { // VrfKeyHandle as object key
            kh::VrfKeyHandle key;
            ASSERT_EQ(recorder->event_with_ref(eventtypes::SERVICE_STARTED, "Vrf",  key, "test msg - %d", index), 0);
        } else {            // SecurityPolicyKey as object key
            kh::SecurityPolicyKey key;
            key.set_security_policy_id(index);
            ASSERT_EQ(recorder->event_with_ref(eventtypes::SERVICE_STARTED, "SecurityGroup", key, "test msg - %d", index), 0);
        }
        usleep(1000 * 10); // 10ms
    }

    pthread_join(reader, &r_status);

    long int reader_ret_val = (intptr_t)r_status;
    ASSERT_NE(reader_ret_val, -1);

    recorder->deinit();
}