#ifndef __PSE_DIGESTS_H__
#define __PSE_DIGESTS_H__

#include <openssl/ossl_typ.h>
#include "nic/sdk/include/sdk/pse_intf.h"

ENGINE_DIGESTS_PTR pse_get_MD_methods(void);

#endif /* __PSE_DIGESTS_H__ */
