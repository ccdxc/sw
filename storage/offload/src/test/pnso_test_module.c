/*
 * {C} Copyright 2018 Pensando Systems Inc.
 * All rights reserved.
 *
 */

#include "osal.h"
#include "pnso_api.h"
#include "pnso_test.h"
#include "pnso_test_sysfs.h"

OSAL_LICENSE("Dual BSD/GPL");

static char *y;
//S_RUGO | S_IWUSR);
module_param(y, charp, 0444);
MODULE_PARM_DESC(y, "simple YAML prefix string");

static long repeat = -1;
module_param(repeat, long, 0444);
MODULE_PARM_DESC(repeat, "default repeat count");

static long batch = -1;
module_param(batch, long, 0444);
MODULE_PARM_DESC(batch, "default batch count");

static int status_interval = -1;
module_param(status_interval, int, 0444);
MODULE_PARM_DESC(status_interval, "status polling interval in seconds (default none)");

static long rate = -1;
module_param(rate, long, 0444);
MODULE_PARM_DESC(rate, "throughput rate limit in bytes/sec");

static long cpu_mask = -1;
module_param(cpu_mask, long, 0444);
MODULE_PARM_DESC(cpu_mask, "cpu mask");

static long feat_mask = 0xffffffff;
module_param(feat_mask, long, 0444);
MODULE_PARM_DESC(feat_mask, "feature bitmask");

static int ctl_core_id = -1;
module_param(ctl_core_id, int, 0444);
MODULE_PARM_DESC(ctl_core_id, "cpu core on which to run control thread");

static char *mode = NULL;
module_param(mode, charp, 0444);
MODULE_PARM_DESC(mode, "mode is sync, async, or poll");

static unsigned int loglevel = OSAL_LOG_LEVEL_WARNING;
module_param(loglevel, uint, 0444);
MODULE_PARM_DESC(loglevel, "logging level: 0=EMERG,1=ALERT,2=CRIT,3=ERR,4=WARN,5=NOTICE,6=INFO,7=DBG");

static osal_thread_t g_main_thread;

static int
pnso_test_mod_init(void)
{
	int rv;

	rv = osal_log_init(loglevel);
	if (rv)
		goto done;
	rv = pnso_test_sysfs_init();

done:	
	return rv;
}

static int
pnso_test_mod_fini(void)
{
	pnso_test_shutdown();

	osal_thread_stop(&g_main_thread);

	pnso_test_sysfs_finit();

	return PNSO_OK;
}

static void
status_output_func(const char *status, void *opaque)
{
	pnso_test_sysfs_write_status_data(status, strlen(status), opaque);
	OSAL_LOG(status);
}

static const unsigned char default_alias_yaml[] =
"alias: 'default_cpu_mask=1'\n"
"alias: 'key1=abcd1234ABCD1234abcd1234ABCD1234'\n"
"alias: 'iv1=000102030405060708090a0b0c0d0e0f'\n"
"alias: 'default_repeat=1'\n"
"alias: 'default_batch=1'\n"
"alias: 'default_mode=sync'\n"
"alias: 'default_turbo=1'\n"
"alias: 'default_rate=0'\n"
"alias: 'default_status_interval=0'\n"
"\n";

static const unsigned char default_global_yaml[] = 
"global_params:\n"
"  per_core_qdepth: 32\n"
"  block_size: 4096\n"
"  cpu_mask: '$default_cpu_mask'\n"
"  limit_rate: '$default_rate'\n"
"  status_interval: '$default_status_interval'\n"
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
"        #random_seed: 123\n"
"        #random_len: 64\n"
"        len: 8192\n"
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
"        pattern: a\n"
"        len: 8192\n"
"      ops:\n"
"        - hash:\n"
"            algo_type: 'sha2_256'\n"
"            flags: 0\n"
"            output_file: 'hash_decompressed.bin'\n"
"  - svc_chain:\n"
"      idx: 4\n"
"      input:\n"
"        pattern: a\n"
"        len: 8192\n"
"      ops:\n"
"        - chksum:\n"
"            algo_type: 'crc32c'\n"
"            flags: 0\n"
"            output_file: 'chksum_decompressed.bin'\n"
"  - svc_chain:\n"
"      idx: 5\n"
"      input:\n"
"        pattern: a\n"
"        len: 8192\n"
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
"            iv_data: '$iv1'\n"
"            output_file: 'encrypted.bin'\n"
"  - svc_chain:\n"
"      idx: 7\n"
"      input:\n"
"        file: 'encrypted.bin'\n"
"      ops:\n"
"        - decrypt:\n"
"            key_idx: 1\n"
"            iv_data: '$iv1'\n"
"            output_file: 'decrypted.bin'\n";

