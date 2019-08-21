#include "e_pse.h"
#include "pse_rand.h"
#include <openssl/rand.h>

static const RAND_METHOD *pse_default_method;
static RAND_METHOD pse_rand_method;
static CRYPTO_THREAD_LOCAL mem_method_key;
static CRYPTO_THREAD_LOCAL salt_key;

static int pse_rand_bytes(unsigned char *buf, int num);
static void pse_rand_cleanup(void);

/*
 * Naples HW does not have a DRNG (deterministic random number generator) block
 * so the default OpenSSL software methods will be used.
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

        if (CRYPTO_THREAD_init_local(&mem_method_key, NULL) <= 0) {
            WARN("Failed CRYPTO_THREAD_init_local for mem_method_key");
        }
        if (CRYPTO_THREAD_init_local(&salt_key, NULL) <= 0) {
            WARN("Failed CRYPTO_THREAD_init_local for salt_key");
        }
    }
    return &pse_rand_method; 
}

int pse_RAND_set_mem_method(const PSE_OFFLOAD_MEM_METHOD *method)
{
    return CRYPTO_THREAD_set_local(&mem_method_key, (void *)method);
}

const PSE_OFFLOAD_MEM_METHOD *pse_RAND_get_mem_method(void)
{
    return (const PSE_OFFLOAD_MEM_METHOD *)CRYPTO_THREAD_get_local(&mem_method_key);
}

int pse_RAND_set_salt_val(PSE_OFFLOAD_MEM *salt_val)
{
    return CRYPTO_THREAD_set_local(&salt_key, (void *)salt_val);
}

PSE_OFFLOAD_MEM *pse_RAND_get_salt_val(void)
{
    return (PSE_OFFLOAD_MEM *)CRYPTO_THREAD_get_local(&salt_key);
}

static int pse_rand_bytes(unsigned char *buf, int num)
{
    const PSE_OFFLOAD_MEM_METHOD *mem_method = pse_RAND_get_mem_method();
    PSE_OFFLOAD_MEM     *salt_val = pse_RAND_get_salt_val();
    uint32_t            salt_len;
    uint32_t            offset;
    int                 ret;

    if (!mem_method || !salt_val || (num <= 0)) {
        return pse_default_method->bytes(buf, num);
    }

    /*
     * If salt_len is 0, copy the OpenSSL generated value
     * into salt_val.
     */
    salt_len = mem_method->content_size_get(salt_val);
    INFO("requested %d bytes salt_len %d", num, salt_len);
    if (salt_len == 0) {
        ret = pse_default_method->bytes(buf, num);
        if (ret > 0) {
            salt_len = min(num, mem_method->line_size_get(salt_val));
            memcpy(mem_method->read(salt_val), buf, salt_len);
            mem_method->content_size_set(salt_val, salt_len);
            mem_method->write_thru(salt_val);
        }
        return ret;
    }

    /*
     * Random number will come from salt_val.
     */
    if (num > (int)salt_len) {
        offset = num - salt_len;
        memset(buf, 0, offset);
        memcpy(buf + offset, mem_method->read(salt_val), salt_len);
    } else {
        memcpy(buf, mem_method->read(salt_val), num);
    }
    return 1;
}

static void pse_rand_cleanup(void)
{
    CRYPTO_THREAD_cleanup_local(&mem_method_key);
    CRYPTO_THREAD_cleanup_local(&salt_key);
}

