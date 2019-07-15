#ifndef __DOL_ENGINE_H__
#define __DOL_ENGINE_H__

#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <openssl/ssl.h>
#include <openssl/engine.h>
#include <openssl/evp.h>
#include <openssl/ec.h>
#include <dole_if.hpp>
#include "dole_logger.hpp"
#include "dole_rsa.hpp"

#ifdef  __cplusplus
extern "C" {
#endif

extern const char *dole_id;
extern const char *dole_name;

# ifdef  __cplusplus
}
# endif

namespace dole {

EVP_PKEY *pkey_load(ENGINE *engine,
                    const char *key_id,
                    UI_METHOD *ui_method,
                    void *callback_data);

} // namespace dole

#endif /* __DOL_ENGINE_H__ */
