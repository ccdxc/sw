#include "eng_if.hpp"
#include "utils.hpp"
#include "crypto_sha.hpp"

namespace eng_if {

/*
 * Part of the app_data area in md_ctx is used to store info about
 * SHA offload which will be referenced by PSE. When not set (particularly
 * the field offload_method), PSE will revert back to the applicable
 * default Openssl method.
 */
static bool
md_ctx_app_data_load(EVP_MD_CTX *md_ctx,
                     const eng_evp_md_t *md,
                     void *sha_hw_ctx,
                     bool wait_for_completion)
{
    PSE_MD_APP_DATA     *app_data;
    uint32_t            app_data_size;

    app_data_size = EVP_MD_meth_get_app_datasize(md);
    if (app_data_size >= sizeof(PSE_MD_APP_DATA)) {
        app_data = (PSE_MD_APP_DATA *)((uint8_t *)EVP_MD_CTX_md_data(md_ctx) +
                                       PSE_MD_APP_DATA_OFFS(app_data_size));
        app_data->sha_hw_ctx = sha_hw_ctx;
        app_data->offload_method = &crypto_sha::pse_md_offload_method;
        app_data->wait_for_completion = wait_for_completion;
        return true;
    }
    return false;
}


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
    uint8_t             *msg_input;
    uint8_t             *hash_output;

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
    md = hash_algo_find(params.hash_algo());
    if (!md) {
        OFFL_FUNC_ERR("Failed to find md for {}", params.hash_algo());
        goto error;
    }

    if (params.msg_is_component()) {

        /*
         * Input msg was already hashed so just copy it
         */
        if (!dp_mem_to_dp_mem(digest, msg)) {
            OFFL_FUNC_ERR("Failed to initialize component digest");
            md = NULL;
            goto error;
        }
        goto done;
    }

    md_ctx = EVP_MD_CTX_create();
    if (!md_ctx) {
        OFFL_FUNC_ERR("Failed to create EVP_MD_CTX");
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
     * Supply offload method; must be done after EVP_DigestInit_ex()
     * has allocated the app_data.
     */
    if (params.sha_hw_ctx()) {
        if (!md_ctx_app_data_load(md_ctx, md, params.sha_hw_ctx(),
                                  params.wait_for_completion())) {
            OFFL_FUNC_ERR("Failed to load app_data for EVP_MD_CTX");
            md = NULL;
            goto error;
        }

        /*
         * HW offload prefers to work directly with dp_mem.
         */
        msg_input = (uint8_t *)msg;
        hash_output = (uint8_t *)digest;

    } else {
        msg_input = msg->read();
        hash_output = digest->read();
    }

    /* 
     * Calculate the digest
     */
    if (EVP_DigestUpdate(md_ctx, msg_input,
                         msg->content_size_get()) <= 0) {
        OFFL_FUNC_ERR("Failed EVP_DigestUpdate size {}",
                      msg->content_size_get());
        md = NULL;
        goto error;
    }

    if (EVP_DigestFinal_ex(md_ctx, hash_output,
                           &digest_size) <= 0) {
        OFFL_FUNC_ERR("Failed EVP_DigestFinal_ex");
        md = NULL;
        goto error;
    }

    OFFL_FUNC_DEBUG("digest input size {} output size {}",
                    msg->content_size_get(), digest_size);
    digest->content_size_set(digest_size);
    if (!params.sha_hw_ctx()) {
        digest->write_thru();
    }

done:
    if (OFFL_IS_LOG_LEVEL_DEBUG() && params.wait_for_completion()) {
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
