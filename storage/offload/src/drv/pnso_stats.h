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
 * 	- replace ktime with osal equivalent
 * 	- reivew service/batch level stats counting
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

	uint64_t pas_latency_time;
	uint64_t pas_hw_latency_time;
};

#ifdef NO_PAS_STATS
#define PAS_INC_NUM_REQUESTS(pcr)
#define PAS_INC_NUM_SERVICES(pcr)
#define PAS_INC_NUM_CHAINS(pcr)
#define PAS_INC_NUM_BATCHES(pcr)

#define PAS_INC_NUM_REQUEST_FAILURES(pcr)
#define PAS_INC_NUM_SERVICE_FAILURES(pcr)
#define PAS_INC_NUM_CHAIN_FAILURES(pcr)
#define PAS_INC_NUM_BATCH_FAILURES(pcr)

#define PAS_INC_NUM_ENC_REQUESTS(pcr)
#define PAS_INC_NUM_DEC_REQUESTS(pcr)
#define PAS_INC_NUM_CP_REQUESTS(pcr)
#define PAS_INC_NUM_DC_REQUESTS(pcr)
#define PAS_INC_NUM_HASH_REQUESTS(pcr)
#define PAS_INC_NUM_CHKSUM_REQUESTS(pcr)

#define PAS_INC_NUM_ENC_REQUEST_FAILURES(pcr)
#define PAS_INC_NUM_DEC_REQUEST_FAILURES(pcr)
#define PAS_INC_NUM_CP_REQUEST_FAILURES(pcr)
#define PAS_INC_NUM_DC_REQUEST_FAILURES(pcr)
#define PAS_INC_NUM_HASH_REQUEST_FAILURES(pcr)
#define PAS_INC_NUM_CHKSUM_REQUEST_FAILURES(pcr)

#define PAS_INC_NUM_ENC_BYTES(pcr, bytes)
#define PAS_INC_NUM_DEC_BYTES(pcr, bytes)

#define PAS_INC_NUM_CP_BYTES_IN(pcr, bytes)
#define PAS_INC_NUM_CP_BYTES_OUT(pcr, bytes)

#define PAS_INC_NUM_DC_BYTES_IN(pcr, bytes)
#define PAS_INC_NUM_DC_BYTES_OUT(pcr, bytes)

#define PAS_INC_NUM_HASH_BYTES_IN(pcr, bytes)
#define PAS_INC_NUM_HASHES(pcr, count)

#define PAS_INC_NUM_CHKSUM_BYTES_IN(pcr, bytes)
#define PAS_INC_NUM_CHKSUMS(pcr, count)

#define PAS_START_PERF()
#define PAS_END_PERF(pcr)

#define PAS_INIT_HW_PERF()
#define PAS_START_HW_PERF()
#define PAS_END_HW_PERF(pcr)

#define PAS_SHOW_STATS(pcr)
#else
#define PAS_INC_NUM_REQUESTS(pcr)					\
	(pcr->pnso_api_stats.pas_num_requests += 1)
#define PAS_INC_NUM_SERVICES(pcr)					\
	(pcr->pnso_api_stats.pas_num_services += 1)
#define PAS_INC_NUM_CHAINS(pcr)						\
	(pcr->pnso_api_stats.pas_num_chains += 1)
#define PAS_INC_NUM_BATCHES(pcr)					\
	(pcr->pnso_api_stats.pas_num_batches += 1)

#define PAS_INC_NUM_REQUEST_FAILURES(pcr)				\
	(pcr->pnso_api_stats.pas_num_request_failures += 1)
#define PAS_INC_NUM_SERVICE_FAILURES(pcr)				\
	(pcr->pnso_api_stats.pas_num_service_failures += 1)
#define PAS_INC_NUM_CHAIN_FAILURES(pcr)					\
	(pcr->pnso_api_stats.pas_num_chain_failures += 1)
#define PAS_INC_NUM_BATCH_FAILURES(pcr)					\
	(pcr->pnso_api_stats.pas_num_batch_failures += 1)

#define PAS_INC_NUM_ENC_REQUESTS(pcr)					\
	(pcr->pnso_api_stats.pas_num_enc_requests += 1)
#define PAS_INC_NUM_DEC_REQUESTS(pcr)					\
	(pcr->pnso_api_stats.pas_num_dec_requests += 1)
