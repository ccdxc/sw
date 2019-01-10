#include <sys/mman.h>
#include <sys/stat.h>        /* For mode constants */
#include <fcntl.h>           /* For O_* constants */
#include <unistd.h>
#include <sys/types.h>
#include <gtest/gtest.h>
#include <stdio.h>
#include <iostream>
#include "nic/utils/agent_api/agent_api.hpp"
#include "nic/utils/agent_api/constants.h"
#include "nic/utils/ipc/constants.h"
#include "lib/thread/thread.hpp"
#include "gen/proto/nwsec.pb.h"
#include "gen/proto/fwlog.pb.h"
#define TEST_THREAD_COUNT 8
#define TEST_ALLOC_SIZE 8
#define TEST_IPC_SIZE 256 
#define EXP_BUF_COUNT (TEST_IPC_SIZE/TEST_ALLOC_SIZE - 1)
#define FWLOG_SHM "/fwlog_ipc_shm"
#define TEST_SHM_SIZE 2048
#define TEST_SHM_BUF_SIZE 256
#define TEST_QSIZE 7
#define TEST_FW_SHM_SIZE 64*1024

#define LOG_SIZE(ev) ev.ByteSizeLong()
#define TYPE_TO_LG_SZ(type, sz_) {                                    \
    if (type == IPC_LOG_TYPE_FW) {                                    \
       fwlog::FWEvent ev; sz_ = (LOG_SIZE(ev) + IPC_HDR_SIZE);        \
    } else {                                                          \
       sz_ = IPC_BUF_SIZE;                                            \
    }                                                                 \
}

typedef struct testInfo_ {
    ipc *ipcUT;
    int bufCount;
    int tryCount;
} testInfo_t;

class ipc_test : public ::testing::Test {
protected:
    Logger logger;
    const char* test_case_name;

    ipc_test() {
    }

    virtual ~ipc_test() {
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

    shm *shm_inst = shm::setup_shm(FWLOG_SHM, TEST_SHM_SIZE, 1, TEST_SHM_BUF_SIZE, logger);
    ASSERT_NE(shm_inst, nullptr);
    ipc *ipc1 = shm_inst->factory();
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

    shm_inst->tear_down_shm();
}

// Multi-instance Test
// Init ipc
// Verify operation with three instances
TEST_F(ipc_test, multi) {
    int ix;
    ipc *ipc_inst[3];

    shm *shm_inst = shm::setup_shm(FWLOG_SHM, TEST_SHM_SIZE*3, 3, TEST_SHM_BUF_SIZE, logger);
    ASSERT_NE(shm_inst, nullptr);
    for (ix = 0; ix < 3; ix++) {
        ipc_inst[ix] = shm_inst->factory();
        ASSERT_NE(ipc_inst[ix], nullptr);
    }
    // next instance should fail.
    ipc *ipc1 = shm_inst->factory();
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
    shm_inst->tear_down_shm();
}

void verify_fw_log(fwlog::FWEvent fwlog, volatile uint8_t *shm) {
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

    ASSERT_EQ(ev.sipv4(), fwlog.sipv4());
    ASSERT_EQ(ev.dipv4(), fwlog.dipv4());
    ASSERT_EQ(ev.ipprot(), fwlog.ipprot());
    ASSERT_EQ(ev.sport(), fwlog.sport());
    ASSERT_EQ(ev.dport(), fwlog.dport());
    ASSERT_EQ(ev.direction(), fwlog.direction());

    // advance readIndex
    *ri = (*ri + 1) % qsize;
}

TEST_F(ipc_test, fw_log_api) {
    int ret;
    int ix;

    ret = ipc_logger::init(logger);
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
    fwlog::FWEvent fk;

    for (ix = 0; ix < 10000; ix++) {
        fk.set_sipv4(0x0A0A0101);
        fk.set_dipv4(0x0A0A0201);
        fk.set_sport(16000 + ix);
        fk.set_dport(12018);
        fk.set_ipprot(types::IPProtocol::IPPROTO_TCP);
        fk.set_fwaction(nwsec::SECURITY_RULE_ACTION_DENY);
        uint8_t *buf = il->get_buffer(LOG_SIZE(fk));
        if (buf != NULL) { 
            if (!fk.SerializeToArray(buf, LOG_SIZE(fk))) {
                return;
            }

            int size = fk.ByteSizeLong();
            il->write_buffer(buf, size);
        }
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
        il->fw_log(&ctx, nwsec::FIREWALL_ACTION_DENY);
        if ((ix % 10) == 0) {
	    sleep(1);
        }
    }
}
*/
