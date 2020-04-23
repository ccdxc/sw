//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// athena flow session context implementation
///
//----------------------------------------------------------------------------

#include <algorithm>
#include <inttypes.h>
#include "pds_flow_session_ctx.hpp"
#include "nic/apollo/core/trace.hpp"
#include "nic/apollo/api/pds_state.hpp"
#include "platform/src/lib/nicmgr/include/pd_client.hpp"
#include "nic/sdk/lib/p4/p4_api.hpp"
#include "nic/sdk/lib/pal/pal.hpp"
#include "gen/p4gen/athena/include/p4pd.h"

/*
 * Session context holds a backpointer from session_info to its corresponding
 * flow cache entry (a 1-to-1 mapping).
 *
 * There are several reasons the backpointer is stored separately from the
 * session_info itself. The primary reason is performance. The backpointer
 * creates a circular dependency between session_info and flow cache, i.e.,
 * flow cache entry points to session_info and session_info points to flow
 * cache. Flow cache and session_info creation would have necessitated
 * 3 operations: 1) create session_info, 2) create flow cache, 3) update
 * session_info. Step 3 is expensive as it is a 64-byte write with byte 
 * swizzling. In total, that would double the time for flow creation and also
 * double flow deletion time.
 * 
 * Moving the context outside of session_info results in shorter write time
 * (4 bytes) and, since SW is the only component reading/writing the context,
 * no byte swizzling is necessary. Futhermore, the context can be in process
 * memory instead of HBM, provided the following condition holds:
 * - If/when stateful process restart became a reality and the design was
 *   to have all flows emptied out upon restart, then process memory usage
 *   would continue to be the most efficient mode. Otherwise, session context
 *   would have to be stored in HBM.
 */
//#define FLOW_SESSION_CTX_HBM_HANDLE     "flow_session_ctx"

/*
 * HBM mode, if used, is only considered for HW to simplify implementation.
 */
#ifdef __aarch64__
#ifdef FLOW_SESSION_CTX_HBM_HANDLE
#define FLOW_SESSION_CTX_MODE_HBM
#endif
#endif

/*
 * Session context entry
 * Fields arrangement intended to minimize memory consumption
 */
#define CACHE_ID_LO_BITS                16
#define CACHE_ID_LO_MASK                ((1 << CACHE_ID_LO_BITS) - 1)
#define CACHE_ID_HI_BITS                6
#define CACHE_ID_MAX                    (1 << (CACHE_ID_LO_BITS + CACHE_ID_HI_BITS))

/*
 * Two different arrangements depending on whether or not context is in HBM.
 */
#ifdef FLOW_SESSION_CTX_MODE_HBM

typedef struct {
    uint16_t            cache_id_lo;
    uint8_t             cache_id_hi;
    uint8_t             primary     : 1,
                        valid       : 1,
                        rsvd        : 6;
} __attribute__((packed)) ctx_entry_t;

typedef struct {
    volatile uint8_t    lock;
} __attribute__((packed)) ctx_lock_t;

#else

/*
 * In process memory mode, context and lock are stored together
 */
typedef struct {
    uint16_t            cache_id_lo;
    uint8_t             cache_id_hi : CACHE_ID_HI_BITS,
                        primary     : 1,
                        valid       : 1;
    volatile uint8_t    lock;
} __attribute__((packed)) ctx_entry_t;

typedef ctx_entry_t     ctx_lock_t;

#endif

static inline void
ctx_entry_cache_id_set(ctx_entry_t *ctx,
                       uint32_t cache_id)
{
    ctx->cache_id_lo = cache_id & CACHE_ID_LO_MASK;
    ctx->cache_id_hi = cache_id >> CACHE_ID_LO_BITS;
}

static inline uint32_t
ctx_entry_cache_id_get(ctx_entry_t *ctx)
{
    return ((uint32_t)ctx->cache_id_hi << CACHE_ID_LO_BITS) |
            (uint32_t)ctx->cache_id_lo;
}

