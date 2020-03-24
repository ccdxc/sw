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

void *rsock;
void *esock;
void *__zmq_context;
const char* __lmodel_env = getenv("ASIC_MOCK_MODE");
const char* __write_verify_enable = getenv("CAPRI_WRITE_VERIFY_ENABLE");
std::mutex g_zmq_mutex;

int lib_model_connect ()
{
    char zmqsockstr[200];
    int rc;
    uint16_t event;
    int timeout_ms = MODEL_ZMQ_SOCK_TIMEOUT_SEC * 1000;

    if (__lmodel_env)
        return 0;

    __zmq_context = zmq_ctx_new ();
    printf ("Connecting to non-blocking ASIC model....\n");


    /* generate random identity */
    unsigned int seed;
    char identity[10] = {};
    /* Set watermark - 10mil messages */
    int wmark = 10000000;

    FILE* urandom = fopen("/dev/urandom", "r");
    fread(&seed, sizeof(int), 1, urandom);
    fclose(urandom);
    srand(seed);
    snprintf(identity, 10, "%04X-%04X", (rand() % 0x10000), (rand() % 0x10000));
    /* Open the dealer socket */
    const char* user_str = std::getenv("PWD");
    snprintf(zmqsockstr, 200, "ipc:///%s/zmqsock1", user_str);
    rsock = zmq_socket (__zmq_context, ZMQ_DEALER);
    rc = zmq_setsockopt (esock, ZMQ_RCVHWM, &wmark, sizeof(int));
    rc = zmq_setsockopt (esock, ZMQ_SNDHWM, &wmark, sizeof(int));
    rc = zmq_setsockopt (rsock, ZMQ_IDENTITY, identity, strlen(identity));
    rc = zmq_setsockopt (rsock, ZMQ_RCVTIMEO, &timeout_ms, sizeof(timeout_ms));
    rc = zmq_setsockopt (rsock, ZMQ_SNDTIMEO, &timeout_ms, sizeof(timeout_ms));
    rc = zmq_connect ((rsock), zmqsockstr);
    assert(rc == 0);

    /* Open the subscriber socket */
    snprintf(zmqsockstr, 200, "ipc:///%s/zmqsock2", user_str);
    esock = zmq_socket (__zmq_context, ZMQ_SUB);
    rc = zmq_setsockopt (esock, ZMQ_RCVTIMEO, &timeout_ms, sizeof(timeout_ms));
    rc = zmq_setsockopt (esock, ZMQ_SNDTIMEO, &timeout_ms, sizeof(timeout_ms));
    rc = zmq_setsockopt (esock, ZMQ_RCVHWM, &wmark, sizeof(int));
    rc = zmq_setsockopt(esock, ZMQ_SUBSCRIBE, "", 0);
    rc = zmq_connect ((esock), zmqsockstr);
    assert(rc == 0);

    /* Monitor the sub socket for the model to connect */
    rc = zmq_socket_monitor (esock, "inproc://monitor.sock", ZMQ_EVENT_ALL);
    assert (rc == 0);
    void *s = zmq_socket(__zmq_context, ZMQ_PAIR);
    assert(s);
    rc = zmq_connect(s, "inproc://monitor.sock");
    assert(rc == 0);
    printf ("Waiting for non-blocking ASIC model server to come up...\n");
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
    printf ("ASIC model server connected!\n");
    zmq_close (s);

    return (rc);
}


int lib_model_conn_close ()
{
    // thread safe
    std::lock_guard<std::mutex> lock(g_zmq_mutex);
    if (__lmodel_env)
        return 0;

    zmq_close(rsock);
    zmq_close(esock);
    zmq_ctx_destroy(__zmq_context);
    return 0;
}

void step_tmr_wheel_update (uint32_t slowfast, uint32_t ctime)
{
    // thread safe
    std::lock_guard<std::mutex> lock(g_zmq_mutex);

    char buffer[MODEL_ZMQ_BUFF_SIZE] = {0};
    int rc;
    buffer_hdr_t *buff;
    buff = (buffer_hdr_t *) buffer;
    buff->type = BUFF_TYPE_STEP_TIMER_WHEEL;
    buff->slowfast = slowfast;
    buff->ctime = ctime;

    if (__lmodel_env)
        return;
    rc = zmq_send(rsock, buffer, MODEL_ZMQ_BUFF_SIZE, 0);
    assert(rc != -1);
    return;
}

