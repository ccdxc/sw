#include "e_pse.h"
#include "pse_rand.h"
#include <openssl/rand.h>

static const RAND_METHOD *pse_default_method;
static RAND_METHOD pse_rand_method;
static CRYPTO_THREAD_LOCAL ex_data_key;

static int pse_rand_bytes(unsigned char *buf, int num);
static void pse_rand_cleanup(void);

/*
 * DRBG (deterministic random bit generator)
 *
 * This module also supports a mode where the random number (salt value) can be
 * preset by a test program (such as FIPS).
 */
RAND_METHOD *pse_get_RAND_methods(void) 
{
    if (!pse_default_method) {
        pse_default_method = RAND_get_rand_method();
        pse_rand_method = *pse_default_method;

        pse_rand_method.bytes = &pse_rand_bytes;
        pse_rand_method.cleanup = &pse_rand_cleanup;
        RAND_set_rand_method(&pse_rand_method);

        if (CRYPTO_THREAD_init_local(&ex_data_key, NULL) <= 0) {
            WARN("Failed CRYPTO_THREAD_init_local for ex_data_key");
        }
    }
    return &pse_rand_method; 
}

int pse_rand_set_ex_data(const PSE_RAND_EX_DATA *ex_data)
{
    return CRYPTO_THREAD_set_local(&ex_data_key, (void *)ex_data);
}

const PSE_RAND_EX_DATA *pse_rand_get_ex_data(void)
{
    return (const PSE_RAND_EX_DATA *)CRYPTO_THREAD_get_local(&ex_data_key);
}

static const PSE_OFFLOAD_MEM_METHOD *pse_rand_get_mem_method(void)
{
    const PSE_RAND_EX_DATA *ex_data = pse_rand_get_ex_data();
    return ex_data && ex_data->rand_method ? ex_data->rand_method->mem_method : NULL;
}

static PSE_OFFLOAD_MEM *pse_rand_get_salt_val(void)
{
    const PSE_RAND_EX_DATA *ex_data = pse_rand_get_ex_data();
    return ex_data ? ex_data->salt_val : NULL;
}

static int pse_rand_get_random(unsigned char *buf, int num)
{
    const PSE_RAND_EX_DATA      *ex_data = pse_rand_get_ex_data();
    PSE_RAND_BYTES_PARAM        param;

    if (ex_data && ex_data->rand_method && (num > 0)) {
        pse_rand_bytes_param_init(&param);
        param.buf = buf;
        param.size = num;
        return ex_data->rand_method->bytes(ex_data->rand_ctx, &param);
    }
    return pse_default_method->bytes(buf, num);
}

static int pse_rand_bytes(unsigned char *buf, int num)
{
    const PSE_OFFLOAD_MEM_METHOD *mem_method = pse_rand_get_mem_method();
    PSE_OFFLOAD_MEM     *salt_val = pse_rand_get_salt_val();
    uint32_t            salt_val_len;
    uint32_t            copy_len;
    uint32_t            offset;
    int                 ret;

    if (!mem_method || !salt_val) {
        return pse_rand_get_random(buf, num);
    }

    /*
     * If salt_val_len is 0, copy the generated random
     * into salt_val.
     */
    salt_val_len = mem_method->content_size_get(salt_val);
    INFO("requested %d bytes salt_val_len %d", num, salt_val_len);
    if (salt_val_len == 0) {
        ret = pse_rand_get_random(buf, num);
        if (ret > 0) {
            copy_len = min(num, mem_method->line_size_get(salt_val));
            memcpy(mem_method->read(salt_val), buf, copy_len);
            mem_method->content_size_set(salt_val, copy_len);
            mem_method->write_thru(salt_val);
        }
        return ret;
    }

    /*
     * Random number will come from salt_val.
     */
    if (num > (int)salt_val_len) {
        offset = num - salt_val_len;
        memset(buf, 0, offset);
        memcpy(buf + offset, mem_method->read(salt_val), salt_val_len);
    } else {
        memcpy(buf, mem_method->read(salt_val), num);
    }
    return 1;
}

static void pse_rand_cleanup(void)
{
    CRYPTO_THREAD_cleanup_local(&ex_data_key);
}

