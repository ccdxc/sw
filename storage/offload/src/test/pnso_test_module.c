/*
 * {C} Copyright 2018 Pensando Systems Inc.
 * All rights reserved.
 *
 */

#include "osal.h"
#include "pnso_api.h"
#include "pnso_test.h"

OSAL_LICENSE("Dual BSD/GPL");

static char *y = NULL;
//S_RUGO | S_IWUSR);
module_param(y, charp, 0444);
MODULE_PARM_DESC(y, "simple YAML prefix string");

static long repeat = -1;
module_param(repeat, long, 0444);
MODULE_PARM_DESC(repeat, "default repeat count");

static long batch = -1;
module_param(batch, long, 0444);
MODULE_PARM_DESC(batch, "default batch count");

static osal_thread_t g_main_thread;

static int
pnso_test_mod_init(void)
{
	int rv;

	rv = osal_log_init(OSAL_LOG_LEVEL_NONE);

	return rv;
}

static int
pnso_test_mod_fini(void)
{
	pnso_test_shutdown();

	osal_thread_stop(&g_main_thread);

	return PNSO_OK;
}

static void
status_output_func(const char *status)
{
	OSAL_LOG(status);
}

static const unsigned char default_global_yaml[] =
"global_params:\n"
"  per_core_qdepth: 32\n"
"  block_size: 4096\n"
"  cpu_mask: 1\n"
"  #limit_rate: 1000000\n"
"  #status_interval: 5\n"
"alias: 'key1=abcd1234ABCD1234abcd1234ABCD1234'\n"
"alias: 'default_repeat=100000'\n"
"alias: 'default_batch=1'\n"
"\n";