static const unsigned char *default_feature_yaml_list[PNSO_SVC_TYPE_MAX] = {
	[PNSO_SVC_TYPE_NONE] = NULL,

	[PNSO_SVC_TYPE_ENCRYPT] =
"tests:\n"
"  - test:\n"
"      idx: 6\n"
"      svc_chains: 6\n"
"      name: 'Encrypt sanity'\n"
"      #repeat: 0\n"
"      repeat: '$default_repeat'\n"
"      batch_depth: '$default_batch'\n"
"      mode: '$default_mode'\n"
"      turbo: '$default_turbo'\n"
"      validations:\n"
"        - retcode_compare:\n"
"            idx: 1\n"
"            retcode: 0\n"
"            svc_retcodes: 0\n"
"        - size_compare:\n"
"            idx: 2\n"
"            file1: 'encrypted.bin'\n"
"            val: 4096\n",

	[PNSO_SVC_TYPE_DECRYPT] =
"tests:\n"
"  - test:\n"
"      idx: 7\n"
"      svc_chains: 7\n"
"      name: 'Decrypt sanity'\n"
"      #repeat: 0\n"
"      repeat: '$default_repeat'\n"
"      batch_depth: '$default_batch'\n"
"      mode: '$default_mode'\n"
"      turbo: '$default_turbo'\n"
"      validations:\n"
"        - retcode_compare:\n"
"            idx: 1\n"
"            retcode: 0\n"
"            svc_retcodes: 0\n"
"        - data_compare:\n"
"            idx: 2\n"
"            file1: 'decrypted.bin'\n"
"            pattern: a\n"
"            len: 4096\n",

	[PNSO_SVC_TYPE_COMPRESS] =
"tests:\n"
"  - test:\n"
"      idx: 1\n"
"      svc_chains: 1\n"
"      name: 'Compress sanity'\n"
"      #repeat: 0\n"
"      repeat: '$default_repeat'\n"
"      batch_depth: '$default_batch'\n"
"      mode: '$default_mode'\n"
"      turbo: '$default_turbo'\n"
"      validations:\n"
"        - retcode_compare:\n"
"            idx: 1\n"
"            retcode: 0\n"
"            svc_retcodes: 0\n",

	[PNSO_SVC_TYPE_DECOMPRESS] =
"tests:\n"
"  - test:\n"
"      idx: 2\n"
"      svc_chains: 2\n"
"      name: 'Decompress sanity'\n"
"      #repeat: 0\n"
"      repeat: '$default_repeat'\n"
"      batch_depth: '$default_batch'\n"
"      mode: '$default_mode'\n"
"      turbo: '$default_turbo'\n"
"      validations:\n"
"        - retcode_compare:\n"
"            idx: 1\n"
"            retcode: 0\n"
"            svc_retcodes: 0\n"
"        - size_compare:\n"
"            idx: 2\n"
"            file1: 'decompressed.bin'\n"
"            val: 8192\n"
"        - data_compare:\n"
"            idx: 3\n"
"            file1: 'decompressed.bin'\n"
"            pattern: a\n"
"            len: 8192\n",

	[PNSO_SVC_TYPE_HASH] =
"tests:\n"
"  - test:\n"
"      idx: 3\n"
"      svc_chains: 3\n"
"      name: 'Hash sanity'\n"
"      #repeat: 0\n"
"      repeat: '$default_repeat'\n"
"      batch_depth: '$default_batch'\n"
"      mode: '$default_mode'\n"
"      turbo: '$default_turbo'\n"
"      validations:\n"
"        - retcode_compare:\n"
"            idx: 1\n"
"            retcode: 0\n"
"            svc_retcodes: 0\n",

	[PNSO_SVC_TYPE_CHKSUM] =
"tests:\n"
"  - test:\n"
"      idx: 4\n"
"      svc_chains: 4\n"
"      name: 'Checksum sanity'\n"
"      #repeat: 0\n"
"      repeat: '$default_repeat'\n"
"      batch_depth: '$default_batch'\n"
"      mode: '$default_mode'\n"
"      turbo: '$default_turbo'\n"
"      validations:\n"
"        - retcode_compare:\n"
"            idx: 1\n"
"            retcode: 0\n"
"            svc_retcodes: 0\n",

	[PNSO_SVC_TYPE_DECOMPACT] = NULL
};


