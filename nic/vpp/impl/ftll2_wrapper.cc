/*
 *  {C} Copyright 2019 Pensando Systems Inc. All rights reserved.
 */

#include <nic/sdk/include/sdk/table.hpp>
#include <lib/table/ftl/ftl_base.hpp>
#include <nic/apollo/p4/include/defines.h>
#include <ftl_wrapper.h>

using namespace sdk;
using namespace sdk::table;
using namespace sdk::platform;

extern "C" {

typedef struct ftll2_cache_s {
    flow_hash_entry_t flow[MAX_FLOW_ENTRIES_PER_BATCH]; 
    flow_hash_entry_t last_read_flow;
    uint32_t hash[MAX_FLOW_ENTRIES_PER_BATCH];
    flow_flags_t flags[MAX_FLOW_ENTRIES_PER_BATCH];
    uint16_t count;
} ftll2_cache_t;

thread_local ftll2_cache_t g_l2_flow_cache;

int
ftll2_remove (ftll2 *obj, flow_hash_entry_t *entry, uint32_t hash, 
              uint8_t log)
{
    return ftl_remove(obj, entry, hash, log);
}

int
ftll2_get_with_handle(ftl *obj, uint32_t index, bool primary)
{
    sdk_table_api_params_t params = {0};
    flow_hash_entry_t entry;
    uint8_t sip[16], dip[16];

    if (get_skip_ftl_program()) {
        return 0;
    }

    if (primary) {
        params.handle.pindex(index);
    } else {
        params.handle.sindex(index);
    }
    params.entry = &entry;

    if (SDK_RET_OK != obj->get_with_handle(&params)) {
        return -1;
    }

    entry.get_key_metadata_src(sip);
    entry.get_key_metadata_dst(dip);
    ftll2_set_key(&g_l2_flow_cache.last_read_flow,
                  sip, dip,
                  // dport is used to store ether type for L2 flows
                  entry.get_key_metadata_dport(),
                  ftl_get_key_lookup_id(&entry));

    return 0;
}

int
ftll2_remove_cached_entry(ftl *obj)
{
    return ftll2_remove(obj, &g_l2_flow_cache.last_read_flow, 0, 0);
}

int
ftll2_clear (ftll2 *obj, bool clear_global_state,
             bool clear_thread_local_state)
{
    return ftl_clear(obj, clear_global_state, clear_thread_local_state);
}

void
ftll2_delete (ftll2 *obj)
{
    ftl_delete(obj);
}

void ftll2_set_key (flow_entry *entry,
                    uint8_t *src,
                    uint8_t *dst,
                    uint16_t ether_type,
                    uint16_t lookup_id)
{
    entry->set_key_metadata_dst(dst);
    entry->set_key_metadata_src(src);
    // dport is used to store ether type for L2 flows
    entry->set_key_metadata_dport(ether_type);
    ftl_set_key_lookup_id(entry, lookup_id);
    entry->set_key_metadata_ktype(KEY_TYPE_MAC);
}

int
ftll2_dump_hw_entries (ftll2 *obj, char *logfile, uint8_t detail)
{
    return ftl_dump_hw_entries(obj, logfile, detail, true);
}

int
ftll2_dump_hw_entry (ftll2 *obj, uint8_t *src, uint8_t *dst,
                     uint16_t ether_type, uint16_t lookup_id,
                     char *buf, int max_len)
{
    sdk_ret_t ret;
    sdk_table_api_params_t params = {0};
    int retcode = 0;
    flow_hash_entry_t entry;

    entry.clear();
    ftll2_set_key(&entry, src, dst, ether_type, lookup_id);
    params.entry = &entry;

    ret = obj->get(&params);
    if (ret != SDK_RET_OK) {
        retcode = -1;
        goto done;
    }
    entry.tostr(buf, max_len);

done:
    return retcode;
}

void
ftll2_init_stats_cache (void)
{
    ftl_init_stats_cache();
}

void
ftll2_cache_stats (ftll2 *obj)
{
    ftl_cache_stats(obj);
}

void
ftll2_dump_stats (ftll2 *obj, char *buf, int max_len)
{
    ftl_dump_stats(obj, buf, max_len);
}

void
ftll2_dump_stats_cache (char *buf, int max_len)
{
    ftl_dump_stats_cache(buf, max_len);
}

uint64_t
ftll2_get_flow_count (ftll2 *obj)
{
    return ftl_get_flow_count(obj, true);
}

void
ftll2_cache_batch_init (void)
{
   g_l2_flow_cache.count = 0;
}

void
ftll2_cache_set_key (uint8_t *src,
                     uint8_t *dst,
                     uint16_t ether_type,
                     uint16_t lookup_id)
{
   ftll2_set_key(g_l2_flow_cache.flow + g_l2_flow_cache.count,
                 src, dst, ether_type, lookup_id);
}

void
ftll2_cache_set_nexthop (uint32_t nhid,
                         uint32_t nhtype,
                         uint8_t nh_valid)
{
   ftl_set_entry_nexthop(g_l2_flow_cache.flow + g_l2_flow_cache.count, nhid, nhtype, 
                         nh_valid);
}

void
ftll2_cache_set_counter_index (uint8_t ctr_idx)
{
    g_l2_flow_cache.flags[g_l2_flow_cache.count].ctr_idx = ctr_idx;
}

int
ftll2_cache_get_count (void)
{
    return g_l2_flow_cache.count;
}

void
ftll2_cache_advance_count (int val)
{
    g_l2_flow_cache.count += val;
}

int
ftll2_cache_program_index (ftll2 *obj, uint16_t id, uint32_t *pindex, 
                           uint32_t *sindex)
{
    return ftl_insert(obj, g_l2_flow_cache.flow + id,
                      g_l2_flow_cache.hash[id],
                      pindex, sindex,
                      g_l2_flow_cache.flags[id].log,
                      g_l2_flow_cache.flags[id].update);
}

int
ftll2_cache_delete_index (ftll2 *obj, uint16_t id)
{
    return ftll2_remove(obj, g_l2_flow_cache.flow + id,
                        g_l2_flow_cache.hash[id],
                        g_l2_flow_cache.flags[id].log);
}

void
ftll2_cache_set_session_index (uint32_t val)
{
    ftl_set_session_index(g_l2_flow_cache.flow + g_l2_flow_cache.count, val);
}

void
ftll2_cache_set_flow_role(uint8_t flow_role)
{
    ftl_set_flow_role(g_l2_flow_cache.flow + g_l2_flow_cache.count, flow_role);
}

uint32_t
ftll2_cache_get_session_index (int id)
{
    return ftl_get_session_id(g_l2_flow_cache.flow + id);
}

uint8_t
ftll2_cache_get_proto(int id)
{
    return ftl_get_proto(g_l2_flow_cache.flow + id);
}

void
ftll2_cache_set_epoch (uint8_t val)
{
    ftl_set_epoch(g_l2_flow_cache.flow + g_l2_flow_cache.count, val);
}

void
ftll2_cache_set_hash_log (uint32_t val, uint8_t log)
{
    g_l2_flow_cache.hash[g_l2_flow_cache.count] = val;
    g_l2_flow_cache.flags[g_l2_flow_cache.count].log = log;
}

void
ftll2_cache_set_update_flag (uint8_t update)
{
    g_l2_flow_cache.flags[g_l2_flow_cache.count].update = update;
}

void
ftll2_cache_set_flow_miss_hit (uint8_t val)
{
    ftl_set_entry_flow_miss_hit(g_l2_flow_cache.flow + g_l2_flow_cache.count, val);
}

void
ftll2_cache_batch_flush (ftll2 *obj, int *status)
{
    int i;
    uint32_t pindex, sindex;

    for (i = 0; i < g_l2_flow_cache.count; i++) {
       status[i] = ftl_insert(obj, g_l2_flow_cache.flow + i,
                              g_l2_flow_cache.hash[i],
                              &pindex, &sindex,
                              g_l2_flow_cache.flags[i].log,
                              g_l2_flow_cache.flags[i].update);
    }
}

}
