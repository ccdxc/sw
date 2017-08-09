#include <zmq.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <assert.h>
#include "buf_hdr.h"
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <cstdlib>
#include <assert.h>
#include <string.h>
#include <iostream>
#include "lib_model_client.h"
#include <stdint.h>

void *__zmq_sock;
void *__zmq_context;
const char* __lmodel_env = getenv("CAPRI_MOCK_MODE");
const char* __write_verify_disable = getenv("CAPRI_WRITE_VERIFY_DISABLE");

int lib_model_connect ()
{
    struct stat st = {0};
    char tmpdir[100];
    char zmqsockstr[100];
    uint16_t event;

    if (__lmodel_env)
        return 0;
    
    printf ("Connecting to ASIC model....\n");
    const char* user_str = std::getenv("PWD");
    snprintf(zmqsockstr, 100, "ipc:///%s/zmqsock", user_str);
    __zmq_context = zmq_ctx_new ();
    __zmq_sock = zmq_socket (__zmq_context, ZMQ_REQ);
    int rc = zmq_connect ((__zmq_sock), zmqsockstr);
    assert(rc == 0);
    
    /* Monitor the socket for the model to connect */
    rc = zmq_socket_monitor (__zmq_sock, "inproc://monitor.sock", ZMQ_EVENT_ALL);
    assert (rc == 0);
    void *s = zmq_socket(__zmq_context, ZMQ_PAIR);
    assert(s);
    rc = zmq_connect(s, "inproc://monitor.sock");
    assert(rc == 0);
    printf ("Waiting for ASIC model to come up...\n");
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


int lib_model_conn_close ()
{
    if (__lmodel_env)
        return 0;
    zmq_close(__zmq_sock);
    zmq_ctx_destroy(__zmq_context);
    return 0;
}


void step_network_pkt (const std::vector<uint8_t> & pkt, uint32_t port)
{
    char buffer[4096] = {0};
    buffer_hdr_t *buff;
    buff = (buffer_hdr_t *) buffer;
    buff->type = BUFF_TYPE_STEP_PKT;
    buff->port = port;
    buff->size = pkt.size();

    if (__lmodel_env)
        return;
    if (pkt.size() > 4000)
        assert(0);
    memcpy(buff->data, pkt.data(), buff->size);
    zmq_send(__zmq_sock, buffer, 4096, 0);
    zmq_recv(__zmq_sock, buffer, 4096, 0);
    return;
}


bool get_next_pkt (std::vector<uint8_t> &pkt, uint32_t &port, uint32_t& cos)
{
    char buffer[4096] = {0};
    buffer_hdr_t *buff;

    if (__lmodel_env)
        return true;
    buff = (buffer_hdr_t *) buffer;
    buff->type = BUFF_TYPE_GET_NEXT_PKT;
    zmq_send(__zmq_sock, buffer, 4096, 0);
    zmq_recv(__zmq_sock, buffer, 4096, 0);
    port = buff->port;
    cos = buff->cos;
    pkt.resize(buff->size);
    memcpy(pkt.data(), buff->data, buff->size);
    return true;
}


bool read_reg (uint64_t addr, uint32_t& data)
{
    char buffer[4096] = {0};
    buffer_hdr_t *buff;

    if (__lmodel_env)
        return true;
    buff = (buffer_hdr_t *) buffer;
    buff->type = BUFF_TYPE_REG_READ;
    buff->addr = addr;
    buff->size = sizeof(uint32_t);
    zmq_send(__zmq_sock, buffer, 4096, 0);
    zmq_recv(__zmq_sock, buffer, 4096, 0);
    memcpy(&data, buff->data, sizeof(uint32_t));
    return true;
}


bool write_reg (uint64_t addr, uint32_t data)
{
    char buffer[4096] = {0};
    buffer_hdr_t *buff;

    if (__lmodel_env)
        return true;
    buff = (buffer_hdr_t *) buffer;
    buff->type = BUFF_TYPE_REG_WRITE;
    buff->addr = addr;
    buff->size = sizeof(uint32_t);
    memcpy(buff->data, &data, sizeof(uint32_t));
    zmq_send(__zmq_sock, buffer, 4096, 0);
    zmq_recv(__zmq_sock, buffer, 4096, 0);

    return true;
}


bool read_mem (uint64_t addr, uint8_t * data, uint32_t size)
{
    char buffer[4096] = {0};
    buffer_hdr_t *buff;

    if (__lmodel_env)
        return true;
    if (size > 4096)
        assert(0);
    buff = (buffer_hdr_t *) buffer;
    buff->type = BUFF_TYPE_MEM_READ;
    buff->addr = addr;
    buff->size = size;
    zmq_send(__zmq_sock, buffer, 4096, 0);
    zmq_recv(__zmq_sock, buffer, 4096, 0);
    memcpy(data, buff->data, size);
    return true;
}


bool write_mem (uint64_t addr, uint8_t * data, uint32_t size)
{
    char buffer[4096] = {0};
    buffer_hdr_t *buff;

    if (__lmodel_env)
        return true;
    buff = (buffer_hdr_t *) buffer;
    buff->type = BUFF_TYPE_MEM_WRITE;
    buff->addr = addr;
    buff->size = size;
    memcpy(buff->data, data, size);
    zmq_send(__zmq_sock, buffer, 4096, 0);
    zmq_recv(__zmq_sock, buffer, 4096, 0);

    if (__write_verify_disable)
        return true;

    uint8_t obuff[4096] = {0};
    read_mem(addr, obuff, size);
    if (memcmp(obuff, data, size))
        assert(0);

    return true;
}


bool dump_hbm ()
{
    char buffer[4096] = {0};
    buffer_hdr_t *buff;

    if (__lmodel_env)
        return true;
    buff = (buffer_hdr_t *) buffer;
    buff->type = BUFF_TYPE_HBM_DUMP;
    zmq_send(__zmq_sock, buffer, 4096, 0);
    return true;
}
