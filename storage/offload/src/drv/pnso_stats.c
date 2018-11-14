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

	OSAL_LOG_NOTICE("%30s: 0x" PRIx64, "=== stats", (uint64_t) stats);

	OSAL_LOG_NOTICE("%30s: %ld", "pas_num_requests",
			stats->pas_num_requests.counter);
	OSAL_LOG_NOTICE("%30s: %ld", "pas_num_services",
			stats->pas_num_services.counter);
	OSAL_LOG_NOTICE("%30s: %ld", "pas_num_chains",
			stats->pas_num_chains.counter);
	OSAL_LOG_NOTICE("%30s: %ld", "pas_num_batches",
			stats->pas_num_batches.counter);

	OSAL_LOG_NOTICE("%30s:", "===");
	OSAL_LOG_NOTICE("%30s: %ld", "pas_num_request_failures",
			stats->pas_num_request_failures.counter);
	OSAL_LOG_NOTICE("%30s: %ld", "pas_num_service_failures",
			stats->pas_num_service_failures.counter);
	OSAL_LOG_NOTICE("%30s: %ld", "pas_num_chain_failures",
			stats->pas_num_chain_failures.counter);
	OSAL_LOG_NOTICE("%30s: %ld", "pas_num_batch_failures",
			stats->pas_num_batch_failures.counter);

	OSAL_LOG_NOTICE("%30s:", "===");
	OSAL_LOG_NOTICE("%30s: %ld", "pas_num_enc_requests",
			stats->pas_num_enc_requests.counter);
	OSAL_LOG_NOTICE("%30s: %ld", "pas_num_dec_requests",
			stats->pas_num_dec_requests.counter);
	OSAL_LOG_NOTICE("%30s: %ld", "pas_num_cp_requests",
			stats->pas_num_cp_requests.counter);
	OSAL_LOG_NOTICE("%30s: %ld", "pas_num_dc_requests",
			stats->pas_num_dc_requests.counter);
	OSAL_LOG_NOTICE("%30s: %ld", "pas_num_hash_requests",
			stats->pas_num_hash_requests.counter);
	OSAL_LOG_NOTICE("%30s: %ld", "pas_num_chksum_requests",
			stats->pas_num_chksum_requests.counter);

	OSAL_LOG_NOTICE("%30s:", "===");
	OSAL_LOG_NOTICE("%30s: %ld", "pas_num_enc_request_failures",
			stats->pas_num_enc_request_failures.counter);
	OSAL_LOG_NOTICE("%30s: %ld", "pas_num_dec_request_failures",
			stats->pas_num_dec_request_failures.counter);
	OSAL_LOG_NOTICE("%30s: %ld", "pas_num_cp_request_failures",
			stats->pas_num_cp_request_failures.counter);
	OSAL_LOG_NOTICE("%30s: %ld", "pas_num_dc_request_failures",
			stats->pas_num_dc_request_failures.counter);
	OSAL_LOG_NOTICE("%30s: %ld", "pas_num_hash_request_failures",
			stats->pas_num_hash_request_failures.counter);
	OSAL_LOG_NOTICE("%30s: %ld", "pas_num_chksum_request_failures",
			stats->pas_num_chksum_request_failures.counter);

	OSAL_LOG_NOTICE("%30s:", "===");
	OSAL_LOG_NOTICE("%30s: %ld", "pas_num_enc_bytes",
			stats->pas_num_enc_bytes.counter);
	OSAL_LOG_NOTICE("%30s: %ld", "pas_num_dec_bytes",
			stats->pas_num_dec_bytes.counter);

	OSAL_LOG_NOTICE("%30s: %ld", "pas_num_cp_bytes_in",
			stats->pas_num_cp_bytes_in.counter);
	OSAL_LOG_NOTICE("%30s: %ld", "pas_num_cp_bytes_out",
			stats->pas_num_cp_bytes_out.counter);

	OSAL_LOG_NOTICE("%30s: %ld", "pas_num_dc_bytes_in",
			stats->pas_num_dc_bytes_in.counter);
	OSAL_LOG_NOTICE("%30s: %ld", "pas_num_dc_bytes_out",
			stats->pas_num_dc_bytes_out.counter);

	OSAL_LOG_NOTICE("%30s: %ld", "pas_num_hash_bytes_in",
			stats->pas_num_hash_bytes_in.counter);
	OSAL_LOG_NOTICE("%30s: %ld", "pas_num_hashes",
			stats->pas_num_hashes.counter);

	OSAL_LOG_NOTICE("%30s: %ld", "pas_num_chksum_bytes_in",
			stats->pas_num_chksum_bytes_in.counter);
	OSAL_LOG_NOTICE("%30s: %ld", "pas_num_chksums",
			stats->pas_num_chksums.counter);

	OSAL_LOG_NOTICE("%30s:", "===");
	OSAL_LOG_NOTICE("%30s: %ld (%s)", "pas_total_latency",
			stats->pas_total_latency.counter, "us");
	OSAL_LOG_NOTICE("%30s: %ld (%s)", "pas_total_hw_latency",
			stats->pas_total_hw_latency.counter, "us");
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
