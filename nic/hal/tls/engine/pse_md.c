#include "e_pse.h"
#include "pse_md.h"

/*
 * Pensando engine message digests
 */
enum {
    PSE_MD_SHA1,
    PSE_MD_SHA224,
    PSE_MD_SHA256,
    PSE_MD_SHA384,
    PSE_MD_SHA512,
#ifdef OPENSSL_WITH_TRUNCATED_SHA_SUPPORT
    PSE_MD_SHA512_224,
    PSE_MD_SHA512_256,
#endif
#ifdef OPENSSL_WITH_SHA3_SUPPORT
    PSE_MD_SHA3_224,
    PSE_MD_SHA3_256,
    PSE_MD_SHA3_384,
    PSE_MD_SHA3_512,
#endif
    PSE_MD_MAX,
};

/*
 * Size of PSE MD app_data
 */
#define PSE_MD_APP_DATA_SIZE(dflt_app_data_sz)                             \
    (((((dflt_app_data_sz) + sizeof(uint64_t) - 1) / sizeof(uint64_t)) *   \
     sizeof(uint64_t)) + sizeof(PSE_MD_APP_DATA))
     
typedef struct {
    int                 md_type;        // Openssl nid
    int                 dflt_app_data_sz;
    const EVP_MD        *(*md_dflt)(void);
    EVP_MD              *md_meth;
} pse_md_t;

/*
 * List of supported nids for pse_md_ptr_get's query
 */
static int          pse_nids_table[PSE_MD_MAX];

static pse_md_t     pse_md_table[PSE_MD_MAX] = {
    [PSE_MD_SHA1] = {
        .md_type = NID_sha1,
        .md_dflt = EVP_sha1,
    },
    [PSE_MD_SHA224] = {
        .md_type = NID_sha224,
        .md_dflt = EVP_sha224,
    },
    [PSE_MD_SHA256] = {
        .md_type = NID_sha256,
        .md_dflt = EVP_sha256,
    },
    [PSE_MD_SHA384] = {
        .md_type = NID_sha384,
        .md_dflt = EVP_sha384,
    },
    [PSE_MD_SHA512] = {
        .md_type = NID_sha512,
        .md_dflt = EVP_sha512,
    },
#ifdef OPENSSL_WITH_TRUNCATED_SHA_SUPPORT
    [PSE_MD_SHA512_224] = {
        .md_type = NID_sha512_224,
        .md_dflt = EVP_sha512_224,
    },
    [PSE_MD_SHA512_256] = {
        .md_type = NID_sha512_256,
        .md_dflt = EVP_sha512_256,
    },
#endif
#ifdef OPENSSL_WITH_SHA3_SUPPORT
    [PSE_MD_SHA3_224] = {
        .md_type = NID_sha3_224,
        .md_dflt = EVP_sha3_224,
    },
    [PSE_MD_SHA3_256] = {
        .md_type = NID_sha3_256,
        .md_dflt = EVP_sha3_256,
    },
    [PSE_MD_SHA3_384] = {
        .md_type = NID_sha3_384,
        .md_dflt = EVP_sha3_384,
    },
    [PSE_MD_SHA3_512] = {
        .md_type = NID_sha3_512,
        .md_dflt = EVP_sha3_512,
    },
#endif
};

/*
 * PSE ENGINE_DIGESTS_PTR
 */
static int pse_md_ptr_get(ENGINE *e,
                          const EVP_MD **md,
                          const int **nids,
                          int nid);
/*
 * PSE MD method functions (legacy)
 */
typedef int (*md_dflt_meth_init)(EVP_MD_CTX *ctx);
typedef int (*md_dflt_meth_update)(EVP_MD_CTX *ctx,
                                   const void *data,
                                   size_t count);
typedef int (*md_dflt_meth_final)(EVP_MD_CTX *ctx,
                                  unsigned char *digest);
typedef int (*md_dflt_meth_cleanup)(EVP_MD_CTX *ctx);

