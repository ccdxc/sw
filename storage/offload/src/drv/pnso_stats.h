/*
 * {C} Copyright 2018 Pensando Systems Inc.
 * All rights reserved.
 *
 */
#ifndef __PNSO_STATS_H__
#define __PNSO_STATS_H__

#include <linux/atomic.h>
#include "pnso_api.h"
#include "osal_time.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Non-atomic versions */
#ifndef __KERNEL__
#define PNSO_DECLARE_STAT(s) int64_t s;
#define PNSO_STAT_INC(pcr, s) pcr->api_stats.s++
#define PNSO_STAT_ADD(pcr, s, count) pcr->api_stats.s += count
#define PNSO_STAT_READ(stats, s) (stats)->s
#define PNSO_STAT_READ_BY_ID(stats, id) ((int64_t*)(stats))[id]
#else
/* Atomic versions */
#ifdef __FreeBSD__
#include <machine/atomic.h>
#define PNSO_DECLARE_STAT(s) int64_t s;
#define PNSO_STAT_INC(pcr, s) \
	atomic_add_64((uint64_t*)&pcr->api_stats.s, 1)
#define PNSO_STAT_ADD(pcr, s, count) \
	atomic_add_64((uint64_t*)&pcr->api_stats.s, count)
#define PNSO_STAT_READ(stats, s) (uint64_t) atomic_load_64((uint64_t*)&(stats)->s)
#define PNSO_STAT_READ_BY_ID(stats, id) \
	(uint64_t) atomic_load_64(&((uint64_t*)(stats))[id])
#else
#define PNSO_DECLARE_STAT(s) atomic64_t s;
#define PNSO_STAT_INC(pcr, s) \
	atomic64_inc_return(&pcr->api_stats.s)
#define PNSO_STAT_ADD(pcr, s, count) \
	atomic64_add(count, &pcr->api_stats.s)
#define PNSO_STAT_READ(stats, s) (uint64_t)atomic64_read(&(stats)->s)
#define PNSO_STAT_READ_BY_ID(stats, id) \
	(uint64_t)atomic64_read(&((atomic64_t*)(stats))[id])
#endif
#endif

#define PNSO_ENUM_STAT(s) pas_ ## s,
#define PNSO_STAT_ID(s) pas_ ## s

#define PNSO_LIST_STATS \
	PNSO_LIST_STAT(num_requests) \
	PNSO_LIST_STAT(num_services) \
	PNSO_LIST_STAT(num_chains) \
	PNSO_LIST_STAT(num_batches) \
\
	PNSO_LIST_STAT(num_request_failures) \
	PNSO_LIST_STAT(num_service_failures) \
	PNSO_LIST_STAT(num_chain_failures) \
	PNSO_LIST_STAT(num_batch_failures) \
\
	PNSO_LIST_STAT(num_enc_requests) \
	PNSO_LIST_STAT(num_dec_requests) \
	PNSO_LIST_STAT(num_cp_requests) \
	PNSO_LIST_STAT(num_dc_requests) \
	PNSO_LIST_STAT(num_hash_requests) \
	PNSO_LIST_STAT(num_chksum_requests) \
\
	PNSO_LIST_STAT(num_enc_request_failures) \
	PNSO_LIST_STAT(num_dec_request_failures) \
	PNSO_LIST_STAT(num_cp_request_failures) \
	PNSO_LIST_STAT(num_dc_request_failures) \
	PNSO_LIST_STAT(num_hash_request_failures) \
	PNSO_LIST_STAT(num_chksum_request_failures) \
\
	PNSO_LIST_STAT(num_enc_bytes) \
	PNSO_LIST_STAT(num_dec_bytes) \
\
	PNSO_LIST_STAT(num_cp_bytes_in) \
	PNSO_LIST_STAT(num_cp_bytes_out) \
\
	PNSO_LIST_STAT(num_dc_bytes_in) \
	PNSO_LIST_STAT(num_dc_bytes_out) \
\
	PNSO_LIST_STAT(num_hash_bytes_in) \
	PNSO_LIST_STAT(num_hashes) \
