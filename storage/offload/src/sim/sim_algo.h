/*
 * {C} Copyright 2018 Pensando Systems Inc.
 * All rights reserved.
 *
 */

/* Algorithm wrappers */
bool algo_sha_gen(void *scratch, unsigned char *hash,
		  const unsigned char *msg, int len, int hash_bits);
uint32_t algo_gen_adler32(unsigned char *msg, size_t bytes);
uint32_t algo_gen_madler(uint64_t *data, size_t len);
uint32_t algo_gen_crc32c(uint8_t *data, uint32_t length);
uint64_t algo_gen_mcrc64(uint8_t *data, uint32_t length);

pnso_error_t algo_openssl_encrypt_xts(void *scratch,
		uint8_t *key1, uint8_t *key2,
		uint32_t key_sz, uint8_t * iv,
		uint8_t * input, uint32_t input_sz,
		uint8_t * output, uint32_t * output_sz);
pnso_error_t algo_openssl_decrypt_xts(void *scratch,
		uint8_t *key1, uint8_t *key2,
		uint32_t key_sz, uint8_t * iv,
		uint8_t * input, uint32_t input_sz,
		uint8_t * output, uint32_t * output_sz);

pnso_error_t algo_encrypt_aes_xts(void *scratch,
		uint8_t *key1, uint8_t *key2,
		uint32_t key_sz, uint8_t *iv,
		uint8_t *data, uint32_t data_sz);
pnso_error_t algo_decrypt_aes_xts(void *scratch,
		uint8_t *key1, uint8_t *key2,
		uint32_t key_sz, uint8_t *iv,
		uint8_t *data, uint32_t data_sz);
