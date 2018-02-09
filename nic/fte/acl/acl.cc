#include "nic/fte/acl/acl.hpp"
#include "nic/fte/acl/acl_ctx.hpp"
#include "sdk/lock.hpp"

namespace acl {

using sdk::sdk_spinlock_t;

//------------------------------------------------------------------------
// lock to protect ctx list
//------------------------------------------------------------------------
static sdk_spinlock_t g_ctx_lock;
static int g_ctx_lock_init = SDK_SPINLOCK_INIT(&g_ctx_lock, PTHREAD_PROCESS_PRIVATE);

static const acl_ctx_t *g_ctx;  // single instance for now

//------------------------------------------------------------------------
// Creates a new ACL context.
// Inserts the ctx into global list, which can be looked up
// by other threads using acl_get(name)
//------------------------------------------------------------------------
const acl_ctx_t *
acl_create(const acl_config_t *cfg)
{
    const acl_ctx_t *ctx = acl_ctx_t::create(cfg);

    // TODO(goli) - single instance for now
    SDK_ASSERT_RETURN(g_ctx == NULL, NULL);

    SDK_ASSERT_RETURN((SDK_SPINLOCK_LOCK(&g_ctx_lock) == 0), NULL);

    // store the instance in global list
    g_ctx = ctx->clone();

    SDK_ASSERT_RETURN((SDK_SPINLOCK_UNLOCK(&g_ctx_lock) == 0), NULL);

    return ctx;
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
    if (g_ctx) {
        ctx = g_ctx->clone();
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
acl_delete(const acl_ctx_t *ctx)
{
    // remove from global list and deref
    SDK_ASSERT(SDK_SPINLOCK_LOCK(&g_ctx_lock) == 0);
    if (g_ctx) {
        g_ctx->deref();
        g_ctx = NULL;
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
    // Update the global list
    SDK_ASSERT_RETURN((SDK_SPINLOCK_LOCK(&g_ctx_lock) == 0), HAL_RET_INVALID_OP);
    if (g_ctx) {
        g_ctx->deref();
    }
    g_ctx = ctx->clone();
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
