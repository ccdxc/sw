/*
 * {C} Copyright 2018 Pensando Systems Inc.
 * All rights reserved.
 *
 */

/* TODO: remove userspace specific includes */
#include <stdlib.h>

#include "osal_stdtypes.h"
#include "osal_mem.h"
#include "pnso_test.h"

int main(int argc, char *argv[])
{
	void *cfg;
	size_t i;
	const char *ifilename = NULL;
	unsigned char test_input[] =
"global_params:\n"
"  cp_hdr_ver: 1\n"
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
"        - data_equal:\n"
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
					ifilename = argv[i+1];
				} else {
					printf("Missing input filename for param -i\n");
					bad_param = 1;
				}
				break;
			default:
				printf("Unsupported parameter %s\n", arg);
				bad_param = 1;
				break;
			}
		}
		if (bad_param) {
			exit(1);
		}
	}


	pnso_test_init_fns(pnso_submit_request, osal_alloc, osal_free, NULL);

	if (ifilename) {
		cfg = pnso_test_parse_file(ifilename);
	} else {
		cfg = pnso_test_parse_buf(test_input, strlen((const char*)test_input));
	}

	if (cfg) {
		pnso_test_run_all(cfg);
	}
	pnso_test_parse_free(cfg);

	return 0;
}
