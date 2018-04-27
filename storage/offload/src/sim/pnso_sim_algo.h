/* Algorithm wrappers */
bool algo_sha_gen(void *scratch, unsigned char *hash,
		  const unsigned char *msg, int len, int hash_bits);
uint32_t algo_gen_adler32(unsigned char *msg, size_t bytes);
uint32_t algo_gen_madler(uint64_t * data, size_t len);
int algo_encrypt_xts(void *scratch, uint8_t * key, uint8_t * iv,
		     uint8_t * input, uint32_t input_sz,
		     uint8_t * output, uint32_t * output_sz);
int algo_decrypt_xts(void *scratch, uint8_t * key, uint8_t * iv,
		     uint8_t * input, uint32_t input_sz,
		     uint8_t * output, uint32_t * output_sz);
