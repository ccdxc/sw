#include "nic/fte/fte.hpp"
#include "nic/fte/fte_db.hpp"


namespace fte {

enum key_fields_t {
    SIP=1,
    SPORT=2,
    DIR=3,
    TID=4,
    MAX_KEY_FIELDS,
};

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
build_wildcard_key(hal::flow_key_t& key, hal::flow_key_t key_, 
                   key_fields_t *fields, uint8_t num_mask)
{
    memcpy(std::addressof(key), &key_, sizeof(hal::flow_key_t));

    if (key.flow_type != hal::FLOW_TYPE_L2) {
        for (int i=0; i<num_mask; i++) {
            switch(fields[i]) {
                case SPORT:
                    key.sport = 0;
                    break;

                case SIP:
                    memset(&key.sip, 0, sizeof(ipvx_addr_t));
                    break;
 
                case DIR:
                    key.dir = 0;
                    break;

                default:
                    break;           
            };
        }
    }

    return HAL_RET_OK;
}

/*-----------------------------------------------------------------------
- Performs lookup on ALG hash table with the given flow key and a wildcard
  key on a flow miss.
-------------------------------------------------------------------------*/
void *
lookup_alg_db(ctx_t *ctx)
{
    uint8_t          i=0, num_keys=0, num_fields=0;
    hal::flow_key_t  keys[MAX_FLOW_KEYS];
    alg_entry_t     *entry = NULL;
    key_fields_t     fields[MAX_KEY_FIELDS];

    keys[num_keys++] = ctx->key();
    if (ctx->arm_lifq() == ALG_CFLOW_LIFQ) {
        // We insert one entry in the DB and get
        // the info in case we have gotten for reverse flow
        hal::flow_key_t key;
   
        key = ctx->key();
        key.sport = ctx->key().dport;
        key.dport = ctx->key().sport;
        key.sip = ctx->key().dip;
        key.dip = ctx->key().sip;
        keys[num_keys++] = key;
    }
    
    //ALG Variations
 
    // TFTP response
    fields[num_fields++] = SPORT;
    build_wildcard_key(keys[num_keys++], ctx->key(), fields, 1);

    // For RPC Data session
    fields[num_fields++] = SIP;
    fields[num_fields++] = DIR;
    build_wildcard_key(keys[num_keys++], ctx->key(), fields, 3);

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

void
insert_alg_entry(alg_entry_t *entry)
{
    g_fte_db->wlock();
    g_fte_db->alg_flow_key_ht()->insert(entry, &entry->flow_key_ht_ctxt);
    g_fte_db->wunlock();
}

/*-----------------------------------------------------------------------
- This API can be used to remove an entry from ALG hash table when either
  there was an error processing the reverse flow or the Iflow/Rflow has been
  successfully installed and we do not need to keep this software entry
  around.
-------------------------------------------------------------------------*/
void *
remove_alg_entry(hal::flow_key_t key)
{
    alg_entry_t   *entry = NULL;

    g_fte_db->wlock();
    entry = (alg_entry_t *)g_fte_db->alg_flow_key_ht()->remove((void *)std::addressof(key));
    g_fte_db->wunlock();

    return entry;
}

/*-----------------------------------------------------------------------
- This API can be used to lookup an entry from ALG hash table.
-------------------------------------------------------------------------*/
const void *
lookup_alg_entry(hal::flow_key_t key)
{
    alg_entry_t   *entry = NULL;

    g_fte_db->rlock();
    entry = (alg_entry_t *)g_fte_db->alg_flow_key_ht()->lookup((void *)std::addressof(key));
    g_fte_db->runlock();

    return entry;
}

/*-----------------------------------------------------------------------
- This API can be used to update an entry in ALG hash table.
-------------------------------------------------------------------------*/
hal_ret_t
update_alg_entry(hal::flow_key_t key, void *new_entry, size_t sz)
{
    void   *entry = NULL;

    g_fte_db->wlock();
    entry = (alg_entry_t *)g_fte_db->alg_flow_key_ht()->lookup((void *)std::addressof(key));
    if (entry == NULL) {
        HAL_TRACE_ERR("Entry not found in ALG table");
        g_fte_db->wunlock();
        return HAL_RET_ERR;
    }

    memcpy(entry, new_entry, sz);
    g_fte_db->wunlock();

    return HAL_RET_OK;
}

std::ostream& operator<<(std::ostream& os, const alg_entry_t& val)
{
    os << "{key=" << val.key;
    os << " ,alg_proto_state=" << val.alg_proto_state;
    os << " ,role=" << val.role;
    os << " ,rpc_frag_cont=" << val.rpc_frag_cont;

    return os << "}";
}

}
