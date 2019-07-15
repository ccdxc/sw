#include "dole_if.hpp"

namespace dole_if {

/*
 * Hash (digest) algorithms map
 */
static const map<string,const dole_evp_md_t*> hash_algo_map = {
    {"SHA1",        EVP_sha1()},
    {"SHA224",      EVP_sha224()},
    {"SHA256",      EVP_sha256()},
    {"SHA384",      EVP_sha384()},
    {"SHA512",      EVP_sha512()},
};

/*
 * Interface to DOL OpenSSL engine
 */
ENGINE  *dole;
BIO     *dole_bio;

bool
init(const char *engine_path)
{
    int         ret;

    /*
     * Initialize DOL OpenSSL engine
     */
    if (dole) {
        return true;
    }

    SSL_library_init();
    dole_bio = BIO_new_fp(stdout, BIO_NOCLOSE);

    ENGINE_load_dynamic();
    ENGINE_load_builtin_engines();
    ENGINE_register_all_complete();

    dole = ENGINE_by_id("dynamic");
    if (dole == NULL) {
        OFFL_FUNC_ERR("failed to load dynamic engine");
        return false;
    }

    OFFL_FUNC_INFO("loading DOL engine from path {}", engine_path);

    if (!ENGINE_ctrl_cmd_string(dole, "SO_PATH", engine_path, 0)) {
       OFFL_FUNC_ERR("failed SO_PATH DOL engine load from {}",
                     engine_path);
       return false;
    }

    if (!ENGINE_ctrl_cmd_string(dole, "ID", "dole", 0)) {
        OFFL_FUNC_ERR("failed engine ID");
        return false;
    }

    if (!ENGINE_ctrl_cmd_string(dole, "LOAD", NULL, 0)) {
        OFFL_FUNC_ERR("ENGINE LOAD_ADD failed, err: {}",
                      ERR_error_string(ERR_get_error(), NULL));
        return false;
    }
    ret = ENGINE_init(dole);
    OFFL_FUNC_INFO("successfully loaded DOL OpenSSL Engine: {} init result: {}",
                   ENGINE_get_name(dole), ret);

    ENGINE_set_default_EC(dole);
    ENGINE_set_default_RSA(dole);
    return true;
}


/*
 * Locate matching message digest function
 */
const dole_evp_md_t *
hash_algo_find(const string& hash_algo)
{
    auto iter = hash_algo_map.find(hash_algo);
    if (iter != hash_algo_map.end()) {
        return iter->second;
    }
    return nullptr;
}

} // namespace dole_if