static const unsigned char default_cp_hash_yaml[] =
"tests:\n"
"  - test:\n"
"      idx: 5\n"
"      svc_chains: 5\n"
"      name: 'Compress+hash sanity'\n"
"      repeat: 0\n"
"      #repeat: '$default_repeat'\n"
"      batch_depth: '$default_batch'\n"
"      mode: '$default_mode'\n"
"      turbo: '$default_turbo'\n"
"      validations:\n"
"        - retcode_compare:\n"
"            idx: 1\n"
"            retcode: 0\n"
"            svc_retcodes: 0,0\n"
"        - data_compare:\n"
"            idx: 2\n"
"            file1: 'compressed1.bin'\n"
"            file2: 'compressed_chain1.bin'\n";

#define MAX_ALIAS_STR_LEN 128
/* Return length of generated alias string */
static uint32_t generate_alias_yaml(char *dst, const char *name, long val_num, const char *val_str)
{
	uint32_t len = 0;

	len += safe_strcpy(dst+len, "alias: '", MAX_ALIAS_STR_LEN-len);
	len += safe_strcpy(dst+len, name, MAX_ALIAS_STR_LEN-len);
	len += safe_strcpy(dst+len, "=", MAX_ALIAS_STR_LEN-len);
	if (val_str) {
		len += safe_strcpy(dst+len, val_str, MAX_ALIAS_STR_LEN-len);
	} else {
		len += safe_itoa(dst+len, MAX_ALIAS_STR_LEN-len, val_num);
	}
	len += safe_strcpy(dst+len, "'\n", MAX_ALIAS_STR_LEN-len);

	return len;
}

