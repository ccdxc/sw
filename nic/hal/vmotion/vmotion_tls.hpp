//-----------------------------------------------------------------------------
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#ifndef __VMOTION_TLS_HPP__
#define __VMOTION_TLS_HPP__

#include "openssl/ssl.h"
#include "openssl/err.h"
#include "nic/include/base.hpp"

namespace hal {

class TLSContext;

typedef struct tls_connection_cfg_s {
    char const  *server_cert_file;
    char const  *server_key_file;
    char const  *client_cert_file;
    char const  *client_key_file;
    char const  *ca_file;
} tls_connection_cfg_t;

class TLSConnection {
public:
    // Factory methods
    static TLSConnection *factory(TLSContext *context, SSL_CTX *ctx, int sock, bool is_server);
    static void           destroy(TLSConnection *conn);
    SSL*                  get_ssl(void) { return ssl_; }

    hal_ret_t init(TLSContext *context, SSL_CTX *ctx, int sock, bool is_server);

private:
    TLSContext  *context_;
    SSL         *ssl_;
    bool         is_server_;
    int          sock_fd_;
};

class TLSContext {
public:
    // Factory methods
    static TLSContext *factory();

    hal_ret_t      init(tls_connection_cfg_t *cfg);
    TLSConnection *init_ssl_connection(int sock, bool is_server);
    hal_ret_t      init_ssl_ctx(SSL_CTX *ctx, const char *cert_file, const char *key_file,
                                const char *ca_file);
    SSL_CTX*       get_client_ctx() { return client_ctx_; }
    SSL_CTX*       get_server_ctx() { return server_ctx_; }

private:
    SSL_CTX   *client_ctx_;
    SSL_CTX   *server_ctx_;
};

} // namespace hal

#endif    // __VMOTION_TLS_HPP__
