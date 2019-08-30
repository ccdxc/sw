#ifndef __PSE_RAND_H__
#define __PSE_RAND_H__

#include <openssl/ossl_typ.h>
#include "pse_intf.h"

typedef struct pse_rand_ex_data_s {
    const PSE_OFFLOAD_RAND_METHOD *rand_method;
    PSE_OFFLOAD_MEM *salt_val;
    void *rand_ctx;
} PSE_RAND_EX_DATA;

RAND_METHOD *pse_get_RAND_methods(void);
int pse_rand_set_ex_data(const PSE_RAND_EX_DATA *ex_data);
const PSE_RAND_EX_DATA * pse_rand_get_ex_data(void);

#endif /* __PSE_RAND_H__ */
