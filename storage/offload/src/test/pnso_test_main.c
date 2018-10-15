/*
 * {C} Copyright 2018 Pensando Systems Inc.
 * All rights reserved.
 *
 */

/* TODO: remove userspace specific includes */
#include <stdlib.h> /* for: atoi, exit */
#include <ctype.h> /* for: isdigit */

#include "osal_stdtypes.h"
#include "osal_mem.h"
#include "osal_errno.h"
#include "pnso_test.h"

void status_output_func(const char *status, void *opaque)
{
	printf("%s", status);
}

static const unsigned char default_test_input[] = 
"global_params:\n"
"  store_output_files: 0\n"
"  per_core_qdepth: 32\n"
"  block_size: 4096\n"
"  limit_rate: 1000000\n"
"  status_interval: 5\n"
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
"svc_chains:\n"
"  - svc_chain:\n"
"      idx: 1\n"
"      input:\n"
"        pattern: ababababababababababababababababababababababababababababababababababababab1234\n"
"        len: 2000\n"
"      ops:\n"
"        - compress:\n"
"            flags: 'zero_pad,insert_header'\n"
"            hdr_fmt_idx: 1\n"
"            threshold_delta: 8\n"
"            output_file: 'compressed_1.bin'\n"
"        - decompress:\n"
"            flags: 'header_present'\n"
"            hdr_fmt_idx: 1\n"
"            output_file: 'decompressed.bin'\n"
"        - compress:\n"
"            flags: 'zero_pad,insert_header'\n"
"            hdr_fmt_idx: 1\n"
"            threshold_delta: 8\n"
"            output_file: 'compressed_2.bin'\n"
"tests:\n"
"  - test:\n"
"      idx: 1\n"
"      svc_chains: 1\n"
"      repeat: 10000\n"
"      batch_depth: 10\n"
"      mode: poll\n"
"      validations:\n"
"        - data_compare:\n"
"            idx: 1\n"
"            file1: 'compressed_1.bin'\n"
"            file2: 'compressed_2.bin'\n"
"\n";

int main(int argc, char *argv[])
{
	struct test_desc *cfg = NULL;
	pnso_error_t err;
	size_t i;
	uint32_t ifilenames_count = 0;
	char **ifilenames = NULL;
//	char *ofilename = NULL;
	char *path_prefix = NULL;
	char *cur_filename;
	char *literyaml = NULL;
	char filename[256];
	char last_opt = '\0';

	osal_log_init(OSAL_LOG_LEVEL_INFO);

	/* Parse cmdline parameters */
	for (i = 1; i < argc; i++) {
		int bad_param = 0;
		const char *arg = argv[i];
		if (arg[0] == '-') {
			switch (arg[1]) {
			case 'i':
				if (i+1 < argc) {
					ifilenames = &argv[++i];
					ifilenames_count = 1;
				} else {
					PNSO_LOG_ERROR("Missing input filename for param -i\n");
					bad_param = 1;
				}
				break;
#if 0
			case 'o':
				if (i+1 < argc) {
					ofilename = argv[++i];
				} else {
					PNSO_LOG_ERROR("Missing output filename for param -o\n");
					bad_param = 1;
				}
				break;
#endif
			case 'p':
				if (i+1 < argc) {
					path_prefix = argv[++i];
				} else {
					PNSO_LOG_ERROR("Missing input path for param -p\n");
					bad_param = 1;
				}
				break;
			case 'y':
				if (i+1 < argc) {
					literyaml = argv[++i];
				} else {
					PNSO_LOG_ERROR("Missing literal YAML string for param -y\n");
					bad_param = 1;
				}
				break;
			case 'h':
				PNSO_LOG_INFO("Usage: pnso_test_main -y <literal_yaml> -o <output_file> -p <base_path> -i <yaml_files>\n");
				PNSO_LOG_INFO("Valid YAML structure:\n");
				test_dump_yaml_desc_tree();
				exit(0);
				break;
			case 'v':
				/* verbose */
				osal_log_init(OSAL_LOG_LEVEL_DEBUG);
				break;
			default:
				PNSO_LOG_ERROR("Unsupported parameter %s\n", arg);
				bad_param = 1;
				break;
			}
			last_opt = arg[1];
		} else {
			switch (last_opt) {
			case 'i':
				ifilenames_count++;
				break;
			default:
				break;
			}
		}
		if (bad_param) {
			exit(EINVAL);
		}
	}

	pnso_test_init_fns(pnso_submit_request, status_output_func,
			   osal_alloc, osal_free, osal_realloc);

	cfg = pnso_test_desc_alloc();
	if (!cfg) {
		PNSO_LOG_ERROR("Cannot allocate test descriptor\n");
		exit(ENOMEM);
	}

	if (literyaml) {
		err = pnso_test_parse_buf((uint8_t *)literyaml,
					  strlen(literyaml), cfg);
		if (err) {
			PNSO_LOG_ERROR("Failed to parse literal YAML input data\n");
			goto done;
		}
	}

	if (ifilenames_count) {
		for (i = 0; i < ifilenames_count; i++) {
			if (path_prefix) {
				strcpy(filename, path_prefix);
				strcat(filename, ifilenames[i]);
				cur_filename = filename;
			} else {
				cur_filename = ifilenames[i];
			}
			err = pnso_test_parse_file(cur_filename, cfg);
			if (err != PNSO_OK) {
				PNSO_LOG_ERROR("Failed to read or parse test input file %s\n",
					       cur_filename);
				goto done;
			}
		}
	} else {
		err = pnso_test_parse_buf(default_test_input,
			strlen((const char *)default_test_input), cfg);
		if (err) {
			PNSO_LOG_ERROR("Failed to parse default test input data\n");
			goto done;
		}
	}
	test_dump_desc(cfg);

	pnso_run_unit_tests(cfg);

	err = pnso_test_run_all(cfg);

done:
	pnso_test_desc_free(cfg);

	pnso_test_set_shutdown_complete();
	return err;
}
