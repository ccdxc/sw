#include "nic/fte/fte.hpp"
#include "nic/fte/fte_db.hpp"


namespace fte {

/*-----------------------------------------------------
    Begin Hash Utility APIs
------------------------------------------------------*/
void *
alg_flow_get_key_func(void *entry)
{
    HAL_ASSERT(entry != NULL);
    return (void *)&(((alg_entry_t *)entry)->key);
}

uint32_t
alg_flow_compute_hash_func (void *key, uint32_t ht_size)
{
    return (hal::utils::hash_algo::fnv_hash(key, \
                       sizeof(hal::flow_key_t)) % ht_size);
}

bool
alg_flow_compare_key_func (void *key1, void *key2)
{
    HAL_ASSERT((key1 != NULL) && (key2 != NULL));
    if (!memcmp(key1, key2, sizeof(hal::flow_key_t))) {
        return true;
    }
    return false;
}

/*-----------------------------------------------------
    End Hash Utility APIs
------------------------------------------------------*/

/*-----------------------------------------------------------------------
- Builds wildcard keys for lookup into ALG table. This will be used when
  a flow miss happens to check if we have any previously saved state for
  the flow.
-------------------------------------------------------------------------*/
static hal_ret_t
build_wildcard_key(hal::flow_key_t& key, hal::flow_key_t key_)
{
    memcpy(std::addressof(key), &key_, sizeof(hal::flow_key_t));

    if (key.flow_type != hal::FLOW_TYPE_L2 && key.proto == IP_PROTO_UDP) {
        key.sport = 0;
    }

    return HAL_RET_OK;
}


/*-----------------------------------------------------------------------
- Performs lookup on ALG hash table with the given flow key and a wildcard
  key on a flow miss.
-------------------------------------------------------------------------*/
alg_entry_t *
lookup_alg_db(ctx_t *ctx)
{
    uint8_t         i=0, num_keys=0;
    hal::flow_key_t keys[MAX_FLOW_KEYS];
    alg_entry_t     *entry = NULL;

    //ALG Variations
    build_wildcard_key(keys[num_keys++], ctx->key());

    g_fte_db->rlock();
    while (i < num_keys) {
        HAL_TRACE_DEBUG("Looking up ALG DB for key: {}", keys[i]);
        entry = (alg_entry_t *)g_fte_db->alg_flow_key_ht()->lookup(std::addressof(keys[i++]));
        if (!entry) {
            continue;
        } else {
            HAL_TRACE_DEBUG("ALG Entry Found with key: {}", keys[i-1]);
            break;
        }
    }
    g_fte_db->runlock();

    return (entry);
}

/*-----------------------------------------------------------------------
- This API can be used to insert a new entry into the ALG wildcard table
  when the firewall has indicated ALG action on the flow.
-------------------------------------------------------------------------*/
alg_entry_t *
insert_alg_entry(ctx_t *ctx, hal::session_t *sess)
{
    alg_entry_t     *entry = NULL;
    hal::flow_key_t  key = ctx->key();
    hal::flow_role_t role = hal::FLOW_ROLE_INITIATOR;

    entry = (alg_entry_t *)HAL_CALLOC(alg_entry_t, sizeof(alg_entry_t));
    if (!entry) {
        return NULL;
    }

    switch (ctx->alg_proto()) {
        case nwsec::APP_SVC_TFTP:
            role = hal::FLOW_ROLE_RESPONDER;
            key = ctx->get_key(role);
            key.sport = 0;
            break;

        case nwsec::APP_SVC_SUN_RPC:
            break;

        default:
            return NULL;
    }

    entry->key = key;
    entry->role = role;
    entry->session = sess;
    entry->alg_proto_state = ctx->alg_proto_state();

    HAL_TRACE_DEBUG("Inserting Key: {} in ALG table", key);

    entry->flow_key_ht_ctxt.reset();
    g_fte_db->wlock();
    g_fte_db->alg_flow_key_ht()->insert(entry, &entry->flow_key_ht_ctxt);
    g_fte_db->wunlock();

    return entry;
}

}
