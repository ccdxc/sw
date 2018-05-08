
#include <stdint.h>
#include <openssl/evp.h>
#include "pnso_sim.h"

/* GCC specific, required for madler */
typedef unsigned __int128 uint128_t;

bool algo_sha_gen(void *scratch, unsigned char *hash,
		  const unsigned char *msg, int len, int hash_bits)
{
	EVP_MD_CTX *ctx = (EVP_MD_CTX *) scratch;
	//EVP_MD_CTX *ctx = EVP_MD_CTX_create();
	if (!ctx)
		return false;
	EVP_MD_CTX_init(ctx);
	bool ok = false;
	int ret = 0;
	int rlen = 0;

	switch (hash_bits) {
	case 160:
		ret = EVP_DigestInit(ctx, EVP_sha1());
		break;
	case 224:
		ret = EVP_DigestInit(ctx, EVP_sha224());
		break;
	case 256:
		ret = EVP_DigestInit(ctx, EVP_sha256());
		break;
	case 384:
		ret = EVP_DigestInit(ctx, EVP_sha384());
		break;
	case 512:
		ret = EVP_DigestInit(ctx, EVP_sha512());
		break;
	default:
		goto error_handler;
	}
	if (1 != ret)
		goto error_handler;

	ret = EVP_DigestUpdate(ctx, msg, len);
	if (1 != ret)
		goto error_handler;
	ret = EVP_DigestFinal(ctx, hash, (unsigned int *) &rlen);
	if (1 != ret)
		goto error_handler;
	if (rlen != hash_bits / 8)
		goto error_handler;

	ok = true;
      error_handler:
	//EVP_MD_CTX_destroy(ctx);
	return ok;
}

uint32_t algo_gen_adler32(unsigned char *msg, size_t bytes)
{
	int a, b;
	a = 1;
	b = 0;
	//cout << "adler32 got length as 0x" << hex << bytes << endl;

	for (size_t i = 0; i < bytes; i++) {
		a += (int) (msg[i]);
		a %= 65521;
		b += a;
		b %= 65521;
	}

	return (uint32_t) ((b << 16) | (a & 0xffff));
}

const uint32_t PRIME_BASE = 65521;
uint32_t algo_gen_madler(uint64_t * data, size_t len)
{
	uint128_t sumA = 1;
	uint128_t sumB = 0;

	for (; len > 0; ++data, --len) {
		sumA += *data;
		sumB += sumA;
	}

	//uint32_t sumA_mod = integer_modulus(sumA, PRIME_BASE);
	//uint32_t sumB_mod = integer_modulus(sumB, PRIME_BASE);
	uint32_t sumA_mod = sumA % PRIME_BASE;
	uint32_t sumB_mod = sumB % PRIME_BASE;

	return (sumB_mod << 16) | sumA_mod;
}

#define XTS_IV_SIZE 16

int algo_encrypt_xts(void *scratch, uint8_t * key, uint8_t * iv,
		     uint8_t * input, uint32_t input_sz, uint8_t * output,
		     uint32_t * output_sz)
{
	int rlen = 0;
	EVP_CIPHER_CTX *cipher_ctx = (EVP_CIPHER_CTX *) scratch;

	if (1 !=
	    EVP_EncryptInit_ex(cipher_ctx, EVP_aes_256_xts(), 0, 0, 0)) {
		return -1;
	}

	if (1 != EVP_EncryptInit_ex(cipher_ctx, 0, 0, key, iv)) {
		return -1;
	}

	if (1 !=
	    EVP_EncryptUpdate(cipher_ctx, output, &rlen, input,
			      input_sz)) {
		return -1;
	}
	*output_sz = (uint32_t) rlen;

	/* TODO: check that input and output lens are the same */

	return 0;
}

int algo_decrypt_xts(void *scratch, uint8_t * key, uint8_t * iv,
		     uint8_t * input, uint32_t input_sz, uint8_t * output,
		     uint32_t * output_sz)
{
	int rlen = 0;
	EVP_CIPHER_CTX *cipher_ctx = (EVP_CIPHER_CTX *) scratch;

	if (1 !=
	    EVP_DecryptInit_ex(cipher_ctx, EVP_aes_256_xts(), 0, 0, 0)) {
		return -1;
	}

	if (1 != EVP_DecryptInit_ex(cipher_ctx, 0, 0, key, iv)) {
		return -1;
	}

	if (1 !=
	    EVP_DecryptUpdate(cipher_ctx, output, &rlen, input,
			      input_sz)) {
		return -1;
	}
	*output_sz = (uint32_t) rlen;

	/* TODO: check that input and output lens are the same */

	return 0;
}
