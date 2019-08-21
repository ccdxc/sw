#include "eng_if.hpp"
#include "utils.hpp"

namespace eng_if {

/*
 * Hash (digest) algorithms map
 */
static const map<string,const eng_evp_md_t*> hash_algo_map = {
    {"SHA1",        EVP_sha1()},
    {"SHA224",      EVP_sha224()},
    {"SHA256",      EVP_sha256()},
    {"SHA384",      EVP_sha384()},
    {"SHA512",      EVP_sha512()},

#ifdef OPENSSL_WITH_TRUNCATED_SHA_SUPPORT
    {"SHA512224",   EVP_sha512_224()},
    {"SHA512256",   EVP_sha512_256()},
#endif
};


/*
 * Interface to DOL OpenSSL engine
 */
ENGINE          *eng_if_engine;
BIO             *eng_if_bio;

/*
 * Initialize this instance of OpenSSL library with the PSE engine
 */
bool
init(const char *engine_path)
{
    bool        success = false;

    if (eng_if_engine) {
        return true;
    }

    SSL_library_init();

    /*
     * Basic I/O is used to interface with, among other things,
     * Openssl ERR_print facilities.
     */
    eng_if_bio = BIO_new_fp(stdout, BIO_NOCLOSE);

    ENGINE_load_dynamic();
    ENGINE_load_builtin_engines();
    ENGINE_register_all_complete();

    eng_if_engine = ENGINE_by_id("dynamic");
    if (eng_if_engine == NULL) {
        OFFL_FUNC_ERR("failed to load dynamic engine");
        goto error;
    }

    OFFL_FUNC_INFO("loading PSE engine from path {}", engine_path);

    if (!ENGINE_ctrl_cmd_string(eng_if_engine, "SO_PATH", engine_path, 0)) {
       OFFL_FUNC_ERR("failed SO_PATH PSE engine load from {}",
                     engine_path);
       goto error;
    }

    if (!ENGINE_ctrl_cmd_string(eng_if_engine, "ID", "pse", 0)) {
        OFFL_FUNC_ERR("failed engine ID");
        goto error;
    }

    if (!ENGINE_ctrl_cmd_string(eng_if_engine, "LOAD", NULL, 0)) {
        OFFL_FUNC_ERR("ENGINE LOAD_ADD failed");
        goto error;
    }
    if (ENGINE_init(eng_if_engine) <= 0) {
        OFFL_FUNC_ERR("ENGINE init failed");
        goto error;
    }
    OFFL_FUNC_INFO("successfully loaded PSE OpenSSL Engine: {}",
                   ENGINE_get_name(eng_if_engine));

    ENGINE_set_default_EC(eng_if_engine);
    ENGINE_set_default_RSA(eng_if_engine);
    success = true;

error:
    ERR_print_errors(eng_if_bio);
    return success;
}

/*
 * Locate matching message digest function
 */
const eng_evp_md_t *
hash_algo_find(const string& hash_algo)
{
    auto iter = hash_algo_map.find(hash_algo);
    if (iter != hash_algo_map.end()) {
        return iter->second;
    }
    return nullptr;
}


/*
 * Copy BIGNUM content into dp_mem with padding up to to_len.
 * If to_len is zero, then apply padding up to line length.
 * If to_len is -1, then apply no padding.
 */
bool
bn_to_dp_mem_pad(const BIGNUM *bn,
                 dp_mem_t *mem,
                 int to_len)
{
    int         line_len = mem->line_size_get();
    int         bn_len;

    /*
     * First, ensure line_len is sufficient for bn
     */
    bn_len = BN_num_bytes(bn);
    if (line_len < bn_len) {
        OFFL_FUNC_ERR("line_len {} < bn_len {}", line_len, bn_len);
        return false;
    }

    if ((to_len == 0) || (to_len > line_len)) {
        to_len = line_len;
    }

    /*
     * BN_bn2binpad() automatically skips padding when to_len is -1
     */
    bn_len = BN_bn2binpad(bn, mem->read(), to_len);
    if (bn_len <= 0) {
        OFFL_FUNC_ERR("Failed BN_bn2binpad");
        return false;
    }

    mem->content_size_set(bn_len);
    mem->write_thru();
    return true;
}

/*
 * Copy BIGNUM content into dp_mem (no padding)
 */
bool
bn_to_dp_mem(const BIGNUM *bn,
             dp_mem_t *mem)
{
    return bn_to_dp_mem_pad(bn, mem, -1);
}

/*
 * Create and copy dp_mem into BIGNUM
 */
BIGNUM *
dp_mem_to_bn(dp_mem_t *mem)
{
    return BN_bin2bn(mem->read(), mem->content_size_get(), NULL);
}


/*
 * Copy src dp_mem to dst dp_mem
 */
bool
dp_mem_to_dp_mem(dp_mem_t *dst,
                 dp_mem_t *src)
{
    uint32_t    copy_len = src->content_size_get();
    if (dst->line_size_get() < copy_len) {
        OFFL_FUNC_ERR("dst size %u too small - need to be at least %u",
                      dst->line_size_get(), copy_len);
        return false;
    }

    memcpy(dst->read(), src->read_thru(), copy_len);
    dst->content_size_set(copy_len);
    dst->write_thru();
    return true;
}


/*
 * Pad a dp_mem message (could also be a modulus n, or e, or d, etc.) 
 * to the desired tolen.
 */
bool
dp_mem_pad_in_place(dp_mem_t *msg,
                    uint32_t to_len)
{
    BIGNUM      *bn = NULL;
    bool        success = true;

    assert(msg->content_size_get() && (msg->line_size_get() >= to_len));

    if (msg->content_size_get() != to_len) {
        bn = BN_bin2bn(msg->read(), msg->content_size_get(), NULL);
        if (!bn) {
            OFFL_FUNC_ERR("failed BN_bin2bn");
            success = false;
            goto error;
        }

        success = bn_to_dp_mem_pad(bn, msg, to_len);
    }

error:
    if (bn) {
        BN_clear_free(bn);
    }
    return success;
}

/*
 * Remove padding from a dp_mem message.
 */
bool
dp_mem_unpad_in_place(dp_mem_t *msg,
                      uint32_t to_len)
{
    uint32_t    cur_len;

    /*
     * Note: we don't use BN_bin2bn() to remove padding because it
     * assumes the pad bytes are zero. Also, we follow BN convention
     * that padded bytes are in the front.
     */
    cur_len = msg->line_size_get();
    if (cur_len > to_len) {
        uint8_t *p = msg->read_thru();
        memmove(p, p + cur_len - to_len, to_len);
        msg->content_size_set(to_len);
        msg->write_thru();
    }
    return true;
}

/*
 * Truncate a dp_mem message to a bits_len and then pad it to the next
 * byte boundary.
 */
bool
dp_mem_trunc_in_place(dp_mem_t *msg,
                      uint32_t bits_len)
{
    BIGNUM      *bn = NULL;
    uint32_t    msg_len;
    uint32_t    ceil_bytes_len;
    bool        success = false;

    ceil_bytes_len = (bits_len + (BITS_PER_BYTE - 1)) / BITS_PER_BYTE;
    msg_len = msg->content_size_get();
    if ((msg_len * BITS_PER_BYTE) != bits_len) {

        /*
         * First truncate whole bytes.
         */
        msg_len = ceil_bytes_len;

        /* 
         * If still too long truncate remaining bits with a shift
         */
        if ((BITS_PER_BYTE * msg_len) > bits_len) {
            bn = BN_bin2bn(msg->read(), msg_len, NULL);
            if (!bn) {
                OFFL_FUNC_ERR("Failed BN_bin2bn");
                goto error;
            }

            if (!BN_rshift(bn, bn, 
                           BITS_PER_BYTE - (bits_len & (BITS_PER_BYTE - 1)))) {
                OFFL_FUNC_ERR("Failed BN_rshift");
                goto error;
            }

            if (!bn_to_dp_mem_pad(bn, msg, msg_len)) {
                OFFL_FUNC_ERR("Failed bn_to_dp_mem_pad");
                goto error;
            }
        }
        msg->content_size_set(msg_len);
    }

    success = true;

error:
    if (bn) {
        BN_clear_free(bn);
    }
    return success;
}

} // namespace eng_if

