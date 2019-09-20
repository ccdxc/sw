#ifndef __PSE_CIPHER_H__
#define __PSE_CIPHER_H__

#include <openssl/ossl_typ.h>
#include "nic/sdk/include/sdk/pse_intf.h"

ENGINE_CIPHERS_PTR pse_get_cipher_methods(void);

#endif /* __PSE_CIPHER_H__ */
