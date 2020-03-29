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
#include <ftl_utils.hpp>
#include <session.h>

extern uint64_t pds_session_get_timestamp(uint32_t ses);

using namespace sdk;
using namespace sdk::table;
using namespace sdk::platform;

extern "C" {

static int skip_ftl_program = 0;

static sdk_table_api_stats_t g_api_stats;
static sdk_table_stats_t g_table_stats;

void
set_skip_ftl_program (int val)
{
    skip_ftl_program = val;
}

int
get_skip_ftl_program (void)
{
    return skip_ftl_program;
}

void
ftl_init_stats_cache (void)
{
    memset(&g_api_stats, 0, sizeof(g_api_stats));
    memset(&g_table_stats, 0, sizeof(g_table_stats));
}

void
ftl_cache_stats (ftl *obj)
{
    sdk_table_api_stats_t api_stats;
    sdk_table_stats_t table_stats;

    obj->stats_get(&api_stats, &table_stats);
    g_api_stats.accumulate(&api_stats);
    g_table_stats.accumulate(&table_stats);
}

static void
ftl_print_stats (sdk_table_api_stats_t *api_stats,
                 sdk_table_stats_t *table_stats,
                 char *buf, int max_len)
{
    char *cur = buf;

    cur += snprintf(buf, max_len,
                    "Insert %lu, Insert_fail_dupl %lu, Insert_fail %lu, "
                    "Insert_fail_recirc %lu\n"
                    "Remove %lu, Remove_not_found %lu, Remove_fail %lu\n"
                    "Update %lu, Update_fail %lu\n"
                    "Get %lu, Get_fail %lu\n"
                    "Reserve %lu, reserve_fail %lu\n"
                    "Release %lu, Release_fail %lu\n"
                    "Tbl_entries %lu, Tbl_collision %lu\n"
                    "Tbl_insert %lu, Tbl_remove %lu, Tbl_read %lu, Tbl_write %lu\n",
                    api_stats->insert,
                    api_stats->insert_duplicate,
                    api_stats->insert_fail,
                    api_stats->insert_recirc_fail,
                    api_stats->remove,
                    api_stats->remove_not_found,
                    api_stats->remove_fail,
                    api_stats->update,
                    api_stats->update_fail,
                    api_stats->get,
                    api_stats->get_fail,
                    api_stats->reserve,
                    api_stats->reserve_fail,
                    api_stats->release,
                    api_stats->release_fail,
                    table_stats->entries, table_stats->collisions,
                    table_stats->insert, table_stats->remove,
                    table_stats->read, table_stats->write);
    for (int i= 0; i < SDK_TABLE_MAX_RECIRC; i++) {
        cur += snprintf(cur, buf + max_len - cur,
                        "Tbl_lvl %u, Tbl_insert %lu, Tbl_remove %lu\n",
                        i, table_stats->insert_lvl[i], table_stats->remove_lvl[i]);
    }
}

void
ftl_dump_stats (ftl *obj, char *buf, int max_len)
{
    sdk_table_api_stats_t api_stats;
    sdk_table_stats_t table_stats;

    obj->stats_get(&api_stats, &table_stats);
    ftl_print_stats(&api_stats, &table_stats, buf, max_len);
}

void
ftl_dump_stats_cache (char *buf, int max_len)
{
    ftl_print_stats(&g_api_stats, &g_table_stats, buf, max_len);
}

void
ftl_aggregate_api_stats (sdk_table_api_stats_t *api_stat1,
                         sdk_table_api_stats_t *api_stat2)
{
    api_stat1->insert += api_stat2->insert;
    api_stat1->insert_duplicate += api_stat2->insert_duplicate;
    api_stat1->insert_fail += api_stat2->insert_fail;
    api_stat1->insert_recirc_fail += api_stat2->insert_recirc_fail;
    api_stat1->remove += api_stat2->remove;
    api_stat1->remove_not_found += api_stat2->remove_not_found;
    api_stat1->remove_fail += api_stat2->remove_fail;
    api_stat1->update += api_stat2->update;
    api_stat1->update_fail += api_stat2->update_fail;
    api_stat1->get += api_stat2->get;
    api_stat1->get_fail += api_stat2->get_fail;
    api_stat1->reserve += api_stat2->reserve;
    api_stat1->reserve_fail += api_stat2->reserve_fail;
    api_stat1->release += api_stat2->release;
    api_stat1->release_fail += api_stat2->release_fail;
    return;
}

void
ftl_aggregate_table_stats (sdk_table_stats_t *table_stat1,
                           sdk_table_stats_t *table_stat2)
{
    table_stat1->insert += table_stat2->insert;
    table_stat1->remove += table_stat2->remove;
    table_stat1->read += table_stat2->read;
    table_stat1->write += table_stat2->write;
    for (int i= 0; i < SDK_TABLE_MAX_RECIRC; i++) {
        table_stat1->insert_lvl[i] += table_stat2->insert_lvl[i];
        table_stat1->remove_lvl[i] += table_stat2->remove_lvl[i];
    }
}

ftl *
ftl_create (void *key2str, void *appdata2str)
{
    sdk_table_factory_params_t factory_params = {0};

    factory_params.key2str = (key2str_t) (key2str);
    factory_params.appdata2str = (appdata2str_t) (appdata2str);

    return flow_hash::factory(&factory_params);
}

int
ftl_insert (ftl *obj, flow_hash_entry_t *entry, uint32_t hash, 
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
        // TODO: avoid memcpy, instead get pointer
        uint8_t src[16], dst[16];
        entry->get_key_metadata_src(src);
        entry->get_key_metadata_dst(dst);
        if (entry->get_key_metadata_ktype() == KEY_TYPE_IPV6) {
            pds_operd_export_flow_ip6(src, dst,
                                      entry->get_key_metadata_proto(),
                                      entry->get_key_metadata_dport(),
                                      entry->get_key_metadata_sport(),
                                      ftl_get_lookup_id(entry), 1, 1);
        } else {
            // Key type MAC
            pds_operd_export_flow_l2(src, dst,
                                     entry->get_key_metadata_dport(),
                                     ftl_get_lookup_id(entry), 1, 1);
        }
    }
    return 0;
}

