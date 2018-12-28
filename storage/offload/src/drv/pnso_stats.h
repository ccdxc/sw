/*
 * {C} Copyright 2018 Pensando Systems Inc.
 * All rights reserved.
 *
 */
#ifndef __PNSO_STATS_H__
#define __PNSO_STATS_H__

#include <linux/atomic.h>
#include "pnso_api.h"
#include "osal_atomic.h"

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
	atomic64_t pas_num_requests;
	atomic64_t pas_num_services;
	atomic64_t pas_num_chains;
	atomic64_t pas_num_batches;

	atomic64_t pas_num_request_failures;
	atomic64_t pas_num_service_failures;
	atomic64_t pas_num_chain_failures;
	atomic64_t pas_num_batch_failures;

	atomic64_t pas_num_enc_requests;
	atomic64_t pas_num_dec_requests;
	atomic64_t pas_num_cp_requests;
	atomic64_t pas_num_dc_requests;
	atomic64_t pas_num_hash_requests;
	atomic64_t pas_num_chksum_requests;

	atomic64_t pas_num_enc_request_failures;
	atomic64_t pas_num_dec_request_failures;
	atomic64_t pas_num_cp_request_failures;
	atomic64_t pas_num_dc_request_failures;
	atomic64_t pas_num_hash_request_failures;
	atomic64_t pas_num_chksum_request_failures;

	atomic64_t pas_num_enc_bytes;
	atomic64_t pas_num_dec_bytes;

	atomic64_t pas_num_cp_bytes_in;
	atomic64_t pas_num_cp_bytes_out;

	atomic64_t pas_num_dc_bytes_in;
	atomic64_t pas_num_dc_bytes_out;

	atomic64_t pas_num_hash_bytes_in;
	atomic64_t pas_num_hashes;

	atomic64_t pas_num_chksum_bytes_in;
	atomic64_t pas_num_chksums;

	atomic64_t pas_total_latency;
	atomic64_t pas_total_hw_latency;
};

#define PAS_INC_NUM_REQUESTS(pcr)					\
	atomic64_inc_return(&pcr->api_stats.pas_num_requests)
#define PAS_INC_NUM_SERVICES(pcr)					\
	atomic64_inc_return(&pcr->api_stats.pas_num_services)
#define PAS_INC_NUM_CHAINS(pcr)						\
	atomic64_inc_return(&pcr->api_stats.pas_num_chains)
#define PAS_INC_NUM_BATCHES(pcr)					\
	atomic64_inc_return(&pcr->api_stats.pas_num_batches)

#define PAS_INC_NUM_REQUEST_FAILURES(pcr)				\
	atomic64_inc_return(&pcr->api_stats.pas_num_request_failures)
#define PAS_INC_NUM_SERVICE_FAILURES(pcr)				\
	atomic64_inc_return(&pcr->api_stats.pas_num_service_failures)
#define PAS_INC_NUM_CHAIN_FAILURES(pcr)					\
	atomic64_inc_return(&pcr->api_stats.pas_num_chain_failures)
#define PAS_INC_NUM_BATCH_FAILURES(pcr)					\
	atomic64_inc_return(&pcr->api_stats.pas_num_batch_failures)

#define PAS_INC_NUM_ENC_REQUESTS(pcr)					\
	atomic64_inc_return(&pcr->api_stats.pas_num_enc_requests)
#define PAS_INC_NUM_DEC_REQUESTS(pcr)					\
	atomic64_inc_return(&pcr->api_stats.pas_num_dec_requests)
#define PAS_INC_NUM_CP_REQUESTS(pcr)					\
	atomic64_inc_return(&pcr->api_stats.pas_num_cp_requests)
#define PAS_INC_NUM_DC_REQUESTS(pcr)					\
	atomic64_inc_return(&pcr->api_stats.pas_num_dc_requests)
#define PAS_INC_NUM_HASH_REQUESTS(pcr)					\
	atomic64_inc_return(&pcr->api_stats.pas_num_hash_requests)
