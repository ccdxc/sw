#include <zmq.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <assert.h>
#include "model_sim/include/buf_hdr.h"
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <cstdlib>
#include <string.h>
#include <iostream>
#include <errno.h>
#include "model_sim/include/lib_model_client.h"
#include <stdint.h>
#include <mutex>

#define MODEL_ZMQ_SOCK_TIMEOUT_SEC      300

static thread_local void *__zmq_sock;
static thread_local void *__zmq_context;
const char* __lmodel_env = getenv("CAPRI_MOCK_MODE");
const char* __lmodel_mock_memory_mode = getenv("CAPRI_MOCK_MEMORY_MODE");
const char* __write_verify_enable = getenv("CAPRI_WRITE_VERIFY_ENABLE");
std::mutex g_zmq_mutex;

uint64_t __mock_mem_base = 0xC0000000ul;
uint64_t __mock_mem_size = 8589934592ull;
uint8_t *__mock_mem;

int mock_memory_init()
{
    __mock_mem = (uint8_t *)malloc(__mock_mem_size);
    memset(__mock_mem, 0, sizeof(__mock_mem_size));
    return 0;
}

bool mock_memory_write (uint64_t addr, uint8_t * data, uint32_t size)
{
    uint64_t offset = 0;
    if (!__mock_mem) mock_memory_init();
    offset = addr - __mock_mem_base;
    if (offset > __mock_mem_size) {
        assert(0);
    }
    //printf("mock_memory_write: addr:%p size:%d\n",
    //       __mock_mem + offset, size);
    memcpy(__mock_mem + offset, data, size);
    return true;
}

bool mock_memory_read (uint64_t addr, uint8_t * data, uint32_t size)
{
    uint64_t offset = 0;
    if (!__mock_mem) mock_memory_init();
    offset = addr - __mock_mem_base;
    if (offset > __mock_mem_size) {
        assert(0);
    }
    //printf("mock_memory_read: addr:%p size:%d\n",
    //       __mock_mem + offset, size);
    memcpy(data, __mock_mem + offset, size);
    return true;
}

void* mock_memory_map (uint64_t addr, uint32_t size)
{
    uint64_t offset = 0;
    if (!__mock_mem) mock_memory_init();
    if (addr >= __mock_mem_base) {
        // Valid only HBM addresses
        offset = addr - __mock_mem_base;
        if (offset > __mock_mem_size) {
            assert(0);
        }
        //printf("mock_memory_map: addr:%p size:%d\n",
        //       __mock_mem + offset, size);
        return (void *)(__mock_mem + offset);
    }
    return NULL;
}

int lib_model_connect ()
{
    char        *zmqsockstr;
    int         rc;
    uint16_t    event;
    int         timeout_ms = MODEL_ZMQ_SOCK_TIMEOUT_SEC * 1000;
    
    if (__lmodel_mock_memory_mode) {
        return mock_memory_init();
    }

    if (__lmodel_env) {
        return 0;
    }

    if (getenv("MODEL_TIMEOUT")) {
        printf("Setting MODEL_TIMEOUT to %s\n", getenv("MODEL_TIMEOUT"));
        timeout_ms = atoi(getenv("MODEL_TIMEOUT")) * 1000;
    }

    printf ("Connecting to ASIC model....\n");

    zmqsockstr = model_utils_get_zmqsockstr();
    __zmq_context = zmq_ctx_new ();
    __zmq_sock = zmq_socket (__zmq_context, ZMQ_REQ);
    rc = zmq_setsockopt (__zmq_sock, ZMQ_RCVTIMEO, &timeout_ms, sizeof(timeout_ms));
    rc = zmq_setsockopt (__zmq_sock, ZMQ_SNDTIMEO, &timeout_ms, sizeof(timeout_ms));
    int val = 1;
    rc = zmq_setsockopt (__zmq_sock, ZMQ_REQ_CORRELATE, &val, sizeof(int));
    rc = zmq_setsockopt (__zmq_sock, ZMQ_REQ_RELAXED, &val, sizeof(int));
    
    /* Monitor the socket for the model to connect */
    rc = zmq_socket_monitor (__zmq_sock, "inproc://monitor.sock", ZMQ_EVENT_ALL);
    assert (rc == 0);
    void *s = zmq_socket(__zmq_context, ZMQ_PAIR);
    assert(s);
    rc = zmq_connect(s, "inproc://monitor.sock");
    assert(rc == 0);
    printf ("Waiting for ASIC model to come up...\n");
    rc = zmq_connect ((__zmq_sock), zmqsockstr);
    assert(rc == 0);
    while (true) {
        bool connected = false;
        zmq_msg_t msg;
        zmq_msg_init (&msg);
        rc = zmq_recvmsg (s, &msg, 0);
        if (rc == -1 && zmq_errno() == ETERM) break;
        assert (rc != -1);
        memcpy (&event, zmq_msg_data (&msg), sizeof (event));
        switch (event) {
            case ZMQ_EVENT_CONNECTED:
                connected = true;
                break;
        }
        zmq_msg_close (&msg);
        if (connected)
            break;
    }
    printf ("ASIC model connected!\n");
    zmq_close (s);
    return (rc);
}

