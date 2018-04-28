#include <stdio.h>

#include "pnso_api.h"

int
print_struct_sizes(void)
{
	/* pnso_api */
	printf("%40s: %4d\n", "sizeof(struct pnso_flat_buffer)",
			sizeof(struct pnso_flat_buffer));
	printf("%40s: %4d\n", "sizeof(struct pnso_buffer_list)",
			sizeof(struct pnso_buffer_list));

	printf("%40s: %4d\n", "sizeof(struct pnso_compression_header)",
			sizeof(struct pnso_compression_header));

	printf("%40s: %4d\n", "sizeof(struct pnso_init_params)",
			sizeof(struct pnso_init_params));

	printf("%40s: %4d\n", "sizeof(struct pnso_crypto_desc)",
			sizeof(struct pnso_crypto_desc));
	printf("%40s: %4d\n", "sizeof(struct pnso_compression_desc)",
			sizeof(struct pnso_compression_desc));
	printf("%40s: %4d\n", "sizeof(struct pnso_decompression_desc)",
			sizeof(struct pnso_decompression_desc));
	printf("%40s: %4d\n", "sizeof(struct pnso_hash_desc)",
			sizeof(struct pnso_hash_desc));
	printf("%40s: %4d\n", "sizeof(struct pnso_checksum_desc)",
			sizeof(struct pnso_checksum_desc));
	printf("%40s: %4d\n", "sizeof(struct pnso_decompaction_desc)",
			sizeof(struct pnso_decompaction_desc));

	printf("%40s: %4d\n", "sizeof(struct pnso_hash_or_chksum_tag)",
			sizeof(struct pnso_hash_or_chksum_tag));

	printf("%40s: %4d\n", "sizeof(struct pnso_service_status)",
			sizeof(struct pnso_service_status));
	printf("%40s: %4d\n", "sizeof(struct pnso_service_result)",
			sizeof(struct pnso_service_result));

	printf("%40s: %4d\n", "sizeof(struct pnso_service)",
			sizeof(struct pnso_service));
	printf("%40s: %4d\n", "sizeof(struct pnso_service_request)",
			sizeof(struct pnso_service_request));

	return 0;
}

int
main(void)
{
	print_struct_sizes();

	return 0;
}