/*
 * Session context anchor
 */
class session_ctx_t {

public:
    session_ctx_t() :
        platform_type(platform_type_t::PLATFORM_TYPE_NONE),
        ctx_table(nullptr),
        ctx_vaddr(nullptr),
        lock_mode(SESSION_CTX_LOCK_INTERNAL)
    {
        session_prop = {0};
        cache_prop = {0};
    }

    ~session_ctx_t()
    {
        fini();
    }

    pds_ret_t init(void);
    void fini(void);

    void lock_mode_set(pds_flow_session_ctx_lock_mode_t mode)
    {
        lock_mode = mode;
        SDK_ASSERT(is_lock_mode_internal() || is_lock_mode_external());
    }

    inline bool is_lock_mode_internal(void)
    {
        return lock_mode == SESSION_CTX_LOCK_INTERNAL;
    }

    inline bool is_lock_mode_external(void)
    {
        return lock_mode == SESSION_CTX_LOCK_EXTERNAL;
    }

    inline bool cache_id_validate(uint32_t cache_id)
    {
        return cache_id < cache_prop.tabledepth;
    }

#ifdef FLOW_SESSION_CTX_MODE_HBM

    inline ctx_entry_t *ctx_entry_get(uint32_t session_id)
    {
        return session_id < session_prop.tabledepth ?
               &ctx_vaddr[session_id] : nullptr;
    }
    inline ctx_lock_t *ctx_lock_get(uint32_t session_id)
    {
        return session_id < session_prop.tabledepth ?
               &ctx_table[session_id] : nullptr;
    }

    inline ctx_entry_t *ctx_entry_lock(uint32_t session_id,
                                        bool force_lock = false)
    {
        if (force_lock || is_lock_mode_internal()) {
            ctx_lock_t *lock = ctx_lock_get(session_id);
            if (lock) {
                while (__atomic_exchange_n(&lock->lock, 1, __ATOMIC_ACQUIRE));
            }
        }
        return ctx_entry_get(session_id);
    }

    inline void ctx_entry_unlock(uint32_t session_id,
                                 bool force_unlock = false)
    {
        if (force_unlock || is_lock_mode_internal()) {
            ctx_lock_t *lock = ctx_lock_get(session_id);
            if (lock) {
                __atomic_store_n(&lock->lock, 0, __ATOMIC_RELEASE);
            }
        }
    }

#else
    inline ctx_entry_t *ctx_entry_get(uint32_t session_id)
    {
        return session_id < session_prop.tabledepth ?
               &ctx_table[session_id] : nullptr;
    }

    inline ctx_entry_t * ctx_entry_lock(uint32_t session_id,
                                        bool force_lock = false)
    {
        ctx_entry_t *ctx = ctx_entry_get(session_id);
        if (ctx && (force_lock || is_lock_mode_internal())) {
            while (__atomic_exchange_n(&ctx->lock, 1, __ATOMIC_ACQUIRE));
        }
        return ctx;
    }

    inline void ctx_entry_unlock(uint32_t session_id,
                                 bool force_unlock = false)
    {
        if (force_unlock || is_lock_mode_internal()) {
            ctx_entry_t *ctx = ctx_entry_get(session_id);
            if (ctx) __atomic_store_n(&ctx->lock, 0, __ATOMIC_RELEASE);
        }
    }
#endif

private:
    platform_type_t             platform_type;
    p4pd_table_properties_t     session_prop;
    p4pd_table_properties_t     cache_prop;
    ctx_lock_t                  *ctx_table;
    ctx_entry_t                 *ctx_vaddr;
    pds_flow_session_ctx_lock_mode_t lock_mode;
};

static session_ctx_t            session_ctx;