\
	PNSO_LIST_STAT(num_chksum_bytes_in) \
	PNSO_LIST_STAT(num_chksums) \
\
	PNSO_LIST_STAT(num_out_of_rmem_bufs) \
	PNSO_LIST_STAT(num_out_of_rmem_status) \
	PNSO_LIST_STAT(num_pdma_exceed_constraints) \
\
	PNSO_LIST_STAT(total_latency) \
	PNSO_LIST_STAT(total_hw_latency)


#undef PNSO_LIST_STAT
#define PNSO_LIST_STAT PNSO_ENUM_STAT
#define PNSO_ENUM_STATS PNSO_LIST_STATS
enum {
	PNSO_ENUM_STATS
	pas_max
};

#undef PNSO_LIST_STAT
#define PNSO_LIST_STAT PNSO_DECLARE_STAT
#define PNSO_DECLARE_STATS PNSO_LIST_STATS

struct pnso_api_stats {
	PNSO_DECLARE_STATS
};

#ifdef PNSO_INIT_STAT
#undef PNSO_LIST_STAT
#define PNSO_LIST_STAT PNSO_INIT_STAT
#define PNSO_INIT_STATS PNSO_LIST_STATS
#endif

#define PAS_INC_NUM_REQUESTS(pcr)					\
	PNSO_STAT_INC(pcr, num_requests)
#define PAS_INC_NUM_BATCH_REQUESTS(pcr, total_requests)			\
	PNSO_STAT_ADD(pcr, num_requests, total_requests)
#define PAS_INC_NUM_SERVICES(pcr)					\
	PNSO_STAT_INC(pcr, num_services)
#define PAS_INC_NUM_CHAINS(pcr)						\
	PNSO_STAT_INC(pcr, num_chains)
#define PAS_INC_NUM_BATCHES(pcr)					\
	PNSO_STAT_INC(pcr, num_batches)

#define PAS_INC_NUM_REQUEST_FAILURES(pcr)				\
	PNSO_STAT_INC(pcr, num_request_failures)
#define PAS_INC_NUM_BATCH_REQUEST_FAILURES(pcr, total_requests)		\
	PNSO_STAT_ADD(pcr, num_request_failures, total_requests)
#define PAS_INC_NUM_SERVICE_FAILURES(pcr)				\
	PNSO_STAT_INC(pcr, num_service_failures)
#define PAS_INC_NUM_CHAIN_FAILURES(pcr)					\
	PNSO_STAT_INC(pcr, num_chain_failures)
#define PAS_INC_NUM_BATCH_FAILURES(pcr)					\
	PNSO_STAT_INC(pcr, num_batch_failures)

#define PAS_INC_NUM_ENC_REQUESTS(pcr)					\
	PNSO_STAT_INC(pcr, num_enc_requests)
#define PAS_INC_NUM_DEC_REQUESTS(pcr)					\
	PNSO_STAT_INC(pcr, num_dec_requests)
#define PAS_INC_NUM_CP_REQUESTS(pcr)					\
	PNSO_STAT_INC(pcr, num_cp_requests)
#define PAS_INC_NUM_DC_REQUESTS(pcr)					\
	PNSO_STAT_INC(pcr, num_dc_requests)
#define PAS_INC_NUM_HASH_REQUESTS(pcr)					\
	PNSO_STAT_INC(pcr, num_hash_requests)
#define PAS_INC_NUM_CHKSUM_REQUESTS(pcr)				\
	PNSO_STAT_INC(pcr, num_chksum_requests)

#define PAS_INC_NUM_ENC_REQUEST_FAILURES(pcr)				\
	PNSO_STAT_INC(pcr, num_enc_request_failures)
#define PAS_INC_NUM_DEC_REQUEST_FAILURES(pcr)				\
	PNSO_STAT_INC(pcr, num_dec_request_failures)
#define PAS_INC_NUM_CP_REQUEST_FAILURES(pcr)				\
	PNSO_STAT_INC(pcr, num_cp_request_failures)
#define PAS_INC_NUM_DC_REQUEST_FAILURES(pcr)				\
	PNSO_STAT_INC(pcr, num_dc_request_failures)
