#pragma once

#include "nic/third-party/openssl/include/openssl/evp.h"
#include "nic/third-party/openssl/include/openssl/ssl.h"
#include "nic/third-party/openssl/include/openssl/err.h"
#include "nic/third-party/openssl/include/openssl/modes.h"
#include "nic/third-party/openssl/include/openssl/aes.h"
#include "nic/include/base.h"

#define MAX_SSL_CONNECTIONS 16000
namespace hal {
namespace tls {

class SSLHelper;
typedef uint32_t conn_id_t;

typedef struct hs_out_args_s {
    unsigned char   *read_key;
    unsigned char   *write_key;
    unsigned char   *read_iv;
    unsigned char   *write_iv;
    unsigned char   *read_seq_num;
    unsigned char   *write_seq_num;
} hs_out_args_t;

// Callbacks
typedef hal_ret_t (*nw_send_cb)(conn_id_t id, uint8_t* data, size_t len);
typedef hal_ret_t (*hs_done_cb)(conn_id_t id, conn_id_t oflowid, hal_ret_t ret, hs_out_args_t* args);

/* Opaque OpenSSL structures to fetch keys */
#define u64 uint64_t
#define u32 uint32_t
#define u8 uint8_t

typedef struct {
  u64 hi, lo;
} u128;

typedef struct {
  /* Following 6 names follow names in GCM specification */
  union {
    u64 u[2];
    u32 d[4];
    u8 c[16];
    size_t t[16 / sizeof(size_t)];
  } Yi, EKi, EK0, len, Xi, H;
  /*
   * Relative position of Xi, H and pre-computed Htable is used in some
   * assembler modules, i.e. don't change the order!
   */
#if TABLE_BITS==8
  u128 Htable[256];
#else
  u128 Htable[16];
  void (*gmult) (u64 Xi[2], const u128 Htable[16]);
  void (*ghash) (u64 Xi[2], const u128 Htable[16], const u8 *inp,
                 size_t len);
#endif
  unsigned int mres, ares;
  block128_f block;
  void *key;
} gcm128_context_alias;

typedef struct {
  union {
    double align;
    AES_KEY ks;
  } ks;                       /* AES key schedule to use */
  int key_set;                /* Set if key initialised */
  int iv_set;                 /* Set if an iv is set */
  gcm128_context_alias gcm;
  unsigned char *iv;          /* Temporary IV store */
  int ivlen;                  /* IV length */
  int taglen;
  int iv_gen;                 /* It is OK to generate IVs */
  int tls_aad_len;            /* TLS AAD length */
  ctr128_f ctr;
} EVP_AES_GCM_CTX;


class SSLConnection {
public:
    SSLConnection() {};
    ~SSLConnection() {};

    hal_ret_t   init(SSLHelper *helper, conn_id_t id, SSL_CTX *_ctx);
    hal_ret_t   terminate();
    hal_ret_t   do_handshake();
    hal_ret_t   process_nw_data(uint8_t* data, size_t len);

    conn_id_t get_oflowid() const {
        return oflowid;
    }

    void set_oflowid(conn_id_t oflowid_) {
        oflowid = oflowid_;
    }

private:
    hal_ret_t   handle_ssl_ret(int ret);
    hal_ret_t   transmit_pending_data();
    void        get_hs_args(hs_out_args_t& args);
    SSLHelper   *helper;
    SSL_CTX     *ctx;
    conn_id_t   id;
    conn_id_t   oflowid;
    SSL         *ssl;
    BIO*        ibio;   // Internal BIO towards SSL
    BIO*        nbio;   // BIO towards the network
};

class SSLHelper {
public:
    SSLHelper();
    ~SSLHelper(){};

    hal_ret_t init(void);
    hal_ret_t start_connection(conn_id_t id, conn_id_t oflowid);
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
