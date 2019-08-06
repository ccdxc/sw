#include "eng_if.hpp"
#include "utils.hpp"

namespace eng_if {

/*
 * Generate the hash digest for msg and store result in digest_output.
 */
const eng_evp_md_t *
digest_gen(digest_params_t& params)
{
    EVP_MD_CTX          *md_ctx = NULL;
    const eng_evp_md_t  *md = NULL;
    unsigned int        digest_size;
    dp_mem_t            *msg = params.msg();
    dp_mem_t            *digest = params.digest();

    /*
     * Using an OpenSSL message digest/hash function, consists of the
     * following steps:
     *
     * - Create a Message Digest context
     * - Initialise the context by identifying the algorithm to be used 
     *   (built-in algorithms are defined in evp.h)
     * - Provide the message whose digest needs to be calculated. Messages
     *   can be divided into sections and provided over a number of calls
     *   to the library if necessary
     * - Caclulate the digest
     * - Clean up the context if no longer required
     *
     * Message digest algorithms are identified using an EVP_MD object. 
     * These are built-in  to the library and obtained through appropriate
     * library calls (e.g. such as EVP_sha256() or EVP_sha512()).
     */
    md_ctx = EVP_MD_CTX_create();
    if (!md_ctx) {
        OFFL_FUNC_ERR("Failed to create EVP_MD_CTX");
        goto error;
    }

    md = hash_algo_find(params.hash_algo());
    if (!md) {
        OFFL_FUNC_ERR("Failed to find md for {}", params.hash_algo());
        goto error;
    }

    /* 
     * Identify the algorithm
     */
    if (EVP_DigestInit_ex(md_ctx, md, NULL) <= 0) {
        OFFL_FUNC_ERR("Failed to initialize EVP_MD_CTX");
        md = NULL;
        goto error;
    }

    /* 
     * Calculate the digest
     */
    if (EVP_DigestUpdate(md_ctx, msg->read(), msg->content_size_get()) <= 0) {
        OFFL_FUNC_ERR("Failed EVP_DigestUpdate size {}",
                      msg->content_size_get());
        md = NULL;
        goto error;
    }

    if (EVP_DigestFinal_ex(md_ctx, digest->read(), &digest_size) <= 0) {
        OFFL_FUNC_ERR("Failed EVP_DigestFinal_ex");
        md = NULL;
        goto error;
    }

    OFFL_FUNC_DEBUG("digest input size {} output size {}",
                    msg->content_size_get(), digest_size);
    digest->content_size_set(digest_size);
    digest->write_thru();

    if (OFFL_IS_LOG_LEVEL_DEBUG()) {
        OFFL_FUNC_DEBUG("digest");
        utils::dump(digest->read(), digest->content_size_get());
    }

error:
    if (!md) {
        ERR_print_errors(eng_if_bio);
    }
    ERR_clear_error();
    if (md_ctx) {
        EVP_MD_CTX_destroy(md_ctx);
    }
    return md;
}


} // namespace eng_intf
