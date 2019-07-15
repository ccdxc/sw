#include "dole.hpp"

/* OpenSSL includes */

#ifdef  __cplusplus
extern "C" {
#endif

/* DOL engine id declarations */
const char *dole_id = "dole";
const char *dole_name = "dole offload";

/*
 * Description:
 *  Connect engine to OpenSSL engine library
 */
static int
dole_bind_helper(ENGINE *eng,
                 const char *id)
{
    int ret = 0;

    dole::logger::init(true, "info");

    if (id && strcmp(id, dole_id)) {
        DOLE_FUNC_WARN("ENGINE_id {} mismatches with {}", id, dole_id);
        goto cleanup;
    }
    if (!ENGINE_set_id(eng, dole_id)) {
        DOLE_FUNC_WARN("ENGINE_set_id failed");
        goto cleanup;
    }

    if (!ENGINE_set_name(eng, dole_name)) {
        DOLE_FUNC_WARN("ENGINE_set_name {} failed", dole_name);
        goto cleanup;
    }
    if (!ENGINE_set_RSA(eng, dole::rsa_methods_get())) {
        DOLE_FUNC_WARN("ENGINE_set_RSA failed");
        goto cleanup;
    }
    if(!ENGINE_set_load_privkey_function(eng, dole::pkey_load)) {
        DOLE_FUNC_WARN("ENGINE_set_load_privkey failed");
        goto cleanup;
    }
    ret = 1;

cleanup:
    DOLE_FUNC_INFO("bind completed. ret {}", ret);
    return ret;
}

IMPLEMENT_DYNAMIC_CHECK_FN();
IMPLEMENT_DYNAMIC_BIND_FN(dole_bind_helper);

# ifdef  __cplusplus
}
# endif


namespace dole {

EVP_PKEY *
pkey_load(ENGINE *engine,
          const char *key_id,
          UI_METHOD *ui_method,
          void *callback_data)
{
   
    EVP_PKEY            *pkey = NULL;
    dole_if::dole_key_t *key = static_cast<dole_if::dole_key_t *>((void *)key_id);

    if (!key) {
        DOLE_FUNC_ERR("no key found");
        return NULL;
    }

    DOLE_FUNC_DEBUG("key_type {} key_idx {}", key->key_type(), key->key_idx());
    switch (key->key_type()) {

    case EVP_PKEY_RSA:
        pkey = rsa_pkey_load(engine, key, ui_method, callback_data);
        break;

    default:
        DOLE_FUNC_ERR("invalid Key type {}", key->key_type());
        break;
    }

    return pkey;
}

} // namespace dole