static const unsigned char default_test_input[] =
"cp_hdr_format:\n"
"  idx: 1\n"
"  cp_hdr_field:\n"
"    type: indata_chksum\n"
"    offset: 0\n"
"    len: 4\n"
"    val: 0\n"
"  cp_hdr_field:\n"
"    type: outdata_length\n"
"    offset: 4\n"
"    len: 2\n"
"    val: 0\n"
"  cp_hdr_field:\n"
"    type: algo\n"
"    offset: 6\n"
"    len: 2\n"
"    val: 1\n"
"cp_hdr_mapping:\n"
"  - entry:\n"
"      pnso_algo: 1\n"
"      hdr_algo: 1\n"
"crypto_keys:\n"
"  - key:\n"
"      idx: 1\n"
"      key1: '$key1'\n"
"      key2: '$key1'\n"
"svc_chains:\n"
"  - svc_chain:\n"
"      idx: 1\n"
"      input:\n"
"        #pattern: abcdefghijklm\n"
"        pattern: a\n"
"        len: 4096\n"
"      ops:\n"
"        - compress:\n"
"            flags: 'zero_pad,insert_header'\n"
"            hdr_fmt_idx: 1\n"
"            threshold_delta: 8\n"
"            output_file: 'compressed_1.bin'\n"
"  - svc_chain:\n"
"      idx: 2\n"
"      input:\n"
"        file: 'compressed_1.bin'\n"
"      ops:\n"
"        - decompress:\n"
"            flags: 'header_present'\n"
"            hdr_fmt_idx: 1\n"
"            output_file: 'decompressed.bin'\n"
"  - svc_chain:\n"
"      idx: 3\n"
"      input:\n"
"        file: 'decompressed.bin'\n"
"      ops:\n"
"        - hash:\n"
"            algo_type: 'sha2_256'\n"
"            flags: 0\n"
"            output_file: 'hash_decompressed.bin'\n"
"  - svc_chain:\n"
"      idx: 4\n"
"      input:\n"
"        file: 'decompressed.bin'\n"
"      ops:\n"
"        - chksum:\n"
"            algo_type: 'crc32c'\n"
"            flags: 0\n"
"            output_file: 'chksum_decompressed.bin'\n"
"  - svc_chain:\n"
"      idx: 5\n"
"      input:\n"
"        pattern: a\n"
"        len: 4096\n"
"      ops:\n"
"        - compress:\n"
"            flags: 'zero_pad,insert_header'\n"
"            hdr_fmt_idx: 1\n"
"            threshold_delta: 8\n"
"            output_file: 'compressed_chain1.bin'\n"
"        - hash:\n"
"            algo_type: 'sha2_256'\n"
"            flags: 0\n"
"            output_file: 'hash_chain1.bin'\n"
"  - svc_chain:\n"
"      idx: 6\n"
"      input:\n"
"        pattern: a\n"
"        len: 4096\n"
"      ops:\n"
"        - encrypt:\n"
"            key_idx: 1\n"
"            output_file: 'encrypted.bin'\n"
"  - svc_chain:\n"
"      idx: 7\n"
"      input:\n"
"        file: 'encrypted.bin'\n"
"      ops:\n"
"        - decrypt:\n"
"            key_idx: 1\n"
"            output_file: 'decrypted.bin'\n"
"tests:\n"
"  - test:\n"
"      idx: 1\n"
"      svc_chains: 1\n"
"      name: 'Compress sanity'\n"
"      repeat: '$default_repeat'\n"
"      batch_depth: '$default_batch'\n"
"      mode: sync\n"
"      validations:\n"
"        - retcode_compare:\n"
"            idx: 1\n"
"            retcode: 0\n"
"            svc_retcodes: 0\n"
"  - test:\n"
"      idx: 2\n"
"      svc_chains: 2\n"
"      name: 'Decompress sanity'\n"
"      repeat: '$default_repeat'\n"
"      batch_depth: '$default_batch'\n"
"      mode: sync\n"
"      validations:\n"
"        - retcode_compare:\n"
"            idx: 1\n"
"            retcode: 0\n"
"            svc_retcodes: 0\n"
"        - size_compare:\n"
"            idx: 2\n"
"            file1: 'decompressed.bin'\n"
"            val: 4096\n"
"        - data_compare:\n"
"            idx: 3\n"
"            file1: 'decompressed.bin'\n"
"            pattern: a\n"
"            len: 4096\n"
"  - test:\n"
"      idx: 3\n"
"      svc_chains: 3\n"
"      name: 'Hash sanity'\n"
"      repeat: '$default_repeat'\n"
"      batch_depth: '$default_batch'\n"
"      mode: sync\n"
"      validations:\n"
"        - retcode_compare:\n"
"            idx: 1\n"
"            retcode: 0\n"
"            svc_retcodes: 0\n"
"  - test:\n"
"      idx: 4\n"
"      svc_chains: 4\n"
"      name: 'Checksum sanity'\n"
"      repeat: '$default_repeat'\n"
"      batch_depth: '$default_batch'\n"
"      mode: sync\n"
"      validations:\n"
"        - retcode_compare:\n"
"            idx: 1\n"
"            retcode: 0\n"
"            svc_retcodes: 0\n"
"  - test:\n"
"      idx: 5\n"
"      svc_chains: 5\n"
"      name: 'Compress+hash sanity'\n"
"      repeat: 0\n"
"      batch_depth: '$default_batch'\n"
"      mode: sync\n"
"      validations:\n"
"        - retcode_compare:\n"
"            idx: 1\n"
"            retcode: 0\n"
"            svc_retcodes: 0,0\n"
"        - data_compare:\n"
"            idx: 2\n"
"            file1: 'compressed1.bin'\n"
"            file2: 'compressed_chain1.bin'\n"
"  - test:\n"
"      idx: 6\n"
"      svc_chains: 6\n"
"      name: 'Encrypt sanity'\n"
"      repeat: 0\n"
"      batch_depth: '$default_batch'\n"
"      mode: sync\n"
"      validations:\n"
"        - retcode_compare:\n"
"            idx: 1\n"
"            retcode: 0\n"
"            svc_retcodes: 0\n"
"        - size_compare:\n"
"            idx: 2\n"
"            file1: 'encrypted.bin'\n"
"            val: 4096\n"
"  - test:\n"
"      idx: 7\n"
"      svc_chains: 7\n"
"      name: 'Decrypt sanity'\n"
"      repeat: 0\n"
"      batch_depth: '$default_batch'\n"
"      mode: sync\n"
"      validations:\n"
"        - retcode_compare:\n"
"            idx: 1\n"
"            retcode: 0\n"
"            svc_retcodes: 0\n"
"        - data_compare:\n"
"            idx: 2\n"
"            file1: 'decrypted.bin'\n"
"            pattern: a\n"
"            len: 4096\n"
"\n";