#define SOCK_OP_GUARD { \
    if(!__zmq_sock) { \
        if (!(__lmodel_mock_memory_mode || __lmodel_env)) { \
            lib_model_connect(); \
        } \
    } \
    std::lock_guard<std::mutex> lock(g_zmq_mutex); \
}

int lib_model_conn_close ()
{
    // thread safe
    std::lock_guard<std::mutex> lock(g_zmq_mutex);
    if (__lmodel_env)
        return 0;

    zmq_close(__zmq_sock);
    zmq_ctx_destroy(__zmq_context);
    __zmq_sock = NULL;
    __zmq_context = NULL;
    return 0;
}

void step_tmr_wheel_update (uint32_t slowfast, uint32_t ctime)
{
    int rc;
    // thread safe
    SOCK_OP_GUARD

    char buffer[MODEL_ZMQ_BUFF_SIZE] = {0};
    buffer_hdr_t *buff;
    buff = (buffer_hdr_t *) buffer;
    buff->type = BUFF_TYPE_STEP_TIMER_WHEEL;
    buff->slowfast = slowfast;
    buff->ctime = ctime;

    if (__lmodel_env)
        return;
    rc = zmq_send(__zmq_sock, buffer, MODEL_ZMQ_BUFF_SIZE, 0);
    assert(rc != -1);
    rc = zmq_recv(__zmq_sock, buffer, MODEL_ZMQ_BUFF_SIZE, 0);
    assert(rc != -1);
    return;
}

void step_network_pkt (const std::vector<uint8_t> & pkt, uint32_t port)
{
    int rc;
    // thread safe
    SOCK_OP_GUARD

    char buffer[MODEL_ZMQ_BUFF_SIZE] = {0};
    buffer_hdr_t *buff;
    buff = (buffer_hdr_t *) buffer;
    buff->type = BUFF_TYPE_STEP_PKT;
    buff->port = port;
    buff->size = pkt.size();

    if (__lmodel_env)
        return;
    if (buff->size > MODEL_ZMQ_BUFF_SIZE)
        assert(0);
    memcpy(buff->data, pkt.data(), buff->size);
    rc = zmq_send(__zmq_sock, buffer, MODEL_ZMQ_BUFF_SIZE, 0);
    assert(rc != -1);
    rc = zmq_recv(__zmq_sock, buffer, MODEL_ZMQ_BUFF_SIZE, 0);
    assert(rc != -1);
    return;
}

void step_cpu_pkt (const uint8_t* pkt, size_t pkt_len)
{
    int rc;
    // thread safe
    SOCK_OP_GUARD

    char buffer[MODEL_ZMQ_BUFF_SIZE] = {0};
    buffer_hdr_t *buff;
    buff = (buffer_hdr_t *) buffer;
    buff->type = BUFF_TYPE_STEP_CPU_PKT;
    buff->size = pkt_len; 

    if (__lmodel_env)
        return;
    if (buff->size > MODEL_ZMQ_BUFF_SIZE)
        assert(0);
    memcpy(buff->data, pkt, buff->size);
    rc = zmq_send(__zmq_sock, buffer, MODEL_ZMQ_BUFF_SIZE, 0);
    assert(rc != -1);
    rc = zmq_recv(__zmq_sock, buffer, MODEL_ZMQ_BUFF_SIZE, 0);
    assert(rc != -1);
    return;
}

