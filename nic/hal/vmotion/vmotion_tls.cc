//-----------------------------------------------------------------------------
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#include "nic/hal/vmotion/vmotion_tls.hpp"
#include "nic/include/hal_mem.hpp"

namespace hal {

#define WHERE_INFO(ssl, w, flag, msg) { \
    if (w & flag) { \
        HAL_TRACE_VERBOSE("\t{} - {} - {}", msg, SSL_state_string(ssl), SSL_state_string_long(ssl)); \
    }\
}
// INFO CALLBACK
void ssl_info_callback(const SSL* ssl, int where, int ret)
{
    if (ret == 0) {
        HAL_TRACE_ERR("SSL error occurred");
        return;
    }
    WHERE_INFO(ssl, where, SSL_CB_LOOP, "LOOP");
    WHERE_INFO(ssl, where, SSL_CB_EXIT, "EXIT");
    WHERE_INFO(ssl, where, SSL_CB_READ, "READ");
    WHERE_INFO(ssl, where, SSL_CB_WRITE, "WRITE");
    WHERE_INFO(ssl, where, SSL_CB_ALERT, "ALERT");
    WHERE_INFO(ssl, where, SSL_CB_READ_ALERT, "READ ALERT");
    WHERE_INFO(ssl, where, SSL_CB_WRITE_ALERT, "WRITE ALERT");
    WHERE_INFO(ssl, where, SSL_CB_ACCEPT_LOOP, "ACCEPT LOOP");
    WHERE_INFO(ssl, where, SSL_CB_ACCEPT_EXIT, "ACCEPT EXIT");
    WHERE_INFO(ssl, where, SSL_CB_CONNECT_LOOP, "CONNECT LOOP");
    WHERE_INFO(ssl, where, SSL_CB_CONNECT_EXIT, "CONNECT EXIT");
    WHERE_INFO(ssl, where, SSL_CB_HANDSHAKE_START, "HANDSHAKE START");
    WHERE_INFO(ssl, where, SSL_CB_HANDSHAKE_DONE, "HANDSHAKE DONE");
}
// MSG CALLBACK
void ssl_msg_callback(int writep, int version, int contentType, const void* buf,size_t len,
                      SSL* ssl, void *arg)
{
    HAL_TRACE_VERBOSE("Message callback with length: {}", len);
}

TLSConnection *
TLSConnection::factory(TLSContext *context, SSL_CTX *ctx, int sock, bool is_server)
{
    void  *mem = HAL_CALLOC(HAL_MEM_ALLOC_VMOTION, sizeof(TLSConnection));

    if (!mem) {
        HAL_TRACE_ERR("OOM failed to allocate memory for TLSConnection");
        return NULL;
    }
    auto conn = new (mem) TLSConnection();

    if (conn->init(context, ctx, sock, is_server) != HAL_RET_OK) {
        return NULL;
    }
    return conn;
}

void
TLSConnection::destroy(TLSConnection *conn)
{
    if (conn) {
        if (conn->get_ssl()) {
            SSL_free(conn->get_ssl());
        }
        HAL_FREE(HAL_MEM_ALLOC_VMOTION, conn);
    }
}

hal_ret_t
TLSConnection::init(TLSContext *context, SSL_CTX *ctx, int sock, bool is_server)
{
    int ret = 0;

    context_ = context;
    ssl_     = SSL_new(ctx);
    sock_fd_ = sock;

    if (!ssl_) {
        HAL_TRACE_ERR("SSL Allocation failed");
        return HAL_RET_ERR;
    }

    if (is_server) {
        is_server_ = TRUE;
        SSL_set_accept_state(ssl_);
    } else {
        SSL_set_connect_state(ssl_);
    }
    SSL_set_fd(ssl_, sock_fd_);

    if (is_server) {
        ret = SSL_accept(ssl_);
    } else {
        ret = SSL_connect(ssl_);
    }
    if (ret != 1) {
        HAL_TRACE_ERR("SSL Connect/Accept is_server:{} failed ret:{} Err:{}", is_server,
                      ret, SSL_get_error(ssl_, ret));
        return HAL_RET_ERR;
    }
    return HAL_RET_OK;
}

