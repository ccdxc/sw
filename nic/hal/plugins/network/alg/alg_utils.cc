#include "nic/hal/plugins/network/net_plugin.hpp"
#include "nic/include/fte_db.hpp"
#include "nic/include/fte.hpp"

namespace fte {

/*-----------------------------------------------------------------------
- This API can be used to insert a new entry into the ALG wildcard table
  when the firewall has indicated ALG action on the flow.
-------------------------------------------------------------------------*/
alg_entry_t *
alloc_and_insert_alg_entry(fte::ctx_t& ctx, hal::flow_role_t role)
{
    alg_entry_t     *entryp = NULL;
    hal::flow_key_t  key = ctx.key();
    alg_entry_t      alg_entry = ctx.alg_entry();

    entryp = (alg_entry_t *)HAL_CALLOC(alg_entry_t, sizeof(alg_entry_t));
    if (!entryp) {
        return NULL;
    }

    memcpy(entryp, &alg_entry, sizeof(alg_entry_t));
    switch (ctx.alg_proto()) {
        case nwsec::APP_SVC_TFTP:
            entryp->role = hal::FLOW_ROLE_RESPONDER;
            entryp->key.sport = 0;
            break;

        case nwsec::APP_SVC_SUN_RPC:
            if (role == hal::FLOW_ROLE_INITIATOR) {
                // Insert a key for responder as well
                // We need to sniff packets on either direction
                alloc_and_insert_alg_entry(ctx, hal::FLOW_ROLE_RESPONDER);
            } else {
                key = ctx.get_key(role);
                entryp->key = key;
                entryp->role = role;
            }
            break;

        default:
            return NULL;
    }

    entryp->session = ctx.session();
    entryp->alg_proto_state = ctx.alg_proto_state();

    HAL_TRACE_DEBUG("Inserting Key: {} in ALG table", key);

    entryp->flow_key_ht_ctxt.reset();

    (void)insert_alg_entry(entryp);

    return entryp;
}

alg_entry_t *
alloc_and_insert_alg_entry(fte::ctx_t& ctx)
{
    return (alloc_and_insert_alg_entry(ctx, hal::FLOW_ROLE_INITIATOR));
}


/*-----------------------------------------------------------------------
- Execution completion handler for ALG to insert wildcard table entry if
  the proto requires any
-------------------------------------------------------------------------*/
void
alg_completion_hdlr (fte::ctx_t& ctx, bool status)
{
    HAL_TRACE_DEBUG("Invoked ALG Completion Handler status: {}", status);

    // Insert ALG entry on completion
    if (status)
        alloc_and_insert_alg_entry(ctx);

}

}
