#ifndef __PSE_RAND_H__
#define __PSE_RAND_H__

#include <openssl/ossl_typ.h>
#include "pse_intf.h"

RAND_METHOD *pse_get_RAND_methods(void);
int pse_RAND_set_mem_method(const PSE_OFFLOAD_MEM_METHOD *method);
int pse_RAND_set_salt_val(PSE_OFFLOAD_MEM *salt_val);
const PSE_OFFLOAD_MEM_METHOD *pse_RAND_get_mem_method(void);
PSE_OFFLOAD_MEM *pse_RAND_get_salt_val(void);

#endif /* __PSE_RAND_H__ */
