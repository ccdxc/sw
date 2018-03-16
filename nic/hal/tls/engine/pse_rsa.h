#ifndef __PSE_RSA_H__
#define __PSE_RSA_H__

#include <openssl/ossl_typ.h>

RSA_METHOD* pse_get_RSA_methods(void);
void pse_free_RSA_methods(void);

#endif /* __PSE_EC_H__ */
