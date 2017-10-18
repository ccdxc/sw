#include "nic/hal/tls/ssl_helper.hpp"


#define WHERE_INFO(ssl, w, flag, msg) { \
    if(w & flag) { \
        HAL_TRACE_DEBUG("\t{} - {} - {}", msg, pen_SSL_state_string(ssl), pen_SSL_state_string_long(ssl)); \
    }\
}

// INFO CALLBACK
void ssl_info_callback(const SSL* ssl, int where, int ret)
{
    if(ret == 0) {
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
void ssl_msg_callback(int writep,
                      int version,
                      int contentType,
                      const void* buf,
                      size_t len,
                      SSL* ssl,
                      void *arg)
{
    HAL_TRACE_DEBUG("SSL: Message callback with length: {}", len);
}

namespace hal {
namespace tls {
/*-----------------------------
 * SSL Connection
 ------------------------------*/
hal_ret_t
SSLConnection::init(SSLHelper* _helper, conn_id_t _id, SSL_CTX *_ctx)
{
    HAL_TRACE_DEBUG("SSL: Init connection for id: {} ", _id);
    helper = _helper;
    ctx = _ctx;
    id = _id;
    ssl = pen_SSL_new(ctx);
    pen_BIO_new_bio_pair(&ibio, 0, &nbio, 0);
    if(ibio == NULL || nbio == NULL) {
        HAL_TRACE_ERR("Failed to allocate bio");
        return HAL_RET_NO_RESOURCE;
    }
    pen_SSL_set_bio(ssl, ibio, ibio);
    // TBD figure out of this needs to be CLIENT or Server
    // Init to client for now
    pen_SSL_set_connect_state(ssl);

    return HAL_RET_OK;
}

hal_ret_t
SSLConnection::terminate()
{
    pen_SSL_free(ssl); // Takes care of freeing internal BIO
    pen_BIO_free(nbio);
    return HAL_RET_OK;
}

hal_ret_t
SSLConnection::do_handshake()
{
    hal_ret_t   ret = HAL_RET_OK;
    int         err = 0;
    if(!ssl) {
        HAL_TRACE_ERR("SSL: connection not initialized");
        return HAL_RET_INVALID_ARG;
    }

    err = pen_SSL_do_handshake(ssl);
    ret = handle_ssl_ret(err);
    if(ret != HAL_RET_OK) {
        HAL_TRACE_ERR("SSL: Failed to start handshake");
        if(helper && helper->get_hs_done_cb()) {
            helper->get_hs_done_cb()(id, err);
        }
        return ret;
    }
    transmit_pending_data();

#if 0
    if(SSL_is_init_finished(ssl)){
        HAL_TRACE_DEBUG("SSL: handshake complete");
        if(helper && helper->get_hs_done_cb()) {
            helper->get_hs_done_cb()(id, err);
        }
    }
#endif

    return ret;
}

hal_ret_t
SSLConnection::transmit_pending_data()
{
    uint8_t buf[1024]; // TODO: avoid this on every transmit
    if(pen_BIO_ctrl_pending(nbio) > 0) {
        int ret = pen_BIO_read(nbio, buf, sizeof(buf));
        if(ret <= 0) {
            if(!pen_BIO_should_retry(nbio)) {
                handle_ssl_ret(ret);
            }
        } else {
            int len = ret;
            // transmit data
            HAL_TRACE_DEBUG("SSL: Transmiting data of len: {}", len);
            if(helper && helper->get_send_cb()) {
               return helper->get_send_cb()(id, buf, len);
            }
        }
    }
    return HAL_RET_OK;
}

hal_ret_t
SSLConnection::process_nw_data(uint8_t* data, size_t len)
{
    hal_ret_t   ret = HAL_RET_OK;

    if(!data || len <= 0 || !ssl) {
        HAL_TRACE_ERR("SSL: process_nw_data: Invalid ARGs");
        return HAL_RET_INVALID_ARG;
    }
    HAL_TRACE_DEBUG("SSL: Received data of len: {}", len);
    int err = pen_BIO_write(nbio, data, len);
    ret = handle_ssl_ret(err);
    if(ret != HAL_RET_OK) {
        HAL_TRACE_ERR("SSL: Failed to process nw data: {}:{}", ret, err);
        return ret;
    }
    if((pen_SSL_in_init(ssl) > 0)) {
        ret = do_handshake();
    }
    return ret;
}

hal_ret_t
SSLConnection::handle_ssl_ret(int ret)
{
    if(ret >= 0) {
        return HAL_RET_OK;
    }

    int error = pen_SSL_get_error(ssl, ret);
    switch(error) {
    case SSL_ERROR_NONE:
    case SSL_ERROR_ZERO_RETURN:
    case SSL_ERROR_WANT_READ:
        break;

    default:
        char buf[256];
        pen_ERR_error_string_n(error, buf, sizeof(buf));
        HAL_TRACE_ERR("SSL: SSL operation failed with error: {} {}",
                        error, buf);
        return HAL_RET_ERR;
    }
    return HAL_RET_OK;
}
/*------------------------------
 * SSL Context/Helper
 *------------------------------*/

SSLHelper::SSLHelper()
{
    HAL_TRACE_DEBUG("SSL Helper: Constructor");
    client_ctx = NULL;
    server_ctx = NULL;
    send_cb = NULL;
    hs_cb = NULL;
}

hal_ret_t
SSLHelper::init(void) {

    pen_SSL_library_init();
    pen_OpenSSL_add_all_algorithms();
    pen_ERR_load_BIO_strings();
    pen_ERR_load_crypto_strings();
    pen_SSL_load_error_strings();

    return this->init_ssl_ctxt();
    return HAL_RET_OK;
}

hal_ret_t
SSLHelper::init_ssl_ctxt()
{

    HAL_TRACE_DEBUG("SSL: Initializing SSL Context");
    // Client
    client_ctx =  pen_SSL_CTX_new(SSLv23_client_method());
    HAL_ASSERT(client_ctx != NULL);
    pen_SSL_CTX_set_options(client_ctx, SSL_OP_NO_SSLv2);
    pen_SSL_CTX_set_cipher_list(client_ctx, "ECDHE-ECDSA-AES128-GCM-SHA256");
    pen_SSL_CTX_set_info_callback(client_ctx, ssl_info_callback);
    pen_SSL_CTX_set_msg_callback(client_ctx, ssl_msg_callback);

    // Server
    server_ctx = pen_SSL_CTX_new(SSLv23_server_method());
    HAL_ASSERT(server_ctx != NULL);
    pen_SSL_CTX_set_options(server_ctx, SSL_OP_NO_SSLv2);
    pen_SSL_CTX_set_cipher_list(server_ctx, "ECDHE-ECDSA-AES128-GCM-SHA256");
    pen_SSL_CTX_set_info_callback(server_ctx, ssl_info_callback);
    pen_SSL_CTX_set_msg_callback(server_ctx, ssl_msg_callback);
    return HAL_RET_OK;
}

hal_ret_t
SSLHelper::start_connection(conn_id_t id)
{
    if(!client_ctx || !server_ctx) {
        HAL_TRACE_ERR("SSL client/server context not initialized");
        return HAL_RET_INVALID_ARG;
    }
    HAL_TRACE_DEBUG("SSL: Starting SSL handshake for id: {}", id);

    // Initialize connection
    conn[id].init(this, id, client_ctx);
    return conn[id].do_handshake();
}

hal_ret_t
SSLHelper::process_nw_data(conn_id_t id, uint8_t* data, size_t len)
{
    return conn[id].process_nw_data(data, len);
}

} // namespace tls
} // namespace hal
