#include "eng_if.hpp"
#include "utils.hpp"
#include "crypto_aes.hpp"
#include "crypto_symm.hpp"

namespace eng_if {

/*
 * Initialize and return a cipher context.
 */
eng_evp_cipher_ctx_t *
cipher_init(cipher_init_params_t& params)
{
    EVP_CIPHER_CTX          *cipher_ctx = NULL;
    PSE_CIPHER_APP_DATA     *app_data = NULL;
    const eng_evp_cipher_t  *cipher = NULL;
    const uint8_t           *key;

    cipher_ctx = EVP_CIPHER_CTX_new();
    if (!cipher_ctx) {
        OFFL_FUNC_ERR("Failed to create EVP_CIPHER_CTX");
        goto error;
    }

    app_data = (PSE_CIPHER_APP_DATA *)OPENSSL_zalloc(sizeof(*app_data));
    if (!app_data) {
        OFFL_FUNC_ERR("Failed to create app_data");
        goto error;
    }
    EVP_CIPHER_CTX_set_app_data(cipher_ctx, app_data);

    /*
     * HW offload prefers to work directly with dp_mem for key.
     */
    if (params.cipher_hw_ctx()) {
        app_data->cipher_hw_ctx = params.cipher_hw_ctx();
        app_data->offload_method = &crypto_aes::pse_cipher_offload_method;
        key = (const uint8_t *)params.key();

    } else {
        key = params.key()->read();
    }

    cipher = cipher_algo_find(params.crypto_symm_type(),
                              params.key()->content_size_get());
    if (!cipher) {
        OFFL_FUNC_ERR("Failed to find cipher for crypto_symm_type {} "
                      "key_len {}", params.crypto_symm_type(),
                      params.key()->content_size_get());
        goto error;
    }

    if (EVP_CipherInit_ex(cipher_ctx, cipher, NULL, key,
                          params.iv()->read(), params.encrypt()) <= 0) {
        OFFL_FUNC_ERR("Failed to initialize EVP_CIPHER_CTX");
        cipher = NULL;
        goto error;
    }

    return cipher_ctx;

error:
    if (!cipher) {
        ERR_print_errors(eng_if_bio);
    }
    ERR_clear_error();
    if (app_data) {
        OPENSSL_free(app_data);
    }
    if (cipher_ctx) {
        EVP_CIPHER_CTX_free(cipher_ctx);
    }
    return nullptr;
}

/*
 * Execute a cipher operation.
 */
bool
cipher_update(cipher_update_params_t& params)
{
    EVP_CIPHER_CTX      *cipher_ctx = params.cipher_sw_ctx();
    dp_mem_t            *msg_input = params.msg_input();
    dp_mem_t            *msg_output = params.msg_output();
    PSE_CIPHER_APP_DATA *app_data;
    uint8_t             *in;
    uint8_t             *out;
    int                 outl;
    bool                success = false;

    /* 
     * Supply offload method; must be done after EVP_CipherInit_ex()
     * has allocated the ctx app_data.
     */
    app_data = (PSE_CIPHER_APP_DATA *)EVP_CIPHER_CTX_get_app_data(cipher_ctx);
    if (app_data->cipher_hw_ctx) {
        app_data->key_idx = params.key_idx();
        app_data->wait_for_completion = params.wait_for_completion();

        /*
         * HW offload prefers to work directly with dp_mem.
         */
        in = (uint8_t *)msg_input;
        out = (uint8_t *)msg_output;

    } else {
        in = msg_input->read();
        out = msg_output->read();
    }

    /* 
     * Calculate the cipher
     */
    if (EVP_CipherUpdate(cipher_ctx, out, &outl, in,
                         msg_input->content_size_get()) <= 0) {
        OFFL_FUNC_ERR("Failed EVP_CipherUpdate size {}",
                      msg_input->content_size_get());
        goto error;
    }

    if (EVP_CipherFinal_ex(cipher_ctx, out, &outl) <= 0) {
        OFFL_FUNC_ERR("Failed EVP_CipherFinal_ex");
        goto error;
    }

    if (!app_data->cipher_hw_ctx) {
        msg_output->write_thru();
    }

    OFFL_FUNC_DEBUG("msg input size {} output size {}",
                    msg_input->content_size_get(),
                    msg_output->content_size_get());
    if (OFFL_IS_LOG_LEVEL_DEBUG() && params.wait_for_completion()) {
        OFFL_FUNC_DEBUG("output");
        utils::dump(msg_output->read(), msg_output->content_size_get());
    }

    success = true;

error:
    if (!success) {
        ERR_print_errors(eng_if_bio);
    }
    ERR_clear_error();

    /*
     * Caution: app_data must be left untouched so the engine's cipher cleanup
     * method can be called during EVP_CIPHER_CTX_free(). The app_data is to
     * be freed after EVP_CIPHER_CTX_free() returns.
     */
    EVP_CIPHER_CTX_free(cipher_ctx);
    OPENSSL_free(app_data);
    return success;
}

} // namespace eng_intf
