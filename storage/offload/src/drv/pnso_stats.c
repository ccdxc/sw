/*
 * {C} Copyright 2018 Pensando Systems Inc.
 * All rights reserved.
 *
 */
#include "osal.h"
#include "pnso_api.h"

#include "sonic_dev.h"
#include "pnso_stats.h"

static void
pprint_pnso_stats(struct pnso_api_stats *stats)
{
	if (!stats)
		return;

	OSAL_LOG_NOTICE("%30s: 0x%llx", "=== stats", (uint64_t) stats);

	OSAL_LOG_NOTICE("%30s: %llu", "pas_num_requests",
			stats->pas_num_requests);
	OSAL_LOG_NOTICE("%30s: %llu", "pas_num_services",
			stats->pas_num_services);
	OSAL_LOG_NOTICE("%30s: %llu", "pas_num_chains",
			stats->pas_num_chains);
	OSAL_LOG_NOTICE("%30s: %llu", "pas_num_batches",
			stats->pas_num_batches);

	OSAL_LOG_NOTICE("%30s:", "===");
	OSAL_LOG_NOTICE("%30s: %llu", "pas_num_request_failures",
			stats->pas_num_request_failures);
	OSAL_LOG_NOTICE("%30s: %llu", "pas_num_service_failures",
			stats->pas_num_service_failures);
	OSAL_LOG_NOTICE("%30s: %llu", "pas_num_chain_failures",
			stats->pas_num_chain_failures);
	OSAL_LOG_NOTICE("%30s: %llu", "pas_num_batch_failures",
			stats->pas_num_batch_failures);

	OSAL_LOG_NOTICE("%30s:", "===");
	OSAL_LOG_NOTICE("%30s: %llu", "pas_num_enc_requests",
			stats->pas_num_enc_requests);
	OSAL_LOG_NOTICE("%30s: %llu", "pas_num_dec_requests",
			stats->pas_num_dec_requests);
	OSAL_LOG_NOTICE("%30s: %llu", "pas_num_cp_requests",
			stats->pas_num_cp_requests);
	OSAL_LOG_NOTICE("%30s: %llu", "pas_num_dc_requests",
			stats->pas_num_dc_requests);
	OSAL_LOG_NOTICE("%30s: %llu", "pas_num_hash_requests",
			stats->pas_num_hash_requests);
	OSAL_LOG_NOTICE("%30s: %llu", "pas_num_chksum_requests",
			stats->pas_num_chksum_requests);

	OSAL_LOG_NOTICE("%30s:", "===");
	OSAL_LOG_NOTICE("%30s: %llu", "pas_num_enc_bytes",
			stats->pas_num_enc_bytes);
	OSAL_LOG_NOTICE("%30s: %llu", "pas_num_dec_bytes",
			stats->pas_num_dec_bytes);

	OSAL_LOG_NOTICE("%30s: %llu", "pas_num_cp_bytes_in",
			stats->pas_num_cp_bytes_in);
	OSAL_LOG_NOTICE("%30s: %llu", "pas_num_cp_bytes_out",
			stats->pas_num_cp_bytes_out);

	OSAL_LOG_NOTICE("%30s: %llu", "pas_num_dc_bytes_in",
			stats->pas_num_dc_bytes_in);
	OSAL_LOG_NOTICE("%30s: %llu", "pas_num_dc_bytes_out",
			stats->pas_num_dc_bytes_out);

	OSAL_LOG_NOTICE("%30s: %llu", "pas_num_hashes",
			stats->pas_num_hashes);
	OSAL_LOG_NOTICE("%30s: %llu", "pas_num_chksums",
			stats->pas_num_chksums);

	OSAL_LOG_NOTICE("%30s:", "===");
	OSAL_LOG_NOTICE("%30s: %llu (%s)", "pas_elapsed_time",
			stats->pas_elapsed_time, "us");
	OSAL_LOG_NOTICE("%30s: %llu (%s)", "pas_elapsed_hw_time",
			stats->pas_elapsed_hw_time, "us");
}

void
pas_init(struct pnso_api_stats *stats)
{
	memset(stats, 0, sizeof(*stats));
}

void
pas_reset_stats(struct pnso_api_stats *stats)
{
	pas_init(stats);
}

void
pas_show_stats(struct pnso_api_stats *stats)
{
	pprint_pnso_stats(stats);
}