#define MAX_ALIAS_STR_LEN 128
/* Return length of generated alias string */
static uint32_t generate_alias_yaml(char *dst, const char *name, long val)
{
	uint32_t len = 0;

	len += safe_strcpy(dst+len, "alias: '", MAX_ALIAS_STR_LEN-len);
	len += safe_strcpy(dst+len, name, MAX_ALIAS_STR_LEN-len);
	len += safe_strcpy(dst+len, "=", MAX_ALIAS_STR_LEN-len);
	len += safe_itoa(dst+len, MAX_ALIAS_STR_LEN-len, val);
	len += safe_strcpy(dst+len, "'\n", MAX_ALIAS_STR_LEN-len);

	return len;
}

static int
body(void *not_used)
{
	struct test_desc *cfg = NULL;
	pnso_error_t err = 0;

	pnso_test_init_fns(pnso_submit_request, status_output_func,
			   osal_alloc, osal_free, osal_realloc);

	cfg = pnso_test_desc_alloc();
	if (!cfg) {
		PNSO_LOG_ERROR("Cannot allocate test descriptor\n");
		err = ENOMEM;
		goto done;
	}

	err = pnso_test_parse_buf(default_global_yaml,
		strlen((const char*)default_global_yaml), cfg);
	if (err) {
		PNSO_LOG_ERROR("Failed to parse default global yaml input data\n");
		goto done;
	}

	/* Parse optional module parameters */
	if (y && *y) {
		err = pnso_test_parse_buf(y, strlen(y), cfg);
		if (err) {
			PNSO_LOG_ERROR("Failed to parse module parameter y\n");
			goto done;
		}
	}
	if (repeat >= 0 || batch >= 0) {
		uint32_t len = 0;
		char alias_str[MAX_ALIAS_STR_LEN+1] = "";

		if (repeat >= 0) {
			len = generate_alias_yaml(alias_str, "default_repeat",
						  repeat);
			PNSO_LOG_WARN("module param repeat: %s\n", alias_str);
			err = pnso_test_parse_buf(alias_str, len, cfg);
			if (err) {
				PNSO_LOG_ERROR("Failed to parse default repeat string '%s'\n",
					       alias_str);
				goto done;
			}
		}
		if (batch >= 0) {
			len = generate_alias_yaml(alias_str, "default_batch",
						  batch);
			PNSO_LOG_WARN("module param batch: %s\n", alias_str);
			err = pnso_test_parse_buf(alias_str, len, cfg);
			if (err) {
				PNSO_LOG_ERROR("Failed to parse default batch string '%s'\n",
					       alias_str);
				goto done;
			}
		}
	}

	err = pnso_test_parse_buf(default_test_input,
		strlen((const char*)default_test_input), cfg);
	if (err) {
		PNSO_LOG_ERROR("Failed to parse default test yaml input data\n");
		goto done;
	}
//	test_dump_desc(cfg);
	pnso_run_unit_tests(cfg);

	err = pnso_test_run_all(cfg);

done:
	pnso_test_desc_free(cfg);

	pnso_test_set_shutdown_complete();
	return err;
}

static int
body_start(void)
{
	int err;

	err = osal_thread_create(&g_main_thread, body, NULL);
	if (!err)
		err = osal_thread_start(&g_main_thread);

	return err;
}

osal_init_fn_t init_fp = pnso_test_mod_init;
osal_init_fn_t fini_fp = pnso_test_mod_fini;

OSAL_SETUP(init_fp, body_start, fini_fp);
