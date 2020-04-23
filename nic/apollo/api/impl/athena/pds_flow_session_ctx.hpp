//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This file defines flow session context API
///
//----------------------------------------------------------------------------

#ifndef __PDS_FLOW_SESSION_CTX_HPP__
#define __PDS_FLOW_SESSION_CTX_HPP__

#include "nic/sdk/include/sdk/base.hpp"
#include "nic/apollo/api/include/athena/pds_base.h"
#include "nic/sdk/include/sdk/table.hpp"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Two modes of session context locking:
 * Internal: flow cache API internally uses lock when making a cache 
 *           entry update.
 * External: Caller engages lock explicitly using pds_flow_session_ctx_lock()/
 *           pds_flow_session_ctx_unlock() which facilitates locking across
 *           multiple operations (e.g., across a pair of session_info and
 *           cache updates).
 */
typedef enum {
    SESSION_CTX_LOCK_INTERNAL,
    SESSION_CTX_LOCK_EXTERNAL,
} pds_flow_session_ctx_lock_mode_t;

pds_ret_t pds_flow_session_ctx_init(pds_flow_session_ctx_lock_mode_t lock_mode);
void pds_flow_session_ctx_fini(void);
void pds_flow_session_ctx_lock(uint32_t session_id);
void pds_flow_session_ctx_unlock(uint32_t session_id);

pds_ret_t pds_flow_session_ctx_set(uint32_t session_id,
                                   uint32_t cache_pindex,
                                   uint32_t cache_sindex,
                                   bool primary);
void pds_flow_session_ctx_clr(uint32_t session_id);
pds_ret_t pds_flow_session_ctx_get(uint32_t session_id,
                                   uint32_t *ret_cache_id,
                                   bool *ret_primary);
pds_ret_t pds_flow_session_ctx_get_clr(uint32_t session_id,
                                       uint32_t *ret_cache_id,
                                       bool *ret_primary);
void pds_flow_session_ctx_move(uint32_t session_id,
                               uint32_t cache_id, 
                               bool primary, 
                               bool move_complete);
#ifdef __cplusplus
}
#endif

#endif // __PDS_FLOW_SESSION_CTX_HPP__


