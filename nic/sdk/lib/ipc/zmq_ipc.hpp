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

#include "ipc.hpp"
#include "ipc_internal.hpp"

namespace sdk {
namespace ipc {

typedef struct zmq_ipc_msg_preamble {
    ipc_msg_type_t type;
    uint32_t sender;
    uint32_t recipient;
    uint32_t msg_code;
    uint32_t serial;
    response_oneshot_cb response_cb;
    const void *cookie;
    bool     is_pointer;
    size_t   real_length;
    uint32_t crc;
    uint32_t tag;
} zmq_ipc_msg_preamble_t;

class zmq_ipc_msg : public ipc_msg {
public:
    zmq_ipc_msg();
    ~zmq_ipc_msg();
    virtual uint32_t code(void) override;
    virtual void *data(void) override;
    virtual size_t length(void) override;
    virtual ipc_msg_type_t type(void) override;
    zmq_msg_t *zmsg(void);
private:
    zmq_msg_t zmsg_;
};
typedef std::shared_ptr<zmq_ipc_msg> zmq_ipc_msg_ptr;

class zmq_ipc_user_msg : public zmq_ipc_msg {
public:
    virtual uint32_t code(void) override;
    virtual void *data(void) override;
    virtual size_t length(void) override;
    virtual ipc_msg_type_t type(void) override;
    std::vector<std::shared_ptr<zmq_ipc_msg> > &headers(void);
    uint32_t sender(void);
    void add_header(std::shared_ptr<zmq_ipc_msg> header);
    zmq_ipc_msg_preamble_t *preamble(void);
    const void *cookie(void);
    uint32_t tag(void);
    response_oneshot_cb response_cb(void);
private:
    std::vector<std::shared_ptr<zmq_ipc_msg> > headers_;
    zmq_ipc_msg_preamble_t preamble_;
};
typedef std::shared_ptr<zmq_ipc_user_msg> zmq_ipc_user_msg_ptr;

class zmq_ipc_endpoint {
public:
    zmq_ipc_endpoint();
    ~zmq_ipc_endpoint();
    bool is_event_pending(void);
    uint32_t get_next_serial(void);
    void send_msg(ipc_msg_type_t type, uint32_t recipient, uint32_t msg_code,
                  const void *data, size_t data_length, response_oneshot_cb cb,
                  const void *cookie, uint32_t tag, bool send_pointer);
    void recv_msg(zmq_ipc_user_msg_ptr msg);
protected:
    uint32_t id_;
    void     *zsocket_;
private:
    uint32_t next_serial_;
};

class zmq_ipc_server : public zmq_ipc_endpoint {
public:
    zmq_ipc_server(uint32_t id);
    ~zmq_ipc_server();
    void subscribe(uint32_t msg_code);
    int fd(void);
    zmq_ipc_user_msg_ptr recv(void);
    void reply(ipc_msg_ptr msg, const void *data,
               size_t data_length);
};
typedef std::shared_ptr<zmq_ipc_server> zmq_ipc_server_ptr;

class zmq_ipc_client : public zmq_ipc_endpoint {
public:
    zmq_ipc_client(uint32_t id);
    zmq_ipc_client();
    ~zmq_ipc_client();
    virtual void create_socket(void) = 0;
    virtual void broadcast(uint32_t msg_code, const void *data,
                           size_t data_length) = 0;
protected:
    void connect_(uint32_t recipient);
protected:
    uint32_t recipient_;
    bool     is_recipient_internal_;
};
typedef std::shared_ptr<zmq_ipc_client> zmq_ipc_client_ptr;

class zmq_ipc_client_async : public zmq_ipc_client {
public:
    zmq_ipc_client_async(uint32_t id, uint32_t recipient);
    ~zmq_ipc_client_async();
    virtual void create_socket(void) override;
    virtual void broadcast(uint32_t msg_code, const void *data,
                           size_t data_length) override;
    int fd(void);
    void send(uint32_t msg_code, const void *data, size_t data_length,
              response_oneshot_cb cb, const void *cookie);
    zmq_ipc_user_msg_ptr recv(void);
};
typedef std::shared_ptr<zmq_ipc_client_async> zmq_ipc_client_async_ptr;

class zmq_ipc_client_sync : public zmq_ipc_client {
public:
    zmq_ipc_client_sync(uint32_t recipient);
    ~zmq_ipc_client_sync();
    virtual void create_socket(void) override;
    virtual void broadcast(uint32_t msg_code, const void *data,
                           size_t data_length) override;
    zmq_ipc_user_msg_ptr send_recv(uint32_t msg_code, const void *data,
                                   size_t data_length);
};
typedef std::shared_ptr<zmq_ipc_client_sync> zmq_ipc_client_sync_ptr;

} // namespace ipc
} // namespace sdk
    
#endif // __SDK_ZMQ_IPC_H__
