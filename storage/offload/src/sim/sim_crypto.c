/*
 * {C} Copyright 2018 Pensando Systems Inc.
 * All rights reserved.
 *
 */

#include "osal_stdtypes.h"

#ifdef OPENSSL
#include <openssl/evp.h>
#endif

#include "sim.h"
#include "sim_algo.h"

#include "xform_enc.h"


#ifdef OPENSSL

#define XTS_IV_SIZE 16

pnso_error_t algo_openssl_encrypt_xts(void *scratch,
		uint8_t *key1, uint8_t *key2,
		uint32_t key_sz, uint8_t *iv,
		uint8_t *input, uint32_t input_sz, uint8_t *output,
		uint32_t *output_sz)
{
	int rlen = 0;
	EVP_CIPHER_CTX *cipher_ctx = (EVP_CIPHER_CTX *) scratch;
	uint8_t padded_key[64];

	switch (key_sz) {
	case 16:
		if (1 !=
		    EVP_EncryptInit_ex(cipher_ctx, EVP_aes_128_xts(),
				       0, 0, 0)) {
			return PNSO_ERR_CRYPTO_AXI_ERROR;
		}
		break;
	case 32:
		if (1 !=
		    EVP_EncryptInit_ex(cipher_ctx, EVP_aes_256_xts(),
				       0, 0, 0)) {
			return PNSO_ERR_CRYPTO_AXI_ERROR;
		}
		break;
	default:
		return PNSO_ERR_CRYPTO_WRONG_KEY_TYPE;
	}

	memcpy(padded_key, key1, key_sz);
	if (key2) {
		memcpy(padded_key+key_sz, key2, key_sz);
	} else {
		memcpy(padded_key+key_sz, key1, key_sz);
	}

	if (1 != EVP_EncryptInit_ex(cipher_ctx, 0, 0, padded_key, iv)) {
		return PNSO_ERR_CRYPTO_AXI_ERROR;
	}

	if (1 !=
	    EVP_EncryptUpdate(cipher_ctx, output, &rlen, input,
			      input_sz)) {
		return PNSO_ERR_CRYPTO_AXI_ERROR;
	}
	if (rlen != input_sz) {
		return PNSO_ERR_CRYPTO_AXI_ERROR;
	}

	*output_sz = (uint32_t) rlen;

	return PNSO_OK;
}

pnso_error_t algo_decrypt_xts(void *scratch,
		uint8_t *key1, uint8_t *key2,
		uint32_t key_sz, uint8_t *iv,
		uint8_t *input, uint32_t input_sz, uint8_t *output,
		uint32_t *output_sz)
{
	int rlen = 0;
	EVP_CIPHER_CTX *cipher_ctx = (EVP_CIPHER_CTX *) scratch;
	uint8_t padded_key[64];

	switch (key_sz) {
	case 16:
		if (1 !=
		    EVP_DecryptInit_ex(cipher_ctx, EVP_aes_128_xts(),
				       0, 0, 0)) {
			return PNSO_ERR_CRYPTO_AXI_ERROR;
		}
		break;
	case 32:
		if (1 !=
		    EVP_DecryptInit_ex(cipher_ctx, EVP_aes_256_xts(),
				       0, 0, 0)) {
			return PNSO_ERR_CRYPTO_AXI_ERROR;
		}
		break;
	default:
		return PNSO_ERR_CRYPTO_WRONG_KEY_TYPE;
	}

	memcpy(padded_key, key1, key_sz);
	if (key2) {
		memcpy(padded_key+key_sz, key2, key_sz);
	} else {
		memcpy(padded_key+key_sz, key1, key_sz);
	}

	if (1 != EVP_DecryptInit_ex(cipher_ctx, 0, 0, padded_key, iv)) {
		return PNSO_ERR_CRYPTO_AXI_ERROR;
	}

	if (1 !=
	    EVP_DecryptUpdate(cipher_ctx, output, &rlen, input,
			      input_sz)) {
		return PNSO_ERR_CRYPTO_AXI_ERROR;
	}
	if (rlen != input_sz) {
		return PNSO_ERR_CRYPTO_AXI_ERROR;
	}
	*output_sz = (uint32_t) rlen;

	return PNSO_OK;
}

#else /* OPENSSL */

pnso_error_t algo_encrypt_aes_xts(void *scratch, uint8_t *key1, uint8_t *key2,
			uint32_t key_sz, uint8_t *iv,
			uint8_t *data, uint32_t data_sz)
{
	size_t i;
	struct aes_xts_ctx *cipher_ctx = (struct aes_xts_ctx *) scratch;

	/* key_sz should be in bits */
	key_sz *= 8;

	rijndael_set_key(&cipher_ctx->key1, key1, key_sz);
	rijndael_set_key(&cipher_ctx->key2, key2, key_sz);

	enc_xform_aes_xts.reinit2((caddr_t)cipher_ctx, iv);

	/* Assumes data_sz is a multiple of AES_XTS_BLOCKSIZE */
	for (i = 0; i < data_sz; i += AES_XTS_BLOCKSIZE) {
		enc_xform_aes_xts.encrypt((caddr_t)cipher_ctx, data + i);
	}

	return PNSO_OK;
}

pnso_error_t algo_decrypt_aes_xts(void *scratch, uint8_t *key1, uint8_t *key2,
			uint32_t key_sz, uint8_t *iv,
			uint8_t *data, uint32_t data_sz)
{
	size_t i;
	struct aes_xts_ctx *cipher_ctx = (struct aes_xts_ctx *) scratch;

	/* key_sz should be in bits */
	key_sz *= 8;

	rijndael_set_key(&cipher_ctx->key1, key1, key_sz);
	rijndael_set_key(&cipher_ctx->key2, key2, key_sz);

	enc_xform_aes_xts.reinit2((caddr_t)cipher_ctx, iv);

	/* Assumes data_sz is a multiple of AES_XTS_BLOCKSIZE */
	for (i = 0; i < data_sz; i += AES_XTS_BLOCKSIZE) {
		enc_xform_aes_xts.decrypt((caddr_t)cipher_ctx, data + i);
	}

	return PNSO_OK;
}

#endif /* OPENSSL */
