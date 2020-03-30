#include "include/sdk/base.hpp"
#include "include/sdk/crypto_apis.hpp"
#include "platform/capri/capri_barco_asym_apis.hpp"
#include "platform/capri/capri_barco_sym_apis.hpp"

namespace sdk {
namespace platform {
namespace capri {

#define SHA1_DIGEST_LENGTH      20
#define SHA224_DIGEST_LENGTH    28
#define SHA256_DIGEST_LENGTH    32
#define SHA384_DIGEST_LENGTH    48
#define SHA512_DIGEST_LENGTH    64

unsigned char    data[] = "abc";
uint8_t digest_output[128];

//Input: abc, 3
static const uint8_t sha1_digest[SHA1_DIGEST_LENGTH] = {
    0xa9, 0x99, 0x3e, 0x36, 0x47, 0x06, 0x81, 0x6a,
    0xba, 0x3e, 0x25, 0x71, 0x78, 0x50, 0xc2, 0x6c,
    0x9c, 0xd0, 0xd8, 0x9d
};

//Input: abc, 3
static const uint8_t sha224_digest[SHA224_DIGEST_LENGTH] = {
    0x23, 0x09, 0x7d, 0x22, 0x34, 0x05, 0xd8, 0x22,
    0x86, 0x42, 0xa4, 0x77, 0xbd, 0xa2, 0x55, 0xb3,
    0x2a, 0xad, 0xbc, 0xe4, 0xbd, 0xa0, 0xb3, 0xf7,
    0xe3, 0x6c, 0x9d, 0xa7
};

//Input: abc, 3
static const uint8_t sha256_digest[SHA256_DIGEST_LENGTH] = {
    0xba, 0x78, 0x16, 0xbf, 0x8f, 0x01, 0xcf, 0xea,
    0x41, 0x41, 0x40, 0xde, 0x5d, 0xae, 0x22, 0x23,
    0xb0, 0x03, 0x61, 0xa3, 0x96, 0x17, 0x7a, 0x9c,
    0xb4, 0x10, 0xff, 0x61, 0xf2, 0x00, 0x15, 0xad
};

//Input: abc, 3
static const uint8_t sha384_digest[SHA384_DIGEST_LENGTH] = {
    0xcb, 0x00, 0x75, 0x3f, 0x45, 0xa3, 0x5e, 0x8b,
    0xb5, 0xa0, 0x3d, 0x69, 0x9a, 0xc6, 0x50, 0x07,
    0x27, 0x2c, 0x32, 0xab, 0x0e, 0xde, 0xd1, 0x63,
    0x1a, 0x8b, 0x60, 0x5a, 0x43, 0xff, 0x5b, 0xed,
    0x80, 0x86, 0x07, 0x2b, 0xa1, 0xe7, 0xcc, 0x23,
    0x58, 0xba, 0xec, 0xa1, 0x34, 0xc8, 0x25, 0xa7
};

//Input: abc, 3
static const uint8_t sha512_digest[SHA512_DIGEST_LENGTH] = {
    0xdd, 0xaf, 0x35, 0xa1, 0x93, 0x61, 0x7a, 0xba,
    0xcc, 0x41, 0x73, 0x49, 0xae, 0x20, 0x41, 0x31,
    0x12, 0xe6, 0xfa, 0x4e, 0x89, 0xa9, 0x7e, 0xa2,
    0x0a, 0x9e, 0xee, 0xe6, 0x4b, 0x55, 0xd3, 0x9a,
    0x21, 0x92, 0x99, 0x2a, 0x27, 0x4f, 0xc1, 0xa8,
    0x36, 0xba, 0x3c, 0x23, 0xa3, 0xfe, 0xeb, 0xbd,
    0x45, 0x4d, 0x44, 0x23, 0x64, 0x3c, 0xe8, 0x0e,
    0x2a, 0x9a, 0xc9, 0x4f, 0xa5, 0x4c, 0xa4, 0x9f
};

static struct st {
    unsigned char key[16];
    int key_len;
    unsigned char data[64];
    int data_len;
    uint8_t digest[64];
} hmac_sha[5] = {
    {//SHA224
        "123456", 6, "My test data", 12,
        {
	  0x96, 0xa3, 0x54, 0x46, 0x71, 0xc6, 0x5d, 0xaf,
	  0x80, 0xe6, 0xfb, 0xae, 0xd9, 0xc1, 0x5f, 0xee,
	  0x37, 0xf9, 0x83, 0x5b, 0x2b, 0x65, 0x5c, 0xc2,
	  0x2b, 0x70, 0xab, 0x3c
	}
    },
    {//SHA256
        "123456", 6, "My test data", 12,
        {
	  0xba, 0xb5, 0x30, 0x58, 0xae, 0x86, 0x1a, 0x7f,
	  0x19, 0x1a, 0xbe, 0x2d, 0x01, 0x45, 0xcb, 0xb1,
	  0x23, 0x77, 0x6a, 0x63, 0x69, 0xee, 0x3f, 0x9d,
	  0x79, 0xce, 0x45, 0x56, 0x67, 0xe4, 0x11, 0xdd
	}
    },
    {//SHA384
        "123456", 6, "My test data", 12,
        {
          0x51, 0x75, 0xf6, 0x21, 0x47, 0x0a, 0x03, 0x98,
	  0x32, 0xf2, 0xc2, 0xf5, 0x15, 0x04, 0xa5, 0x81,
	  0x32, 0xc1, 0x5b, 0x52, 0x57, 0x34, 0x67, 0xb8,
          0x34, 0x99, 0x6e, 0xda, 0xc3, 0x11, 0x05, 0x72,
          0xb4, 0x90, 0x85, 0x2b, 0x0a, 0xe2, 0x48, 0xf2,
          0xc9, 0xd7, 0xb9, 0x33, 0xd2, 0xe1, 0xda, 0xdf
	}
    },
    {//SHA512
        "123456", 6, "My test data", 12,
        {
          0xef, 0x6e, 0x05, 0x97, 0xb0, 0x91, 0x70, 0x20,
	  0xec, 0xda, 0x1d, 0x79, 0xb1, 0x9a, 0xa3, 0x18,
	  0xca, 0xf2, 0xd2, 0x40, 0x29, 0x79, 0x7c, 0xca,
	  0xfb, 0xe5, 0xbb, 0x2a, 0x7d, 0xbc, 0x56, 0x7d,
	  0x2f, 0x3e, 0x41, 0x92, 0xb3, 0x2e, 0xbc, 0x4e,
	  0xfc, 0xa8, 0x56, 0x07, 0xd0, 0x46, 0xab, 0xd9,
	  0xd9, 0xce, 0x98, 0x07, 0xe0, 0x14, 0x3f, 0xb6,
	  0xdd, 0xca, 0xb1, 0xcb, 0xd4, 0x30, 0x81, 0x2d
	}
    },
    {//SHA1
        "12345", 5, "My test data again", 18,
	{
	  0xa1, 0x23, 0x96, 0xce, 0xdd, 0xd2, 0xa8, 0x5f,
	  0x4c, 0x65, 0x6b, 0xc1, 0xe0, 0xaa, 0x50, 0xc7,
	  0x8c, 0xff, 0xde, 0x3e
	}
    }
};

static struct st2 {
    unsigned char key[64];
    int key_len;
    unsigned char iv[64];
    int iv_len;
    uint8_t aad[128];
    int aad_len;
    uint8_t auth_tag[64];
    int auth_tag_len;
    uint8_t plaintext[128];
    int plaintext_len;
    uint8_t ciphertext[128];
    int ciphertext_len;
} aes_enc[2] = {
  {//AES-CCM
    {//key
      0x1b, 0xde, 0x32, 0x51, 0xd4, 0x1a, 0x8b, 0x5e,
      0xa0, 0x13, 0xc1, 0x95, 0xae, 0x12, 0x8b, 0x21,
      0x8b, 0x3e, 0x03, 0x06, 0x37, 0x63, 0x57, 0x07,
      0x7e, 0xf1, 0xc1, 0xc7, 0x85, 0x48, 0xb9, 0x2e
    }, 32,
    {//IV
      0x5b, 0x8e, 0x40, 0x74, 0x6f, 0x6b, 0x98, 0xe0,
      0x0f, 0x1d, 0x13, 0xff, 0x41
    }, 13,
    {//AAD (header)
      0xc1, 0x7a, 0x32, 0x51, 0x4e, 0xb6, 0x10, 0x3f,
      0x32, 0x49, 0xe0, 0x76, 0xd4, 0xc8, 0x71, 0xdc,
      0x97, 0xe0, 0x4b, 0x28, 0x66, 0x99, 0xe5, 0x44,
      0x91, 0xdc, 0x18, 0xf6, 0xd7, 0x34, 0xd4, 0xc0
    }, 32,
    {//Auth-tag
      0x20, 0x24, 0x93, 0x1d, 0x73, 0xbc, 0xa4, 0x80,
      0xc2, 0x4a, 0x24, 0xec, 0xe6, 0xb6, 0xc2, 0xbf
    }, 16,
    {//plaintext
      0x53, 0xbd, 0x72, 0xa9, 0x70, 0x89, 0xe3, 0x12,
      0x42, 0x2b, 0xf7, 0x2e, 0x24, 0x23, 0x77, 0xb3,
      0xc6, 0xee, 0x3e, 0x20, 0x75, 0x38, 0x9b, 0x99,
      0x9c, 0x4e, 0xf7, 0xf2, 0x8b, 0xd2, 0xb8, 0x0a
    }, 32,
    {//ciphertext
      0x9a, 0x5f, 0xcc, 0xcd, 0xb4, 0xcf, 0x04, 0xe7,
      0x29, 0x3d, 0x27, 0x75, 0xcc, 0x76, 0xa4, 0x88,
      0xf0, 0x42, 0x38, 0x2d, 0x94, 0x9b, 0x43, 0xb7,
      0xd6, 0xbb, 0x2b, 0x98, 0x64, 0x78, 0x67, 0x26
    }, 32
  },
  {//AES-CCM
    {//key
      0x30, 0xdb, 0x63, 0x16, 0x2b, 0x2f, 0x4e, 0xb6,
      0xce, 0x4b, 0xbd, 0x21, 0x7e, 0xf7, 0x64, 0xc6,
      0xdb, 0xfb, 0xf9, 0xec, 0x3b, 0x83, 0x8b, 0xa9,
      0x19, 0xf8, 0xfd, 0xb8, 0x81, 0xbd, 0x9a, 0xc3
    }, 32,
    {//IV
      0x1a, 0x8c, 0x86, 0x99, 0x94, 0xcb, 0x98, 0x62,
      0x80, 0xff, 0xdb, 0x23
    }, 12,
    {//AAD (header)
      0x17, 0x03, 0x03, 0x00, 0x58, 0x94, 0xcb, 0x98,
      0x62, 0x80, 0xff, 0xdb, 0x23
    }, 13,
    {//Auth-tag
      0x48, 0xfe, 0xa4, 0x1b, 0xe5, 0x8f, 0xe3, 0x96,
      0x2f, 0x66, 0x8d, 0xad, 0x79, 0x5f, 0x9a, 0xb1
    }, 16,
    {//plaintext
      0x53, 0xbd, 0x72, 0xa9, 0x70, 0x89, 0xe3, 0x12,
      0x42, 0x2b, 0xf7, 0x2e, 0x24, 0x23, 0x77, 0xb3,
      0xc6, 0xee, 0x3e, 0x20, 0x75, 0x38, 0x9b, 0x99,
      0x9c, 0x4e, 0xf7, 0xf2, 0x8b, 0xd2, 0xb8, 0x0a,
      0x9a, 0x5f, 0xcc, 0xcd, 0xb4, 0xcf, 0x04, 0xe7,
      0x29, 0x3d, 0x27, 0x75, 0xcc, 0x76, 0xa4, 0x88,
      0xf0, 0x42, 0x38, 0x2d, 0x94, 0x9b, 0x43, 0xb7,
      0xd6, 0xbb, 0x2b, 0x98, 0x64, 0x78, 0x67, 0x26
    }, 64,
    {//ciphertext
      0xa3, 0xf8, 0xea, 0x42, 0xe5, 0xcb, 0x44, 0x28,
      0x17, 0x27, 0xe5, 0x2b, 0xdc, 0xc8, 0x5e, 0x5c,
      0x1a, 0xbc, 0x6e, 0x6c, 0x7e, 0x58, 0x41, 0x7f,
      0xe1, 0xed, 0x46, 0x6d, 0xee, 0x24, 0xa8, 0xef,
      0xbd, 0xef, 0xbf, 0x21, 0x5a, 0x72, 0x21, 0xa1,
      0x8e, 0x99, 0x02, 0xcc, 0x07, 0xeb, 0x39, 0x29,
      0x6c, 0x16, 0x3a, 0x4f, 0x88, 0xd9, 0xa5, 0x3f,
      0xc4, 0x12, 0xe8, 0x78, 0x44, 0xc2, 0xa2, 0x60
    }, 64
  }
};

#define _API_PARAM_DEBUG_

sdk_ret_t
capri_barco_sym_hash_sha_test (CryptoApiHashType hash_type, bool generate)
{
    sdk_ret_t           ret = SDK_RET_OK;
    const uint8_t       *exp_digest;
    int                 exp_digestlen, key_len, input_datalen;
    unsigned char       *key, *input_data;

    key = NULL;
    key_len  = 0;
    input_data = data;
    input_datalen = 3;

    memset(digest_output, 0, sizeof(digest_output));

    switch (hash_type) {
    case CRYPTOAPI_HASHTYPE_SHA1:
      exp_digest = sha1_digest;
      exp_digestlen = sizeof(sha1_digest);
      break;
    case CRYPTOAPI_HASHTYPE_SHA224:
      exp_digest = sha224_digest;
      exp_digestlen = sizeof(sha224_digest);
      break;
    case CRYPTOAPI_HASHTYPE_SHA256:
      exp_digest = sha256_digest;
      exp_digestlen = sizeof(sha256_digest);
      break;
    case CRYPTOAPI_HASHTYPE_SHA384:
      exp_digest = sha384_digest;
      exp_digestlen = sizeof(sha384_digest);
      break;
    case CRYPTOAPI_HASHTYPE_SHA512:
      exp_digest = sha512_digest;
      exp_digestlen = sizeof(sha512_digest);
      break;
    case CRYPTOAPI_HASHTYPE_HMAC_SHA224:
      key = hmac_sha[0].key;
      key_len = hmac_sha[0].key_len;
      input_data = hmac_sha[0].data;
      input_datalen = hmac_sha[0].data_len;
      exp_digest = hmac_sha[0].digest;
      exp_digestlen = sizeof(sha224_digest);
      break;
    case CRYPTOAPI_HASHTYPE_HMAC_SHA256:
      key = hmac_sha[1].key;
      key_len = hmac_sha[1].key_len;
      input_data = hmac_sha[1].data;
      input_datalen = hmac_sha[1].data_len;
      exp_digest = hmac_sha[1].digest;
      exp_digestlen = sizeof(sha256_digest);
      break;
    case CRYPTOAPI_HASHTYPE_HMAC_SHA384:
      key = hmac_sha[2].key;
      key_len = hmac_sha[2].key_len;
      input_data = hmac_sha[2].data;
      input_datalen = hmac_sha[2].data_len;
      exp_digest = hmac_sha[2].digest;
      exp_digestlen = sizeof(sha384_digest);
      break;
    case CRYPTOAPI_HASHTYPE_HMAC_SHA512:
      key = hmac_sha[3].key;
      key_len = hmac_sha[3].key_len;
      input_data = hmac_sha[3].data;
      input_datalen = hmac_sha[3].data_len;
      exp_digest = hmac_sha[3].digest;
      exp_digestlen = sizeof(sha512_digest);
      break;
    case CRYPTOAPI_HASHTYPE_HMAC_SHA1:
      key = hmac_sha[4].key;
      key_len = hmac_sha[4].key_len;
      input_data = hmac_sha[4].data;
      input_datalen = hmac_sha[4].data_len;
      exp_digest = hmac_sha[4].digest;
      exp_digestlen = sizeof(sha1_digest);
      break;
     default:
       return ret;
    }

    SDK_TRACE_DEBUG("Running %s-%s test on data: 0x%llx, data-len: %d:, key: 0x%llx, key-len: %d\n",
                    CryptoApiHashType_Name(hash_type),
                    generate ? "generate" : "verify",
                    input_data, input_datalen,
                    key ? key : (unsigned char *)"", key_len);

    CAPRI_BARCO_API_PARAM_HEXDUMP((char *)"Input Data bytes:",
                                  (char *)input_data, input_datalen);

    ret = capri_barco_sym_hash_process_request(hash_type, generate,
                                               key, key_len,
                                               input_data, input_datalen,
                                               generate ? digest_output :
                                               (uint8_t *) exp_digest,
                                               generate ?
                                               sizeof(digest_output) :
                                               exp_digestlen);

    if (key_len) {
        CAPRI_BARCO_API_PARAM_HEXDUMP((char *)"Input HMAC Key:", (char *)key,
                                      key_len);
    }

    if (generate) {
        CAPRI_BARCO_API_PARAM_HEXDUMP((char *)"Output Digest:",
                                      (char *)digest_output,
                                      exp_digestlen);

        CAPRI_BARCO_API_PARAM_HEXDUMP((char *)"Expected Digest:",
                                      (char *)exp_digest,
                                      exp_digestlen);

        if (memcmp(digest_output, exp_digest, exp_digestlen)) {
            SDK_TRACE_DEBUG("%s Hash generate test FAILED",
                            CryptoApiHashType_Name(hash_type));
        } else {
            SDK_TRACE_DEBUG("%s Hash generate test PASSED",
                            CryptoApiHashType_Name(hash_type));
        }
    } else {
        CAPRI_BARCO_API_PARAM_HEXDUMP((char *)"Verified Digest:",
                                      (char *)exp_digest,
                                      exp_digestlen);
        SDK_TRACE_DEBUG("%s Hash verify test %s",
                        CryptoApiHashType_Name(hash_type),
                        ret == 0 ? "PASSED" : "FAILED");
    }

    return ret;
}

sdk_ret_t
capri_barco_sym_hash_run_tests (void)
{
    sdk_ret_t       ret = SDK_RET_OK;

    ret = capri_barco_sym_hash_sha_test(CRYPTOAPI_HASHTYPE_SHA1, true);
    ret = capri_barco_sym_hash_sha_test(CRYPTOAPI_HASHTYPE_SHA224, true);
    ret = capri_barco_sym_hash_sha_test(CRYPTOAPI_HASHTYPE_SHA256, true);
    ret = capri_barco_sym_hash_sha_test(CRYPTOAPI_HASHTYPE_SHA384, true);
    ret = capri_barco_sym_hash_sha_test(CRYPTOAPI_HASHTYPE_SHA512, true);

    ret = capri_barco_sym_hash_sha_test(CRYPTOAPI_HASHTYPE_SHA1, false);
    ret = capri_barco_sym_hash_sha_test(CRYPTOAPI_HASHTYPE_SHA224, false);
    ret = capri_barco_sym_hash_sha_test(CRYPTOAPI_HASHTYPE_SHA256, false);
    ret = capri_barco_sym_hash_sha_test(CRYPTOAPI_HASHTYPE_SHA384, false);
    ret = capri_barco_sym_hash_sha_test(CRYPTOAPI_HASHTYPE_SHA512, false);

    ret = capri_barco_sym_hash_sha_test(CRYPTOAPI_HASHTYPE_HMAC_SHA224, true);
    ret = capri_barco_sym_hash_sha_test(CRYPTOAPI_HASHTYPE_HMAC_SHA256, true);
    ret = capri_barco_sym_hash_sha_test(CRYPTOAPI_HASHTYPE_HMAC_SHA384, true);
    ret = capri_barco_sym_hash_sha_test(CRYPTOAPI_HASHTYPE_HMAC_SHA512, true);
    ret = capri_barco_sym_hash_sha_test(CRYPTOAPI_HASHTYPE_HMAC_SHA1, true);

    ret = capri_barco_sym_hash_sha_test(CRYPTOAPI_HASHTYPE_HMAC_SHA224, false);
    ret = capri_barco_sym_hash_sha_test(CRYPTOAPI_HASHTYPE_HMAC_SHA256, false);
    ret = capri_barco_sym_hash_sha_test(CRYPTOAPI_HASHTYPE_HMAC_SHA384, false);
    ret = capri_barco_sym_hash_sha_test(CRYPTOAPI_HASHTYPE_HMAC_SHA512, false);
    ret = capri_barco_sym_hash_sha_test(CRYPTOAPI_HASHTYPE_HMAC_SHA1, false);

    uint8_t auth_tag[64], ciphertext[128];

#if 0
    ret = capri_barco_sym_aes_encrypt_process_request(CAPRI_SYMM_ENCTYPE_AES_CCM, true,
						      aes_enc[0].key, aes_enc[0].key_len,
						      aes_enc[0].aad, aes_enc[0].aad_len,
						      aes_enc[0].plaintext, aes_enc[0].plaintext_len,
						      aes_enc[0].iv, aes_enc[0].iv_len,
						      ciphertext, aes_enc[0].ciphertext_len,
						      auth_tag, aes_enc[0].auth_tag_len);

#endif

    ret = capri_barco_sym_aes_encrypt_process_request(CAPRI_SYMM_ENCTYPE_AES_CCM,
                                                      true,
                                                      aes_enc[1].key,
                                                      aes_enc[1].key_len,
                                                      aes_enc[1].aad,
                                                      aes_enc[1].aad_len,
                                                      aes_enc[1].plaintext,
                                                      aes_enc[1].plaintext_len,
                                                      aes_enc[1].iv,
                                                      aes_enc[1].iv_len,
                                                      ciphertext,
                                                      aes_enc[1].ciphertext_len,
                                                      auth_tag,
                                                      aes_enc[1].auth_tag_len,
                                                      true);
    if (ret != SDK_RET_OK) {
        SDK_TRACE_DEBUG("AES-CCM Encrypt test FAILED: api failure");
    } else {
        CAPRI_BARCO_API_PARAM_HEXDUMP((char *)"Received cipher-text:",
                                      (char *)ciphertext,
                                      aes_enc[1].ciphertext_len);
        CAPRI_BARCO_API_PARAM_HEXDUMP((char *)"Expected cipher-text:",
                                      (char *)aes_enc[1].ciphertext,
                                      aes_enc[1].ciphertext_len);

        if (memcmp(ciphertext, aes_enc[1].ciphertext,
                   aes_enc[1].ciphertext_len)) {
            SDK_TRACE_DEBUG("AES-CCM Encrypt test FAILED: ciphertext do not match!!");
        } else {
            SDK_TRACE_DEBUG("AES-CCM Encrypt test PASSED: ciphertexts match");
        }

        CAPRI_BARCO_API_PARAM_HEXDUMP((char *)"Received auth-tag:",
                                      (char *)auth_tag,
                                      aes_enc[1].auth_tag_len);
        CAPRI_BARCO_API_PARAM_HEXDUMP((char *)"Expected auth-tag:",
                                      (char *)aes_enc[1].auth_tag,
                                      aes_enc[1].auth_tag_len);

        if (memcmp(auth_tag, aes_enc[1].auth_tag, aes_enc[1].auth_tag_len)) {
            SDK_TRACE_DEBUG("AES-CCM Encrypt test FAILED: auth-tag do not match!!");
        } else {
            SDK_TRACE_DEBUG("AES-CCM Encrypt test PASSED: auth-tag match");
        }
    }

    uint8_t plaintext[128];

    ret = capri_barco_sym_aes_encrypt_process_request(CAPRI_SYMM_ENCTYPE_AES_CCM,
                                                      false,
                                                      aes_enc[1].key,
                                                      aes_enc[1].key_len,
                                                      aes_enc[1].aad,
                                                      aes_enc[1].aad_len,
                                                      plaintext,
                                                      aes_enc[1].plaintext_len,
                                                      aes_enc[1].iv,
                                                      aes_enc[1].iv_len,
                                                      aes_enc[1].ciphertext,
                                                      aes_enc[1].ciphertext_len,
                                                      aes_enc[1].auth_tag,
                                                      aes_enc[1].auth_tag_len,
                                                      true);

    if (ret != SDK_RET_OK) {
        SDK_TRACE_DEBUG("AES-CCM Decrypt test FAILED: api failure %d", ret);
    } else {
        CAPRI_BARCO_API_PARAM_HEXDUMP((char *)"Received plain-text:",
                                      (char *)plaintext,
                                      aes_enc[1].plaintext_len);
        CAPRI_BARCO_API_PARAM_HEXDUMP((char *)"Expected plain-text:",
                                      (char *)aes_enc[1].plaintext,
                                      aes_enc[1].plaintext_len);


        if (memcmp(plaintext, aes_enc[1].plaintext, aes_enc[1].plaintext_len)) {
            SDK_TRACE_DEBUG("AES-CCM Decrypt test FAILED: plaintext do not match!!");
        } else {
            SDK_TRACE_DEBUG("AES-CCM Decrypt test PASSED: plaintexts match");
        }
    }

    return ret;
}

sdk_ret_t
capri_barco_sym_run_aes_gcm_1K_test (void)
{
    sdk_ret_t       ret = SDK_RET_OK;

    uint8_t auth_tag[64], ciphertext[128];

    for (int i = 0; i < 1000; i++) {
        ret = capri_barco_sym_aes_encrypt_process_request(CAPRI_SYMM_ENCTYPE_AES_GCM,
                                                          true,
                                                          aes_enc[1].key,
                                                          aes_enc[1].key_len,
                                                          aes_enc[1].aad,
                                                          aes_enc[1].aad_len,
                                                          aes_enc[1].plaintext,
                                                          aes_enc[1].plaintext_len,
                                                          aes_enc[1].iv,
                                                          aes_enc[1].iv_len,
                                                          ciphertext,
                                                          aes_enc[1].ciphertext_len,
                                                          auth_tag,
                                                          aes_enc[1].auth_tag_len,
                                                          false);
    }

    ret = capri_barco_sym_aes_encrypt_process_request(CAPRI_SYMM_ENCTYPE_AES_GCM,
                                                      true,
                                                      aes_enc[1].key,
                                                      aes_enc[1].key_len,
                                                      aes_enc[1].aad,
                                                      aes_enc[1].aad_len,
                                                      aes_enc[1].plaintext,
                                                      aes_enc[1].plaintext_len,
                                                      aes_enc[1].iv,
                                                      aes_enc[1].iv_len,
                                                      ciphertext,
                                                      aes_enc[1].ciphertext_len,
                                                      auth_tag,
                                                      aes_enc[1].auth_tag_len,
                                                      true);
    if (ret != SDK_RET_OK) {
        SDK_TRACE_DEBUG("AES-GCM Encrypt test FAILED: api failure");
    } else {
        CAPRI_BARCO_API_PARAM_HEXDUMP((char *)"Received cipher-text:",
                                      (char *)ciphertext,
                                      aes_enc[1].ciphertext_len);
        CAPRI_BARCO_API_PARAM_HEXDUMP((char *)"Expected cipher-text:",
                                      (char *)aes_enc[1].ciphertext,
                                      aes_enc[1].ciphertext_len);

        if (memcmp(ciphertext, aes_enc[1].ciphertext,
                   aes_enc[1].ciphertext_len)) {
            SDK_TRACE_DEBUG("AES-CCM Encrypt test FAILED: ciphertext do not match!!");
        } else {
            SDK_TRACE_DEBUG("AES-CCM Encrypt test PASSED: ciphertexts match");
        }

        CAPRI_BARCO_API_PARAM_HEXDUMP((char *)"Received auth-tag:",
                                      (char *)auth_tag,
                                      aes_enc[1].auth_tag_len);
        CAPRI_BARCO_API_PARAM_HEXDUMP((char *)"Expected auth-tag:",
                                      (char *)aes_enc[1].auth_tag,
                                      aes_enc[1].auth_tag_len);

        if (memcmp(auth_tag, aes_enc[1].auth_tag, aes_enc[1].auth_tag_len)) {
            SDK_TRACE_DEBUG("AES-CCM Encrypt test FAILED: auth-tag do not match!!");
        } else {
            SDK_TRACE_DEBUG("AES-CCM Encrypt test PASSED: auth-tag match");
        }
    }

#if 0

    //GCM Decrypt
    uint8_t plaintext[128];

    ret = capri_barco_sym_aes_encrypt_process_request(CAPRI_SYMM_ENCTYPE_AES_GCM,
                                                      false,
                                                      aes_enc[1].key,
                                                      aes_enc[1].key_len,
                                                      aes_enc[1].aad,
                                                      aes_enc[1].aad_len,
                                                      plaintext,
                                                      aes_enc[1].plaintext_len,
                                                      aes_enc[1].iv,
                                                      aes_enc[1].iv_len,
                                                      aes_enc[1].ciphertext,
                                                      aes_enc[1].ciphertext_len,
                                                      aes_enc[1].auth_tag,
                                                      aes_enc[1].auth_tag_len,
                                                      true);
    if (ret != SDK_RET_OK) {
        SDK_TRACE_DEBUG("AES-CCM Decrypt test FAILED: api failure %d", ret);
    } else {
        CAPRI_BARCO_API_PARAM_HEXDUMP((char *)"Received plain-text:",
                                      (char *)plaintext,
                                      aes_enc[1].plaintext_len);
        CAPRI_BARCO_API_PARAM_HEXDUMP((char *)"Expected plain-text:",
                                      (char *)aes_enc[1].plaintext,
                                      aes_enc[1].plaintext_len);


        if (memcmp(plaintext, aes_enc[1].plaintext, aes_enc[1].plaintext_len)) {
            SDK_TRACE_DEBUG("AES-CCM Decrypt test FAILED: plaintext do not match!!");
        } else {
            SDK_TRACE_DEBUG("AES-CCM Decrypt test PASSED: plaintexts match");
        }
    }
#endif

    return ret;
}

}    // namespace capri
}    // namespace platform
}    // namespace sdk
