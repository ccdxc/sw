#include <sys/mman.h>
#include <sys/stat.h>        /* For mode constants */
#include <fcntl.h>           /* For O_* constants */
#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#include <gtest/gtest.h>
#include <stdio.h>
#include <iostream>
#include "nic/utils/agent_api/agent_api.hpp"
#include "nic/utils/agent_api/shared_constants.h"
#include "sdk/thread.hpp"
#include "nic/gen/proto/hal/nwsec.pb.h"
#include "nic/gen/proto/hal/fwlog.pb.h"
#define TEST_THREAD_COUNT 8
#define TEST_ALLOC_SIZE 8
#define TEST_IPC_SIZE 128
#define EXP_BUF_COUNT (TEST_IPC_SIZE/TEST_ALLOC_SIZE - 1)
#define FWLOG_SHM "/fwlog_ipc_shm"
#define TEST_SHM_SIZE 512
#define TEST_QSIZE 7
#define TEST_FW_SHM_SIZE 64*1024

typedef struct testInfo_ {
    ipc *ipcUT;
    int bufCount;
    int tryCount;
} testInfo_t;

class ipc_test : public ::testing::Test {
protected:
    ipc_test() {
    }

    virtual ~ipc_test() {
    }

    // will be called immediately after the constructor before each test
    virtual void SetUp() {
    }

    // will be called immediately after each test before the destructor
    virtual void TearDown() {
    }
};


void verify_snd_rcv(ipc *ipcUT, volatile uint32_t *ipcBlock) {
    int ret;
    uint8_t *buf;
    int ix;

    for (ix = 0; ix < (TEST_QSIZE-1); ix++) {
        buf = ipcUT->get_buffer(32);
        ASSERT_NE(buf, nullptr);
        ret = ipcUT->put_buffer(buf, 32);
        ASSERT_EQ(ret, 0);
    }

    // Next alloc should fail
    buf = ipcUT->get_buffer(32);
    ASSERT_EQ(buf, nullptr);

    ASSERT_EQ(ipcBlock[7], 0);
    for (ix = 0; ix < 1000; ix++) {
        ipcBlock[7] = (ipcBlock[7] + 1) % TEST_QSIZE;
        // Next alloc should pass
        buf = ipcUT->get_buffer(32);
        ASSERT_NE(buf, nullptr);
        ret = ipcUT->put_buffer(buf, 32);
        ASSERT_EQ(ret, 0);

        // Next alloc should fail
        buf = ipcUT->get_buffer(32);
        ASSERT_EQ(buf, nullptr);
    }
}