#define PAS_INC_NUM_HASH_REQUEST_FAILURES(pcr)				\
	PNSO_STAT_INC(pcr, num_hash_request_failures)
#define PAS_INC_NUM_CHKSUM_REQUEST_FAILURES(pcr)			\
	PNSO_STAT_INC(pcr, num_chksum_request_failures)
#define PAS_INC_NUM_OUT_OF_RMEM_BUFS(pcr)				\
	PNSO_STAT_INC(pcr, num_out_of_rmem_bufs)
#define PAS_INC_NUM_OUT_OF_RMEM_STATUS(pcr)				\
	PNSO_STAT_INC(pcr, num_out_of_rmem_status)
#define PAS_INC_NUM_PDMA_EXCEED_CONSTRAINTS(pcr)			\
	PNSO_STAT_INC(pcr, num_pdma_exceed_constraints)

#define PAS_INC_NUM_ENC_BYTES(pcr, bytes)				\
	PNSO_STAT_ADD(pcr, num_enc_bytes, bytes)
#define PAS_INC_NUM_DEC_BYTES(pcr, bytes)				\
	PNSO_STAT_ADD(pcr, num_dec_bytes, bytes)

#define PAS_INC_NUM_CP_BYTES_IN(pcr, bytes)				\
	PNSO_STAT_ADD(pcr, num_cp_bytes_in, bytes)
#define PAS_INC_NUM_CP_BYTES_OUT(pcr, bytes)				\
	PNSO_STAT_ADD(pcr, num_cp_bytes_out, bytes)

#define PAS_INC_NUM_DC_BYTES_IN(pcr, bytes)				\
	PNSO_STAT_ADD(pcr, num_dc_bytes_in, bytes)
#define PAS_INC_NUM_DC_BYTES_OUT(pcr, bytes)				\
	PNSO_STAT_ADD(pcr, num_dc_bytes_out, bytes)

#define PAS_INC_NUM_HASH_BYTES_IN(pcr, bytes)				\
	PNSO_STAT_ADD(pcr, num_hash_bytes_in, bytes)
#define PAS_INC_NUM_HASHES(pcr, count)					\
	PNSO_STAT_ADD(pcr, num_hashes, count)

#define PAS_INC_NUM_CHKSUM_BYTES_IN(pcr, bytes)				\
	PNSO_STAT_ADD(pcr, num_chksum_bytes_in, bytes)
#define PAS_INC_NUM_CHKSUMS(pcr, count)					\
	PNSO_STAT_ADD(pcr, num_chksums, count)

/* for measuring the time spent in both software and hardware */
#ifdef NDEBUG
#define PAS_DECL_SW_PERF()
#define PAS_START_SW_PERF(x)
#define PAS_END_SW_PERF(pcr)
#define PAS_SET_SW_PERF(x)
#define PAS_DECL_HW_PERF()
#define PAS_START_HW_PERF(x)
#define PAS_END_HW_PERF(pcr)
#define PAS_SET_HW_PERF(x)
#else
#define PAS_DECL_SW_PERF()	uint64_t s = 0
#define PAS_START_SW_PERF(x)	x = s = osal_get_clock_nsec()
#define PAS_END_SW_PERF(pcr)						\
	PNSO_STAT_ADD(pcr, total_latency, (osal_get_clock_nsec() - s))
#define PAS_SET_SW_PERF(x)	s = x

/* for measuring the time spent just in hardware */
#define PAS_DECL_HW_PERF()	uint64_t h = 0
#define PAS_START_HW_PERF(x)	x = h = osal_get_clock_nsec()
#define PAS_END_HW_PERF(pcr)						\
	PNSO_STAT_ADD(pcr, total_hw_latency, (osal_get_clock_nsec() - h))
#define PAS_SET_HW_PERF(x)	h = x
#endif /* NDEBUG */

#define PAS_SHOW_STATS(pcr)	pas_show_stats(&pcr->api_stats);

void pas_init(struct pnso_api_stats *stats);

void pas_reset_stats(struct pnso_api_stats *stats);

void pas_show_stats(struct pnso_api_stats *stats);

#ifdef __cplusplus
}
#endif

#endif /* __PNSO_STATS_H__ */
