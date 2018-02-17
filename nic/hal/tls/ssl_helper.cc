#include <unistd.h>
#include <string>
#include <sstream>
#include <ostream>
#include <iomanip>
#include "nic/hal/tls/ssl_helper.hpp"


#define WHERE_INFO(ssl, w, flag, msg) { \
    if(w & flag) { \
        HAL_TRACE_DEBUG("\t{} - {} - {}", msg, SSL_state_string(ssl), SSL_state_string_long(ssl)); \
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
// byte array to hex string for logging
std::string hex_dump(const uint8_t *buf, size_t sz)
{
    std::ostringstream result;

    for(size_t i = 0; i < sz; i+=8) {
        result << " 0x";
        for (size_t j = i ; j < sz && j < i+8; j++) {
            result << std::setw(2) << std::setfill('0') << std::hex << (int)buf[j];
        }
    }

    return result.str();
}
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
    ssl = SSL_new(ctx);
    BIO_new_bio_pair(&ibio, 0, &nbio, 0);
    if(ibio == NULL || nbio == NULL) {
        HAL_TRACE_ERR("Failed to allocate bio");
        return HAL_RET_NO_RESOURCE;
    }
    SSL_set_bio(ssl, ibio, ibio);
    // TBD figure out of this needs to be CLIENT or Server
    // Init to client for now
    SSL_set_connect_state(ssl);

    return HAL_RET_OK;
}

hal_ret_t
SSLConnection::terminate()
{
    SSL_free(ssl); // Takes care of freeing internal BIO
    BIO_free(nbio);
    return HAL_RET_OK;
}

void
SSLConnection::get_hs_args(hs_out_args_t& args)
{
    EVP_CIPHER_CTX * write_ctx = ssl->enc_write_ctx;
    EVP_CIPHER_CTX * read_ctx = ssl->enc_read_ctx;

    EVP_AES_GCM_CTX* gcm_write = (EVP_AES_GCM_CTX*)(write_ctx->cipher_data);
    EVP_AES_GCM_CTX* gcm_read = (EVP_AES_GCM_CTX*)(read_ctx->cipher_data);

    args.write_key = (unsigned char*)(gcm_write->gcm.key);
    args.read_key = (unsigned char*)(gcm_read->gcm.key);

    HAL_TRACE_DEBUG("Key, write: {}, read: {}", hex_dump(args.write_key, 16), hex_dump(args.read_key,16));

    args.write_iv = gcm_write->iv;
    args.read_iv = gcm_read->iv;
    HAL_TRACE_DEBUG("IV, write: {}, read: {}", hex_dump(args.write_iv,4), hex_dump(args.read_iv,4));

    args.write_seq_num = ssl->s3->write_sequence;
    args.read_seq_num = ssl->s3->read_sequence;
    HAL_TRACE_DEBUG("SeqNum, write: {}, read: {}", hex_dump(args.write_seq_num,8), hex_dump(args.read_seq_num,8));
}

hal_ret_t
SSLConnection::do_handshake()
{
    hal_ret_t       ret = HAL_RET_OK;
    int             err = 0;
    hs_out_args_t   hsargs = {0};

    if(!ssl) {
        HAL_TRACE_ERR("SSL: connection not initialized");
        return HAL_RET_INVALID_ARG;
    }

    err = SSL_do_handshake(ssl);
    ret = handle_ssl_ret(err);
    if(ret != HAL_RET_OK) {
        HAL_TRACE_ERR("SSL: handshake failed for id: {}", id);
        if(helper && helper->get_hs_done_cb()) {
            helper->get_hs_done_cb()(id, oflowid, ret, NULL, is_v4_flow);
        }
        return ret;
    }
    transmit_pending_data();

    if(SSL_is_init_finished(ssl)){
        HAL_TRACE_DEBUG("SSL: handshake complete");
        if(helper && helper->get_hs_done_cb()) {
            get_hs_args(hsargs);
            helper->get_hs_done_cb()(id, oflowid, ret, &hsargs, is_v4_flow);
        }
    }

    return ret;
}

hal_ret_t
SSLConnection::transmit_pending_data()
{
    uint8_t buf[1024]; // TODO: avoid this on every transmit
    if(BIO_ctrl_pending(nbio) > 0) {
        int ret = BIO_read(nbio, buf, sizeof(buf));
        if(ret <= 0) {
            if(!BIO_should_retry(nbio)) {
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
    int err = BIO_write(nbio, data, len);
    ret = handle_ssl_ret(err);
    if(ret != HAL_RET_OK) {
        HAL_TRACE_ERR("SSL: Failed to process nw data: {}:{}", ret, err);
        return ret;
    }
    if((SSL_in_init(ssl) > 0)) {
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

    int error = SSL_get_error(ssl, ret);
    switch(error) {
    case SSL_ERROR_NONE:
    case SSL_ERROR_ZERO_RETURN:
    case SSL_ERROR_WANT_READ:
        break;

    default:
        char buf[256];
        ERR_error_string_n(error, buf, sizeof(buf));
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

    SSL_library_init();
    OpenSSL_add_all_algorithms();
    ERR_load_BIO_strings();
    ERR_load_crypto_strings();
    SSL_load_error_strings();

    return this->init_ssl_ctxt();
    return HAL_RET_OK;
}

hal_ret_t
SSLHelper::init_ssl_ctxt()
{

    HAL_TRACE_DEBUG("SSL: Initializing SSL Context");
    // Client
    client_ctx =  SSL_CTX_new(SSLv23_client_method());
    HAL_ASSERT(client_ctx != NULL);
    SSL_CTX_set_options(client_ctx, SSL_OP_NO_SSLv2);
    SSL_CTX_set_cipher_list(client_ctx, "ECDHE-ECDSA-AES128-GCM-SHA256");
    SSL_CTX_set_info_callback(client_ctx, ssl_info_callback);
    SSL_CTX_set_msg_callback(client_ctx, ssl_msg_callback);

    // Server
    server_ctx = SSL_CTX_new(SSLv23_server_method());
    HAL_ASSERT(server_ctx != NULL);
    SSL_CTX_set_options(server_ctx, SSL_OP_NO_SSLv2);
    SSL_CTX_set_cipher_list(server_ctx, "ECDHE-ECDSA-AES128-GCM-SHA256");
    SSL_CTX_set_info_callback(server_ctx, ssl_info_callback);
    SSL_CTX_set_msg_callback(server_ctx, ssl_msg_callback);
    return HAL_RET_OK;
}

hal_ret_t
SSLHelper::start_connection(conn_id_t id, conn_id_t oflow_id, bool type)
{
    if(!client_ctx || !server_ctx) {
        HAL_TRACE_ERR("SSL client/server context not initialized");
        return HAL_RET_INVALID_ARG;
    }
    HAL_TRACE_DEBUG("SSL: Starting SSL handshake for id: {}", id);

    // Initialize connection
    conn[id].init(this, id, client_ctx);
    conn[id].set_oflowid(oflow_id);
    conn[id].set_flow_type(type);
    return conn[id].do_handshake();
}

hal_ret_t
SSLHelper::process_nw_data(conn_id_t id, uint8_t* data, size_t len)
{
    return conn[id].process_nw_data(data, len);
}

} // namespace tls
} // namespace hal