int
ftl_remove (ftl *obj, flow_hash_entry_t *entry, uint32_t hash, 
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
        // TODO: avoid memcpy, instead get pointer
        uint8_t src[16], dst[16];
        entry->get_key_metadata_src(src);
        entry->get_key_metadata_dst(dst);
        if (entry->get_key_metadata_ktype() == KEY_TYPE_IPV6) {
            pds_operd_export_flow_ip6(src, dst,
                                      entry->get_key_metadata_proto(),
                                      entry->get_key_metadata_dport(),
                                      entry->get_key_metadata_sport(),
                                      ftl_get_lookup_id(entry), 0, 1);
        } else {
            // Key type MAC
            pds_operd_export_flow_l2(src, dst,
                                     entry->get_key_metadata_dport(),
                                     ftl_get_lookup_id(entry), 0, 1);
        }
    }
    return 0;
}

int
ftl_remove_with_handle(ftl *obj, uint32_t index, bool primary)
{
    sdk_table_api_params_t params = {0};
    flow_hash_entry_t entry;

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

    if (!ftl_remove(obj, &entry, 0, 0)) {
        return -1;
    }

    return 0;
}

int
ftl_clear (ftl *obj, bool clear_global_state,
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
ftl_delete (ftl *obj)
{
    ftl::destroy(obj);
}

uint32_t ftlv6_entry_count;
uint32_t ftll2_entry_count;

static void
ftl_dump_hw_entry_iter_cb (sdk_table_api_params_t *params)
{
    flow_hash_entry_t *hwentry =  (flow_hash_entry_t *) params->entry;
    FILE *fp = (FILE *)params->cbdata;
    char buf[FTL_ENTRY_STR_MAX];

    if (hwentry->entry_valid && 
        (params->key_type == hwentry->key_metadata_ktype)) {
        if (hwentry->key_metadata_ktype == KEY_TYPE_IPV6) {
            ftlv6_entry_count++;
        } else {
            ftll2_entry_count++;
        }
        buf[FTL_ENTRY_STR_MAX - 1] = 0;
        hwentry->key2str(buf, FTL_ENTRY_STR_MAX - 1);
        fprintf(fp, "%s\n", buf);
    }
}

static void
ftl_dump_hw_entry_detail_iter_cb (sdk_table_api_params_t *params)
{
    flow_hash_entry_t *hwentry =  (flow_hash_entry_t *) params->entry;
    FILE *fp = (FILE *)params->cbdata;
    uint8_t *entry;
    uint32_t size;
    char buf[FTL_ENTRY_STR_MAX];

    if (hwentry->entry_valid && 
        (params->key_type == hwentry->key_metadata_ktype)) {
        if (hwentry->key_metadata_ktype == KEY_TYPE_IPV6) {
            ftlv6_entry_count++;
        } else {
            ftll2_entry_count++;
        }
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
ftl_dump_hw_entries (ftl *obj, char *logfile, uint8_t detail, bool v6)
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
                    ftl_dump_hw_entry_detail_iter_cb :
                    ftl_dump_hw_entry_iter_cb;
    params.cbdata = logfp;
    params.force_hwread = false;
    if (v6) {
        params.key_type = KEY_TYPE_IPV6;
        ftlv6_entry_count = 0;
    } else {
        params.key_type = KEY_TYPE_MAC;
        ftll2_entry_count = 0;
    }

    // flow_hash_entry_t::keyheader2str(buf, FTL_ENTRY_STR_MAX - 1);
    // fprintf(logfp, "%s", buf);

    ret = obj->iterate(&params);
    if (ret != SDK_RET_OK) {
        retcode = -1;
    } else {
        if (v6) {
            retcode = ftlv6_entry_count;
        } else {
            retcode = ftll2_entry_count;
        }
    }

    fprintf(logfp, "\n%s", buf);
    fclose(logfp);

end:
    return retcode;
}

static void
ftl_hw_entry_count_cb (sdk_table_api_params_t *params)
{
    flow_hash_entry_t *hwentry =  (flow_hash_entry_t *) params->entry;

    if (hwentry->entry_valid &&
        (params->key_type == hwentry->key_metadata_ktype)) {
        uint64_t *count = (uint64_t *)params->cbdata;
        (*count)++;
    }
}

uint64_t
ftl_get_flow_count (ftl *obj, bool v6)
{
    sdk_ret_t ret;
    sdk_table_api_params_t params = {0};
    uint64_t count = 0;

    params.itercb = ftl_hw_entry_count_cb;
    params.cbdata = &count;
    params.force_hwread = false;
    if (v6) {
        params.key_type = KEY_TYPE_IPV6;
    } else {
        params.key_type = KEY_TYPE_MAC;
    }
    ret = obj->iterate(&params);
    if (ret != SDK_RET_OK) {
        count = ~0L;
    }

    return count;
}

void
ftl_set_session_index (flow_hash_entry_t *entry, uint32_t session)
{
    entry->set_session_index(session);
}

void
ftl_set_epoch (flow_hash_entry_t *entry, uint8_t val)
{
    entry->set_epoch(val);
}

uint32_t
ftl_get_session_id (flow_hash_entry_t *entry)
{
    return entry->get_session_index();
}

uint8_t
ftl_get_proto(flow_hash_entry_t *entry)
{
    return entry->get_key_metadata_proto();
}

void
ftl_set_thread_id (ftl *obj, uint32_t thread_id)
{
    obj->set_thread_id(thread_id);
    return;
}

void
ftl_set_key_lookup_id (flow_hash_entry_t *entry, uint16_t lookup_id)
{
    ftl_set_lookup_id(entry, lookup_id);
}

void
ftl_set_entry_flow_miss_hit (flow_hash_entry_t *entry, uint8_t val)
{
    ftl_set_entry_flow_miss_hit(entry, val);
}

void
ftl_set_entry_nexthop (flow_hash_entry_t *entry, uint32_t nhid, uint32_t nhtype,
                       uint8_t nhvalid)
{
    ftl_set_nexthop(entry, nhid, nhtype, nhvalid);
}

void
ftlv4_set_entry_nexthop (ipv4_flow_hash_entry_t *entry, uint32_t nhid, 
                         uint32_t nhtype, uint8_t nhvalid)
{
    ftlv4_set_nexthop(entry, nhid, nhtype, nhvalid);
}

void
ftlv4_set_key_lookup_id (ipv4_flow_hash_entry_t *entry, uint16_t lookup_id)
{
    ftlv4_set_lookup_id(entry, lookup_id);
}

uint16_t
ftlv4_get_key_lookup_id (ipv4_flow_hash_entry_t *entry)
{
    return ftlv4_get_lookup_id(entry);
}

void
ftlv4_set_entry_flow_miss_hit (ipv4_flow_hash_entry_t *entry, uint8_t val)
{
    ftlv4_set_flow_miss_hit(entry, val);
}

}
