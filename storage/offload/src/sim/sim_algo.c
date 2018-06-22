/*
 * {C} Copyright 2018 Pensando Systems Inc.
 * All rights reserved.
 *
 */

#if 0
#include <stdint.h>
#endif

#include "osal_stdtypes.h"

#ifdef OPENSSL
#include <openssl/evp.h>
#endif

#include "sha256.h"
#include "sha512.h"
#include "sim.h"

/* GCC specific, required for madler */
typedef unsigned __int128 uint128_t;


uint32_t
calculate_crc32c(uint32_t crc32c,
		 const unsigned char *buffer,
		 unsigned int length);


#ifdef OPENSSL
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
#else

bool algo_sha_gen(void *scratch, unsigned char *hash,
		  const unsigned char *msg, int len, int hash_bits)
{
	switch (hash_bits) {
	case 256:
		SHA256_Init((SHA256_CTX *) scratch);
		SHA256_Update((SHA256_CTX *) scratch, msg, len);
		SHA256_Final(hash, (SHA256_CTX *) scratch);
		break;
	case 512:
		SHA512_Init((SHA512_CTX *) scratch);
		SHA512_Update((SHA512_CTX *) scratch, msg, len);
		SHA512_Final(hash, (SHA512_CTX *) scratch);
		break;
	default:
		return false;
	}

	return true;
}
#endif /* OPENSSL */

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

uint32_t algo_gen_crc32c(uint8_t *data, uint32_t length)
{
	return calculate_crc32c(0, data, length);
}

const uint64_t MCRC64_POLY = 0x9a6c9329ac4bc9b5ULL;
uint64_t algo_gen_mcrc64(uint8_t *data, uint32_t length)
{
	size_t i, j;
	uint64_t crc = 0xFFFFFFFFFFFFFFFFULL;
	for (i = 0; i < length; i++) {
		crc ^= data[i];
		for (j = 0; j < 8; j++) {
			if (crc & 1) {
				crc = (crc >> 1) ^ MCRC64_POLY;
			} else {
				crc = (crc >> 1);
			}
		}
	}
	return (crc ^ 0xFFFFFFFFFFFFFFFFULL);
}