#define PAS_INC_NUM_CHKSUM_REQUESTS(pcr)				\
	atomic64_inc_return(&pcr->api_stats.pas_num_chksum_requests)

#define PAS_INC_NUM_ENC_REQUEST_FAILURES(pcr)				\
	atomic64_inc_return(&pcr->api_stats.pas_num_enc_request_failures)
#define PAS_INC_NUM_DEC_REQUEST_FAILURES(pcr)				\
	atomic64_inc_return(&pcr->api_stats.pas_num_dec_request_failures)
#define PAS_INC_NUM_CP_REQUEST_FAILURES(pcr)				\
	atomic64_inc_return(&pcr->api_stats.pas_num_cp_request_failures)
#define PAS_INC_NUM_DC_REQUEST_FAILURES(pcr)				\
	atomic64_inc_return(&pcr->api_stats.pas_num_dc_request_failures)
#define PAS_INC_NUM_HASH_REQUEST_FAILURES(pcr)				\
	atomic64_inc_return(&pcr->api_stats.pas_num_hash_request_failures)
#define PAS_INC_NUM_CHKSUM_REQUEST_FAILURES(pcr)			\
	atomic64_inc_return(&pcr->api_stats.pas_num_chksum_request_failures)

#define PAS_INC_NUM_ENC_BYTES(pcr, bytes)				\
	atomic64_add(bytes, &pcr->api_stats.pas_num_enc_bytes)
#define PAS_INC_NUM_DEC_BYTES(pcr, bytes)				\
	atomic64_add(bytes, &pcr->api_stats.pas_num_dec_bytes)

#define PAS_INC_NUM_CP_BYTES_IN(pcr, bytes)				\
	atomic64_add(bytes, &pcr->api_stats.pas_num_cp_bytes_in)
#define PAS_INC_NUM_CP_BYTES_OUT(pcr, bytes)				\
	atomic64_add(bytes, &pcr->api_stats.pas_num_cp_bytes_out)

#define PAS_INC_NUM_DC_BYTES_IN(pcr, bytes)				\
	atomic64_add(bytes, &pcr->api_stats.pas_num_dc_bytes_in)
#define PAS_INC_NUM_DC_BYTES_OUT(pcr, bytes)				\
	atomic64_add(bytes, &pcr->api_stats.pas_num_dc_bytes_out)

/* TODO-stats: Separate count for different algo?? */
#define PAS_INC_NUM_HASH_BYTES_IN(pcr, bytes)				\
	atomic64_add(bytes, &pcr->api_stats.pas_num_hash_bytes_in)
#define PAS_INC_NUM_HASHES(pcr, count)					\
	atomic64_add(count, &pcr->api_stats.pas_num_hashes)

#define PAS_INC_NUM_CHKSUM_BYTES_IN(pcr, bytes)				\
	atomic64_add(bytes, &pcr->api_stats.pas_num_chksum_bytes_in)
#define PAS_INC_NUM_CHKSUMS(pcr, count)					\
	atomic64_add(count, &pcr->api_stats.pas_num_chksums)

/* for measuring the time spent in both software and hardware */
#define PAS_DECL_PERF()		ktime_t s
#define PAS_START_PERF()	s = ktime_get()
#define PAS_END_PERF(pcr)						\
	atomic64_add(ktime_us_delta(ktime_get(), s),\
			&pcr->api_stats.pas_total_latency)

/* for measuring the time spent just in hardware */
#define PAS_DECL_HW_PERF()	ktime_t h
#define PAS_START_HW_PERF()	h = ktime_get()
#define PAS_END_HW_PERF(pcr)						\
	atomic64_add(ktime_us_delta(ktime_get(), h),			\
			&pcr->api_stats.pas_total_hw_latency)

#define PAS_SHOW_STATS(pcr)	pas_show_stats(&pcr->api_stats);

void pas_init(struct pnso_api_stats *stats);

void pas_reset_stats(struct pnso_api_stats *stats);

void pas_show_stats(struct pnso_api_stats *stats);

#ifdef __cplusplus
}
#endif

#endif /* __PNSO_STATS_H__ */
