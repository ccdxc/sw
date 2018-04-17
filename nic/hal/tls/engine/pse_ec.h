#ifndef __PSE_EC_H__
#define __PSE_EC_H__

#include <openssl/ossl_typ.h>

typedef struct pse_ec_ex_data_s {
    uint32_t    hw_key_index;    
} PSE_EC_EX_DATA;

EC_KEY_METHOD* pse_get_EC_methods(void);
void pse_free_EC_methods(void);

EVP_PKEY* pse_ec_get_evp_key(ENGINE* engine, PSE_KEY *key,
                             UI_METHOD *ui_method, void *callback_data);

#endif /* __PSE_EC_H__ */
