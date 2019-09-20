#include "eng_if.hpp"
#include "utils.hpp"

namespace eng_if {

/*
 * Hash (digest) algorithms map
 */
static const map<string,int> hash_algo_map = {
    {"SHA1",        NID_sha1},
    {"SHA224",      NID_sha224},
    {"SHA256",      NID_sha256},
    {"SHA384",      NID_sha384},
    {"SHA512",      NID_sha512},

#ifdef OPENSSL_WITH_TRUNCATED_SHA_SUPPORT
    {"SHA512224",   NID_sha512_224},
    {"SHA512256",   NID_sha512_256},
#endif
#ifdef OPENSSL_WITH_SHA3_SUPPORT
    {"SHA3-224",    NID_sha3_224},
    {"SHA3-256",    NID_sha3_256},
    {"SHA3-384",    NID_sha3_384},
    {"SHA3-512",    NID_sha3_512},
#endif
};

/*
 * Cipher algorithms map
 */
typedef uint64_t        cipher_algo_key_t;

#define CIPHER_ALGO_KEY_MAKE(type, key_len)     \
    (((cipher_algo_key_t)(type) << 32) | (key_len))

static const map<cipher_algo_key_t,int> cipher_algo_map = {
    {CIPHER_ALGO_KEY_MAKE(crypto_symm::CRYPTO_SYMM_TYPE_AES_CBC,
                         128 / BITS_PER_BYTE), NID_aes_128_cbc},
    {CIPHER_ALGO_KEY_MAKE(crypto_symm::CRYPTO_SYMM_TYPE_AES_CBC,
                          192 / BITS_PER_BYTE), NID_aes_192_cbc},
    {CIPHER_ALGO_KEY_MAKE(crypto_symm::CRYPTO_SYMM_TYPE_AES_CBC,
                          256 / BITS_PER_BYTE), NID_aes_256_cbc},
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

    ENGINE_set_default_digests(eng_if_engine);
    ENGINE_set_default_ciphers(eng_if_engine);
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
        return ENGINE_get_digest(eng_if_engine, iter->second);
    }
    return nullptr;
}


/*
 * Locate matching cipher function
 */
const eng_evp_cipher_t *
cipher_algo_find(crypto_symm::crypto_symm_type_t type,
                 uint32_t key_len)
{
    auto iter = cipher_algo_map.find(CIPHER_ALGO_KEY_MAKE(type, key_len));
    if (iter != cipher_algo_map.end()) {
        return ENGINE_get_cipher(eng_if_engine, iter->second);
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
 * Copy src dp_mem to dst dp_mem, starting from a given
 * src offset.
 */
bool
dp_mem_to_dp_mem(dp_mem_t *dst,
                 dp_mem_t *src,
                 uint32_t src_offs)
{
    uint32_t    copy_len = src->content_size_get() > src_offs ?
                           src->content_size_get() - src_offs : 0;
    if (dst->line_size_get() < copy_len) {
        OFFL_FUNC_ERR("dst size {} too small - need to be at least {}",
                      dst->line_size_get(), copy_len);
        return false;
    }

    if (dst == src) {
        memmove(dst->read(), src->read_thru() + src_offs, copy_len);
    } else {
        memcpy(dst->read(), src->read_thru() + src_offs, copy_len);
    }
    dst->content_size_set(copy_len);
    dst->write_thru();
    return true;
}


/*
 * Concatenate src dp_mem to dst dp_mem
 */
bool
dp_mem_cat_dp_mem(dp_mem_t *dst,
                  dp_mem_t *src)
{
    uint32_t    src_len = src->content_size_get();
    uint32_t    dst_len = dst->content_size_get();
    uint32_t    total_len = dst_len + src_len;
    if (dst->line_size_get() < total_len) {
        OFFL_FUNC_ERR("dst size {} too small - need to be at least {}",
                      dst->line_size_get(), total_len);
        return false;
    }

    if (dst == src) {
        memmove(dst->read() + dst_len, src->read_thru(), src_len);
    } else {
        memcpy(dst->read() + dst_len, src->read_thru(), src_len);
    }
    dst->content_size_set(total_len);
    dst->write_thru();
    return true;
}


/*
 * XOR src dp_mem to dst dp_mem
 */
bool
dp_mem_xor_dp_mem(dp_mem_t *dst,
                  dp_mem_t *src)
{
    /*
     * XOR allows src and dst of different lengths
     */
    uint32_t    xor_nbytes = min(dst->content_size_get(),
                                 src->content_size_get());
    uint32_t    xor_nints = xor_nbytes / sizeof(int);
    uint32_t    rem_nbytes = xor_nbytes - (xor_nints * sizeof(int));
    int         *isp;
    int         *idp;
    char        *sp;
    char        *dp;

    isp = (int *)src->read_thru();
    idp = (int *)dst->read();
    while (xor_nints--) {
        *idp++ ^= *isp++;
    }

    sp = (char *)isp;
    dp = (char *)idp;
    while (rem_nbytes--) {
        *dp++ ^= *sp++;
    }
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

