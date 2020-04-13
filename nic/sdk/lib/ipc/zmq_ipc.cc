//------------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//------------------------------------------------------------------------------

#include "ipc.hpp"
#include "zmq_ipc.hpp"
#include "subscribers.hpp"

#include <memory>
#include <string>
#include <sstream>
#include <vector>

#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <zmq.h>

#include "include/sdk/base.hpp"
#include "include/sdk/mem.hpp"
#include "lib/utils/crc_fast.hpp"

namespace sdk {
namespace ipc {

static void *g_zmq_ctx = zmq_ctx_new();
static bool g_internal_endpoints[IPC_MAX_ID + 1];

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

uint32_t
zmq_ipc_msg::code(void) {
    return 0;
}

ipc_msg_type_t
zmq_ipc_msg::type(void) {
    return DIRECT;
}

std::string
zmq_ipc_msg::debug(void) {
    return "Internal ipc message";
}

void *
zmq_ipc_msg::data(void) {
    return zmq_msg_data(&this->zmsg_);
}

size_t
zmq_ipc_msg::length(void) {
    return zmq_msg_size(&this->zmsg_);
}

uint32_t
zmq_ipc_user_msg::code(void) {
    return this->preamble_.msg_code;
}

ipc_msg_type_t
zmq_ipc_user_msg::type(void) {
    return this->preamble_.type;
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

response_oneshot_cb
zmq_ipc_user_msg::response_cb() {
    return this->preamble_.response_cb;
}

const void *
zmq_ipc_user_msg::cookie(void) {
    return this->preamble_.cookie;
}

uint32_t
zmq_ipc_user_msg::tag(void) {
    return this->preamble_.tag;
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

std::string
zmq_ipc_user_msg::debug(void) {
    std::stringstream ss;

    ss << "type: " << this->preamble_.type <<
        ", sender: " <<  this->preamble_.sender <<
        ", recipient: " <<  this->preamble_.recipient <<
        ", msg_code: " <<  this->preamble_.msg_code <<
        ", serial: " <<  this->preamble_.serial <<
        ", cookie: " <<  this->preamble_.cookie <<
        ", pointer: " <<  this->preamble_.is_pointer <<
        ", real_length: " <<  this->preamble_.real_length <<
        ", crc32: " <<  this->preamble_.crc <<
        ", tag: " <<  this->preamble_.tag;

    return ss.str();
}

zmq_ipc_endpoint::zmq_ipc_endpoint() {
    this->id_ = IPC_MAX_ID + 1;
    this->zsocket_ = NULL;
    this->next_serial_ = 1;
}

zmq_ipc_endpoint::~zmq_ipc_endpoint() {
}

void
zmq_ipc_endpoint::zlock(void) {
    this->zlock_.lock();
}

void
zmq_ipc_endpoint::zunlock(void) {
    this->zlock_.unlock();
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

    // Take ZMQ LOCK
    this->zlock();

    zmq_getsockopt(this->zsocket_, ZMQ_EVENTS, &zevents, &zevents_len);

    // Release ZMQ LOCK
    this->zunlock();

    return (zevents & ZMQ_POLLIN);
}

void
zmq_ipc_endpoint::send_msg(ipc_msg_type_t type, uint32_t recipient,
                           uint32_t msg_code, const void *data,
                           size_t data_length, response_oneshot_cb cb,
                           const void *cookie, uint32_t tag,
                           bool send_pointer) {
    int rc;
    zmq_ipc_msg_preamble_t preamble;

    send_pointer = false;
    memset(&preamble, 0, sizeof(preamble));
    preamble.type = type;
    preamble.sender = this->id_;
    preamble.recipient = recipient;
    preamble.msg_code = msg_code;
    preamble.serial = this->get_next_serial();
    preamble.response_cb = cb;
    preamble.cookie = cookie;
    preamble.is_pointer = send_pointer;
    preamble.real_length = data_length;
    preamble.crc = sdk::utils::crc32((const unsigned char *)data, data_length,
                                     sdk::utils::CRC32_POLYNOMIAL_TYPE_CRC32);
    if (tag != 0) {
        preamble.tag = tag;
    } else {
        preamble.tag = rand();
        preamble.tag = sdk::utils::crc32((unsigned char *)&preamble,
                                         sizeof(preamble),
                                         sdk::utils::CRC32_POLYNOMIAL_TYPE_CRC32);
    }

    // Take ZMQ LOCK
    this->zlock();

    rc = zmq_send(this->zsocket_, &preamble, sizeof(preamble), ZMQ_SNDMORE);
    assert(rc != -1);
    SDK_TRACE_DEBUG("0x%x: Sent message: type: %u, sender: %u, recipient: %u, "
                    "msg_code: %u, serial: %u, cookie: 0x%x, pointer: %d, "
                    "real_length: %zu, crc32: %u, tag: %u",
                    pthread_self(),
                    preamble.type, preamble.sender, preamble.recipient,
                    preamble.msg_code, preamble.serial, preamble.cookie,
                    preamble.is_pointer, preamble.real_length, preamble.crc,
                    preamble.tag);

     if (send_pointer) {
         rc = zmq_send(this->zsocket_, &data, sizeof(data), 0);
     } else {
         rc = zmq_send(this->zsocket_, data, data_length, 0);
     }

     // Release ZMQ LOCK
     this->zunlock();
     
     assert(rc != -1);
}

void
zmq_ipc_endpoint::recv_msg(zmq_ipc_user_msg_ptr msg) {
    int rc;
    zmq_ipc_msg_preamble_t *preamble = msg->preamble();

    // Take ZMQ LOCK
    this->zlock();

    rc = zmq_recv(this->zsocket_, preamble, sizeof(*preamble), 0);
    assert(rc == sizeof(*preamble));

    SDK_TRACE_DEBUG("0x%x: Received message: type: %u, sender: %u, recipient: %u, "
                    "msg_code: %u, serial: %u, cookie: 0x%x, pointer: %d, "
                    "real_length: %zu, crc32: %u, tag: %u",
                    pthread_self(),
                    preamble->type, preamble->sender, preamble->recipient,
                    preamble->msg_code, preamble->serial, preamble->cookie,
                    preamble->is_pointer, preamble->real_length, preamble->crc,
                    preamble->tag);
    assert(preamble->recipient == this->id_);

    rc = zmq_recvmsg(this->zsocket_, msg->zmsg(), 0);
    assert(rc != -1);

    // Release ZMQ LOCK
    this->zunlock();


    // Check crc32 to make sure the message contents haven't changed
    // This can happen if we were passed a stack pointer
    uint32_t crc = sdk::utils::crc32(
        (const unsigned char *)msg->data(), msg->length(),
        sdk::utils::CRC32_POLYNOMIAL_TYPE_CRC32);
    assert(crc == preamble->crc);
}


zmq_ipc_server::~zmq_ipc_server() {
    zmq_close(this->zsocket_);
}

zmq_ipc_server::zmq_ipc_server(uint32_t id) {
    int rc;

    assert(id <= IPC_MAX_ID);

    this->id_ = id;

    // Take ZMQ LOCK
    this->zlock();
    
    this->zsocket_ = zmq_socket(g_zmq_ctx, ZMQ_ROUTER);
    rc = zmq_bind(this->zsocket_, ipc_path_external(id).c_str());
    assert(rc == 0);
    SDK_TRACE_DEBUG("0x%x: listening on %s", pthread_self(),
                    ipc_path_external(id).c_str());

    rc = zmq_bind(this->zsocket_, ipc_path_internal(id).c_str());
    assert(rc == 0);
    SDK_TRACE_DEBUG("0x%x: listening on %s", pthread_self(),
                    ipc_path_internal(id).c_str());

    // Release ZMQ LOCK
    this->zunlock();


    g_internal_endpoints[id] = true;

    for (int i = 0; i <= IPC_MAX_ID; i++) {
        subscribers::instance()->clear(i, id);
    }
}

void zmq_ipc_server::subscribe(uint32_t msg_code) {
    SDK_TRACE_DEBUG("0x%x: subscribe: client: %u msg_code: %u",
                    pthread_self(), this->id_, msg_code);
    subscribers::instance()->set(msg_code, this->id_);
}

int
zmq_ipc_server::fd(void) {
    int    fd;
    size_t fd_len;

    fd_len = sizeof(fd);


    // Take ZMQ LOCK
    this->zlock();

    zmq_getsockopt(this->zsocket_, ZMQ_FD, &fd, &fd_len);

    // Release ZMQ LOCK
    this->zunlock();

    return fd;
}

zmq_ipc_user_msg_ptr
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

        // Take ZMQ LOCK
        this->zlock();

        rc = zmq_recvmsg(this->zsocket_, header->zmsg(), 0);
        assert(rc != -1);

        // Release ZMQ LOCK
        this->zunlock();
        
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

    // Check crc32 to make sure the message contents haven't changed
    // This can happen if we were passed a stack pointer
    uint32_t crc = sdk::utils::crc32(
        (const unsigned char *)msg->data(), msg->length(),
        sdk::utils::CRC32_POLYNOMIAL_TYPE_CRC32);
    assert(crc == zmsg->preamble()->crc);

    assert(zmsg->preamble()->recipient == this->id_);

    // Take ZMQ LOCK
    this->zlock();

    
    // See ZMQ Router to understand why we do this
    for (auto header: zmsg->headers()) {
        rc = zmq_send(this->zsocket_, header->data(), header->length(),
                      ZMQ_SNDMORE);
        assert(rc != -1);
    }

    // Release ZMQ LOCK
    this->zunlock();


    this->send_msg(DIRECT, zmsg->sender(), zmsg->code(), data, data_length,
                   zmsg->response_cb(), zmsg->cookie(), zmsg->tag(), false);
}

zmq_ipc_client::~zmq_ipc_client() {
    zmq_close(this->zsocket_);
}

zmq_ipc_client::zmq_ipc_client(uint32_t id) {
    this->id_ = id;
    this->recipient_ = 0;
    this->is_recipient_internal_ = true;
}

zmq_ipc_client::zmq_ipc_client() : zmq_ipc_client(IPC_MAX_ID + 1) {
}

void
zmq_ipc_client::connect_(uint32_t recipient) {
    string path;
    int    rc;

    this->recipient_ = recipient;
    this->is_recipient_internal_ = g_internal_endpoints[recipient];

    this->create_socket();

    if (this->is_recipient_internal_) {
        path = ipc_path_internal(this->recipient_);
    } else {
        path = ipc_path_external(this->recipient_);
    }

    // Take ZMQ LOCK
    this->zlock();

    rc = zmq_connect(this->zsocket_, path.c_str());

    // Release ZMQ LOCK
    this->zunlock();

    
    SDK_TRACE_DEBUG("0x%x: connecting to %s", pthread_self(), path.c_str());
    assert(rc != -1);
}

zmq_ipc_client_async::zmq_ipc_client_async(uint32_t id, uint32_t recipient)
    : zmq_ipc_client(id) {
    this->connect_(recipient);
}

zmq_ipc_client_async::~zmq_ipc_client_async() {
}

void
zmq_ipc_client_async::create_socket(void) {
    // Take ZMQ LOCK
    this->zlock();

    assert(this->zsocket_ == NULL);
    this->zsocket_ = zmq_socket(g_zmq_ctx, ZMQ_DEALER);
    assert(this->zsocket_ != NULL);

    // Take ZMQ LOCK
    this->zunlock();
}

int
zmq_ipc_client_async::fd(void) {
    int    fd = 0;
    size_t fd_len;

    fd_len = sizeof(fd);

    // Take ZMQ LOCK
    this->zlock();
    
    zmq_getsockopt(this->zsocket_, ZMQ_FD, &fd, &fd_len);

    // Release ZMQ LOCK
    this->zunlock();

    return fd;
}

void
zmq_ipc_client_async::send(uint32_t msg_code, const void *data,
                           size_t data_length, response_oneshot_cb cb,
                           const void *cookie) {
    int rc;

    // Take ZMQ LOCK
    this->zlock();
    
    // We use a Dealer socket talking to Router socket. See ZMQ documentation
    // why we need this
    rc = zmq_send(this->zsocket_, NULL, 0, ZMQ_SNDMORE);
    assert(rc != -1);

    // Release ZMQ LOCK
    this->zunlock();


    this->send_msg(DIRECT, this->recipient_, msg_code, data, data_length,
                   cb, cookie, 0, this->is_recipient_internal_);
}

void
zmq_ipc_client_async::broadcast(uint32_t msg_code, const void *data,
                                size_t data_length) {
    int rc;

    // Take ZMQ LOCK
    this->zlock();

    // We use a Dealer socket talking to Router socket. See ZMQ documentation
    // why we need this
    rc = zmq_send(this->zsocket_, NULL, 0, ZMQ_SNDMORE);
    assert(rc != -1);

    // Release ZMQ LOCK
    this->zunlock();

    this->send_msg(BROADCAST, this->recipient_, msg_code, data, data_length,
                   NULL, NULL, 0, this->is_recipient_internal_);
}

zmq_ipc_user_msg_ptr
zmq_ipc_client_async::recv(void) {
    int rc;
    zmq_ipc_user_msg_ptr msg = std::make_shared<zmq_ipc_user_msg>();

    if (!this->is_event_pending()) {
        return nullptr;
    }

    // Take ZMQ LOCK
    this->zlock();

    // We use a Dealer socket talking to Router socket. See ZMQ documentation
    // why we need this
    rc = zmq_recv(this->zsocket_, NULL, 0, 0);
    assert(rc != -1);

    // Release ZMQ LOCK
    this->zunlock();

    this->recv_msg(msg);

    return msg;
}

zmq_ipc_client_sync::zmq_ipc_client_sync(uint32_t recipient) {
    this->connect_(recipient);
}

zmq_ipc_client_sync::~zmq_ipc_client_sync() {
}

void
zmq_ipc_client_sync::create_socket(void) {
    // Take ZMQ LOCK
    this->zlock();

    assert(this->zsocket_ == NULL);
    this->zsocket_ = zmq_socket(g_zmq_ctx, ZMQ_DEALER);
    assert(this->zsocket_ != NULL);

    // Release ZMQ LOCK
    this->zunlock();
}

zmq_ipc_user_msg_ptr
zmq_ipc_client_sync::send_recv(uint32_t msg_code, const void *data,
                               size_t data_length) {
    zmq_ipc_user_msg_ptr msg = std::make_shared<zmq_ipc_user_msg>();
    int rc;

    // Take ZMQ LOCK
    this->zlock();
    
    // We use Dealer socket
    rc = zmq_send(this->zsocket_, NULL, 0, ZMQ_SNDMORE);
    assert(rc != -1);

    // Take ZMQ LOCK
    this->zunlock();
    
    this->send_msg(DIRECT, this->recipient_, msg_code, data, data_length, NULL,
                   NULL, 0, this->is_recipient_internal_);

    // Take ZMQ LOCK
    this->zlock();

    // We use a Dealer socket talking to Router socket. See ZMQ documentation
    // why we need this
    rc = zmq_recv(this->zsocket_, NULL, 0, 0);
    assert(rc != -1);

    // Release ZMQ LOCK
    this->zunlock();

    this->recv_msg(msg);
    assert(msg->type() != BROADCAST);

    return msg;
}

void
zmq_ipc_client_sync::broadcast(uint32_t msg_code, const void *data,
                          size_t data_length) {
    zmq_ipc_user_msg_ptr msg = std::make_shared<zmq_ipc_user_msg>();
    int rc;

    // Take ZMQ LOCK
    this->zlock();

    // We use Dealer socket
    rc = zmq_send(this->zsocket_, NULL, 0, ZMQ_SNDMORE);
    assert(rc != -1);

    // Release ZMQ LOCK
    this->zunlock();
    
    this->send_msg(BROADCAST, this->recipient_, msg_code, data, data_length,
                   NULL, NULL, 0, false);
}

} // namespace ipc
} // namespace sdk