pds_ret_t
session_ctx_t::init(void)
{
    p4pd_error_t    p4pd_error;

    platform_type = api::g_pds_state.platform_type();

    p4pd_error = p4pd_table_properties_get(P4TBL_ID_FLOW,
                                           &cache_prop);
    if ((p4pd_error != P4PD_SUCCESS) || !cache_prop.tabledepth) {
        PDS_TRACE_ERR("failed flow cache properties: count %u "
                      "error %d", cache_prop.tabledepth, p4pd_error);
        return PDS_RET_ERR;
    }
    if (cache_prop.tabledepth > CACHE_ID_MAX) {
        PDS_TRACE_ERR("flow cache table depth %u greater than expected (%d)",
                      cache_prop.tabledepth, CACHE_ID_MAX);
        return PDS_RET_ERR;
    }

    /*
     * Mock mode has no support for P4TBL_ID_SESSION_INFO
     */
    if (std::getenv("ASIC_MOCK_MODE")) {
        session_prop.tabledepth = cache_prop.tabledepth;
    } else {
        p4pd_error = p4pd_table_properties_get(P4TBL_ID_SESSION_INFO,
                                               &session_prop);
        if ((p4pd_error != P4PD_SUCCESS) || !session_prop.tabledepth) {
            PDS_TRACE_ERR("failed session info properties: count %u "
                          "error %d", session_prop.tabledepth, p4pd_error);
            return PDS_RET_ERR;
        }
    }
    //PDS_TRACE_DEBUG("Session context init session depth %u cache depth %u",
    //                session_prop.tabledepth, cache_prop.tabledepth);
#ifdef FLOW_SESSION_CTX_MODE_HBM
    uint64_t hbm_paddr = api::g_pds_state.mempartition()->start_addr(
                                          FLOW_SESSION_CTX_HBM_HANDLE);
    uint32_t hbm_bytes = api::g_pds_state.mempartition()->size(
                                          FLOW_SESSION_CTX_HBM_HANDLE);
    uint32_t table_bytes = sizeof(ctx_entry_t) * session_prop.tabledepth;
    if ((hbm_paddr == INVALID_MEM_ADDRESS) || (hbm_bytes < table_bytes)) {
        PDS_TRACE_ERR("failed to obtain enough HBM for %s",
                      FLOW_SESSION_CTX_HBM_HANDLE);
        return PDS_RET_NO_RESOURCE;
    }
    if (!platform_is_hw(platform_type)) {
        PDS_TRACE_ERR("Platform must be HW to use HBM");
        return PDS_RET_ERR;
    }
    ctx_vaddr = (ctx_entry_t *)sdk::lib::pal_mem_map(hbm_paddr, table_bytes);
    if (!ctx_vaddr) {
        PDS_TRACE_ERR("failed to memory map addr 0x%" PRIx64 " size %u",
                      hbm_paddr, table_bytes);
        return PDS_RET_ERR;
    }
    memset((void *)ctx_vaddr, 0, table_bytes);
#endif
    ctx_table = (ctx_lock_t *)SDK_CALLOC(SDK_MEM_ALLOC_FLOW_SESSION_CTX,
                                         session_prop.tabledepth *
                                         sizeof(ctx_lock_t));
    if (!ctx_table) {
        PDS_TRACE_ERR("fail to allocate ctx_table");
        return PDS_RET_OOM;
    }
    return PDS_RET_OK;
}

void
session_ctx_t::fini(void)
{
#ifdef FLOW_SESSION_CTX_MODE_HBM
    if (ctx_vaddr) {
        sdk::lib::pal_mem_unmap((void *)ctx_vaddr);
        ctx_vaddr = nullptr;
    }
#endif
    if (ctx_table) {
        SDK_FREE(SDK_MEM_ALLOC_FLOW_SESSION_CTX, ctx_table);
        ctx_table = nullptr;
    }
}

