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
			PNSO_STAT_READ(stats, num_requests));
	OSAL_LOG_NOTICE("%30s: %ld", "pas_num_services",
			PNSO_STAT_READ(stats, num_services));
	OSAL_LOG_NOTICE("%30s: %ld", "pas_num_chains",
			PNSO_STAT_READ(stats, num_chains));
	OSAL_LOG_NOTICE("%30s: %ld", "pas_num_batches",
			PNSO_STAT_READ(stats, num_batches));

	OSAL_LOG_NOTICE("%30s:", "===");
	OSAL_LOG_NOTICE("%30s: %ld", "pas_num_request_failures",
			PNSO_STAT_READ(stats, num_request_failures));
	OSAL_LOG_NOTICE("%30s: %ld", "pas_num_service_failures",
			PNSO_STAT_READ(stats, num_service_failures));
	OSAL_LOG_NOTICE("%30s: %ld", "pas_num_chain_failures",
			PNSO_STAT_READ(stats, num_chain_failures));
	OSAL_LOG_NOTICE("%30s: %ld", "pas_num_batch_failures",
			PNSO_STAT_READ(stats, num_batch_failures));

	OSAL_LOG_NOTICE("%30s:", "===");
	OSAL_LOG_NOTICE("%30s: %ld", "pas_num_enc_requests",
			PNSO_STAT_READ(stats, num_enc_requests));
	OSAL_LOG_NOTICE("%30s: %ld", "pas_num_dec_requests",
			PNSO_STAT_READ(stats, num_dec_requests));
	OSAL_LOG_NOTICE("%30s: %ld", "pas_num_cp_requests",
			PNSO_STAT_READ(stats, num_cp_requests));
	OSAL_LOG_NOTICE("%30s: %ld", "pas_num_dc_requests",
			PNSO_STAT_READ(stats, num_dc_requests));
	OSAL_LOG_NOTICE("%30s: %ld", "pas_num_hash_requests",
			PNSO_STAT_READ(stats, num_hash_requests));
	OSAL_LOG_NOTICE("%30s: %ld", "pas_num_chksum_requests",
			PNSO_STAT_READ(stats, num_chksum_requests));

	OSAL_LOG_NOTICE("%30s:", "===");
	OSAL_LOG_NOTICE("%30s: %ld", "pas_num_enc_request_failures",
			PNSO_STAT_READ(stats, num_enc_request_failures));
	OSAL_LOG_NOTICE("%30s: %ld", "pas_num_dec_request_failures",
			PNSO_STAT_READ(stats, num_dec_request_failures));
	OSAL_LOG_NOTICE("%30s: %ld", "pas_num_cp_request_failures",
			PNSO_STAT_READ(stats, num_cp_request_failures));
	OSAL_LOG_NOTICE("%30s: %ld", "pas_num_dc_request_failures",
			PNSO_STAT_READ(stats, num_dc_request_failures));
	OSAL_LOG_NOTICE("%30s: %ld", "pas_num_hash_request_failures",
			PNSO_STAT_READ(stats, num_hash_request_failures));
	OSAL_LOG_NOTICE("%30s: %ld", "pas_num_chksum_request_failures",
			PNSO_STAT_READ(stats, num_chksum_request_failures));

	OSAL_LOG_NOTICE("%30s:", "===");
	OSAL_LOG_NOTICE("%30s: %ld", "pas_num_enc_bytes",
			PNSO_STAT_READ(stats, num_enc_bytes));
	OSAL_LOG_NOTICE("%30s: %ld", "pas_num_dec_bytes",
			PNSO_STAT_READ(stats, num_dec_bytes));

	OSAL_LOG_NOTICE("%30s: %ld", "pas_num_cp_bytes_in",
			PNSO_STAT_READ(stats, num_cp_bytes_in));
	OSAL_LOG_NOTICE("%30s: %ld", "pas_num_cp_bytes_out",
			PNSO_STAT_READ(stats, num_cp_bytes_out));

	OSAL_LOG_NOTICE("%30s: %ld", "pas_num_dc_bytes_in",
			PNSO_STAT_READ(stats, num_dc_bytes_in));
	OSAL_LOG_NOTICE("%30s: %ld", "pas_num_dc_bytes_out",
			PNSO_STAT_READ(stats, num_dc_bytes_out));

	OSAL_LOG_NOTICE("%30s: %ld", "pas_num_hash_bytes_in",
			PNSO_STAT_READ(stats, num_hash_bytes_in));
	OSAL_LOG_NOTICE("%30s: %ld", "pas_num_hashes",
			PNSO_STAT_READ(stats, num_hashes));

	OSAL_LOG_NOTICE("%30s: %ld", "pas_num_chksum_bytes_in",
			PNSO_STAT_READ(stats, num_chksum_bytes_in));
	OSAL_LOG_NOTICE("%30s: %ld", "pas_num_chksums",
			PNSO_STAT_READ(stats, num_chksums));

	OSAL_LOG_NOTICE("%30s: %ld", "pas_num_out_of_rmem_bufs",
			PNSO_STAT_READ(stats, num_out_of_rmem_bufs));
	OSAL_LOG_NOTICE("%30s: %ld", "pas_num_out_of_rmem_status",
			PNSO_STAT_READ(stats, num_out_of_rmem_status));
	OSAL_LOG_NOTICE("%30s: %ld", "pas_num_pdma_exceed_constraints",
			PNSO_STAT_READ(stats, num_pdma_exceed_constraints));

	OSAL_LOG_NOTICE("%30s:", "===");
	OSAL_LOG_NOTICE("%30s: %ld (%s)", "pas_total_latency",
			PNSO_STAT_READ(stats, total_latency), "us");
	OSAL_LOG_NOTICE("%30s: %ld (%s)", "pas_total_hw_latency",
			PNSO_STAT_READ(stats, total_hw_latency), "us");
}

void
pas_init(struct pnso_api_stats *stats)
{
	OSAL_STATIC_ASSERT(sizeof(struct pnso_api_stats) == (sizeof(atomic64_t) * pas_max));

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