TLSConnection *
TLSContext::init_ssl_connection(int sock, bool is_server)
{
    auto ctx = (is_server ? get_server_ctx() : get_client_ctx());

    return (TLSConnection::factory(this, ctx, sock, is_server));
}

hal_ret_t
TLSContext::init_ssl_ctx(SSL_CTX *ctx, const char *cert_file, const char *key_file,
                        const char *ca_file)
{
    SDK_ASSERT(ctx != NULL);

    SSL_CTX_set_info_callback(ctx, ssl_info_callback);
    SSL_CTX_set_msg_callback(ctx, ssl_msg_callback);
    SSL_CTX_set_mode(ctx, SSL_MODE_ASYNC);

    if (ca_file) {
        // Load the CAs we trust
        if (!SSL_CTX_load_verify_locations(ctx, ca_file, NULL)) {
            ERR_print_errors_fp(stderr);
            HAL_TRACE_ERR("CA File verification failed {}", ca_file);
            return HAL_RET_ERR;
        }
        // sets the list of CAs sent to the client when requesting a client certificate
        SSL_CTX_set_client_CA_list(ctx, SSL_load_client_CA_file(ca_file));

        // Verify Flag - fail if the client doesn’t provide a valid certificate
        SSL_CTX_set_verify(ctx, (SSL_VERIFY_PEER | SSL_VERIFY_CLIENT_ONCE | \
                                 SSL_VERIFY_FAIL_IF_NO_PEER_CERT), NULL);
    }
    // set the certificate from file
    if (SSL_CTX_use_certificate_file(ctx, cert_file, SSL_FILETYPE_PEM) <= 0) {
        HAL_TRACE_ERR("Failed to add certficate file {}", cert_file);
        return HAL_RET_SSL_CERT_KEY_ADD_ERR;
    }
    // set the private key from file
    if (SSL_CTX_use_PrivateKey_file(ctx, key_file, SSL_FILETYPE_PEM) <= 0) {
        HAL_TRACE_ERR("Failed to add key file {}", key_file);
        return HAL_RET_SSL_CERT_KEY_ADD_ERR;
    }
    // verify the private key
    if (!SSL_CTX_check_private_key(ctx)) {
        HAL_TRACE_ERR("Private key does not match the public certificate");
        return HAL_RET_SSL_CERT_KEY_ADD_ERR;
    }
    return HAL_RET_OK;
}

TLSContext*
TLSContext::factory()
{
    tls_connection_cfg_t  cfg;

    cfg.server_cert_file = "/nic/conf/openssl/certs/rsa/server.crt";
    cfg.server_key_file  = "/nic/conf/openssl/certs/rsa/server.key";
    cfg.client_cert_file = "/nic/conf/openssl/certs/rsa/client.crt";
    cfg.client_key_file  = "/nic/conf/openssl/certs/rsa/client.key";
    cfg.ca_file          = "/nic/conf/openssl/certs/rsa/ca.crt";

    void *mem =  HAL_CALLOC(HAL_MEM_ALLOC_VMOTION, sizeof(TLSContext));
    if (!mem) {
        HAL_TRACE_ERR("OOM failed to allocate memory for TLSContext");
        return NULL;
    }
    auto context = new (mem) TLSContext();

    if (context->init(&cfg) != HAL_RET_OK) {
        HAL_FREE(HAL_MEM_ALLOC_VMOTION, mem);
        return NULL;
    }
    return context;
}

hal_ret_t
TLSContext::init(tls_connection_cfg_t *cfg)
{
    SSL_library_init();
    OpenSSL_add_all_algorithms();
    ERR_load_BIO_strings();
    ERR_load_crypto_strings();
    SSL_load_error_strings();

    server_ctx_ = SSL_CTX_new(TLS_server_method()); 
    if (init_ssl_ctx(server_ctx_, cfg->server_cert_file, cfg->server_key_file,
                     cfg->ca_file) != HAL_RET_OK) {
        return HAL_RET_ERR;
    }

    client_ctx_ = SSL_CTX_new(TLS_client_method()); 
    if (init_ssl_ctx(client_ctx_, cfg->client_cert_file, cfg->client_key_file,
                     NULL) != HAL_RET_OK) {
        return HAL_RET_ERR;
    }

    return HAL_RET_OK;
}

} // namespace hal