bool get_next_pkt (std::vector<uint8_t> &pkt, uint32_t &port, uint32_t& cos)
{
    int rc;
     // thread safe
    SOCK_OP_GUARD

    char buffer[MODEL_ZMQ_BUFF_SIZE] = {0};
    buffer_hdr_t *buff;

    if (__lmodel_env)
        return true;
    buff = (buffer_hdr_t *) buffer;
    buff->type = BUFF_TYPE_GET_NEXT_PKT;
    rc = zmq_send(__zmq_sock, buffer, MODEL_ZMQ_BUFF_SIZE, 0);
    assert(rc != -1);
    rc = zmq_recv(__zmq_sock, buffer, MODEL_ZMQ_BUFF_SIZE, 0);
    assert(rc != -1);
    port = buff->port;
    cos = buff->cos;
    pkt.resize(buff->size);
    memcpy(pkt.data(), buff->data, buff->size);
    return true;
}

bool read_reg (uint64_t addr, uint32_t& data)
{
    int rc;
    // thread safe
    SOCK_OP_GUARD

    char buffer[MODEL_ZMQ_BUFF_SIZE] = {0};
    buffer_hdr_t *buff;

    if (__lmodel_env)
        return true;
    buff = (buffer_hdr_t *) buffer;
    buff->type = BUFF_TYPE_REG_READ;
    buff->addr = addr;
    buff->size = sizeof(uint32_t);
    rc = zmq_send(__zmq_sock, buffer, MODEL_ZMQ_BUFF_SIZE, 0);
    assert(rc != -1);
    rc = zmq_recv(__zmq_sock, buffer, MODEL_ZMQ_BUFF_SIZE, 0);
    assert(rc != -1);
    memcpy(&data, buff->data, sizeof(uint32_t));
    return true;
}


bool write_reg (uint64_t addr, uint32_t data)
{
    int rc;
     // thread safe
    SOCK_OP_GUARD

    char buffer[MODEL_ZMQ_BUFF_SIZE] = {0};
    buffer_hdr_t *buff;

    if (__lmodel_env)
        return true;
    buff = (buffer_hdr_t *) buffer;
    buff->type = BUFF_TYPE_REG_WRITE;
    buff->addr = addr;
    buff->size = sizeof(uint32_t);
    memcpy(buff->data, &data, sizeof(uint32_t));
    rc = zmq_send(__zmq_sock, buffer, MODEL_ZMQ_BUFF_SIZE, 0);
    assert(rc != -1);
    rc = zmq_recv(__zmq_sock, buffer, MODEL_ZMQ_BUFF_SIZE, 0);
    assert(rc != -1);

    return true;
}

void* mem_map (uint64_t addr, uint32_t size)
{
    if (__lmodel_mock_memory_mode) {
        return mock_memory_map(addr, size);
    }
    return NULL;
}


bool read_mem (uint64_t addr, uint8_t * data, uint32_t size)
{
    int rc;
     // thread safe
    SOCK_OP_GUARD

    char buffer[MODEL_ZMQ_MEM_BUFF_SIZE] = {0};
    buffer_hdr_t *buff;
    
    if (__lmodel_mock_memory_mode) {
        return mock_memory_read(addr, data, size);
    }

    if (__lmodel_env) {
        return true;
    }
    if (size > MODEL_ZMQ_MEM_BUFF_SIZE)
        assert(0);
    buff = (buffer_hdr_t *) buffer;
    buff->type = BUFF_TYPE_MEM_READ;
    buff->addr = addr;
    buff->size = size;
    rc = zmq_send(__zmq_sock, buffer, MODEL_ZMQ_MEM_BUFF_SIZE, 0);
    assert(rc != -1);
    rc = zmq_recv(__zmq_sock, buffer, MODEL_ZMQ_MEM_BUFF_SIZE, 0);
    assert(rc != -1);
    memcpy(data, buff->data, size);
    return true;
}

bool write_mem_pcie (uint64_t addr, uint8_t * data, uint32_t size)
{
    int rc;
     // thread safe
    SOCK_OP_GUARD

    char buffer[MODEL_ZMQ_MEM_BUFF_SIZE] = {0};
    buffer_hdr_t *buff;
    
    if (__lmodel_mock_memory_mode) {
        return mock_memory_write(addr, data, size);
    }

    if (__lmodel_env) {
        return true;
    }
    buff = (buffer_hdr_t *) buffer;
    buff->type = BUFF_TYPE_MEM_WRITE_PCIE;
    buff->addr = addr;
    buff->size = size;
    memcpy(buff->data, data, size);
    rc = zmq_send(__zmq_sock, buffer, MODEL_ZMQ_MEM_BUFF_SIZE, 0);
    assert(rc != -1);
    rc = zmq_recv(__zmq_sock, buffer, MODEL_ZMQ_MEM_BUFF_SIZE, 0);
    assert(rc != -1);

    if (!__write_verify_enable)
        return true;

    uint8_t obuff[MODEL_ZMQ_MEM_BUFF_SIZE] = {0};
    read_mem(addr, obuff, size);
    if (memcmp(obuff, data, size))
        assert(0);

    return true;
}



