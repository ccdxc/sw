#include "nic/hal/hal.hpp"
#include "nic/hal/pd/capri/capri_barco_asym_apis.hpp"
#include "nic/hal/pd/capri/capri_barco_sym_apis.hpp"

namespace hal {
namespace pd {

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

#define _API_PARAM_DEBUG_

  hal_ret_t capri_barco_sym_hash_sha_test (cryptoapis::CryptoApiHashType hash_type, bool generate)
{
    hal_ret_t           ret = HAL_RET_OK;
    const uint8_t       *exp_digest;
    int                 exp_digestlen, key_len, input_datalen;
    unsigned char       *key, *input_data;

    key = NULL;
    key_len  = 0;
    input_data = data;
    input_datalen = 3;

    memset(digest_output, 0, sizeof(digest_output));

    switch (hash_type) {
    case cryptoapis::CRYPTOAPI_HASHTYPE_SHA1:
      exp_digest = sha1_digest;
      exp_digestlen = sizeof(sha1_digest);
      break;
    case cryptoapis::CRYPTOAPI_HASHTYPE_SHA224:
      exp_digest = sha224_digest;
      exp_digestlen = sizeof(sha224_digest);
      break;
    case cryptoapis::CRYPTOAPI_HASHTYPE_SHA256:
      exp_digest = sha256_digest;
      exp_digestlen = sizeof(sha256_digest);
      break;
    case cryptoapis::CRYPTOAPI_HASHTYPE_SHA384:
      exp_digest = sha384_digest;
      exp_digestlen = sizeof(sha384_digest);
      break;
    case cryptoapis::CRYPTOAPI_HASHTYPE_SHA512:
      exp_digest = sha512_digest;
      exp_digestlen = sizeof(sha512_digest);
      break;
    case cryptoapis::CRYPTOAPI_HASHTYPE_HMAC_SHA224:
      key = hmac_sha[0].key;
      key_len = hmac_sha[0].key_len;
      input_data = hmac_sha[0].data;
      input_datalen = hmac_sha[0].data_len;
      exp_digest = hmac_sha[0].digest;
      exp_digestlen = sizeof(sha224_digest);
      break;
    case cryptoapis::CRYPTOAPI_HASHTYPE_HMAC_SHA256:
      key = hmac_sha[1].key;
      key_len = hmac_sha[1].key_len;
      input_data = hmac_sha[1].data;
      input_datalen = hmac_sha[1].data_len;
      exp_digest = hmac_sha[1].digest;
      exp_digestlen = sizeof(sha256_digest);
      break;
    case cryptoapis::CRYPTOAPI_HASHTYPE_HMAC_SHA384:
      key = hmac_sha[2].key;
      key_len = hmac_sha[2].key_len;
      input_data = hmac_sha[2].data;
      input_datalen = hmac_sha[2].data_len;
      exp_digest = hmac_sha[2].digest;
      exp_digestlen = sizeof(sha384_digest);
      break;
    case cryptoapis::CRYPTOAPI_HASHTYPE_HMAC_SHA512:
      key = hmac_sha[3].key;
      key_len = hmac_sha[3].key_len;
      input_data = hmac_sha[3].data;
      input_datalen = hmac_sha[3].data_len;
      exp_digest = hmac_sha[3].digest;
      exp_digestlen = sizeof(sha512_digest);
      break;
    case cryptoapis::CRYPTOAPI_HASHTYPE_HMAC_SHA1:
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

    HAL_TRACE_DEBUG("Running {}-{} test on data: {}, data-len: {:d}:, key: {}, key-len: {:d}\n",
                    CryptoApiHashType_Name(hash_type), generate ? "generate" : "verify",
                    input_data, input_datalen, key ? key : (unsigned char *)"", key_len);

    CAPRI_BARCO_API_PARAM_HEXDUMP((char *)"Input Data bytes:", (char *)input_data, input_datalen);

    ret = capri_barco_sym_hash_process_request(hash_type, generate,
                                               key, key_len,
                                               input_data, input_datalen,
                                               generate ? digest_output : (uint8_t *) exp_digest,
					       generate ? sizeof(digest_output) : exp_digestlen);

    if (key_len) {
	CAPRI_BARCO_API_PARAM_HEXDUMP((char *)"Input HMAC Key:", (char *)key, key_len);
    }

    if (generate) {
        CAPRI_BARCO_API_PARAM_HEXDUMP((char *)"Output Digest:", (char *)digest_output,
                                      exp_digestlen);

	CAPRI_BARCO_API_PARAM_HEXDUMP((char *)"Expected Digest:", (char *)exp_digest,
				      exp_digestlen);
	if (memcmp(digest_output, exp_digest, exp_digestlen)) {
  	    HAL_TRACE_DEBUG("{} Hash generate test FAILED", CryptoApiHashType_Name(hash_type));
	} else {
  	    HAL_TRACE_DEBUG("{} Hash generate test PASSED", CryptoApiHashType_Name(hash_type));
	}
    } else {
	CAPRI_BARCO_API_PARAM_HEXDUMP((char *)"Verified Digest:", (char *)exp_digest,
				      exp_digestlen);
        HAL_TRACE_DEBUG("{} Hash verify test {}", CryptoApiHashType_Name(hash_type),
			ret == 0 ? "PASSED" : "FAILED");
    }
  
    return ret;
}

hal_ret_t capri_barco_sym_hash_run_tests (void)
{
    hal_ret_t       ret = HAL_RET_OK;

    ret = capri_barco_sym_hash_sha_test(cryptoapis::CRYPTOAPI_HASHTYPE_SHA1, true);
    ret = capri_barco_sym_hash_sha_test(cryptoapis::CRYPTOAPI_HASHTYPE_SHA224, true);
    ret = capri_barco_sym_hash_sha_test(cryptoapis::CRYPTOAPI_HASHTYPE_SHA256, true);
    ret = capri_barco_sym_hash_sha_test(cryptoapis::CRYPTOAPI_HASHTYPE_SHA384, true);
    ret = capri_barco_sym_hash_sha_test(cryptoapis::CRYPTOAPI_HASHTYPE_SHA512, true);

    ret = capri_barco_sym_hash_sha_test(cryptoapis::CRYPTOAPI_HASHTYPE_SHA1, false);
    ret = capri_barco_sym_hash_sha_test(cryptoapis::CRYPTOAPI_HASHTYPE_SHA224, false);
    ret = capri_barco_sym_hash_sha_test(cryptoapis::CRYPTOAPI_HASHTYPE_SHA256, false);
    ret = capri_barco_sym_hash_sha_test(cryptoapis::CRYPTOAPI_HASHTYPE_SHA384, false);
    ret = capri_barco_sym_hash_sha_test(cryptoapis::CRYPTOAPI_HASHTYPE_SHA512, false);

    ret = capri_barco_sym_hash_sha_test(cryptoapis::CRYPTOAPI_HASHTYPE_HMAC_SHA224, true);
    ret = capri_barco_sym_hash_sha_test(cryptoapis::CRYPTOAPI_HASHTYPE_HMAC_SHA256, true);
    ret = capri_barco_sym_hash_sha_test(cryptoapis::CRYPTOAPI_HASHTYPE_HMAC_SHA384, true);
    ret = capri_barco_sym_hash_sha_test(cryptoapis::CRYPTOAPI_HASHTYPE_HMAC_SHA512, true);
    ret = capri_barco_sym_hash_sha_test(cryptoapis::CRYPTOAPI_HASHTYPE_HMAC_SHA1, true);

    ret = capri_barco_sym_hash_sha_test(cryptoapis::CRYPTOAPI_HASHTYPE_HMAC_SHA224, false);
    ret = capri_barco_sym_hash_sha_test(cryptoapis::CRYPTOAPI_HASHTYPE_HMAC_SHA256, false);
    ret = capri_barco_sym_hash_sha_test(cryptoapis::CRYPTOAPI_HASHTYPE_HMAC_SHA384, false);
    ret = capri_barco_sym_hash_sha_test(cryptoapis::CRYPTOAPI_HASHTYPE_HMAC_SHA512, false);
    ret = capri_barco_sym_hash_sha_test(cryptoapis::CRYPTOAPI_HASHTYPE_HMAC_SHA1, false);

    return ret;
}

}    // namespace pd
}    // namespace hal
