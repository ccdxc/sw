#ifndef __PSE_EC_H__
#define __PSE_EC_H__

#include <openssl/ossl_typ.h>

EC_KEY_METHOD* pse_get_EC_methods(void);
void pse_free_EC_methods(void);

#endif /* __PSE_EC_H__ */
