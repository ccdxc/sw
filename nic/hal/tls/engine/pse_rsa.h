#ifndef __PSE_RSA_H__
#define __PSE_RSA_H__

#include <openssl/ossl_typ.h>
#include "pse_rand.h"

typedef struct pse_rsa_ex_data_s {
    int32_t   sig_gen_key_id;
    int32_t   decrypt_key_id;
    PSE_RSA_KEY_OFFLOAD offload;
    void      *caller_ctx;
    PSE_RAND_EX_DATA rand_ex_data;
} PSE_RSA_EX_DATA;

RSA_METHOD* pse_get_RSA_methods(void);
void pse_free_RSA_methods(void);
EVP_PKEY* pse_rsa_get_evp_key(ENGINE* engine, PSE_KEY *key,
                              UI_METHOD *ui_method, void *callback_data);

#endif /* __PSE_RSA_H__ */
