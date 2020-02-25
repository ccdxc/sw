#include "nic/fte/acl/acl.hpp"
#include "nic/fte/acl/acl_ctx.hpp"
#include "nic/sdk/include/sdk/lock.hpp"

namespace acl {

using sdk::lib::ht;
using sdk::lib::ht_ctxt_t;

//------------------------------------------------------------------------
// lock to protect ctx list
//------------------------------------------------------------------------
static sdk_spinlock_t g_ctx_lock;
static int g_ctx_lock_init = SDK_SPINLOCK_INIT(&g_ctx_lock, PTHREAD_PROCESS_PRIVATE);

static void *acl_ctx_get_key_func_(void *entry)
{
    return (void *)((acl_ctx_t *)entry)->name();
}

static uint32_t acl_ctx_key_size_(void) {
    return ACL_NAMESIZE;
}

static ht *g_ctx_ht = ht::factory(16, acl_ctx_get_key_func_,
                                  acl_ctx_key_size_(),
                                  true /* thread_safe */,
                                  true /* key is a string */);

//------------------------------------------------------------------------
// Creates a new ACL context.
//------------------------------------------------------------------------
const acl_ctx_t *
acl_create(const char *name, const acl_config_t *cfg)
{
    return acl_ctx_t::create(name, cfg);
}

//------------------------------------------------------------------------
// Lookup an existing ctx by name
// Should be released later using acl_deref()
//------------------------------------------------------------------------
const acl_ctx_t *
acl_get(const char *name)
{
    const acl_ctx_t *ctx = NULL;

    SDK_ASSERT_RETURN((SDK_SPINLOCK_LOCK(&g_ctx_lock) == 0), NULL);
    ctx = (const acl_ctx_t *)g_ctx_ht->lookup((void *)name);
    if (ctx) {
        ctx = ctx->clone();
    }
    SDK_ASSERT_RETURN((SDK_SPINLOCK_UNLOCK(&g_ctx_lock) == 0), NULL);

    return ctx;
}

inline void
print_ref_count(const acl_ctx_t *ctx)
{
    HAL_TRACE_VERBOSE("ctx_name: {} ref_count: {}", ctx->name(), ctx->print_ref_count());
}

//------------------------------------------------------------------------
// Release the ctx reference
//------------------------------------------------------------------------
void
acl_deref(const acl_ctx_t *ctx)
{
#if ACL_DEBUG
    HAL_TRACE_DEBUG("deref for point {:#x}", (uint64_t) ctx);
#endif
    print_ref_count(ctx);
    ctx->deref();
}

//------------------------------------------------------------------------
// Remove the ctx from the global list and release the reference.
//------------------------------------------------------------------------
void
acl_delete(const acl_ctx_t *ctx)
{
    print_ref_count(ctx);
    // remove from global list and deref
    SDK_ASSERT(SDK_SPINLOCK_LOCK(&g_ctx_lock) == 0);
    const acl_ctx_t *gctx = (const acl_ctx_t *)g_ctx_ht->remove((void *)ctx->name());
    if (gctx) {
        gctx->deref();
    }
    SDK_ASSERT(SDK_SPINLOCK_UNLOCK(&g_ctx_lock) == 0);


    // deref the user pointer
    ctx->deref();
}

//------------------------------------------------------------------------
// Commits the updated ctx into global list
//------------------------------------------------------------------------
hal_ret_t
acl_commit(const acl_ctx_t *ctx, acl_update_cb_t cb)
{
    const acl_ctx_t *old;

    // Update the global list
    SDK_ASSERT_RETURN((SDK_SPINLOCK_LOCK(&g_ctx_lock) == 0), HAL_RET_INVALID_OP);
    ctx = ctx->clone();

    old = (const acl_ctx_t *)g_ctx_ht->remove((void *)ctx->name());
    if (old) {
        print_ref_count(old);
        old->deref();
    }

    g_ctx_ht->insert((void *)ctx, ctx->ht_ctxt());
    SDK_ASSERT_RETURN((SDK_SPINLOCK_UNLOCK(&g_ctx_lock) == 0), HAL_RET_INVALID_OP);

    return HAL_RET_OK;
}

//------------------------------------------------------------------------
// Add a rule
//------------------------------------------------------------------------
hal_ret_t
acl_add_rule(const acl_ctx_t **ctxp, const acl_rule_t *rule)
{
    return acl_ctx_t::add_rule(ctxp, rule);
}

//------------------------------------------------------------------------
//------------------------------------------------------------------------
hal_ret_t
acl_del_rule(const acl_ctx_t **ctxp, const acl_rule_t *rule)
{
    return acl_ctx_t::del_rule(ctxp, rule);
}

//------------------------------------------------------------------------
//------------------------------------------------------------------------
hal_ret_t
acl_classify(const acl_ctx_t *ctx, const uint8_t *key,
             const acl_rule_t *rules[],  uint32_t categories)
{
    return ctx->classify(key, rules, categories);
}

hal_ret_t
acl_dump(const acl_ctx_t *ctx, uint32_t categories, print_cb_t print_cb)
{
    return ctx->dump(categories, print_cb);
}

} // namespace acl
