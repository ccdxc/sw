/*
 *  {C} Copyright 2019 Pensando Systems Inc. All rights reserved.
 */

#include <stdint.h>
#include <cstddef>
#include <cstring>

#include <nic/sdk/include/sdk/table.hpp>
#include <lib/table/ftl/ftl_base.hpp>
#include <nic/sdk/lib/p4/p4_api.hpp>
#include <nic/sdk/platform/capri/capri_tbl_rw.hpp>
#include <nic/sdk/lib/p4/p4_utils.hpp>
#include "gen/p4gen/p4/include/ftl_table.hpp"
#include <nic/apollo/p4/include/defines.h>
#include "nic/vpp/infra/operd/flow_export.h"
#include <pd_utils.h>
#include <ftl_wrapper.h>
#include <session.h>

extern uint64_t pds_session_get_timestamp(uint32_t ses);

using namespace sdk;
using namespace sdk::table;
using namespace sdk::platform;

extern "C" {

typedef struct ftlv4_cache_s {
    ipv4_flow_hash_entry_t ip4_flow[MAX_FLOW_ENTRIES_PER_BATCH];
    uint32_t ip4_hash[MAX_FLOW_ENTRIES_PER_BATCH];
    flow_flags_t flags[MAX_FLOW_ENTRIES_PER_BATCH];
    uint16_t count;
} ftlv4_cache_t;

thread_local ftlv4_cache_t g_ip4_flow_cache;

ftlv4 *
ftlv4_create (void *key2str,
              void *appdata2str)
{
    sdk_table_factory_params_t factory_params = {0};

    factory_params.key2str = (key2str_t) (key2str);
    factory_params.appdata2str = (appdata2str_t) (appdata2str);

    return ipv4_flow_hash::factory(&factory_params);
}

static int
ftlv4_insert (ftlv4 *obj, ipv4_flow_hash_entry_t *entry, uint32_t hash,
              uint32_t *pindex, uint32_t *sindex, uint8_t log,
              uint8_t update)
{
    sdk_table_api_params_t params = {0};

    if (get_skip_ftl_program()) {
        return 0;
    }

    if (hash) {
        params.hash_32b = hash;
        params.hash_valid = 1;
    }
    params.entry = entry;
    if (unlikely(update)) {
        if (SDK_RET_OK != obj->update(&params)) {
            return -1;
        }
        return 0;
    }

    if (SDK_RET_OK != obj->insert(&params)) {
        return -1;
    }

    if (params.handle.svalid()) {
        *pindex = (uint32_t) (~0L);
        *sindex = params.handle.sindex();
    } else {
        *pindex = params.handle.pindex();
    }

    if (log) {
        pds_operd_export_flow_ip4(entry->get_key_metadata_ipv4_src(),
                                  entry->get_key_metadata_ipv4_dst(),
                                  entry->get_key_metadata_proto(),
                                  entry->get_key_metadata_dport(),
                                  entry->get_key_metadata_sport(),
                                  ftlv4_get_key_lookup_id(entry), 1, 1);
    }
    return 0;
}

int
ftlv4_remove (ftlv4 *obj, ipv4_flow_hash_entry_t *entry, uint32_t hash,
              uint8_t log)
{
    sdk_table_api_params_t params = {0};

    if (get_skip_ftl_program()) {
        return 0;
    }

    if (hash) {
        params.hash_32b = hash;
        params.hash_valid = 1;
    }
    params.entry = entry;
    if (SDK_RET_OK != obj->remove(&params)) {
        return -1;
    }
    if (log) {
        pds_operd_export_flow_ip4(entry->get_key_metadata_ipv4_src(),
                                  entry->get_key_metadata_ipv4_dst(),
                                  entry->get_key_metadata_proto(),
                                  entry->get_key_metadata_dport(),
                                  entry->get_key_metadata_sport(),
                                  ftlv4_get_key_lookup_id(entry), 0, 1);
    }
    return 0;
}

int 
ftlv4_remove_with_handle(ftlv4 *obj, uint32_t index, bool primary)
{
    sdk_table_api_params_t params = {0};
    ipv4_flow_hash_entry_t v4entry;

    if (get_skip_ftl_program()) {
        return 0;
    }

    if (primary) {
        params.handle.pindex(index);
    } else {
        params.handle.sindex(index);
    }
    params.entry = &v4entry;

    if (SDK_RET_OK != obj->get_with_handle(&params)) {
        return -1;
    }

    if (!ftlv4_remove(obj, &v4entry, 0, 0)) {
        return -1;
    }

    return 0;
}   

int
ftlv4_clear (ftlv4 *obj, bool clear_global_state,
             bool clear_thread_local_state)
{
    sdk_table_api_params_t params = {0};

    if (SDK_RET_OK != obj->clear(clear_global_state,
                                 clear_thread_local_state,
                                 &params)) {
        return -1;
    }
    return 0;
}

void
ftlv4_delete (ftlv4 *obj)
{
    ftlv4::destroy(obj);
}

void 
ftlv4_set_key (ipv4_flow_hash_entry_t *entry,
               uint32_t sip,
               uint32_t dip,
               uint8_t ip_proto,
               uint16_t src_port,
               uint16_t dst_port,
               uint16_t lookup_id)
{
    entry->set_key_metadata_ipv4_dst(dip);
    entry->set_key_metadata_ipv4_src(sip);
    entry->set_key_metadata_proto(ip_proto);
    entry->set_key_metadata_dport(dst_port);
    entry->set_key_metadata_sport(src_port);
    ftlv4_set_key_lookup_id(entry, lookup_id);
}

uint32_t ftlv4_entry_count;

static void
ftlv4_dump_hw_entry_iter_cb (sdk_table_api_params_t *params)
{
    ipv4_flow_hash_entry_t *hwentry =  (ipv4_flow_hash_entry_t *) params->entry;
    FILE *fp = (FILE *)params->cbdata;
    char buf[FTL_ENTRY_STR_MAX];

    if (hwentry->entry_valid) {
        ftlv4_entry_count++;
        buf[FTL_ENTRY_STR_MAX - 1] = 0;
        hwentry->key2str(buf, FTL_ENTRY_STR_MAX - 1);
        fprintf(fp, "%s\n", buf);
    }
}

static void
ftlv4_dump_hw_entry_detail_iter_cb (sdk_table_api_params_t *params)
{
    ipv4_flow_hash_entry_t *hwentry =  (ipv4_flow_hash_entry_t *) params->entry;
    FILE *fp = (FILE *)params->cbdata;
    uint8_t *entry;
    uint32_t size;
    char buf[FTL_ENTRY_STR_MAX];

    if (hwentry->entry_valid) {
        ftlv4_entry_count++;
        buf[FTL_ENTRY_STR_MAX - 1] = 0;
        hwentry->tostr(buf, FTL_ENTRY_STR_MAX - 1);
        fprintf(fp, "%s\n", buf);
        session_get_addr(hwentry->get_session_index(), &entry, &size);
        fprintf(fp, " Session data: ");
        for (uint32_t i = 0; i < size; i++) {
            fprintf(fp, "%02x", entry[i]);
        }
        fprintf(fp, "\n");
        uint32_t ses = hwentry->get_session_index();
        fprintf(fp, "Timestamp %lu addr 0x%p data %lu start 0x%p ses %u\n",
                pds_session_get_timestamp(ses), entry + 36,
                (uint64_t)((uint64_t *) (entry + 36)), entry, ses);
        fprintf(fp, "\n");
    }
}

int
ftlv4_dump_hw_entries (ftlv4 *obj, char *logfile, uint8_t detail)
{
    sdk_ret_t ret;
    sdk_table_api_params_t params = {0};
    FILE *logfp = fopen(logfile, "a");
    int retcode = -1;
    char buf[FTL_ENTRY_STR_MAX];

    if (logfp == NULL) {
        goto end;
    }

    buf[FTL_ENTRY_STR_MAX - 1] = 0;

    params.itercb = detail ?
                    ftlv4_dump_hw_entry_detail_iter_cb :
                    ftlv4_dump_hw_entry_iter_cb;
    params.cbdata = logfp;
    params.force_hwread = false;
    ftlv4_entry_count = 0;

    if (!detail) {
        // ipv4_flow_hash_entry_t::keyheader2str(buf, FTL_ENTRY_STR_MAX - 1);
        // fprintf(logfp, "%s", buf);
    }

    ret = obj->iterate(&params);
    if (ret != SDK_RET_OK) {
        retcode = -1;
    } else {
        retcode = ftlv4_entry_count;
    }

    if (!detail) {
        fprintf(logfp, "\n%s", buf);
    }
    fclose(logfp);

end:
    return retcode;
}

int
ftlv4_dump_hw_entry (ftlv4 *obj, uint32_t src, uint32_t dst,
                     uint8_t ip_proto, uint16_t sport,
                     uint16_t dport, uint16_t lookup_id,
                     char *buf, int max_len)
{
    sdk_ret_t ret;
    sdk_table_api_params_t params = {0};
    int retcode = 0;
    ipv4_flow_hash_entry_t entry;

    entry.clear();
    ftlv4_set_key(&entry, src, dst, ip_proto, sport, dport, lookup_id);
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
ftlv4_init_stats_cache (void)
{
    ftl_init_stats_cache();
}

void
ftlv4_cache_stats (ftlv4 *obj)
{
    ftl_cache_stats(obj);
}

void
ftlv4_dump_stats (ftlv4 *obj, char *buf, int max_len)
{
    ftl_dump_stats(obj, buf, max_len);
}

void
ftlv4_dump_stats_cache (char *buf, int max_len)
{
    ftl_dump_stats_cache(buf, max_len);
}

static void
ftlv4_hw_entry_count_cb (sdk_table_api_params_t *params)
{
    ipv4_flow_hash_entry_t *hwentry =  (ipv4_flow_hash_entry_t *) params->entry;

    if (hwentry->entry_valid) {
        uint64_t *count = (uint64_t *)params->cbdata;
        (*count)++;
    }
}

uint64_t
ftlv4_get_flow_count (ftlv4 *obj)
{
    sdk_ret_t ret;
    sdk_table_api_params_t params = {0};
    uint64_t count = 0;

    params.itercb = ftlv4_hw_entry_count_cb;
    params.cbdata = &count;
    params.force_hwread = false;

    ret = obj->iterate(&params);
    if (ret != SDK_RET_OK) {
        count = ~0L;
    }

    return count;
}

static void
ftlv4_set_session_index (ipv4_flow_hash_entry_t *entry, uint32_t session)
{
    entry->set_session_index(session);
}

static void
ftlv4_set_epoch (ipv4_flow_hash_entry_t *entry, uint8_t val)
{
    entry->set_epoch(val);
}

void
ftlv4_cache_set_hash_log (uint32_t val, uint8_t log)
{
    g_ip4_flow_cache.ip4_hash[g_ip4_flow_cache.count] = val;
    g_ip4_flow_cache.flags[g_ip4_flow_cache.count].log = log;
}

void
ftlv4_cache_set_update_flag (uint8_t update)
{
    g_ip4_flow_cache.flags[g_ip4_flow_cache.count].update = update;
}

void
ftlv4_cache_set_flow_miss_hit (uint8_t val)
{
    ftlv4_set_entry_flow_miss_hit(g_ip4_flow_cache.ip4_flow + g_ip4_flow_cache.count,
                                  val);
}

static uint32_t
ftlv4_get_session_id (ipv4_flow_hash_entry_t *entry)
{
    return entry->get_session_index();
}

static uint8_t
ftlv4_get_proto(ipv4_flow_hash_entry_t *entry)
{
    return entry->get_key_metadata_proto();
}

void
ftlv4_cache_batch_init (void)
{
   g_ip4_flow_cache.count = 0;
}

void
ftlv4_cache_set_key (uint32_t sip,
                     uint32_t dip,
                     uint8_t ip_proto,
                     uint16_t src_port,
                     uint16_t dst_port,
                     uint16_t lookup_id)
{
   ftlv4_set_key(g_ip4_flow_cache.ip4_flow + g_ip4_flow_cache.count,
                 sip, dip, ip_proto, src_port, dst_port, lookup_id);
}

void
ftlv4_cache_set_nexthop (uint32_t nhid,
                         uint32_t nhtype,
                         uint8_t nh_valid)
{
   ftlv4_set_entry_nexthop(g_ip4_flow_cache.ip4_flow + g_ip4_flow_cache.count,
                           nhid, nhtype, nh_valid);
}

int
ftlv4_cache_get_count (void)
{
    return g_ip4_flow_cache.count;
}

void
ftlv4_cache_advance_count (int val)
{
    g_ip4_flow_cache.count += val;
}

int
ftlv4_cache_program_index (ftlv4 *obj, uint16_t id, uint32_t *pindex, 
                           uint32_t *sindex)
{
    return ftlv4_insert(obj, g_ip4_flow_cache.ip4_flow + id,
                        g_ip4_flow_cache.ip4_hash[id],
                        pindex, sindex,
                        g_ip4_flow_cache.flags[id].log,
                        g_ip4_flow_cache.flags[id].update);
}

int
ftlv4_cache_delete_index (ftlv4 *obj, uint16_t id)
{
    return ftlv4_remove(obj, g_ip4_flow_cache.ip4_flow + id,
                        g_ip4_flow_cache.ip4_hash[id],
                        g_ip4_flow_cache.flags[id].log);
}

void
ftlv4_cache_set_session_index (uint32_t val)
{
    ftlv4_set_session_index(g_ip4_flow_cache.ip4_flow + g_ip4_flow_cache.count, val);
}

uint32_t
ftlv4_cache_get_session_index (int id)
{
    return ftlv4_get_session_id(g_ip4_flow_cache.ip4_flow + id);
}

uint8_t
ftlv4_cache_get_proto(int id)
{
    return ftlv4_get_proto(g_ip4_flow_cache.ip4_flow + id);
}

void
ftlv4_cache_set_epoch (uint8_t val)
{
    ftlv4_set_epoch(g_ip4_flow_cache.ip4_flow + g_ip4_flow_cache.count, val);
}

void
ftlv4_cache_batch_flush (ftlv4 *obj, int *status)
{
    int i;
    uint32_t pindex, sindex;

    for (i = 0; i < g_ip4_flow_cache.count; i++) {
       status[i] = ftlv4_insert(obj, g_ip4_flow_cache.ip4_flow + i,
                                g_ip4_flow_cache.ip4_hash[i],
                                &pindex, &sindex,
                                g_ip4_flow_cache.flags[i].log,
                                g_ip4_flow_cache.flags[i].update);
    }
}

void
ftlv4_set_thread_id (ftlv4 *obj, uint32_t thread_id)
{
    obj->set_thread_id(thread_id);
    return;
}

}