void step_network_pkt (const std::vector<uint8_t> & pkt, uint32_t port)
{
    // thread safe
    std::lock_guard<std::mutex> lock(g_zmq_mutex);

    char buffer[MODEL_ZMQ_BUFF_SIZE] = {0};
    int rc;
    buffer_hdr_t *buff;
    buff = (buffer_hdr_t *) buffer;
    buff->type = BUFF_TYPE_STEP_PKT;
    buff->port = port;
    buff->size = pkt.size();

    if (__lmodel_env)
        return;
    if (pkt.size() > MODEL_ZMQ_BUFF_SIZE)
        assert(0);
    memcpy(buff->data, pkt.data(), buff->size);
    rc = zmq_send(rsock, buffer, MODEL_ZMQ_BUFF_SIZE, 0);
    assert(rc != -1);
    return;
}

void step_cpu_pkt (const uint8_t* pkt, size_t pkt_len)
{
    // thread safe
    std::lock_guard<std::mutex> lock(g_zmq_mutex);

    char buffer[MODEL_ZMQ_BUFF_SIZE] = {0};
    int rc;
    buffer_hdr_t *buff;
    buff = (buffer_hdr_t *) buffer;
    buff->type = BUFF_TYPE_STEP_CPU_PKT;
    buff->size = pkt_len;

    if (__lmodel_env)
        return;
    if (buff->size > MODEL_ZMQ_BUFF_SIZE)
        assert(0);
    memcpy(buff->data, pkt, buff->size);
    rc = zmq_send(rsock, buffer, MODEL_ZMQ_BUFF_SIZE, 0);
    assert(rc != -1);
    return;
}

bool get_next_pkt (std::vector<uint8_t> &pkt, uint32_t &port, uint32_t& cos)
{
     // thread safe
    std::lock_guard<std::mutex> lock(g_zmq_mutex);

    char buffer[MODEL_ZMQ_BUFF_SIZE] = {0};
    buffer_hdr_t *buff;
    int rc;

    if (__lmodel_env)
        return true;
    buff = (buffer_hdr_t *) buffer;
    buff->type = BUFF_TYPE_GET_NEXT_PKT;
    rc = zmq_recv(esock, buffer, MODEL_ZMQ_BUFF_SIZE, 0);
    assert(rc != -1);
    port = buff->port;
    cos = buff->cos;
    pkt.resize(buff->size);
    memcpy(pkt.data(), buff->data, buff->size);
    return true;
}


bool read_reg (uint64_t addr, uint32_t& data)
{
     // thread safe
    std::lock_guard<std::mutex> lock(g_zmq_mutex);

    char sbuffer[MODEL_ZMQ_BUFF_SIZE] = {0};
    char rbuffer[MODEL_ZMQ_BUFF_SIZE] = {0};
    buffer_hdr_t *buff;
    zmq_msg_t msg;
    zmq_msg_init(&msg);
    int rc;

    if (__lmodel_env)
        return true;
    buff = (buffer_hdr_t *) sbuffer;
    buff->type = BUFF_TYPE_REG_READ;
    buff->addr = addr;
    buff->size = sizeof(uint32_t);
    rc = zmq_send(rsock, sbuffer, MODEL_ZMQ_BUFF_SIZE, 0);
    assert(rc != -1);
    rc = zmq_recv(rsock, rbuffer, MODEL_ZMQ_BUFF_SIZE, 0);
    assert(rc != -1);
    buff = (buffer_hdr_t *) rbuffer;
    memcpy(&data, buff->data, sizeof(uint32_t));
    return true;
}


