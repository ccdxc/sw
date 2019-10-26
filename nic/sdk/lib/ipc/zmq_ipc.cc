//------------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//------------------------------------------------------------------------------

#include "ipc.hpp"
#include "zmq_ipc.hpp"

#include <memory>
#include <string>
#include <vector>

#include <assert.h>
#include <stdint.h>
#include <zmq.h>

#include "include/sdk/base.hpp"
#include "include/sdk/mem.hpp"
#include "lib/utils/crc_fast.hpp"

namespace sdk {
namespace lib {
namespace ipc {

static void *g_zmq_ctx = zmq_ctx_new();

static std::string
ipc_path_external (uint32_t id)
{
    return "ipc:///tmp/pen_sdk_ipc_" + std::to_string(id);
}

static std::string
ipc_path_internal (uint32_t id)
{
    return "inproc://pen_sdk_ipc_" + std::to_string(id);
}

zmq_ipc_msg::zmq_ipc_msg() {
    int rc = zmq_msg_init(&this->zmsg_);
    assert(rc == 0);
}

zmq_ipc_msg::~zmq_ipc_msg() {
    zmq_msg_close(&this->zmsg_);
}

zmq_msg_t *
zmq_ipc_msg::zmsg() {
    return &this->zmsg_;
}

void *
zmq_ipc_msg::data(void) {
    return zmq_msg_data(&this->zmsg_);
}

size_t
zmq_ipc_msg::length(void) {
    return zmq_msg_size(&this->zmsg_);
}

void *
zmq_ipc_user_msg::data(void) {
    if (this->preamble_.is_pointer) {
        return *(void **)zmq_msg_data(this->zmsg());
    } else {
        return zmq_msg_data(this->zmsg());
    }
}

size_t
zmq_ipc_user_msg::length(void) {
    if (this->preamble_.is_pointer) {
        return this->preamble_.real_length;
    } else {
        return zmq_msg_size(this->zmsg());
    }
}

const void *
zmq_ipc_user_msg::cookie(void) {
    return this->preamble_.cookie;
}

uint32_t
zmq_ipc_user_msg::sender(void) {
    return this->preamble_.sender;
}

void
zmq_ipc_user_msg::add_header(std::shared_ptr<zmq_ipc_msg> header) {
    this->headers_.push_back(header);
}

std::vector<zmq_ipc_msg_ptr> &
zmq_ipc_user_msg::headers(void) {
    return this->headers_;
}

zmq_ipc_msg_preamble_t *
zmq_ipc_user_msg::preamble(void) {
    return &this->preamble_;
}

zmq_ipc_endpoint::zmq_ipc_endpoint() {
    this->id_ = IPC_MAX_ID + 1;
    this->zsocket_ = NULL;
    this->next_serial_ = 1;
}

zmq_ipc_endpoint::~zmq_ipc_endpoint() {
}

uint32_t
zmq_ipc_endpoint::get_next_serial(void) {
    return this->next_serial_++;
}

bool
zmq_ipc_endpoint::is_event_pending(void) {
    int zevents;
    size_t zevents_len;

    zevents_len = sizeof(zevents);

    zmq_getsockopt(this->zsocket_, ZMQ_EVENTS, &zevents, &zevents_len);

    return (zevents & ZMQ_POLLIN);
}

void
zmq_ipc_endpoint::send_msg(uint32_t recipient, const void *data,
                           size_t data_length, const void *cookie,
                           bool send_pointer) {
    int rc;
    zmq_ipc_msg_preamble_t preamble;

    preamble.sender = this->id_;
    preamble.recipient = recipient;
    preamble.serial = this->get_next_serial();
    preamble.cookie = cookie;
    preamble.is_pointer = send_pointer;
    preamble.real_length = data_length;
    preamble.crc = sdk::utils::crc32((const unsigned char *)data, data_length,
                         sdk::utils::CRC32_POLYNOMIAL_TYPE_CRC32);
    rc = zmq_send(this->zsocket_, &preamble, sizeof(preamble), ZMQ_SNDMORE);
    assert(rc != -1);
    SDK_TRACE_DEBUG("Sent message: sender: %u, recipient: %u, serial: %u, "
                    "cookie: 0x%x, pointer: %d, real_length: %zu, crc32: %u",
                    preamble.sender, preamble.recipient, preamble.serial,
                    preamble.cookie, preamble.is_pointer, preamble.real_length,
                    preamble.crc);

    if (send_pointer) {
        rc = zmq_send(this->zsocket_, &data, sizeof(data), 0);
        assert(rc != -1);
    } else {
        rc = zmq_send(this->zsocket_, data, data_length, 0);
    }
}

void
zmq_ipc_endpoint::recv_msg(zmq_ipc_user_msg_ptr msg) {
    int rc;
    zmq_ipc_msg_preamble_t *preamble = msg->preamble();

    rc = zmq_recv(this->zsocket_, preamble, sizeof(*preamble), 0);
    assert(rc == sizeof(*preamble));

    SDK_TRACE_DEBUG("Received message: sender: %u, recipient: %u, serial: %u, "
                    "cookie: 0x%x, pointer: %d, real_length: %zu, crc32: %u",
                    preamble->sender, preamble->recipient, preamble->serial,
                    preamble->cookie, preamble->is_pointer,
                    preamble->real_length, preamble->crc);
    assert(preamble->recipient == this->id_);

    rc = zmq_recvmsg(this->zsocket_, msg->zmsg(), 0);
    assert(rc != -1);
}


zmq_ipc_server::zmq_ipc_server() {
}

zmq_ipc_server::~zmq_ipc_server() {
    zmq_close(this->zsocket_);
}

zmq_ipc_server *
zmq_ipc_server::factory(uint32_t id) {
    int            rc;
    void           *mem;
    zmq_ipc_server *new_server;

    mem = SDK_CALLOC(SDK_MEM_ALLOC_LIB_IPC_SERVER, sizeof(*new_server));
    if (!mem) {
        return NULL;
    }
    new_server = new (mem) zmq_ipc_server();
    rc = new_server->init(id);
    if (rc < 0) {
        new_server->~zmq_ipc_server();
        SDK_FREE(SDK_MEM_ALLOC_LIB_IPC_SERVER, new_server);
        return NULL;
    }

    return new_server;
}

void
zmq_ipc_server::destroy(zmq_ipc_server *server) {
    assert(server != NULL);
    server->~zmq_ipc_server();
    SDK_FREE(SDK_MEM_ALLOC_LIB_IPC_SERVER, server);
}

int
zmq_ipc_server::init(uint32_t id) {
    int rc;

    assert(id <= IPC_MAX_ID);

    this->id_ = id;

    this->zsocket_ = zmq_socket(g_zmq_ctx, ZMQ_ROUTER);
    rc = zmq_bind(this->zsocket_, ipc_path_external(id).c_str());
    assert(rc == 0);
    SDK_TRACE_DEBUG("Listening on %s", ipc_path_external(id).c_str());

    rc = zmq_bind(this->zsocket_, ipc_path_internal(id).c_str());
    assert(rc == 0);
    SDK_TRACE_DEBUG("Listening on %s", ipc_path_internal(id).c_str());

    return 0;
}

int
zmq_ipc_server::fd(void) {
    int    fd;
    size_t fd_len;

    fd_len = sizeof(fd);

    zmq_getsockopt(this->zsocket_, ZMQ_FD, &fd, &fd_len);

    return fd;
}

ipc_msg_ptr
zmq_ipc_server::recv(void) {
    int rc;

    if (!this->is_event_pending()) {
        return nullptr;
    }

    zmq_ipc_user_msg_ptr msg =
        std::make_shared<zmq_ipc_user_msg>();

    // See ZMQ Router to understand why we do this
    while(1) {
        std::shared_ptr<zmq_ipc_msg> header =
            std::make_shared<zmq_ipc_msg>();
        rc = zmq_recvmsg(this->zsocket_, header->zmsg(), 0);
        assert(rc != -1);
        msg->add_header(header);
        if (header->length() == 0) {
            break;
        }
    }

    this->recv_msg(msg);

    return msg;
}

void
zmq_ipc_server::reply(ipc_msg_ptr msg, const void *data,
                      size_t data_length) {
    int rc;
    zmq_ipc_user_msg_ptr zmsg =
        std::dynamic_pointer_cast<zmq_ipc_user_msg>(msg);

    // See ZMQ Router to understand why we do this
    for (auto header: zmsg->headers()) {
        rc = zmq_send(this->zsocket_, header->data(), header->length(),
                      ZMQ_SNDMORE);
        assert(rc != -1);
    }

    this->send_msg(zmsg->sender(), data, data_length, zmsg->cookie(), false);
}

zmq_ipc_client::~zmq_ipc_client() {
    zmq_close(this->zsocket_);
}

zmq_ipc_client::zmq_ipc_client() {
    /// All clients have the same id
    this->id_ = IPC_MAX_ID + 1;
    this->recipient_ = 0;
    this->is_recipient_internal_ = true;
}

void
zmq_ipc_client::connect(uint32_t recipient) {
    string path;
    int    rc;

    this->recipient_ = recipient;
    // Todo: Fixme: check local/remote
    this->is_recipient_internal_ = true;

    this->create_socket();

    if (this->is_recipient_internal_) {
        path = ipc_path_internal(this->recipient_);
    } else {
        path = ipc_path_external(this->recipient_);
    }

    rc = zmq_connect(this->zsocket_, path.c_str());
    SDK_TRACE_DEBUG("Connecting to %s", path.c_str());
    assert(rc != -1);
}

zmq_ipc_client_async::zmq_ipc_client_async() {
}

zmq_ipc_client_async::~zmq_ipc_client_async() {
}

int
zmq_ipc_client_async::init(uint32_t recipient) {
    this->connect(recipient);
    return 0;
}

zmq_ipc_client_async *
zmq_ipc_client_async::factory(uint32_t recipient)
{
    int                  rc;
    void                 *mem;
    zmq_ipc_client_async *new_client;

    mem = SDK_CALLOC(SDK_MEM_ALLOC_LIB_IPC_CLIENT, sizeof(*new_client));
    if (!mem) {
        return NULL;
    }
    new_client = new (mem) zmq_ipc_client_async();
    rc = new_client->init(recipient);
    if (rc < 0) {
        new_client->~zmq_ipc_client_async();
        SDK_FREE(SDK_MEM_ALLOC_LIB_IPC_CLIENT, new_client);
        return NULL;
    }

    return new_client;
}

void
zmq_ipc_client_async::destroy(zmq_ipc_client_async *client)
{
    assert(client != NULL);
    client->~zmq_ipc_client_async();
    SDK_FREE(SDK_MEM_ALLOC_LIB_IPC_CLIENT, client);
}

void
zmq_ipc_client_async::create_socket(void) {
    assert(this->zsocket_ == NULL);
    this->zsocket_ = zmq_socket(g_zmq_ctx, ZMQ_DEALER);
    assert(this->zsocket_ != NULL);
}

int
zmq_ipc_client_async::fd(void) {
    int    fd = 0;
    size_t fd_len;

    fd_len = sizeof(fd);

    zmq_getsockopt(this->zsocket_, ZMQ_FD, &fd, &fd_len);

    return fd;
}

void
zmq_ipc_client_async::send(const void *data, size_t data_length,
                           const void *cookie) {
    int rc;

    // We use a Dealer socket talking to Router socket. See ZMQ documentation
    // why we need this
    rc = zmq_send(this->zsocket_, NULL, 0, ZMQ_SNDMORE);
    assert(rc != -1);

    this->send_msg(this->recipient_, data, data_length, cookie,
                   this->is_recipient_internal_);
}

ipc_msg_ptr
zmq_ipc_client_async::recv(const void **cookie) {
    int rc;
    zmq_ipc_user_msg_ptr msg = std::make_shared<zmq_ipc_user_msg>();

    if (!this->is_event_pending()) {
        return nullptr;
    }

    // We use a Dealer socket talking to Router socket. See ZMQ documentation
    // why we need this
    rc = zmq_recv(this->zsocket_, NULL, 0, 0);
    assert(rc != -1);

    this->recv_msg(msg);

    *cookie = msg->cookie();
    return msg;
}

void
zmq_ipc_client_sync::create_socket(void) {
    assert(this->zsocket_ == NULL);
    this->zsocket_ = zmq_socket(g_zmq_ctx, ZMQ_REQ);
    assert(this->zsocket_ != NULL);
}

zmq_ipc_user_msg_ptr
zmq_ipc_client_sync::send_recv(const void *data, size_t data_length) {
    zmq_ipc_user_msg_ptr msg = std::make_shared<zmq_ipc_user_msg>();

    this->send_msg(this->recipient_, data, data_length, NULL,
                   this->is_recipient_internal_);

    this->recv_msg(msg);

    return msg;
}

} // namespace ipc
} // namespace lib
} // namespace sdk
