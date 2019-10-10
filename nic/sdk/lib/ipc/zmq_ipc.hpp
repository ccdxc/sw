//------------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//------------------------------------------------------------------------------

#ifndef __SDK_ZMQ_IPC_H__
#define __SDK_ZMQ_IPC_H__

#include <memory>
#include <vector>

#include <stddef.h>
#include <stdint.h>
#include <zmq.h>

namespace sdk {
namespace lib {
namespace ipc {

#define IPC_MAX_ID 63

typedef struct zmq_ipc_msg_preamble {
    uint32_t sender;
    uint32_t recipient;
    uint32_t serial;
    bool     is_pointer;
    size_t   real_length;
} zmq_ipc_msg_preamble_t;

typedef struct zmq_ipc_active_connection_ {
    bool is_internal;
    void *zsocket;
} zmq_ipc_active_connection_t;

class zmq_ipc_msg : public ipc_msg {
public:
    virtual void *data(void);
    virtual size_t length(void);
    std::vector<std::shared_ptr<zmq_ipc_msg> > &headers(void);
    uint32_t sender(void);
    zmq_ipc_msg();
    ~zmq_ipc_msg();
    zmq_msg_t *zmsg(void);
    void add_header(std::shared_ptr<zmq_ipc_msg> header);
    void set_sender(uint32_t sender);
    zmq_ipc_msg_preamble_t *preamble(void);
    void set_preamble_received(bool received);
private:
    std::vector<std::shared_ptr<zmq_ipc_msg> > headers_;
    zmq_msg_t zmsg_;
    uint32_t sender_;
    zmq_ipc_msg_preamble_t preamble_;
    bool preamble_received_;
};
typedef std::shared_ptr<zmq_ipc_msg> zmq_ipc_msg_ptr;

class zmq_ipc_client : public ipc_client {
public:
    static zmq_ipc_client *factory(uint32_t id, bool bidirectional);
    static void destroy(zmq_ipc_client *client);
    static ipc_msg_ptr send_recv_once(uint32_t recipinent, const void *data,
                                      size_t data_length);
    virtual ipc_msg_ptr send_recv(uint32_t recipient, const void *data,
                                  size_t data_length);
    virtual ipc_msg_ptr recv(void);
    virtual void reply(ipc_msg_ptr msg, const void *data,
                       size_t data_length);
    virtual int fd(void);
private:
    int init(uint32_t id, bool bidirectional);
    zmq_ipc_client();
    ~zmq_ipc_client();
    uint32_t id_;
    uint32_t next_serial_;
    void *zsocket_;
    zmq_ipc_active_connection_t zconnections_[IPC_MAX_ID + 1];
    void *connect_(uint32_t recipient, bool *is_internal);
    int is_event_pending(void);
    void send_preamble_(void *socket, uint32_t recipient, bool is_pointer,
                        size_t real_length);
    void recv_preamble_(void *socket, zmq_ipc_msg_preamble_t *preamble);
};

} // namespace ipc
} // napespace lib
} // namespace sdk
    
#endif // __SDK_ZMQ_IPC_H__
