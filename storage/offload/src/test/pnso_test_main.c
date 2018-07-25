/*
 * {C} Copyright 2018 Pensando Systems Inc.
 * All rights reserved.
 *
 */

/* TODO: remove userspace specific includes */
#include <stdlib.h>

#include "osal_stdtypes.h"
#include "osal_mem.h"
#include "osal_errno.h"
#include "pnso_test.h"

int main(int argc, char *argv[])
{
	struct test_desc *cfg = NULL;
	pnso_error_t err;
	size_t i;
	uint32_t ifilenames_count = 0;
	char **ifilenames = NULL;
	char *path_prefix = NULL;
	char *cur_filename;
	char *literyaml = NULL;
	char filename[256];
	char last_opt = '\0';
	unsigned char test_input[] =
"global_params:\n"
"  delete_output_files: 1\n"
"  per_core_qdepth: 32\n"
"  block_size: 8192\n"
"svc_chains:\n"
"  - svc_chain:\n"
"      idx: 1\n"
"      input:\n"
"        pattern: ababababababababababababababababababababababababababababababababababababab1234\n"
"        buffer_size: 2000\n"
"      ops:\n"
"        - compress:\n"
"            flags: 'zero_pad,insert_header'\n"
"            threshold_delta: 8\n"
"            output_file: '%chain_id%_compressed_1.bin'\n"
"        - decompress:\n"
"            flags: 'header_present'\n"
"            output_file: '%chain_id%_decompressed.bin'\n"
"        - compress:\n"
"            flags: 'zero_pad,insert_header'\n"
"            threshold_delta: 8\n"
"            output_file: '%chain_id%_compressed_2.bin'\n"
"tests:\n"
"  - test:\n"
"      idx: 1\n"
"      svc_chains: 1\n"
"      repeat: 1\n"
"      validations:\n"
"        - data_compare:\n"
"            idx: 1\n"
"            file1: '%chain_id%_compressed_1.bin'\n"
"            file2: '%chain_id%_compressed_2.bin'\n"
"\n";

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
				PNSO_LOG_INFO("Usage: pnso_test_main -y <literal_yaml> -p <base_path> -i <yaml_files>\n");
				PNSO_LOG_INFO("Valid YAML structure:\n");
				test_dump_yaml_desc_tree();
				exit(0);
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
			exit(1);
		}
	}


	pnso_test_init_fns(pnso_submit_request, osal_alloc, osal_free, NULL);

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
		err = pnso_test_parse_buf(test_input,
					  strlen((const char*)test_input), cfg);
		if (err) {
			PNSO_LOG_ERROR("Failed to parse test input data\n");
			goto done;
		}
	}

	test_dump_desc(cfg);

	err = pnso_test_run_all(cfg);

done:
	pnso_test_desc_free(cfg);

	return err;
}