#ifdef __cplusplus
extern "C" {
#endif

pds_ret_t
pds_flow_session_ctx_init(pds_flow_session_ctx_lock_mode_t lock_mode)
{
    pds_ret_t ret = session_ctx.init();
    if (ret == PDS_RET_OK) {
        session_ctx.lock_mode_set(lock_mode);
    }
    return ret;
}

void
pds_flow_session_ctx_fini(void)
{
    session_ctx.fini();
}

pds_ret_t
pds_flow_session_ctx_set(uint32_t session_id,
                         uint32_t cache_pindex,
                         uint32_t cache_sindex,
                         bool primary)
{
    uint32_t cache_id = primary ? cache_pindex : cache_sindex;
    if (session_ctx.cache_id_validate(cache_id)) {
        ctx_entry_t *ctx = session_ctx.ctx_entry_lock(session_id);
        if (ctx) {
            ctx_entry_cache_id_set(ctx, cache_id);
            ctx->primary = primary;
            ctx->valid = true;
            session_ctx.ctx_entry_unlock(session_id);
            return PDS_RET_OK;
        }
    }
    PDS_TRACE_ERR("failed session_id %u or cache_id %u", session_id, cache_id);
    return PDS_RET_INVALID_ARG;
}

void
pds_flow_session_ctx_clr(uint32_t session_id)
{
    ctx_entry_t *ctx = session_ctx.ctx_entry_lock(session_id);
    if (ctx) {
        ctx->valid = false;
        ctx->primary = false;
        ctx_entry_cache_id_set(ctx, 0);
        session_ctx.ctx_entry_unlock(session_id);
    }
}

pds_ret_t
pds_flow_session_ctx_get(uint32_t session_id,
                         uint32_t *ret_cache_id,
                         bool *ret_primary)
{
    ctx_entry_t *ctx = session_ctx.ctx_entry_lock(session_id);
    if (ctx) {
        if (ctx->valid) {
            *ret_cache_id = ctx_entry_cache_id_get(ctx);
            *ret_primary = ctx->primary;
            session_ctx.ctx_entry_unlock(session_id);
            return PDS_RET_OK;
        }
        session_ctx.ctx_entry_unlock(session_id);
    }
    return PDS_RET_ENTRY_NOT_FOUND;
}

pds_ret_t
pds_flow_session_ctx_get_clr(uint32_t session_id,
                             uint32_t *ret_cache_id,
                             bool *ret_primary)
{
    ctx_entry_t *ctx = session_ctx.ctx_entry_lock(session_id);
    if (ctx) {
        if (ctx->valid) {
            *ret_cache_id = ctx_entry_cache_id_get(ctx);
            *ret_primary = ctx->primary;

            ctx->valid = false;
            ctx->primary = false;
            ctx_entry_cache_id_set(ctx, 0);
            session_ctx.ctx_entry_unlock(session_id);
            return PDS_RET_OK;
        }
        session_ctx.ctx_entry_unlock(session_id);
    }
    return PDS_RET_ENTRY_NOT_FOUND;
}

void
pds_flow_session_ctx_move(uint32_t session_id,
                          uint32_t cache_id, 
                          bool primary, 
                          bool move_complete)
{
    if (session_ctx.cache_id_validate(cache_id)) {

        /*
         * 2-step move (see Bucket::defragment_())
         */
        if (move_complete) {
            session_ctx.ctx_entry_unlock(session_id);
        } else {
            ctx_entry_t *ctx = session_ctx.ctx_entry_lock(session_id);
            if (ctx) {
                ctx_entry_cache_id_set(ctx, cache_id);
                ctx->primary = primary;
                ctx->valid = true;
            }
        }
    }
}

/*
 * These 2 API should only be used if the user wants to have full control
 * of the lock/unlock operations.
 */
void
pds_flow_session_ctx_lock(uint32_t session_id)
{
    SDK_ASSERT(session_ctx.is_lock_mode_external());
    session_ctx.ctx_entry_lock(session_id, true);
}

void
pds_flow_session_ctx_unlock(uint32_t session_id)
{
    SDK_ASSERT(session_ctx.is_lock_mode_external());
    session_ctx.ctx_entry_unlock(session_id, true);
}

#ifdef __cplusplus
}
#endif