static int
body(void *not_used)
{
	struct test_desc *cfg = NULL;
	pnso_error_t err = 0;
	uint32_t i;
	bool ctl_repeat = 0;

	pnso_test_init_fns(pnso_submit_request, status_output_func,
			   osal_alloc, osal_free, osal_realloc);

	cfg = pnso_test_desc_alloc();
	if (!cfg) {
		PNSO_LOG_ERROR("Cannot allocate test descriptor\n");
		err = ENOMEM;
		goto done;
	}

	err = pnso_test_parse_buf(default_alias_yaml,
		strlen((const char*)default_alias_yaml), cfg);
	if (err) {
		PNSO_LOG_ERROR("Failed to parse default alias yaml input data\n");
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
	if (repeat >= 0 || batch >= 0 || cpu_mask >= 0 || rate >= 0 ||
	    status_interval >= 0 || mode != NULL) {
		uint32_t len = 0;
		char alias_str[MAX_ALIAS_STR_LEN+1] = "";

		if (repeat >= 0) {
			len = generate_alias_yaml(alias_str, "default_repeat",
						  repeat, NULL);
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
						  batch, NULL);
			PNSO_LOG_WARN("module param batch: %s\n", alias_str);
			err = pnso_test_parse_buf(alias_str, len, cfg);
			if (err) {
				PNSO_LOG_ERROR("Failed to parse default batch string '%s'\n",
					       alias_str);
				goto done;
			}
		}
		if (rate >= 0) {
			len = generate_alias_yaml(alias_str, "default_rate",
						  rate, NULL);
			PNSO_LOG_WARN("module param rate: %s\n", alias_str);
			err = pnso_test_parse_buf(alias_str, len, cfg);
			if (err) {
				PNSO_LOG_ERROR("Failed to parse default rate string '%s'\n",
					       alias_str);
				goto done;
			}
		}
		if (status_interval >= 0) {
			len = generate_alias_yaml(alias_str, "default_status_interval",
						  status_interval, NULL);
			PNSO_LOG_WARN("module param status_interval: %s\n", alias_str);
			err = pnso_test_parse_buf(alias_str, len, cfg);
			if (err) {
				PNSO_LOG_ERROR("Failed to parse default status_interval string '%s'\n",
					       alias_str);
				goto done;
			}
		}
		if (cpu_mask >= 0) {
			len = generate_alias_yaml(alias_str, "default_cpu_mask",
						  cpu_mask, NULL);
			PNSO_LOG_WARN("module param cpu_mask: %s\n", alias_str);
			err = pnso_test_parse_buf(alias_str, len, cfg);
			if (err) {
				PNSO_LOG_ERROR("Failed to parse default cpu_mask string '%s'\n",
					       alias_str);
				goto done;
			}
		}
		if (mode) {
			len = generate_alias_yaml(alias_str, "default_mode",
						  0, mode);
			PNSO_LOG_WARN("module param mode: %s\n", alias_str);
			err = pnso_test_parse_buf(alias_str, len, cfg);
			if (err) {
				PNSO_LOG_ERROR("Failed to parse default mode string '%s'\n",
					       alias_str);
				goto done;
			}
		}
	}

	err = pnso_test_parse_buf(default_global_yaml,
		strlen((const char*)default_global_yaml), cfg);
	if (err) {
		PNSO_LOG_ERROR("Failed to parse default global yaml input data\n");
		goto done;
	}

	for (i = 0; i < PNSO_SVC_TYPE_MAX; i++) {
		if (default_feature_yaml_list[i] && (feat_mask & (1 << i))) {
			err = pnso_test_parse_buf(default_feature_yaml_list[i],
				strlen((const char*)default_feature_yaml_list[i]),
				cfg);
			if (err) {
				PNSO_LOG_ERROR("Failed to parse yaml testcase for svc %u\n",
					       i);
				goto done;
			}
		}
	}
	if ((feat_mask & (1<<PNSO_SVC_TYPE_COMPRESS)) &&
	    (feat_mask & (1<<PNSO_SVC_TYPE_HASH))) {
		/* compress + hash chain */
		err = pnso_test_parse_buf(default_cp_hash_yaml,
			strlen(default_cp_hash_yaml),
			cfg);
		if (err) {
			PNSO_LOG_ERROR("Failed to parse yaml testcase for cp+hash\n");
			goto done;
		}
	}

//	test_dump_desc(cfg);
	pnso_run_unit_tests(cfg);

	/* run default testcases */
	err = pnso_test_run_all(cfg);
	pnso_test_desc_free(cfg);
	cfg = NULL;
	PNSO_LOG_INFO("PenCAKE completed all default testcases, status %d\n", err);

	/* loop running testcases from ctl program */
	while (!pnso_test_is_shutdown()) {
		int ctl_state;

		ctl_state = pnso_test_sysfs_read_ctl();
		switch (ctl_state) {
		case CTL_STATE_START:
		case CTL_STATE_REPEAT:
			/* setup configuration */
			pnso_test_desc_free(cfg);
			cfg = pnso_test_desc_alloc();
			if (cfg) {
				char *cfg_str;

				cfg_str = pnso_test_sysfs_alloc_and_get_cfg();
				if (cfg_str) {
					err = pnso_test_parse_buf(cfg_str,
						strlen(cfg_str), cfg);
					if (err) {
						pnso_test_desc_free(cfg);
						cfg = NULL;
					}
					TEST_FREE(cfg_str);
				} else {
					PNSO_LOG_ERROR("unable to get config string\n");
					pnso_test_desc_free(cfg);
					cfg = NULL;
				}
			}
			ctl_repeat = cfg && (ctl_state == CTL_STATE_REPEAT);
			break;
		case CTL_STATE_STOP:
			ctl_repeat = 0;
			pnso_test_desc_free(cfg);
			cfg = NULL;
			break;
		default:
			break;
		}

		if (cfg && (ctl_repeat || ctl_state == CTL_STATE_START)) {
			err = pnso_test_run_all(cfg);
		}
		osal_yield();
	}

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
	if (!err && ctl_core_id >= 0) {
		PNSO_LOG_INFO("Binding pencake ctl thread to core %d\n",
			      ctl_core_id);
		err = osal_thread_bind(&g_main_thread, ctl_core_id);
	}
	if (!err)
		err = osal_thread_start(&g_main_thread);

	if (err)
		PNSO_LOG_ERROR("Failed to start pencake ctl thread, err %d\n",
			       err);

	return err;
}

osal_init_fn_t init_fp = pnso_test_mod_init;
osal_init_fn_t fini_fp = pnso_test_mod_fini;

OSAL_SETUP(init_fp, body_start, fini_fp);
