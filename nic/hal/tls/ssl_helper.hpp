#pragma once

#include "openssl/evp.h"
#include "openssl/ssl.h"
#include "openssl/err.h"
#include "openssl/modes.h"
#include "openssl/aes.h"
#include "nic/include/base.h"
#include "nic/hal/tls/engine/pse_intf.h"
#include "nic/hal/src/internal/proxy.hpp"
#include "nic/hal/src/internal/crypto_cert_store.hpp"

#define MAX_SSL_CONNECTIONS 16000
#define MAX_KEY_SIZE        16
#define MAX_IV_SIZE         4
#define MAX_SEQ_NUM_SIZE    8

namespace hal {
namespace tls {

class SSLHelper;
typedef uint32_t conn_id_t;

typedef struct hs_out_args_s {
    bool            is_v4_flow;
    bool            is_server;
    uint32_t        read_key_index;
    uint32_t        write_key_index;
    unsigned char   *read_iv;
    unsigned char   *write_iv;
    unsigned char   *read_seq_num;
    unsigned char   *write_seq_num;
} hs_out_args_t;

typedef struct ssl_conn_args_s {
    conn_id_t              id;            // Identifier for the connection
    conn_id_t              oflow_id;      // Identifier for the other flow 
    bool                   is_v4_flow;    // true if this is ipv4 flow
    bool                   is_server_ctxt; // true if SSL server context 
    tls_proxy_flow_info_t  *tls_flow_cfg;  // TLS proxy config for the flow   
} ssl_conn_args_t;

// Callbacks
typedef hal_ret_t (*nw_send_cb)(conn_id_t id, uint8_t* data, size_t len);
typedef hal_ret_t (*hs_done_cb)(conn_id_t id, conn_id_t oflowid, hal_ret_t ret,
                                hs_out_args_t* args);
typedef hal_ret_t (*key_prog_cb)(conn_id_t id, const uint8_t* key, size_t key_len,
                                 uint32_t* key_hw_index);

class SSLConnection {
public:
    SSLConnection() {};
    ~SSLConnection() {};

    hal_ret_t   init(SSLHelper *helper, conn_id_t id, SSL_CTX *_ctx,
                     const tls_proxy_flow_info_t *tls_flow_cfg);
    hal_ret_t   terminate();
    hal_ret_t   do_handshake();
    hal_ret_t   process_nw_data(uint8_t* data, size_t len);
    hal_ret_t   process_hw_oper_done(void);

    conn_id_t   get_id() const {return id;};
    conn_id_t   get_oflowid() const {return oflowid;};
    void        set_oflowid(conn_id_t oflowid_) {oflowid = oflowid_; };
    bool        get_flow_type() const {return is_v4_flow;};
    void        set_flow_type(bool type) {is_v4_flow = type;};
    void        set_is_server(bool is_server_) {is_server = is_server_;};
    bool        get_is_server() const {return is_server;};
    void        ssl_msg_cb(int writep, int version, int contentType,
                           const void* buf, size_t len, SSL* ssl, void *arg);
private:
    hal_ret_t       handle_ssl_ret(int ret);
    hal_ret_t       handle_ssl_async();
    hal_ret_t       transmit_pending_data();
    void            get_hs_args(hs_out_args_t& args);
    hal_ret_t       load_certs_key(const hal::tls_proxy_flow_info_t *tls_flow_cfg) const;
    hal_ret_t       get_pse_key_rsa(PSE_KEY &pse_key,
                                    const hal::tls_proxy_flow_info_t &tls_flow_cfg,
                                    const hal::crypto_cert_t &cert) const;
    hal_ret_t       get_pse_key(PSE_KEY &pse_key,
                                const hal::tls_proxy_flow_info_t *tls_flow_cfg,
                                const hal::crypto_cert_t *cert) const;

    SSLHelper       *helper;
    SSL_CTX         *ctx;
    conn_id_t       id;
    conn_id_t       oflowid;
    bool            is_v4_flow;
    bool            is_server;
    SSL             *ssl;
    BIO*            ibio;   // Internal BIO towards SSL
    BIO*            nbio;   // BIO towards the network
    uint32_t        read_key_index;
    uint32_t        write_key_index;
    uint8_t         read_iv[MAX_IV_SIZE];
    uint8_t         write_iv[MAX_IV_SIZE];
    uint8_t         read_seq_num[MAX_SEQ_NUM_SIZE];
    uint8_t         write_seq_num[MAX_SEQ_NUM_SIZE];
};

class SSLHelper {
public:
    SSLHelper();
    ~SSLHelper(){};

    hal_ret_t init(void);
    hal_ret_t start_connection(const ssl_conn_args_t &args);
    hal_ret_t process_nw_data(conn_id_t id, uint8_t* data, size_t len);
    hal_ret_t process_hw_oper_done(conn_id_t id);

    void set_send_cb(nw_send_cb cb) {send_cb = cb;};
    nw_send_cb get_send_cb(void){return send_cb;};

    void set_hs_done_cb(hs_done_cb cb) {hs_cb = cb;};
    hs_done_cb get_hs_done_cb(void){return hs_cb;};

    void set_key_prog_cb(key_prog_cb cb) {key_cb = cb;};
    key_prog_cb get_key_prog_cb(void) {return key_cb;};

    ENGINE* get_engine() {return engine;};
private:
    hal_ret_t init_ssl_ctxt(void);
    hal_ret_t init_pse_engine(void);

    SSL_CTX         *client_ctx;
    SSL_CTX         *server_ctx;
    ENGINE          *engine;
    SSLConnection   conn[MAX_SSL_CONNECTIONS];

    // Callbacks
    nw_send_cb      send_cb;
    hs_done_cb      hs_cb;
    key_prog_cb     key_cb;
};

} // namespace tls
} // namespace hal
