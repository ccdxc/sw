#include "e_pse.h"
#include "pse_cipher.h"

/*
 * Pensando engine symmetric ciphers
 */
enum {
    PSE_AES_128_CBC,
    PSE_AES_192_CBC,
    PSE_AES_256_CBC,
    PSE_CIPHER_MAX,
};

typedef struct {
    int                 type;
    int                 block_size;
    int                 key_len;
    int                 iv_len;
    int                 flags;
    int                 dflt_ctx_sz;
    const EVP_CIPHER    *(*cipher_dflt)(void);
    EVP_CIPHER          *cipher_meth;
} pse_cipher_t;

/*
 * List of supported nids for pse_cipher_ptr_get's query
 */
static int              pse_cipher_nids_table[PSE_CIPHER_MAX];

static pse_cipher_t     pse_cipher_table[PSE_CIPHER_MAX] = {
    [PSE_AES_128_CBC] = {
        .type           = NID_aes_128_cbc,
        .block_size     = AES_BLOCK_SIZE,
        .key_len        = 128 / BITS_PER_BYTE,
        .iv_len         = EVP_MAX_IV_LENGTH,
        .flags          = EVP_CIPH_FLAG_CUSTOM_CIPHER   |
                          EVP_CIPH_CBC_MODE,
        .cipher_dflt    = EVP_aes_128_cbc,
    },
    [PSE_AES_192_CBC] = {
        .type           = NID_aes_192_cbc,
        .block_size     = AES_BLOCK_SIZE,
        .key_len        = 192 / BITS_PER_BYTE,
        .iv_len         = EVP_MAX_IV_LENGTH,
        .flags          = EVP_CIPH_FLAG_CUSTOM_CIPHER   |
                          EVP_CIPH_CBC_MODE,
        .cipher_dflt    = EVP_aes_192_cbc,
    },
    [PSE_AES_256_CBC] = {
        .type           = NID_aes_256_cbc,
        .block_size     = AES_BLOCK_SIZE,
        .key_len        = 256 / BITS_PER_BYTE,
        .iv_len         = EVP_MAX_IV_LENGTH,
        .flags          = EVP_CIPH_FLAG_CUSTOM_CIPHER   |
                          EVP_CIPH_CBC_MODE,
        .cipher_dflt    = EVP_aes_256_cbc,
    },
};

/*
 * PSE ENGINE_DIGESTS_PTR
 */
static int pse_cipher_ptr_get(ENGINE *e,
                              const EVP_CIPHER **md,
                              const int **nids,
                              int nid);
/*
 * PSE cipher method functions (legacy)
 */
typedef int (*cipher_dflt_meth_init)(EVP_CIPHER_CTX *ctx,
                                     const unsigned char *key,
                                     const unsigned char *iv,
                                     int enc);
typedef int (*cipher_dflt_meth_do_cipher)(EVP_CIPHER_CTX *ctx,
                                          unsigned char *out,
                                          const unsigned char *in,
                                          size_t inl);
typedef int (*cipher_dflt_meth_ctrl)(EVP_CIPHER_CTX *,
                                     int type,
                                     int arg,
                                     void *ptr);
typedef int (*cipher_dflt_meth_cleanup)(EVP_CIPHER_CTX *ctx);

static int pse_cipher_init(EVP_CIPHER_CTX *ctx,
                           const unsigned char *key,
                           const unsigned char *iv,
                           int enc);
static int pse_cipher_do_cipher(EVP_CIPHER_CTX *ctx,
                                unsigned char *out,
                                const unsigned char *in,
                                size_t inl);
static int pse_cipher_cleanup(EVP_CIPHER_CTX *ctx);

ENGINE_CIPHERS_PTR 
pse_get_cipher_methods(void) 
{
    pse_cipher_t        *cipher;
    int                 idx;
    const EVP_CIPHER    *cipher_dflt;

    for (idx = 0, cipher = &pse_cipher_table[0]; 
         idx < PSE_CIPHER_MAX; 
         idx++, cipher++) {

        pse_cipher_nids_table[idx] = cipher->type;
        cipher_dflt = (*cipher->cipher_dflt)();
        cipher->dflt_ctx_sz = EVP_CIPHER_impl_ctx_size(cipher_dflt);
        cipher->cipher_meth = EVP_CIPHER_meth_new(cipher->type, cipher->block_size,
                                                  cipher->key_len);
        /*
         * EVP_CIPHER is opaque and can only be accessed with the below 
         * set/get functions.
         */
        if (!cipher->cipher_meth ||
            !EVP_CIPHER_meth_set_iv_length(cipher->cipher_meth,
                                           cipher->iv_len)              ||
            !EVP_CIPHER_meth_set_flags(cipher->cipher_meth,
                                       cipher->flags)                   ||
            !EVP_CIPHER_meth_set_impl_ctx_size(cipher->cipher_meth,
                                               cipher->dflt_ctx_sz)     ||
            !EVP_CIPHER_meth_set_init(cipher->cipher_meth,
                                      pse_cipher_init)                  ||
            !EVP_CIPHER_meth_set_do_cipher(cipher->cipher_meth,
                                           pse_cipher_do_cipher)        ||
            !EVP_CIPHER_meth_set_cleanup(cipher->cipher_meth,
                                         pse_cipher_cleanup)) {

            WARN("Failed to create cipher method for type %d", cipher->type);
        }
    }

    return &pse_cipher_ptr_get;
}