static int pse_sha_init(EVP_MD_CTX *ctx);
static int pse_sha_update(EVP_MD_CTX *ctx,
                          const void *data,
                          size_t count);
static int pse_sha_final(EVP_MD_CTX *ctx,
                         unsigned char *digest);
static int pse_sha_cleanup(EVP_MD_CTX *ctx);

ENGINE_DIGESTS_PTR 
pse_get_MD_methods(void) 
{
    pse_md_t        *md;
    int             idx;
    const EVP_MD    *md_dflt;

    for (idx = 0, md = &pse_md_table[0]; 
         idx < PSE_MD_MAX; 
         idx++, md++) {

        pse_nids_table[idx] = md->md_type;
        md_dflt = (*md->md_dflt)();
        md->md_meth = EVP_MD_meth_new(md->md_type, EVP_MD_pkey_type(md_dflt));
        md->dflt_app_data_sz = EVP_MD_meth_get_app_datasize(md_dflt);

        /*
         * EVP_MD is opaque and can only be accessed with the below 
         * set/get functions.
         */
        if (!md->md_meth ||
            !EVP_MD_meth_set_result_size(md->md_meth,
                                    EVP_MD_meth_get_result_size(md_dflt))    ||
            !EVP_MD_meth_set_input_blocksize(md->md_meth,
                                    EVP_MD_meth_get_input_blocksize(md_dflt))||
            !EVP_MD_meth_set_app_datasize(md->md_meth,
                                    PSE_MD_APP_DATA_SIZE(md->dflt_app_data_sz))||
            !EVP_MD_meth_set_flags(md->md_meth, 
                                    EVP_MD_meth_get_flags(md_dflt))          ||
            !EVP_MD_meth_set_init(md->md_meth, pse_sha_init)                 ||
            !EVP_MD_meth_set_update(md->md_meth, pse_sha_update)             ||
            !EVP_MD_meth_set_final(md->md_meth, pse_sha_final)               ||
            !EVP_MD_meth_set_cleanup(md->md_meth, pse_sha_cleanup)) {

            WARN("Failed to create md method for md_type %d", md->md_type);
        }
    }

    return &pse_md_ptr_get;
}

static int
pse_md_nids_get(const int **nids)
{
    *nids = pse_nids_table;
    return PSE_MD_MAX;
}

static inline int
pse_md_idx_get(int nid)
{
    int idx;

    switch (nid) {

    case NID_sha1:
        idx = PSE_MD_SHA1;
        break;
    case NID_sha224:
        idx = PSE_MD_SHA224;
        break;
    case NID_sha256:
        idx = PSE_MD_SHA256;
        break;
    case NID_sha384:
        idx = PSE_MD_SHA384;
        break;
    case NID_sha512:
        idx = PSE_MD_SHA512;
        break;
#ifdef OPENSSL_WITH_TRUNCATED_SHA_SUPPORT
    case NID_sha512_224:
        idx = PSE_MD_SHA512_224,
        break;
    case NID_sha512_256:
        idx = PSE_MD_SHA512_256,
        break;
#endif
#ifdef OPENSSL_WITH_SHA3_SUPPORT
    case NID_sha3_224:
        idx = PSE_MD_SHA3_224;
        break;
    case NID_sha3_256:
        idx = PSE_MD_SHA3_256;
        break;
    case NID_sha3_384:
        idx = PSE_MD_SHA3_384;
        break;
    case NID_sha3_512:
        idx = PSE_MD_SHA3_512;
        break;
#endif
    default:
        idx = PSE_MD_MAX;
        break;
    }
    return idx;
}

static int
pse_md_ptr_get(ENGINE *e,
               const EVP_MD **md,
               const int **nids,
               int nid)
{
    int idx;

    if (!md) {
        return pse_md_nids_get(nids);
    }

    /* We are being asked for a specific md */
    idx = pse_md_idx_get(nid);
    if (idx < PSE_MD_MAX) {
        *md = pse_md_table[idx].md_meth;
        return 1;
    }

    *md = NULL;
    return 0;
}