bool write_mem (uint64_t addr, uint8_t * data, uint32_t size)
{
    int rc;
     // thread safe
    SOCK_OP_GUARD

    char buffer[MODEL_ZMQ_MEM_BUFF_SIZE] = {0};
    buffer_hdr_t *buff;

    if (__lmodel_mock_memory_mode) {
        return mock_memory_write(addr, data, size);
    }

    if (__lmodel_env)
        return true;
    buff = (buffer_hdr_t *) buffer;
    do {
        auto tmp_size = size < MODEL_ZMQ_MEM_BUFF_SIZE-offsetof(buffer_hdr_t,data) ?
                               size : MODEL_ZMQ_MEM_BUFF_SIZE-offsetof(buffer_hdr_t,data);
        buff->type = BUFF_TYPE_MEM_WRITE;
        buff->addr = addr;
        buff->size = tmp_size;
        memcpy(buff->data, data, tmp_size);
        rc = zmq_send(__zmq_sock, buffer, MODEL_ZMQ_MEM_BUFF_SIZE, 0);
        assert(rc != -1);
        rc = zmq_recv(__zmq_sock, buffer, MODEL_ZMQ_MEM_BUFF_SIZE, 0);
        assert(rc != -1);

        if (__write_verify_enable) {
            uint8_t obuff[MODEL_ZMQ_MEM_BUFF_SIZE] = {0};
            read_mem(addr, obuff, tmp_size);
            if (memcmp(obuff, data, tmp_size))
                assert(0);
        }
        size -= tmp_size;
        data += tmp_size;
        addr += tmp_size;
    } while (size);
    return true;
}

void step_doorbell (uint64_t addr, uint64_t data)
{
    int rc;
     // thread safe
    SOCK_OP_GUARD

    char buffer[MODEL_ZMQ_BUFF_SIZE] = {0};
    buffer_hdr_t *buff;

    buff = (buffer_hdr_t *) buffer;
    buff->type = BUFF_TYPE_DOORBELL;
    buff->size = sizeof(uint64_t);

    if (__lmodel_env)
        return;
    buff->addr = addr;
    memcpy(buff->data, &data, buff->size);
    rc = zmq_send(__zmq_sock, buffer, MODEL_ZMQ_BUFF_SIZE, 0);
    assert(rc != -1);
    rc = zmq_recv(__zmq_sock, buffer, MODEL_ZMQ_BUFF_SIZE, 0);
    while ((rc == -1) && (errno == EINTR)) {
      rc = zmq_recv(__zmq_sock, buffer, MODEL_ZMQ_BUFF_SIZE, 0);
    }
    assert(rc != -1);
    return;
}


bool dump_hbm ()
{
    int rc;
     // thread safe
    std::lock_guard<std::mutex> lock(g_zmq_mutex);

    char buffer[1024] = {0};
    buffer_hdr_t *buff;

    if (__lmodel_env)
        return true;
    buff = (buffer_hdr_t *) buffer;
    buff->type = BUFF_TYPE_HBM_DUMP;
    rc = zmq_send(__zmq_sock, buffer, 1024, 0);
    assert(rc != -1);
    return true;
}

int lib_model_mac_msg_send (uint32_t port_num,
                            uint32_t speed,
                            uint32_t type,
                            uint32_t num_lanes,
                            uint32_t val)
{
    int rc = 0;
    // thread safe
    SOCK_OP_GUARD

    char buffer[MODEL_ZMQ_BUFF_SIZE] = {0};
    buffer_hdr_t *buff;

    if (__lmodel_env)
        return true;
    buff = (buffer_hdr_t *) buffer;
    buff->type = (buff_type_e)type;
    buff->port = port_num;

    buff->size = sizeof(buffer_port_t);

    buffer_port_t *port_buf = (buffer_port_t*)&buff->data;
    port_buf->speed = speed;
    port_buf->num_lanes = num_lanes;
    port_buf->val = val;

    rc = zmq_send(__zmq_sock, buffer, MODEL_ZMQ_BUFF_SIZE, 0);
    assert(rc != -1);
    rc = zmq_recv(__zmq_sock, buffer, MODEL_ZMQ_BUFF_SIZE, 0);
    assert(rc != -1);

    return rc;
}

