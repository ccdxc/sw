#include "nic/hal/plugins/network/net_plugin.hpp"
#include "nic/include/fte_db.hpp"
#include "nic/include/fte.hpp"
#include "nic/hal/plugins/network/alg/alg_utils.hpp"
#include "nic/hal/plugins/sfw/core.hpp"

namespace fte {

std::ostream& operator<<(std::ostream& os, const fte::alg_entry_t& val)
{
    os << "{key=" << val.entry.key;
    os << " ,alg_proto_state=" << val.alg_proto_state;
    os << " ,rpc_frag_cont=" << val.rpcinfo.rpc_frag_cont;

    return os << "}";
}

hal_ret_t
expected_flow_handler(fte::ctx_t &ctx, fte::expected_flow_t *wentry)
{
    hal::plugins::sfw::sfw_info_t *sfw_info =
        (hal::plugins::sfw::sfw_info_t*)ctx.feature_state(hal::plugins::sfw::FTE_FEATURE_SFW);
    fte::alg_entry_t *entry = (fte::alg_entry_t*)wentry;
    sfw_info->skip_sfw = entry->skip_sfw;
    ctx.set_alg_entry((void *)entry);
    return HAL_RET_OK;
}

/*-----------------------------------------------------------------------
- This API can be used to allocate and init an ALG entry to be used for 
  further processing by the respective ALGs
-------------------------------------------------------------------------*/
fte::alg_entry_t*
alloc_and_init_alg_entry(fte::ctx_t& ctx)
{
    fte::alg_entry_t *entryp = NULL;

    entryp = (fte::alg_entry_t *)HAL_CALLOC(hal::HAL_MEM_ALLOC_ALG,
                                       sizeof(fte::alg_entry_t));
    if (!entryp) {
        return NULL;
    }

    memset(entryp, 0, sizeof(fte::alg_entry_t));

    entryp->entry.key = ctx.key();
    ctx.set_alg_entry((void *)entryp);

    return entryp;
}

/*-----------------------------------------------------------------------
- This API can be used to insert a new entry into the FTE Expected flow table
  when the stateful firewall has indicated ALG action on the flow.
-------------------------------------------------------------------------*/
fte::alg_entry_t *
insert_alg_entry(fte::ctx_t& ctx)
{
    fte::alg_entry_t     *entryp = NULL;

    entryp = (fte::alg_entry_t*)ctx.alg_entry();
    if (entryp != NULL) {
        entryp->entry.handler = expected_flow_handler;
        entryp->session = ctx.session();
        HAL_TRACE_DEBUG("Inserting Key: {} in ALG table", entryp->entry.key);
        HAL_TRACE_DEBUG("New entry: {}", *entryp);

        (void)insert_expected_flow((fte::expected_flow_t*)entryp);
    } 

    return entryp;
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
    if (status) {
        insert_alg_entry(ctx);
    }
}

}

namespace hal {
namespace net {

void
insert_rpc_entry(fte::ctx_t& ctx, fte::RPCMap *map, 
                 fte::alg_proto_state_t proto_state)
{
    fte::alg_entry_t *entry = NULL;

    entry = (fte::alg_entry_t *)HAL_CALLOC(hal::HAL_MEM_ALLOC_ALG,
                                           sizeof(fte::alg_entry_t));
    if (!entry) {
        HAL_TRACE_ERR("Could not allocated memory for RPC ALG entry");
        return;
    }

    memset(&entry->entry.key, 0, sizeof(hal::flow_key_t));
    entry->entry.key.vrf_id = ctx.key().vrf_id;
    entry->entry.key.dip = map->ip;
    entry->entry.key.dport = map->dport;
    entry->entry.key.proto = (types::IPProtocol)map->prot;
    entry->entry.key.flow_type = (map->addr_family == fte::ALG_ADDRESS_FAMILY_IPV6)?FLOW_TYPE_V6:FLOW_TYPE_V4;
    entry->alg_proto_state = proto_state;
    entry->skip_sfw = TRUE;

    // Save the program number and SUN/MS RPC control dport (could be user specified)
    // We would replace this with the incoming one for Firewall lookup.
    entry->rpcinfo.rpc_map.prog_num = map->prog_num;
    memcpy(&entry->rpcinfo.rpc_map.uuid, &map->uuid, sizeof(map->uuid));
    entry->rpcinfo.rpc_map.vers = map->vers;
    entry->rpcinfo.rpc_map.dport = ctx.key().dport;

    // Need to add the entry with a timer
    // Todo(Pavithra) add timer to every ALG entry
    HAL_TRACE_DEBUG("Inserting RPC entry with key: {}", entry->entry.key);
    
    entry->entry.handler = fte::expected_flow_handler;
    fte::insert_expected_flow((fte::expected_flow_t*)entry);
}

void
cleanup_alg_entry(fte::ctx_t& ctx, hal::flow_key_t key) 
{
    fte::alg_entry_t *entry = (fte::alg_entry_t *)fte::remove_expected_flow(key);

    if (entry != NULL) {
        HAL_FREE(hal::HAL_MEM_ALLOC_ALG, entry);
    }

}

}
}