// Basic Test
// Init ipc
// Verify buffer allocation, release.
TEST_F(ipc_test, basic) {
    int ret;
    uint8_t *buf;
    int ix;

    ret = ipc::setup_shm(TEST_SHM_SIZE, 1);
    ASSERT_EQ(ret, 0);
    ipc *ipc1 = ipc::factory();
    ASSERT_NE(ipc1, nullptr);

    for (ix = 0; ix < (TEST_QSIZE-1); ix++) {
        buf = ipc1->get_buffer(32);
        ASSERT_NE(buf, nullptr);
        ret = ipc1->put_buffer(buf, 32);
        ASSERT_EQ(ret, 0);
    }

    // Next alloc should fail
    buf = ipc1->get_buffer(32);
    ASSERT_EQ(buf, nullptr);

    // open shared mem
    int fd = shm_open(FWLOG_SHM, O_RDWR, 0666);
    ASSERT_GE(fd, 0);
    void *p = mmap(0, TEST_SHM_SIZE, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
    ASSERT_NE(p, MAP_FAILED);

    volatile uint32_t *p32 = (uint32_t *)p;
    ASSERT_EQ(p32[7], 0);
    for (ix = 0; ix < 1000; ix++) {
        p32[7] = (p32[7] + 1) % TEST_QSIZE;
        // Next alloc should pass
        buf = ipc1->get_buffer(32);
        ASSERT_NE(buf, nullptr);
        ret = ipc1->put_buffer(buf, 32);
        ASSERT_EQ(ret, 0);

        // Next alloc should fail
        buf = ipc1->get_buffer(32);
        ASSERT_EQ(buf, nullptr);
    }

    ipc::tear_down_shm();
}

// Multi-instance Test
// Init ipc
// Verify operation with three instances
TEST_F(ipc_test, multi) {
    int ret;
    int ix;
    ipc *ipc_inst[3];

    ret = ipc::setup_shm(TEST_SHM_SIZE*3, 3);
    ASSERT_EQ(ret, 0);
    for (ix = 0; ix < 3; ix++) {
        ipc_inst[ix] = ipc::factory();
        ASSERT_NE(ipc_inst[ix], nullptr);
    }
    // next instance should fail.
    ipc *ipc1 = ipc::factory();
    ASSERT_EQ(ipc1, nullptr);

    // open shared mem
    int fd = shm_open(FWLOG_SHM, O_RDWR, 0666);
    ASSERT_GE(fd, 0);
    void *p = mmap(0, 3*TEST_SHM_SIZE, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
    ASSERT_NE(p, MAP_FAILED);

    uint8_t *p8 = (uint8_t *)p;
    for (ix = 0; ix < 3; ix++) {
        verify_snd_rcv(ipc_inst[ix], (volatile uint32_t *)&p8[ix*TEST_SHM_SIZE]);
    }
    ipc::tear_down_shm();
}

void verify_fw_log(hal::flow_key_t &key, volatile uint8_t *shm) {
    uint32_t offset;
    volatile uint8_t *datap;

    volatile uint32_t *ri = (volatile uint32_t *)&shm[IPC_READ_OFFSET];
    volatile uint32_t *wi = (volatile uint32_t *)&shm[IPC_WRITE_OFFSET];
    int qsize = (TEST_FW_SHM_SIZE - IPC_OVH_SIZE)/IPC_BUF_SIZE;

    int msgCount = (*wi + qsize - *ri) % qsize;

    ASSERT_EQ(msgCount, 1);
    offset = *ri * IPC_BUF_SIZE + IPC_OVH_SIZE;
    int msgSize = *((uint32_t *) &shm[offset]);

    datap = &shm[offset + IPC_HDR_SIZE];
    fwlog::FWEvent ev;
    bool ok = ev.ParseFromArray((const void *)datap, msgSize);
    ASSERT_EQ(ok, true);

    ASSERT_EQ(ev.sipv4(), key.sip.v4_addr);
    ASSERT_EQ(ev.dipv4(), key.dip.v4_addr);
    ASSERT_EQ(ev.ipprot(), key.proto);
    ASSERT_EQ(ev.sport(), key.sport);
    ASSERT_EQ(ev.dport(), key.dport);
    ASSERT_EQ(ev.direction(), key.dir);

    // advance readIndex
    *ri = (*ri + 1) % qsize;
}

TEST_F(ipc_test, fw_log_api) {
    int ret;
    int ix;

    ret = ipc_logger::init();
    ASSERT_EQ(ret, 0);

    ipc_logger *il = ipc_logger::factory();
    ASSERT_NE(il, nullptr);

    // open shared mem
    int fd = shm_open(FWLOG_SHM, O_RDWR, 0666);
    ASSERT_GE(fd, 0);
    ret = ftruncate(fd, TEST_FW_SHM_SIZE);
    ASSERT_EQ(ret, 0);
    void *p = mmap(0, TEST_FW_SHM_SIZE, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
    ASSERT_NE(p, MAP_FAILED);

    // setup a test context
    fte::ctx_t ctx = {};
    hal::flow_key_t fk;

    for (ix = 0; ix < 10000; ix++) {
        fk.flow_type = hal::FLOW_TYPE_V4;
        fk.sip.v4_addr = 0x0A0A0101;
        fk.dip.v4_addr = 0x0A0A0201;
        fk.sport = 16000 + ix;
        fk.dport = 12018;
        fk.proto = (types::IPProtocol)IP_PROTO_TCP;
        ctx.set_key(fk);
        il->firew_log(&ctx, nwsec::FIREWALL_ACTION_DENY);
        verify_fw_log(fk, (volatile uint8_t*)p);
    }
}

/* used only for IT.
TEST_F(ipc_test, looper) {
    int ret;
    int ix;

    ret = ipc_logger::init();
    ASSERT_EQ(ret, 0);

    ipc_logger *il = ipc_logger::factory();
    ASSERT_NE(il, nullptr);

    // open shared mem
    int fd = shm_open(FWLOG_SHM, O_RDWR, 0666);
    ASSERT_GE(fd, 0);
    ret = ftruncate(fd, TEST_FW_SHM_SIZE);
    ASSERT_EQ(ret, 0);
    void *p = mmap(0, TEST_FW_SHM_SIZE, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
    ASSERT_NE(p, MAP_FAILED);

    // setup a test context
    fte::ctx_t ctx = {};
    hal::flow_key_t fk;

    for (ix = 0; ix < 10000; ix++) {
        fk.flow_type = hal::FLOW_TYPE_V4;
        fk.sip.v4_addr = 0x0A0A0101;
        fk.dip.v4_addr = 0x0A0A0201;
        fk.sport = 16000 + ix;
        fk.dport = 12018;
        fk.proto = (types::IPProtocol)IP_PROTO_TCP;
        ctx.set_key(fk);
        il->firew_log(&ctx, nwsec::FIREWALL_ACTION_DENY);
        if ((ix % 10) == 0) {
	    sleep(1);
        }
    }
}
*/
