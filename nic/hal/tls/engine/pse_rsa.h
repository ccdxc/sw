#ifndef __PSE_RSA_H__
#define __PSE_RSA_H__

#include <openssl/ossl_typ.h>

typedef struct pse_rsa_ex_data_s {
    uint32_t  hw_key_index;
} PSE_RSA_EX_DATA;

RSA_METHOD* pse_get_RSA_methods(void);
void pse_free_RSA_methods(void);
EVP_PKEY* pse_rsa_get_evp_key(ENGINE* engine, const char* key_id, 
                              UI_METHOD *ui_method, PSE_KEY* key);

#endif /* __PSE_EC_H__ */
