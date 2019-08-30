#ifndef __PSE_INTF_H__
#define __PSE_INTF_H__

#include <stdbool.h>
#include <string.h>
#include <openssl/evp.h>
#include <openssl/ec.h>

#ifdef __cplusplus
extern "C" {
#endif 

typedef struct pse_offload_mem_st PSE_OFFLOAD_MEM;

typedef struct pse_offload_mem_method_s {
    uint32_t (*line_size_get)(PSE_OFFLOAD_MEM *mem);
    void     (*content_size_set)(PSE_OFFLOAD_MEM *mem, uint32_t size);
    uint32_t (*content_size_get)(PSE_OFFLOAD_MEM *mem);
    uint8_t *(*read)(PSE_OFFLOAD_MEM *mem);
    uint8_t *(*read_thru)(PSE_OFFLOAD_MEM *mem);
    void     (*write_thru)(PSE_OFFLOAD_MEM *mem);
} PSE_OFFLOAD_MEM_METHOD;

typedef struct pse_rand_bytes_param_s {
    uint8_t             *buf;
    uint32_t            size;
} PSE_RAND_BYTES_PARAM;

typedef struct pse_offload_rand_method_s {
    int (*bytes)(void *ctx, const PSE_RAND_BYTES_PARAM *params);

    const PSE_OFFLOAD_MEM_METHOD *mem_method;
} PSE_OFFLOAD_RAND_METHOD;

typedef struct pse_rsa_sign_param_s {
    uint32_t            key_size;
    int32_t             key_idx ;
    uint8_t             *n;
    uint8_t             *d;
    uint8_t             *hash_input;
    uint8_t             *sig_output;
    const uint8_t       *caller_unique_id;
    bool                async;
    bool                wait_for_completion;
} PSE_RSA_SIGN_PARAM;

typedef struct pse_rsa_encrypt_param_s {
    uint32_t            key_size;
    int32_t             key_idx;
    uint8_t             *n;
    uint8_t             *e;
    uint8_t             *plain_input;
    uint8_t             *ciphered_output;
    const uint8_t       *caller_unique_id;
    bool                async;
    bool                wait_for_completion;
} PSE_RSA_ENCRYPT_PARAM;

typedef struct pse_rsa_decrypt_param_s {
    uint32_t            key_size;
    int32_t             key_idx;
    uint8_t             *p;
    uint8_t             *q;
    uint8_t             *dp;
    uint8_t             *dq;
    uint8_t             *qinv;
    uint8_t             *ciphered_input;
    uint8_t             *plain_output;
    const uint8_t       *caller_unique_id;
    bool                async;
    bool                wait_for_completion;
} PSE_RSA_DECRYPT_PARAM;

typedef struct pse_rsa_offload_method_s {
    int (*sign)(void *ctx, const PSE_RSA_SIGN_PARAM *params);
    int (*encrypt)(void *ctx, const PSE_RSA_ENCRYPT_PARAM *param);
    int (*decrypt)(void *ctx, const PSE_RSA_DECRYPT_PARAM *param);

    const PSE_OFFLOAD_MEM_METHOD *mem_method;
    const PSE_OFFLOAD_RAND_METHOD *rand_method;
} PSE_RSA_OFFLOAD_METHOD;

typedef struct pse_ec_sign_param_s {
    int32_t             key_idx;
    uint8_t             *k_random;
    uint8_t             *hash_input;
    uint8_t             *sig_output_vec;
    uint8_t             *r_output;
    uint8_t             *s_output;
    const uint8_t       *caller_unique_id;
    bool                async;
    bool                wait_for_completion;
} PSE_EC_SIGN_PARAM;

typedef struct pse_ec_verify_param_s {
    int32_t             key_idx;
    const uint8_t       *caller_unique_id;
    uint8_t             *hash_input;
    uint8_t             *sig_input_vec;
    uint8_t             *r;
    uint8_t             *s;
    bool                async;
    bool                wait_for_completion;

    /*
     * Required params if key hasn't been programmed
     */
    uint8_t             *p;
    uint8_t             *n;
    uint8_t             *xg;
    uint8_t             *yg;
    uint8_t             *a;
    uint8_t             *b;
    uint8_t             *xq;
    uint8_t             *yq;
} PSE_EC_VERIFY_PARAM;

typedef struct pse_ec_offload_method_s {
    int (*sign)(void *ctx, const PSE_EC_SIGN_PARAM *params);
    int (*verify)(void *ctx, const PSE_EC_VERIFY_PARAM *params);

    const PSE_OFFLOAD_MEM_METHOD *mem_method;
} PSE_EC_OFFLOAD_METHOD;

static inline bool
pse_key_idx_valid(int32_t key_idx)
{
    return key_idx >= 0;
}

static inline void
pse_rand_bytes_param_init(PSE_RAND_BYTES_PARAM *param)
{
    memset(param, 0, sizeof(*param));
}

static inline void
pse_rsa_sign_param_init(PSE_RSA_SIGN_PARAM *param)
{
    memset(param, 0, sizeof(*param));
    param->key_idx = -1;
}

static inline void
pse_rsa_encrypt_param_init(PSE_RSA_ENCRYPT_PARAM *param)
{
    memset(param, 0, sizeof(*param));
    param->key_idx = -1;
}

static inline void
pse_rsa_decrypt_param_init(PSE_RSA_DECRYPT_PARAM *param)
{
    memset(param, 0, sizeof(*param));
    param->key_idx = -1;
}

static inline void
pse_ec_sign_param_init(PSE_EC_SIGN_PARAM *param)
{
    memset(param, 0, sizeof(*param));
    param->key_idx = -1;
}

static inline void
pse_ec_verify_param_init(PSE_EC_VERIFY_PARAM *param)
{
    memset(param, 0, sizeof(*param));
    param->key_idx = -1;
}

typedef struct pse_buffer_s {
    uint32_t    len;
    uint8_t     *data;
} PSE_BUFFER;

static inline void
pse_buffer_init(PSE_BUFFER *buf)
{
    buf->len = 0;
    buf->data = NULL;
}

typedef struct PSE_rsa_key_offload_st {
    const PSE_RSA_OFFLOAD_METHOD *offload_method;
    PSE_OFFLOAD_MEM *digest_padded_mem;
    PSE_OFFLOAD_MEM *salt_val;
    void        *rand_ctx;
    bool        wait_for_completion;
} PSE_RSA_KEY_OFFLOAD;

typedef struct PSE_rsa_key_st {
    int32_t     sign_key_id;    // index where key is stored
    int32_t     decrypt_key_id; // index where key is stored
    PSE_BUFFER  rsa_n;
    PSE_BUFFER  rsa_e;
    PSE_RSA_KEY_OFFLOAD offload;
} PSE_RSA_KEY;

typedef struct PSE_ec_key_sign_offload_st {
    PSE_OFFLOAD_MEM  *k_random;
    PSE_OFFLOAD_MEM  *sig_output_vec;
} PSE_EC_KEY_SIGN_OFFLOAD;

typedef struct PSE_ec_key_verify_offload_st {
    PSE_OFFLOAD_MEM  *sig_r;
    PSE_OFFLOAD_MEM  *sig_s;
} PSE_EC_KEY_VERIFY_OFFLOAD;

typedef struct PSE_ec_key_offload_st {
    int              curve_nid;
    const PSE_EC_OFFLOAD_METHOD *offload_method;
    union {
        PSE_EC_KEY_SIGN_OFFLOAD sign;
        PSE_EC_KEY_VERIFY_OFFLOAD verify;
    };
    bool             skip_DER;   // skip Distinguished Encoding Rules
    bool             wait_for_completion;
} PSE_EC_KEY_OFFLOAD;

typedef struct PSE_ec_key_st {
    int32_t          key_id;    // index where key is stored
    const EC_GROUP   *group;
    const EC_POINT   *point;
    PSE_EC_KEY_OFFLOAD offload;
} PSE_EC_KEY;

/* PSE Key Handle */
typedef struct PSE_key_st {
    const char       *label;    // Label to identify the key
    int              type;      // Underlying type of the KEY from openssl
    union {
        PSE_RSA_KEY  rsa_key;   // Public parameters for RSA
        PSE_EC_KEY   ec_key;    // Public parameters for EC
    } u;
}PSE_KEY;

#ifdef __cplusplus
}
#endif /* __cpluspls */

#endif /* __PSE_INTF_H__ */