static int
pse_cipher_nids_get(const int **nids)
{
    *nids = pse_cipher_nids_table;
    return PSE_CIPHER_MAX;
}

static inline int
pse_cipher_idx_get(int nid)
{
    int idx;

    switch (nid) {

    case NID_aes_128_cbc:
        idx = PSE_AES_128_CBC;
        break;
    case NID_aes_192_cbc:
        idx = PSE_AES_192_CBC;
        break;
    case NID_aes_256_cbc:
        idx = PSE_AES_256_CBC;
        break;
    default:
        idx = PSE_CIPHER_MAX;
        break;
    }
    return idx;
}

static int
pse_cipher_ptr_get(ENGINE *e,
                   const EVP_CIPHER **cipher,
                   const int **nids,
                   int nid)
{
    int idx;

    if (!cipher) {
        return pse_cipher_nids_get(nids);
    }

    /* We are being asked for a specific md */
    idx = pse_cipher_idx_get(nid);
    if (idx < PSE_CIPHER_MAX) {
        *cipher = pse_cipher_table[idx].cipher_meth;
        return 1;
    }

    *cipher = NULL;
    return 0;
}

static inline pse_cipher_t *
pse_cipher_get(EVP_CIPHER_CTX *ctx)
{
    const EVP_CIPHER *cipher = EVP_CIPHER_CTX_cipher(ctx);
    int idx = pse_cipher_idx_get(EVP_CIPHER_nid(cipher));

    assert(idx < PSE_CIPHER_MAX);
    return &pse_cipher_table[idx];

}

static inline PSE_CIPHER_APP_DATA *
pse_cipher_app_data_get(EVP_CIPHER_CTX *ctx)
{
    return EVP_CIPHER_CTX_get_app_data(ctx);
}

/*
 * Default methods when offload is not present
 */
static inline const EVP_CIPHER *
pse_cipher_dflt_get(EVP_CIPHER_CTX *ctx)
{
    pse_cipher_t *pse_cipher = pse_cipher_get(ctx);
    return (*pse_cipher->cipher_dflt)();
}

static inline cipher_dflt_meth_init
pse_cipher_dflt_method_get_init(EVP_CIPHER_CTX *ctx)
{
    return EVP_CIPHER_meth_get_init(pse_cipher_dflt_get(ctx));
}

static inline cipher_dflt_meth_do_cipher
pse_cipher_dflt_method_get_do_cipher(EVP_CIPHER_CTX *ctx)
{
    return EVP_CIPHER_meth_get_do_cipher(pse_cipher_dflt_get(ctx));
}

static inline cipher_dflt_meth_cleanup
pse_cipher_dflt_method_get_cleanup(EVP_CIPHER_CTX *ctx)
{
    return EVP_CIPHER_meth_get_cleanup(pse_cipher_dflt_get(ctx));
}

/*
 * PSE CIPHER method functions (legacy)
 */
int
pse_cipher_init(EVP_CIPHER_CTX *ctx,
                const unsigned char *key,
                const unsigned char *iv,
                int enc)
{
    PSE_CIPHER_APP_DATA     *app_data = pse_cipher_app_data_get(ctx);
    PSE_CIPHER_INIT_PARAM   params;

    if (!app_data->offload_method) {
        return (pse_cipher_dflt_method_get_init(ctx))(ctx, key, iv, enc);
    }

    pse_cipher_init_param_init(&params);
    params.key = (uint8_t *)key;
    return app_data->offload_method->init(app_data->cipher_hw_ctx, &params);
}

int
pse_cipher_do_cipher(EVP_CIPHER_CTX *ctx,
                     unsigned char *out,
                     const unsigned char *in,
                     size_t inl)
{
    PSE_CIPHER_APP_DATA         *app_data = pse_cipher_app_data_get(ctx);
    PSE_CIPHER_DO_CIPHER_PARAM  params;

    if (!app_data->offload_method) {
        return (pse_cipher_dflt_method_get_do_cipher(ctx))(ctx, out, in, inl);
    }

    pse_cipher_do_cipher_param_init(&params);
    params.key_idx = app_data->key_idx;
    params.msg_input = (uint8_t *)in;
    params.msg_output = out;
    params.msg_size = inl;
    params.wait_for_completion = app_data->wait_for_completion;
    return app_data->offload_method->do_cipher(app_data->cipher_hw_ctx, &params);
}

int
pse_cipher_cleanup(EVP_CIPHER_CTX *ctx)
{
    PSE_CIPHER_APP_DATA         *app_data = pse_cipher_app_data_get(ctx);
    PSE_CIPHER_CLEANUP_PARAM    params;
    cipher_dflt_meth_cleanup    cleanup;

    if (app_data->offload_method) {
        pse_cipher_cleanup_param_init(&params);
        app_data->offload_method->cleanup(app_data->cipher_hw_ctx, &params);
    }

    cleanup = pse_cipher_dflt_method_get_cleanup(ctx);
    if (cleanup) {
        return (*cleanup)(ctx);
    }
    return 1;
}

