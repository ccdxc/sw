#include "nic/fte/acl/acl.hpp"
#include "nic/fte/acl/acl_ctx.hpp"
#include "sdk/lock.hpp"

namespace acl {

using sdk::sdk_spinlock_t;
using sdk::lib::ht;
using sdk::lib::ht_ctxt_t;

//------------------------------------------------------------------------
// lock to protect ctx list
//------------------------------------------------------------------------
static sdk_spinlock_t g_ctx_lock;
static int g_ctx_lock_init = SDK_SPINLOCK_INIT(&g_ctx_lock, PTHREAD_PROCESS_PRIVATE);

static void *ctx_get_key_func_(void *entry)
{
    return (void *)((acl_ctx_t *)entry)->name();
}

static uint32_t ctx_compute_hash_func_(void *key, uint32_t ht_size)
{
    return sdk::lib::hash_algo::fnv_hash(key, strlen((char *)key)) % ht_size;
}

static bool  ctx_compare_key_func_(void *key1, void *key2) {
    return strcmp((char *)key1, (char *)key2) == 0;
}

static ht *g_ctx_ht = ht::factory(16, ctx_get_key_func_,
                                  ctx_compute_hash_func_,
                                  ctx_compare_key_func_,
                                  false /* not thread_safe */);

//------------------------------------------------------------------------
// Creates a new ACL context.
//------------------------------------------------------------------------
const acl_ctx_t *
lib_acl_create(const char *name, const acl_config_t *cfg)
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

//------------------------------------------------------------------------
// Release the ctx reference
//------------------------------------------------------------------------
void
acl_deref(const acl_ctx_t *ctx)
{
    ctx->deref();
}

//------------------------------------------------------------------------
// Remove the ctx from the global list and release the reference.
//------------------------------------------------------------------------
void
lib_acl_delete(const acl_ctx_t *ctx)
{
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

} // namespace acl
