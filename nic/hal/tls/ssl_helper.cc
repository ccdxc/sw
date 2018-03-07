#include <unistd.h>
#include <string>
#include <sstream>
#include <ostream>
#include <iomanip>
#include "nic/hal/tls/ssl_helper.hpp"
#include <openssl/engine.h>

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
void ssl_msg_callback(int writep, int version, int contentType,
                const void* buf, size_t len, SSL* ssl, void *arg)
{
    HAL_TRACE_DEBUG("SSL: Message callback: writep: {}, type: {} len: {}",
                        writep, contentType, len);
    if(arg) {
        hal::tls::SSLConnection* conn = (hal::tls::SSLConnection *)arg;
        HAL_TRACE_DEBUG("SSL: id: {}", conn->get_id());
        conn->ssl_msg_cb(writep, version, contentType, buf, len, ssl, arg); 
    }
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
    SSL_set_msg_callback(ssl, ssl_msg_callback);
    SSL_set_msg_callback_arg(ssl, this);

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
    args.read_key_index = read_key_index;
    args.write_key_index = write_key_index;
    args.read_iv = read_iv;
    args.write_iv = write_iv;
    args.read_seq_num = read_seq_num;
    args.write_seq_num = write_seq_num;
    HAL_TRACE_DEBUG("Key, write: {}, read: {}", args.write_key_index, args.read_key_index);
    HAL_TRACE_DEBUG("IV, write: {}, read: {}", hex_dump(args.write_iv,4), hex_dump(args.read_iv,4));
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

void
SSLConnection::ssl_msg_cb(int writep, int version, int contentType,
                          const void* buf, size_t len, SSL* ssl, void *arg)
{
    if(writep != 2 && writep != PEN_MSG_WRITEP)
        return;
 
    switch(contentType) {
        case TLS1_RT_CRYPTO_READ | TLS1_RT_CRYPTO_KEY:
            HAL_TRACE_DEBUG("read key: {}",
                                hex_dump((uint8_t *)buf, len));
            if(helper && helper->get_key_prog_cb()) {
                helper->get_key_prog_cb()(id, 
                                          (const uint8_t *)buf,
                                          len,
                                          &read_key_index);     
            }
            break;

        case TLS1_RT_CRYPTO_WRITE | TLS1_RT_CRYPTO_KEY:
            HAL_TRACE_DEBUG("write key: {}",
                                hex_dump((uint8_t *)buf, len));
            if(helper && helper->get_key_prog_cb()) {
                helper->get_key_prog_cb()(id,
                                          (const uint8_t*)buf,
                                          len,
                                          &write_key_index);     
            }
            break;
        
        case TLS1_RT_CRYPTO_READ | TLS1_RT_CRYPTO_IV:
        case TLS1_RT_CRYPTO_READ | TLS1_RT_CRYPTO_FIXED_IV:
            HAL_TRACE_DEBUG("read iv: {}",
                                hex_dump((uint8_t *)buf, len));
            memcpy(read_iv, buf, len);
            break;
        
        case TLS1_RT_CRYPTO_WRITE | TLS1_RT_CRYPTO_IV:
        case TLS1_RT_CRYPTO_WRITE | TLS1_RT_CRYPTO_FIXED_IV:
            HAL_TRACE_DEBUG("write iv: {}",
                                hex_dump((uint8_t *)buf, len));
            memcpy(write_iv, buf, len);
            break;
        
        case PEN_TLS_RT_HS_SEQ_NUM | PEN_TLS_RT_HS_READ:
            HAL_TRACE_DEBUG("Read_seq_num: {}",
                                hex_dump((uint8_t *)buf, len));
            memcpy(read_seq_num, buf, len);
            break;
        
        case PEN_TLS_RT_HS_SEQ_NUM | PEN_TLS_RT_HS_WRITE:
            HAL_TRACE_DEBUG("write_seq_num: {}",
                                hex_dump((uint8_t *)buf, len));
            memcpy(write_seq_num, buf, len);
            break;

        default:
            HAL_TRACE_ERR("Invalid content type: {}", contentType);
            break;
    }
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
SSLHelper::init(void)
{
    hal_ret_t ret = HAL_RET_OK;

    SSL_library_init();
    OpenSSL_add_all_algorithms();
    ERR_load_BIO_strings();
    ERR_load_crypto_strings();
    SSL_load_error_strings();

    ret = init_pse_engine();
    if(ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to init pse engine: {}", ret);
        return ret;
    }

    ret = init_ssl_ctxt();
    if(ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to init ssl ctxt: {}", ret);
    }
    return HAL_RET_OK;
}

hal_ret_t
SSLHelper::init_pse_engine()
{
    char        *cfg_path;
    std::string eng_path;

    ENGINE_load_dynamic();
    ENGINE_load_builtin_engines();
    ENGINE_register_all_complete();

    ENGINE* pse_engine = ENGINE_by_id("dynamic");
    if(pse_engine == NULL) {
        HAL_TRACE_ERR("Failed to load dynamic engine");
        return HAL_RET_OPENSSL_ENGINE_NOT_FOUND;
    }

    cfg_path = getenv("HAL_CONFIG_PATH");
    if (!cfg_path) {
        HAL_TRACE_ERR("Please set HAL_CONFIG_PATH env. variable");
        HAL_ASSERT_RETURN(0, HAL_RET_ERR);
    }

    eng_path =  std::string(cfg_path) + "/openssl/engine/libpse.so";
    HAL_TRACE_DEBUG("Loading pensando engine from path: {}", eng_path);

    if(!ENGINE_ctrl_cmd_string(pse_engine, "SO_PATH", eng_path.c_str(), 0)) {
        HAL_TRACE_ERR("SO_PATH failed!!");
        return HAL_RET_OPENSSL_ENGINE_NOT_FOUND;
    }

    if(!ENGINE_ctrl_cmd_string(pse_engine, "ID", "pse", 0)) {
        HAL_TRACE_ERR("ID failed!!");
        return HAL_RET_OPENSSL_ENGINE_NOT_FOUND;
    }

    if(!ENGINE_ctrl_cmd_string(pse_engine, "LOAD", NULL, 0)) {
        HAL_TRACE_ERR("ENGINE LOAD_ADD failed, err: {}",
            ERR_error_string(ERR_get_error(), NULL));
        return HAL_RET_OPENSSL_ENGINE_NOT_FOUND;
    }
    int ret = ENGINE_init(pse_engine);
    HAL_TRACE_DEBUG("Successfully loaded OpenSSL Engine: {} init result: {}",
                            ENGINE_get_name(pse_engine), ret);

    ret = ENGINE_set_default_EC(pse_engine);
    HAL_TRACE_DEBUG("Setting PSE as the default for ECDSA: {}", ret);
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

    // Server
    server_ctx = SSL_CTX_new(SSLv23_server_method());
    HAL_ASSERT(server_ctx != NULL);
    SSL_CTX_set_options(server_ctx, SSL_OP_NO_SSLv2);
    SSL_CTX_set_cipher_list(server_ctx, "ECDHE-ECDSA-AES128-GCM-SHA256");
    SSL_CTX_set_info_callback(server_ctx, ssl_info_callback);
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
