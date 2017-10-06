#pragma once

#include "nic/third-party/openssl/include/openssl/pen_ssl_wrapper.h"
#include "nic/include/base.h"

#define MAX_SSL_CONNECTIONS 16000
namespace hal {
namespace tls {

class SSLHelper;
typedef uint32_t conn_id_t;

// Callbacks
typedef hal_ret_t (*nw_send_cb)(conn_id_t id, uint8_t* data, size_t len);
typedef hal_ret_t (*hs_done_cb)(conn_id_t id, int err);

class SSLConnection {
public:
    SSLConnection() {};
    ~SSLConnection() {};

    hal_ret_t   init(SSLHelper *helper, conn_id_t id, SSL_CTX *_ctx);
    hal_ret_t   terminate();
    hal_ret_t   do_handshake();
    hal_ret_t   process_nw_data(uint8_t* data, size_t len);

private:
    hal_ret_t   handle_ssl_ret(int ret);
    hal_ret_t   transmit_pending_data();
    SSLHelper   *helper;
    SSL_CTX     *ctx;
    conn_id_t   id;
    SSL         *ssl;
    BIO*        ibio;   // Internal BIO towards SSL
    BIO*        nbio;   // BIO towards the network
};

class SSLHelper {
public:
    SSLHelper();
    ~SSLHelper(){};

    hal_ret_t init(void);
    hal_ret_t start_connection(conn_id_t id);
    hal_ret_t process_nw_data(conn_id_t id, uint8_t* data, size_t len);

    void set_send_cb(nw_send_cb cb) {send_cb = cb;};
    nw_send_cb get_send_cb(void){return send_cb;};

    void set_hs_done_cb(hs_done_cb cb) {hs_cb = cb;};
    hs_done_cb get_hs_done_cb(void){return hs_cb;};

private:
    hal_ret_t init_ssl_ctxt(void);

    SSL_CTX         *client_ctx;
    SSL_CTX         *server_ctx;
    SSLConnection   conn[MAX_SSL_CONNECTIONS];

    // Callbacks
    nw_send_cb      send_cb;
    hs_done_cb      hs_cb;
};

} // namespace tls
} // namespace hal
