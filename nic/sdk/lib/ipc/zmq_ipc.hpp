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
namespace ipc {

typedef struct zmq_ipc_msg_preamble {
    uint32_t sender;
    uint32_t recipient;
    uint32_t msg_code;
    uint32_t serial;
    const void *cookie;
    bool     is_pointer;
    size_t   real_length;
    uint32_t crc;
} zmq_ipc_msg_preamble_t;

class zmq_ipc_msg : public ipc_msg {
public:
    zmq_ipc_msg();
    ~zmq_ipc_msg();
    virtual uint32_t code(void) override;
    virtual void *data(void) override;
    virtual size_t length(void) override;
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
    std::vector<std::shared_ptr<zmq_ipc_msg> > &headers(void);
    uint32_t sender(void);
    void add_header(std::shared_ptr<zmq_ipc_msg> header);
    zmq_ipc_msg_preamble_t *preamble(void);
    const void *cookie(void);
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
    void send_msg(uint32_t recipient, uint32_t msg_code, const void *data,
                  size_t data_length, const void *cookie, bool send_pointer);
    void recv_msg(zmq_ipc_user_msg_ptr msg);
protected:
    uint32_t id_;
    void     *zsocket_;
private:
    uint32_t next_serial_;
};

class zmq_ipc_server : public ipc_server,
                       public zmq_ipc_endpoint {
public:
    static zmq_ipc_server *factory(uint32_t id);
    static void destroy(zmq_ipc_server *server);
    virtual int fd(void);
    virtual ipc_msg_ptr recv(void);
    virtual void reply(ipc_msg_ptr msg, const void *data, size_t data_length);
private:
    int init(uint32_t id);
    zmq_ipc_server();
    ~zmq_ipc_server();
};

class zmq_ipc_client : public zmq_ipc_endpoint {
public:
    virtual void create_socket(void) = 0;
    void connect(uint32_t recipient);
    zmq_ipc_client();
    ~zmq_ipc_client();
protected:
    uint32_t recipient_;
    bool     is_recipient_internal_;
};
typedef std::shared_ptr<zmq_ipc_client> zmq_ipc_client_ptr;

class zmq_ipc_client_async : public ipc_client,
                             public zmq_ipc_client {
public:
    static zmq_ipc_client_async *factory(uint32_t recipient);
    static void destroy(zmq_ipc_client_async *client);
    int init(uint32_t recipient);
    virtual void create_socket(void);
    virtual int fd(void);
    virtual void send(uint32_t msg_code, const void *data, size_t data_length,
                      const void *cookie);
    virtual ipc_msg_ptr recv(const void** cookie);
private:
    zmq_ipc_client_async();
    ~zmq_ipc_client_async();
};
typedef std::shared_ptr<zmq_ipc_client_async> zmq_ipc_client_async_ptr;

class zmq_ipc_client_sync : public zmq_ipc_client {
public:
    virtual void create_socket(void);
    zmq_ipc_user_msg_ptr send_recv(uint32_t msg_code, const void *data,
                                   size_t data_length);
};
typedef std::shared_ptr<zmq_ipc_client_sync> zmq_ipc_client_sync_ptr;

} // namespace ipc
} // namespace sdk
    
#endif // __SDK_ZMQ_IPC_H__