#define PAS_INC_NUM_CP_REQUESTS(pcr)					\
	(pcr->pnso_api_stats.pas_num_cp_requests += 1)
#define PAS_INC_NUM_DC_REQUESTS(pcr)					\
	(pcr->pnso_api_stats.pas_num_dc_requests += 1)
#define PAS_INC_NUM_HASH_REQUESTS(pcr)					\
	(pcr->pnso_api_stats.pas_num_hash_requests += 1)
#define PAS_INC_NUM_CHKSUM_REQUESTS(pcr)				\
	(pcr->pnso_api_stats.pas_num_chksum_requests += 1)

#define PAS_INC_NUM_ENC_REQUEST_FAILURES(pcr)				\
	(pcr->pnso_api_stats.pas_num_enc_request_failures += 1)
#define PAS_INC_NUM_DEC_REQUEST_FAILURES(pcr)				\
	(pcr->pnso_api_stats.pas_num_dec_request_failures += 1)
#define PAS_INC_NUM_CP_REQUEST_FAILURES(pcr)				\
	(pcr->pnso_api_stats.pas_num_cp_request_failures += 1)
#define PAS_INC_NUM_DC_REQUEST_FAILURES(pcr)				\
	(pcr->pnso_api_stats.pas_num_dc_request_failures += 1)
#define PAS_INC_NUM_HASH_REQUEST_FAILURES(pcr)				\
	(pcr->pnso_api_stats.pas_num_hash_request_failures += 1)
#define PAS_INC_NUM_CHKSUM_REQUEST_FAILURES(pcr)			\
	(pcr->pnso_api_stats.pas_num_chksum_request_failures += 1)

#define PAS_INC_NUM_ENC_BYTES(pcr, bytes)				\
	(pcr->pnso_api_stats.pas_num_enc_bytes += bytes)
#define PAS_INC_NUM_DEC_BYTES(pcr, bytes)				\
	(pcr->pnso_api_stats.pas_num_dec_bytes += bytes)

#define PAS_INC_NUM_CP_BYTES_IN(pcr, bytes)				\
	(pcr->pnso_api_stats.pas_num_cp_bytes_in += bytes)
#define PAS_INC_NUM_CP_BYTES_OUT(pcr, bytes)				\
	(pcr->pnso_api_stats.pas_num_cp_bytes_out += bytes)

#define PAS_INC_NUM_DC_BYTES_IN(pcr, bytes)				\
	(pcr->pnso_api_stats.pas_num_dc_bytes_in += bytes)
#define PAS_INC_NUM_DC_BYTES_OUT(pcr, bytes)				\
	(pcr->pnso_api_stats.pas_num_dc_bytes_out += bytes)

/* TODO-stats: Separate count for different algo?? */
#define PAS_INC_NUM_HASH_BYTES_IN(pcr, bytes)				\
	(pcr->pnso_api_stats.pas_num_hash_bytes_in += bytes)
#define PAS_INC_NUM_HASHES(pcr, count)					\
	(pcr->pnso_api_stats.pas_num_hashes += count)

#define PAS_INC_NUM_CHKSUM_BYTES_IN(pcr, bytes)				\
	(pcr->pnso_api_stats.pas_num_chksum_bytes_in += bytes)
#define PAS_INC_NUM_CHKSUMS(pcr, count)					\
	(pcr->pnso_api_stats.pas_num_chksums += count)

/* for measuring the time spent in both software and hardware */
#define PAS_START_PERF()	ktime_t s; s = ktime_get()
#define PAS_END_PERF(pcr)						\
	pcr->pnso_api_stats.pas_latency_time +=				\
		ktime_us_delta(ktime_get(), s);				\
	pas_show_stats(&pcr->pnso_api_stats);

/* for measuring the time spent just in hardware */
#define PAS_INIT_HW_PERF()	ktime_t h
#define PAS_START_HW_PERF()	h = ktime_get()
#define PAS_END_HW_PERF(pcr)						\
	(pcr->pnso_api_stats.pas_hw_latency_time +=			\
		ktime_us_delta(ktime_get(), h))

#define PAS_SHOW_STATS(pcr)	pas_show_stats(&pcr->pnso_api_stats);
#endif

void pas_init(struct pnso_api_stats *stats);

void pas_reset_stats(struct pnso_api_stats *stats);

void pas_show_stats(struct pnso_api_stats *stats);

#ifdef __cplusplus
}
#endif

#endif /* __PNSO_STATS_H__ */
