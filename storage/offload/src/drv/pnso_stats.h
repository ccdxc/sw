/*
 * {C} Copyright 2018 Pensando Systems Inc.
 * All rights reserved.
 *
 */
#ifndef __PNSO_STATS_H__
#define __PNSO_STATS_H__

#include "pnso_api.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * TODO-stats:
 *	- replace ktime with osal equivalent
 *	- reivew/add service/batch level stats, failure stat counting
 *	- add 'get/per-core aggregate' stats api
 *	- track emitting stats at module unload
 *
 */
struct pnso_api_stats {
	uint64_t pas_num_requests;
	uint64_t pas_num_services;
	uint64_t pas_num_chains;
	uint64_t pas_num_batches;

	uint64_t pas_num_request_failures;
	uint64_t pas_num_service_failures;
	uint64_t pas_num_chain_failures;
	uint64_t pas_num_batch_failures;

	uint64_t pas_num_enc_requests;
	uint64_t pas_num_dec_requests;
	uint64_t pas_num_cp_requests;
	uint64_t pas_num_dc_requests;
	uint64_t pas_num_hash_requests;
	uint64_t pas_num_chksum_requests;

	uint64_t pas_num_enc_request_failures;
	uint64_t pas_num_dec_request_failures;
	uint64_t pas_num_cp_request_failures;
	uint64_t pas_num_dc_request_failures;
	uint64_t pas_num_hash_request_failures;
	uint64_t pas_num_chksum_request_failures;

	uint64_t pas_num_enc_bytes;
	uint64_t pas_num_dec_bytes;

	uint64_t pas_num_cp_bytes_in;
	uint64_t pas_num_cp_bytes_out;

	uint64_t pas_num_dc_bytes_in;
	uint64_t pas_num_dc_bytes_out;

	uint64_t pas_num_hash_bytes_in;
	uint64_t pas_num_hashes;

	uint64_t pas_num_chksum_bytes_in;
	uint64_t pas_num_chksums;

	uint64_t pas_total_latency;
	uint64_t pas_total_hw_latency;
};

#define PAS_INC_NUM_REQUESTS(pcr)					\
	(pcr->api_stats.pas_num_requests += 1)
#define PAS_INC_NUM_SERVICES(pcr)					\
	(pcr->api_stats.pas_num_services += 1)
#define PAS_INC_NUM_CHAINS(pcr)						\
	(pcr->api_stats.pas_num_chains += 1)
#define PAS_INC_NUM_BATCHES(pcr)					\
	(pcr->api_stats.pas_num_batches += 1)

#define PAS_INC_NUM_REQUEST_FAILURES(pcr)				\
	(pcr->api_stats.pas_num_request_failures += 1)
#define PAS_INC_NUM_SERVICE_FAILURES(pcr)				\
	(pcr->api_stats.pas_num_service_failures += 1)
#define PAS_INC_NUM_CHAIN_FAILURES(pcr)					\
	(pcr->api_stats.pas_num_chain_failures += 1)
#define PAS_INC_NUM_BATCH_FAILURES(pcr)					\
	(pcr->api_stats.pas_num_batch_failures += 1)

#define PAS_INC_NUM_ENC_REQUESTS(pcr)					\
	(pcr->api_stats.pas_num_enc_requests += 1)
#define PAS_INC_NUM_DEC_REQUESTS(pcr)					\
	(pcr->api_stats.pas_num_dec_requests += 1)
#define PAS_INC_NUM_CP_REQUESTS(pcr)					\
	(pcr->api_stats.pas_num_cp_requests += 1)
#define PAS_INC_NUM_DC_REQUESTS(pcr)					\
	(pcr->api_stats.pas_num_dc_requests += 1)
#define PAS_INC_NUM_HASH_REQUESTS(pcr)					\
	(pcr->api_stats.pas_num_hash_requests += 1)
#define PAS_INC_NUM_CHKSUM_REQUESTS(pcr)				\
	(pcr->api_stats.pas_num_chksum_requests += 1)

#define PAS_INC_NUM_ENC_REQUEST_FAILURES(pcr)				\
	(pcr->api_stats.pas_num_enc_request_failures += 1)
#define PAS_INC_NUM_DEC_REQUEST_FAILURES(pcr)				\
	(pcr->api_stats.pas_num_dec_request_failures += 1)
#define PAS_INC_NUM_CP_REQUEST_FAILURES(pcr)				\
	(pcr->api_stats.pas_num_cp_request_failures += 1)
#define PAS_INC_NUM_DC_REQUEST_FAILURES(pcr)				\
	(pcr->api_stats.pas_num_dc_request_failures += 1)
#define PAS_INC_NUM_HASH_REQUEST_FAILURES(pcr)				\
	(pcr->api_stats.pas_num_hash_request_failures += 1)
#define PAS_INC_NUM_CHKSUM_REQUEST_FAILURES(pcr)			\
	(pcr->api_stats.pas_num_chksum_request_failures += 1)

#define PAS_INC_NUM_ENC_BYTES(pcr, bytes)				\
	(pcr->api_stats.pas_num_enc_bytes += bytes)
#define PAS_INC_NUM_DEC_BYTES(pcr, bytes)				\
	(pcr->api_stats.pas_num_dec_bytes += bytes)

#define PAS_INC_NUM_CP_BYTES_IN(pcr, bytes)				\
	(pcr->api_stats.pas_num_cp_bytes_in += bytes)
#define PAS_INC_NUM_CP_BYTES_OUT(pcr, bytes)				\
	(pcr->api_stats.pas_num_cp_bytes_out += bytes)

#define PAS_INC_NUM_DC_BYTES_IN(pcr, bytes)				\
	(pcr->api_stats.pas_num_dc_bytes_in += bytes)
#define PAS_INC_NUM_DC_BYTES_OUT(pcr, bytes)				\
	(pcr->api_stats.pas_num_dc_bytes_out += bytes)

/* TODO-stats: Separate count for different algo?? */
#define PAS_INC_NUM_HASH_BYTES_IN(pcr, bytes)				\
	(pcr->api_stats.pas_num_hash_bytes_in += bytes)
#define PAS_INC_NUM_HASHES(pcr, count)					\
	(pcr->api_stats.pas_num_hashes += count)

#define PAS_INC_NUM_CHKSUM_BYTES_IN(pcr, bytes)				\
	(pcr->api_stats.pas_num_chksum_bytes_in += bytes)
#define PAS_INC_NUM_CHKSUMS(pcr, count)					\
	(pcr->api_stats.pas_num_chksums += count)

/* for measuring the time spent in both software and hardware */
#define PAS_DECL_PERF()		ktime_t s
#define PAS_START_PERF()	s = ktime_get()
#define PAS_END_PERF(pcr)						\
	pcr->api_stats.pas_total_latency +=				\
		ktime_us_delta(ktime_get(), s);

/* for measuring the time spent just in hardware */
#define PAS_DECL_HW_PERF()	ktime_t h
#define PAS_START_HW_PERF()	h = ktime_get()
#define PAS_END_HW_PERF(pcr)						\
	(pcr->api_stats.pas_total_hw_latency +=				\
		ktime_us_delta(ktime_get(), h))

#define PAS_SHOW_STATS(pcr)	pas_show_stats(&pcr->api_stats);

void pas_init(struct pnso_api_stats *stats);

void pas_reset_stats(struct pnso_api_stats *stats);

void pas_show_stats(struct pnso_api_stats *stats);

#ifdef __cplusplus
}
#endif

#endif /* __PNSO_STATS_H__ */