static inline pse_md_t *
pse_md_get(EVP_MD_CTX *ctx)
{
    const EVP_MD *md = EVP_MD_CTX_md(ctx);
    int idx = pse_md_idx_get(EVP_MD_type(md));

    assert(idx < PSE_MD_MAX);
    return &pse_md_table[idx];

}

static inline PSE_MD_APP_DATA *
pse_md_app_data_get(EVP_MD_CTX *ctx)
{
    pse_md_t *md = pse_md_get(ctx);

    return EVP_MD_CTX_md_data(ctx) + 
           PSE_MD_APP_DATA_OFFS(EVP_MD_meth_get_app_datasize(md->md_meth));
}

/*
 * Default methods when offload is not present
 */
static inline const EVP_MD *
pse_md_dflt_get(EVP_MD_CTX *ctx)
{
    pse_md_t *pse_md = pse_md_get(ctx);
    return (*pse_md->md_dflt)();
}

static inline md_dflt_meth_init
pse_md_dflt_method_get_init(EVP_MD_CTX *ctx)
{
    return EVP_MD_meth_get_init(pse_md_dflt_get(ctx));
}

static inline md_dflt_meth_update
pse_md_dflt_method_get_update(EVP_MD_CTX *ctx)
{
    return EVP_MD_meth_get_update(pse_md_dflt_get(ctx));
}

static inline md_dflt_meth_final
pse_md_dflt_method_get_final(EVP_MD_CTX *ctx)
{
    return EVP_MD_meth_get_final(pse_md_dflt_get(ctx));
}

static inline md_dflt_meth_cleanup
pse_md_dflt_method_get_cleanup(EVP_MD_CTX *ctx)
{
    return EVP_MD_meth_get_cleanup(pse_md_dflt_get(ctx));
}

/*
 * PSE MD method functions (legacy)
 */
int
pse_sha_init(EVP_MD_CTX *ctx)
{
    PSE_MD_APP_DATA     *app_data = pse_md_app_data_get(ctx);
    PSE_MD_INIT_PARAM   params;

    if (!app_data->offload_method) {
        return (pse_md_dflt_method_get_init(ctx))(ctx);
    }

    pse_md_init_param_init(&params);
    return app_data->offload_method->init(app_data->sha_hw_ctx, &params);
}

int
pse_sha_update(EVP_MD_CTX *ctx,
               const void *data,
               size_t count)
{
    PSE_MD_APP_DATA     *app_data = pse_md_app_data_get(ctx);
    PSE_MD_UPDATE_PARAM params;

    if (!app_data->offload_method) {
        return (pse_md_dflt_method_get_update(ctx))(ctx, data, count);
    }

    pse_md_update_param_init(&params);
    params.msg_input = (uint8_t *)data;
    params.msg_size = count;
    return app_data->offload_method->update(app_data->sha_hw_ctx, &params);
}

int
pse_sha_final(EVP_MD_CTX *ctx,
              unsigned char *digest)
{
    PSE_MD_APP_DATA     *app_data = pse_md_app_data_get(ctx);
    PSE_MD_FINAL_PARAM  params;

    if (!app_data->offload_method) {
        return (pse_md_dflt_method_get_final(ctx))(ctx, digest);
    }

    pse_md_final_param_init(&params);
    params.hash_output = digest;
    params.wait_for_completion = app_data->wait_for_completion;
    return app_data->offload_method->final(app_data->sha_hw_ctx, &params);
}

int
pse_sha_cleanup(EVP_MD_CTX *ctx)
{
    PSE_MD_APP_DATA         *app_data = pse_md_app_data_get(ctx);
    PSE_MD_CLEANUP_PARAM    params;
    md_dflt_meth_cleanup    cleanup;

    if (app_data->offload_method) {
        pse_md_cleanup_param_init(&params);
        app_data->offload_method->cleanup(app_data->sha_hw_ctx, &params);
    }

    cleanup = pse_md_dflt_method_get_cleanup(ctx);
    if (cleanup) {
        return (*cleanup)(ctx);
    }
    return 1;
}