void register_mem_addr(uint64_t addr)
{
    int rc;
    // thread safe
    SOCK_OP_GUARD

    char buffer[MODEL_ZMQ_BUFF_SIZE] = {0};
    buffer_hdr_t *buff;

    buff = (buffer_hdr_t *) buffer;
    buff->type = BUFF_TYPE_REGISTER_MEM_ADDR;

    if (__lmodel_env)
        return;
    buff->addr = addr;
    rc = zmq_send(__zmq_sock, buffer, MODEL_ZMQ_BUFF_SIZE, 0);
    assert(rc != -1);
    rc = zmq_recv(__zmq_sock, buffer, MODEL_ZMQ_BUFF_SIZE, 0);
    assert(rc != -1);
    return;
}

void exit_simulation()
{
    int rc;
    // thread safe
    SOCK_OP_GUARD

    char buffer[MODEL_ZMQ_BUFF_SIZE] = {0};
    buffer_hdr_t *buff;

    buff = (buffer_hdr_t *) buffer;
    buff->type = BUFF_TYPE_EXIT_SIM;

    if (__lmodel_env)
        return;
    rc = zmq_send(__zmq_sock, buffer, MODEL_ZMQ_BUFF_SIZE, 0);
    assert(rc != -1);
    rc = zmq_recv(__zmq_sock, buffer, MODEL_ZMQ_BUFF_SIZE, 0);
    assert(rc != -1);
    return;
}

void config_done()
{
    int rc;
    // thread safe
    SOCK_OP_GUARD

    char buffer[MODEL_ZMQ_BUFF_SIZE] = {0};
    buffer_hdr_t *buff;

    buff = (buffer_hdr_t *) buffer;
    buff->type = BUFF_TYPE_CONFIG_DONE;

    if (__lmodel_env)
        return;
    rc = zmq_send(__zmq_sock, buffer, MODEL_ZMQ_BUFF_SIZE, 0);
    assert(rc != -1);
    rc = zmq_recv(__zmq_sock, buffer, MODEL_ZMQ_BUFF_SIZE, 0);
    assert(rc != -1);
    return;
}

void testcase_begin(int tcid, int loopid)
{
    int rc;
    // thread safe
    SOCK_OP_GUARD

    char buffer[MODEL_ZMQ_BUFF_SIZE] = {0};
    buffer_hdr_t *buff;

    buff = (buffer_hdr_t *) buffer;
    buff->type = BUFF_TYPE_TESTCASE_BEGIN;
    buff->size = tcid;
    buff->loopid = loopid;

    if (__lmodel_env)
        return;
    rc = zmq_send(__zmq_sock, buffer, MODEL_ZMQ_BUFF_SIZE, 0);
    assert(rc != -1);
    rc = zmq_recv(__zmq_sock, buffer, MODEL_ZMQ_BUFF_SIZE, 0);
    assert(rc != -1);
    return;
}

void testcase_end(int tcid, int loopid)
{
    int rc;
    // thread safe
    SOCK_OP_GUARD

    char buffer[MODEL_ZMQ_BUFF_SIZE] = {0};
    buffer_hdr_t *buff;

    buff = (buffer_hdr_t *) buffer;
    buff->type = BUFF_TYPE_TESTCASE_END;
    buff->tcid = tcid;
    buff->loopid = loopid;

    if (__lmodel_env)
        return;
    rc = zmq_send(__zmq_sock, buffer, MODEL_ZMQ_BUFF_SIZE, 0);
    assert(rc != -1);
    rc = zmq_recv(__zmq_sock, buffer, MODEL_ZMQ_BUFF_SIZE, 0);
    assert(rc != -1);
    return;
}

void eos_ignore_addr(uint64_t addr, uint32_t size)
{
    int rc;
    // thread safe
    SOCK_OP_GUARD

    char buffer[MODEL_ZMQ_BUFF_SIZE] = {0};
    buffer_hdr_t *buff;

    buff = (buffer_hdr_t *) buffer;
    buff->type = BUFF_TYPE_EOS_IGNORE_ADDR;
    buff->size = size;
    buff->addr = addr;

    if (__lmodel_env)
        return;
    rc = zmq_send(__zmq_sock, buffer, MODEL_ZMQ_BUFF_SIZE, 0);
    assert(rc != -1);
    rc = zmq_recv(__zmq_sock, buffer, MODEL_ZMQ_BUFF_SIZE, 0);
    assert(rc != -1);
    return;
}