bool write_reg (uint64_t addr, uint32_t data)
{
     // thread safe
    std::lock_guard<std::mutex> lock(g_zmq_mutex);

    char buffer[MODEL_ZMQ_BUFF_SIZE] = {0};
    buffer_hdr_t *buff;
    int rc;

    if (__lmodel_env)
        return true;
    buff = (buffer_hdr_t *) buffer;
    buff->type = BUFF_TYPE_REG_WRITE;
    buff->addr = addr;
    buff->size = sizeof(uint32_t);
    memcpy(buff->data, &data, sizeof(uint32_t));
    rc = zmq_send(rsock, buffer, MODEL_ZMQ_BUFF_SIZE, 0);
    assert(rc != -1);

    return true;
}


bool read_mem (uint64_t addr, uint8_t * data, uint32_t size)
{
     // thread safe
    std::lock_guard<std::mutex> lock(g_zmq_mutex);

    char sbuffer[MODEL_ZMQ_MEM_BUFF_SIZE] = {0};
    char rbuffer[MODEL_ZMQ_MEM_BUFF_SIZE] = {0};
    buffer_hdr_t *buff;
    zmq_msg_t msg;
    zmq_msg_init(&msg);
    int rc;

    if (__lmodel_env)
        return true;
    if (size > MODEL_ZMQ_MEM_BUFF_SIZE)
        assert(0);
    buff = (buffer_hdr_t *) sbuffer;
    buff->type = BUFF_TYPE_MEM_READ;
    buff->addr = addr;
    buff->size = size;
    rc = zmq_send(rsock, sbuffer, MODEL_ZMQ_MEM_BUFF_SIZE, 0);
    assert(rc != -1);
    rc = zmq_recv(rsock, rbuffer, MODEL_ZMQ_MEM_BUFF_SIZE, 0);
    assert(rc != -1);
    buff = (buffer_hdr_t *) rbuffer;
    memcpy(data, buff->data, buff->size);
    return true;
}


bool write_mem (uint64_t addr, uint8_t * data, uint32_t size)
{
     // thread safe
    std::lock_guard<std::mutex> lock(g_zmq_mutex);

    char buffer[MODEL_ZMQ_MEM_BUFF_SIZE] = {0};
    buffer_hdr_t *buff;
    int rc;

    if (__lmodel_env)
        return true;
    buff = (buffer_hdr_t *) buffer;
    buff->type = BUFF_TYPE_MEM_WRITE;
    buff->addr = addr;
    buff->size = size;
    memcpy(buff->data, data, size);
    rc = zmq_send(rsock, buffer, MODEL_ZMQ_MEM_BUFF_SIZE, 0);
    assert(rc != -1);

    if (!__write_verify_enable)
        return true;

    uint8_t obuff[MODEL_ZMQ_MEM_BUFF_SIZE] = {0};
    read_mem(addr, obuff, size);
    if (memcmp(obuff, data, size))
        assert(0);

    return true;
}

void step_doorbell (uint64_t addr, uint64_t data)
{
     // thread safe
    std::lock_guard<std::mutex> lock(g_zmq_mutex);

    char buffer[MODEL_ZMQ_BUFF_SIZE] = {0};
    buffer_hdr_t *buff;
    int rc;

    buff = (buffer_hdr_t *) buffer;
    buff->type = BUFF_TYPE_DOORBELL;
    buff->size = sizeof(uint64_t);

    if (__lmodel_env)
        return;
    buff->addr = addr;
    memcpy(buff->data, &data, buff->size);
    rc = zmq_send(rsock, buffer, MODEL_ZMQ_BUFF_SIZE, 0);
    assert(rc != -1);
    //rc = zmq_recv(rsock, buffer, MODEL_ZMQ_BUFF_SIZE, 0);
    //assert(rc != -1);
    return;
}


bool dump_hbm ()
{
     // thread safe
    std::lock_guard<std::mutex> lock(g_zmq_mutex);

    char buffer[1024] = {0};
    buffer_hdr_t *buff;
    int rc;

    if (__lmodel_env)
        return true;
    buff = (buffer_hdr_t *) buffer;
    buff->type = BUFF_TYPE_HBM_DUMP;
    rc = zmq_send(rsock, buffer, 1024, 0);
    assert(rc != -1);
    return true;
}
