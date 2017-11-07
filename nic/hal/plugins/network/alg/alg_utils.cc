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

        default:
            break;
    }

    entryp->session = ctx.session();
    //entryp->alg_proto_state = ctx.alg_proto_state();

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

namespace hal {
namespace net {

void
insert_rpc_entry(fte::ctx_t& ctx, fte::RPCMap *map)
{
    fte::alg_entry_t *entry = NULL;

    entry = (fte::alg_entry_t *)HAL_CALLOC(alg_entry_t, sizeof(fte::alg_entry_t));
    if (!entry) {
        HAL_TRACE_ERR("Could not allocated memory for RPC ALG entry");
        return;
    }

    memset(&entry->key, 0, sizeof(hal::flow_key_t));
    entry->key.tenant_id = ctx.key().tenant_id;
    entry->key.dip = ctx.key().dip;
    entry->key.dport = map->dport;
    entry->key.proto = (types::IPProtocol)map->prot;
    entry->key.flow_type = (map->addr_family == fte::ALG_ADDRESS_FAMILY_IPV6)?FLOW_TYPE_V6:FLOW_TYPE_V4;
    entry->alg_proto_state = fte::ALG_PROTO_STATE_RPC_DATA;

    // Save the program number and SUN/MS RPC control dport (could be user specified)
    // We would replace this with the incoming one for Firewall lookup.
    entry->rpc_map.num_map = 1;
    entry->rpc_map.maps[entry->rpc_map.num_map-1].prog_num = map->prog_num;
    memcpy(&entry->rpc_map.maps[entry->rpc_map.num_map-1].uuid, &map->uuid, sizeof(map->uuid));
    entry->rpc_map.maps[entry->rpc_map.num_map-1].vers = map->vers;
    entry->rpc_map.maps[entry->rpc_map.num_map-1].dport = ctx.key().dport;

    // Need to add the entry with a timer
    // Todo(Pavithra) add timer to every ALG entry
    HAL_TRACE_DEBUG("Inserting RPC entry with key: {}", entry->key);
    insert_alg_entry(entry);
}

}
}
