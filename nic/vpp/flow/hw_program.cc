/*
 *  {C} Copyright 2019 Pensando Systems Inc. All rights reserved.
 */

#include <stdint.h>
#include <cstddef>

#include <nic/sdk/include/sdk/table.hpp>
#include <cstring>
#include <nic/utils/ftl/ftlv4.hpp>
#include <nic/utils/ftl/ftlv6.hpp>
#include <nic/sdk/lib/p4/p4_api.hpp>
#include <nic/sdk/platform/capri/capri_tbl_rw.hpp>
#include <nic/sdk/lib/p4/p4_utils.hpp>
#include <pd_utils.h>
#include "hw_program.h"

using namespace sdk;
using namespace sdk::table;
using namespace sdk::platform;

typedef char* (*key2str_t)(void *key);
typedef char* (*appdata2str_t)(void *data);

extern "C" {

#define FTL_ENTRY_STR_MAX   2048

p4pd_table_properties_t g_session_tbl_ctx;

int
initialize_flow(void)
{
    p4pd_error_t p4pd_ret;

    p4pd_ret = p4pd_table_properties_get(P4TBL_ID_SESSION, &g_session_tbl_ctx);
    SDK_ASSERT(p4pd_ret == P4PD_SUCCESS);

    return 0;
}

int
session_program(uint32_t ses_id, void *action)
{
    p4pd_error_t p4pd_ret0;
    uint32_t tableid = P4TBL_ID_SESSION;

    p4pd_ret0 = p4pd_global_entry_write(tableid, ses_id,
                                        NULL, NULL, action);
    if (p4pd_ret0 != P4PD_SUCCESS) {
        return -1;
    }
    return 0;
}
void
session_insert(uint32_t ses_id, void *ses_info)
{
    uint64_t entry_addr = (ses_id * g_session_tbl_ctx.hbm_layout.entry_width);
    uint64_t *src_addr = (uint64_t *)ses_info;

    if (likely(g_session_tbl_ctx.base_mem_va)) {
        uint64_t *dst_addr = (uint64_t *)
                              (g_session_tbl_ctx.base_mem_va + entry_addr);
        for (uint16_t i = 0;
             i < (g_session_tbl_ctx.hbm_layout.entry_width / sizeof(uint64_t)); i++) {
            dst_addr[i] = src_addr[i];
        }
    } else {
        pal_mem_write(g_session_tbl_ctx.base_mem_pa, (uint8_t *)src_addr, 
                      g_session_tbl_ctx.hbm_layout.entry_width);
    }
    capri::capri_hbm_table_entry_cache_invalidate(g_session_tbl_ctx.cache,
                                                  entry_addr,
                                                  g_session_tbl_ctx.hbm_layout.entry_width,
                                                  g_session_tbl_ctx.base_mem_pa);
}

void
session_get_addr(uint32_t ses_id, uint8_t **ses_addr, uint32_t *entry_size)
{
    static thread_local uint8_t *ret_addr =
        (uint8_t *) calloc(g_session_tbl_ctx.hbm_layout.entry_width, 1);
    *entry_size = g_session_tbl_ctx.hbm_layout.entry_width;
    if (likely(g_session_tbl_ctx.base_mem_va)) {
        *ses_addr = (uint8_t *) (g_session_tbl_ctx.base_mem_va +
                    (ses_id * (*entry_size)));
    } else {
       pal_mem_read((g_session_tbl_ctx.base_mem_pa + (ses_id * (*entry_size))),
                    ret_addr, *entry_size);
       *ses_addr = ret_addr;
    }
    return;
}

static void
ftl_print_stats(sdk_table_api_stats_t *api_stats,
                sdk_table_stats_t *table_stats,
                char *buf, int max_len)
{
    char *cur = buf;

    cur += snprintf(buf, max_len,
                    "Insert %u, Insert_fail_dupl %u, Insert_fail %u, "
                    "Insert_fail_recirc %u\n"
                    "Remove %u, Remove_not_found %u, Remove_fail %u\n"
                    "Update %u, Update_fail %u\n"
                    "Get %u, Get_fail %u\n"
                    "Reserve %u, reserve_fail %u\n"
                    "Release %u, Release_fail %u\n"
                    "Tbl_insert %u, Tbl_remove %u, Tbl_read %u, Tbl_write %u\n",
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
                    table_stats->insert, table_stats->remove,
                    table_stats->read, table_stats->write);
    for (int i= 0; i < SDK_TABLE_MAX_RECIRC; i++) {
        cur += snprintf(cur, buf + max_len - cur,
                        "Tbl_lvl %u, Tbl_insert %u, Tbl_remove %u\n",
                        i, table_stats->insert_lvl[i], table_stats->remove_lvl[i]);
    }
}

void
ftl_aggregate_api_stats(sdk_table_api_stats_t *api_stat1,
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
ftl_aggregate_table_stats(sdk_table_stats_t *table_stat1,
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

ftlv4 *
ftlv4_create(void *key2str,
             void *appdata2str,
             uint32_t thread_id)
{
    sdk_table_factory_params_t factory_params = {0};

    factory_params.table_id = P4TBL_ID_IPV4_FLOW;
    factory_params.num_hints = 2;
    factory_params.max_recircs = 8;
    factory_params.key2str = (key2str_t) (key2str);
    factory_params.appdata2str = (appdata2str_t) (appdata2str);
    factory_params.thread_id = thread_id;

    return ftlv4::factory(&factory_params);
}

int
ftlv4_insert(ftlv4 *obj, ftlv4_entry_t *entry, uint32_t hash)
{
    sdk_table_api_params_t params = {0};

    if (hash) {
        params.hash_32b = hash;
        params.hash_valid = 1;
    }
    params.entry = entry;
    if (SDK_RET_OK != obj->insert(&params)) {
        return -1;
    }
    return 0;
}

int
ftlv4_remove(ftlv4 *obj, ftlv4_entry_t *entry, uint32_t hash)
{
    sdk_table_api_params_t params = {0};

    if (hash) {
        params.hash_32b = hash;
        params.hash_valid = 1;
    }
    params.entry = entry;
    if (SDK_RET_OK != obj->remove(&params)) {
        return -1;
    }
    return 0;
}

int
ftlv4_clear(ftlv4 *obj, bool clear_global_state,
            bool clear_thread_local_state)
{
    if (SDK_RET_OK != obj->clear(clear_global_state,
                                 clear_thread_local_state)) {
        return -1;
    }
    return 0;
}

void
ftlv4_delete(ftlv4 *obj)
{
    ftlv4::destroy(obj);
}

uint32_t ftlv4_entry_count;

static void
ftlv4_dump_hw_entry_iter_cb(sdk_table_api_params_t *params)
{
    ftlv4_entry_t *hwentry =  (ftlv4_entry_t *) params->entry;
    FILE *fp = (FILE *)params->cbdata;
    char buf[FTL_ENTRY_STR_MAX];

    if (hwentry->entry_valid) {
        ftlv4_entry_count++;
        buf[FTL_ENTRY_STR_MAX - 1] = 0;
        hwentry->key2str(buf, FTL_ENTRY_STR_MAX - 1);
        fprintf(fp, "%s", buf);
    }
}

static void
ftlv4_dump_hw_entry_detail_iter_cb(sdk_table_api_params_t *params)
{
    ftlv4_entry_t *hwentry =  (ftlv4_entry_t *) params->entry;
    FILE *fp = (FILE *)params->cbdata;
    uint8_t *entry;
    uint32_t size;
    char buf[FTL_ENTRY_STR_MAX];

    if (hwentry->entry_valid) {
        ftlv4_entry_count++;
        buf[FTL_ENTRY_STR_MAX - 1] = 0;
        hwentry->tostr(buf, FTL_ENTRY_STR_MAX - 1);
        fprintf(fp, "%s", buf);
        session_get_addr(hwentry->get_session_index(), &entry, &size);
        fprintf(fp, " Session data: ");
        for (uint32_t i = 0; i < size; i++) {
            fprintf(fp, "%x", entry[i]);
        }
        fprintf(fp, "\n");
    }
}

int
ftlv4_dump_hw_entries(ftlv4 *obj, char *logfile, uint8_t detail)
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
        ftlv4_entry_t::keyheader2str(buf, FTL_ENTRY_STR_MAX - 1);
        fprintf(logfp, "%s", buf);
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
ftlv4_dump_hw_entry(ftlv4 *obj, uint32_t src, uint32_t dst,
                    uint8_t ip_proto, uint16_t sport,
                    uint16_t dport, uint16_t lookup_id,
                    char *buf, int max_len)
{
    sdk_ret_t ret;
    sdk_table_api_params_t params = {0};
    int retcode = 0;
    ftlv4_entry_t entry;

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
ftlv4_dump_stats(ftlv4 *obj, char *buf, int max_len)
{
    sdk_table_api_stats_t api_stats;
    sdk_table_stats_t table_stats;

    obj->stats_get(&api_stats, &table_stats);
    ftl_print_stats(&api_stats, &table_stats, buf, max_len);
}

static void
ftlv4_hw_entry_count_cb(sdk_table_api_params_t *params)
{
    ftlv4_entry_t *hwentry =  (ftlv4_entry_t *) params->entry;

    if (hwentry->entry_valid) {
        uint64_t *count = (uint64_t *)params->cbdata;
        (*count)++;
    }
}

uint64_t
ftlv4_get_flow_count(ftlv4 *obj)
{
    sdk_ret_t ret;
    sdk_table_api_params_t params = {0};
    uint64_t count = 0;

    params.itercb = ftlv4_hw_entry_count_cb;
    params.cbdata = &count;
    params.force_hwread = true;

    ret = obj->iterate(&params);
    if (ret != SDK_RET_OK) {
        count = ~0L;
    }

    return count;
}

void
ftlv4_dump_stats_summary(ftlv4 **obj_arr, uint32_t obj_count,
                         char *buf, int max_len)
{
    sdk_table_api_stats_t api_stats, api_tmp_stats;
    sdk_table_stats_t table_stats, table_tmp_stats;
    ftlv4 *obj = NULL;

    if (!obj_count || !obj_arr || !obj_arr[0]) {
        return;
    }

    obj = obj_arr[0];
    obj->stats_get(&api_stats, &table_stats);

    for (uint32_t j = 1; j < obj_count; j++) {
        obj = obj_arr[j];
        obj->stats_get(&api_tmp_stats, &table_tmp_stats);

        ftl_aggregate_api_stats(&api_stats, &api_tmp_stats);
        ftl_aggregate_table_stats(&table_stats, &table_tmp_stats);
    }
    ftl_print_stats(&api_stats, &table_stats, buf, max_len);
}

void ftlv4_set_session_index(ftlv4_entry_t *entry, uint32_t session)
{
    entry->set_session_index(session);
}

void ftlv4_set_epoch(ftlv4_entry_t *entry, uint8_t val)
{
    entry->set_epoch(val);
}

void ftlv4_set_key(ftlv4_entry_t *entry,
             uint32_t sip,
             uint32_t dip,
             uint8_t ip_proto,
             uint16_t src_port,
             uint16_t dst_port,
             uint16_t lookup_id)
{
    entry->set_key(sip, dip, ip_proto, src_port, dst_port, lookup_id);
}

uint32_t
ftlv4_get_session_id(ftlv4_entry_t *entry)
{
    return entry->get_session_index();
}

ftlv6 *
ftlv6_create(void *key2str,
             void *appdata2str,
             uint32_t thread_id)
{
    sdk_table_factory_params_t factory_params = {0};

    factory_params.table_id = P4TBL_ID_FLOW;
    factory_params.num_hints = 4;
    factory_params.max_recircs = 8;
    factory_params.key2str = (key2str_t) (key2str);
    factory_params.appdata2str = (appdata2str_t) (appdata2str);
    factory_params.thread_id = thread_id;

    return ftlv6::factory(&factory_params);
}

int
ftlv6_insert(ftlv6 *obj, ftlv6_entry_t *entry, uint32_t hash)
{
    sdk_table_api_params_t params = {0};

    if (hash) {
        params.hash_32b = hash;
        params.hash_valid = 1;
    }
    params.entry = entry;
    if (SDK_RET_OK != obj->insert(&params)) {
        return -1;
    }
    return 0;
}

int
ftlv6_remove(ftlv6 *obj, ftlv6_entry_t *entry, uint32_t hash)
{
    sdk_table_api_params_t params = {0};

    if (hash) {
        params.hash_32b = hash;
        params.hash_valid = 1;
    }
    params.entry = entry;
    if (SDK_RET_OK != obj->remove(&params)) {
        return -1;
    }
    return 0;
}

int
ftlv6_clear(ftlv6 *obj, bool clear_global_state,
            bool clear_thread_local_state)
{
    if (SDK_RET_OK != obj->clear(clear_global_state,
                                 clear_thread_local_state)) {
        return -1;
    }
    return 0;
}

void
ftlv6_delete(ftlv6 *obj)
{
    ftlv6::destroy(obj);
}

uint32_t ftlv6_entry_count;

static void
ftlv6_dump_hw_entry_iter_cb(sdk_table_api_params_t *params)
{
    ftlv6_entry_t *hwentry =  (ftlv6_entry_t *) params->entry;
    FILE *fp = (FILE *)params->cbdata;
    char buf[FTL_ENTRY_STR_MAX];

    if (hwentry->entry_valid) {
        ftlv6_entry_count++;
        buf[FTL_ENTRY_STR_MAX - 1] = 0;
        hwentry->key2str(buf, FTL_ENTRY_STR_MAX - 1);
        fprintf(fp, "%s", buf);
    }
}

static void
ftlv6_dump_hw_entry_detail_iter_cb(sdk_table_api_params_t *params)
{
    ftlv6_entry_t *hwentry =  (ftlv6_entry_t *) params->entry;
    FILE *fp = (FILE *)params->cbdata;
    uint8_t *entry;
    uint32_t size;
    char buf[FTL_ENTRY_STR_MAX];

    if (hwentry->entry_valid) {
        ftlv6_entry_count++;
        buf[FTL_ENTRY_STR_MAX - 1] = 0;
        hwentry->tostr(buf, FTL_ENTRY_STR_MAX - 1);
        fprintf(fp, "%s", buf);
        session_get_addr(hwentry->get_session_index(), &entry, &size);
        fprintf(fp, " Session data: ");
        for (uint32_t i = 0; i < size; i++) {
            fprintf(fp, "%x", entry[i]);
        }
        fprintf(fp, "\n");
    }
}

int
ftlv6_dump_hw_entries(ftlv6 *obj, char *logfile, uint8_t detail)
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
                    ftlv6_dump_hw_entry_detail_iter_cb :
                    ftlv6_dump_hw_entry_iter_cb;
    params.cbdata = logfp;
    params.force_hwread = false;
    ftlv6_entry_count = 0;

    ftlv6_entry_t::keyheader2str(buf, FTL_ENTRY_STR_MAX - 1);
    fprintf(logfp, "%s", buf);

    ret = obj->iterate(&params);
    if (ret != SDK_RET_OK) {
        retcode = -1;
    } else {
        retcode = ftlv6_entry_count;
    }

    fprintf(logfp, "\n%s", buf);
    fclose(logfp);

end:
    return retcode;
}

int
ftlv6_dump_hw_entry(ftlv6 *obj, uint8_t *src, uint8_t *dst,
                    uint8_t ip_proto, uint16_t sport,
                    uint16_t dport, uint16_t lookup_id,
                    char *buf, int max_len)
{
    sdk_ret_t ret;
    sdk_table_api_params_t params = {0};
    int retcode = 0;
    ftlv6_entry_t entry = {0};

    ftlv6_set_key(&entry, src, dst, ip_proto, sport, dport, lookup_id, 0);
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

static void
ftlv6_hw_entry_count_cb(sdk_table_api_params_t *params)
{
    ftlv6_entry_t *hwentry =  (ftlv6_entry_t *) params->entry;

    if (hwentry->entry_valid) {
        uint64_t *count = (uint64_t *)params->cbdata;
        (*count)++;
    }
}

uint64_t
ftlv6_get_flow_count(ftlv6 *obj)
{
    sdk_ret_t ret;
    sdk_table_api_params_t params = {0};
    uint64_t count = 0;

    params.itercb = ftlv6_hw_entry_count_cb;
    params.cbdata = &count;
    params.force_hwread = true;

    ret = obj->iterate(&params);
    if (ret != SDK_RET_OK) {
        count = ~0L;
    }

    return count;
}

void
ftlv6_dump_stats(ftlv6 *obj, char *buf, int max_len)
{
    sdk_table_api_stats_t api_stats;
    sdk_table_stats_t table_stats;

    obj->stats_get(&api_stats, &table_stats);
    ftl_print_stats(&api_stats, &table_stats, buf, max_len);
}

void
ftlv6_dump_stats_summary(ftlv6 **obj_arr, uint32_t obj_count,
                         char *buf, int max_len)
{
    sdk_table_api_stats_t api_stats, api_tmp_stats;
    sdk_table_stats_t table_stats, table_tmp_stats;
    ftlv6 *obj = NULL;

    if (!obj_count || !obj_arr || !obj_arr[0]) {
        return;
    }

    obj = obj_arr[0];
    obj->stats_get(&api_stats, &table_stats);

    for (uint32_t j = 1; j < obj_count; j++) {
        obj = obj_arr[j];
        obj->stats_get(&api_tmp_stats, &table_tmp_stats);

        ftl_aggregate_api_stats(&api_stats, &api_tmp_stats);
        ftl_aggregate_table_stats(&table_stats, &table_tmp_stats);
    }
    ftl_print_stats(&api_stats, &table_stats, buf, max_len);
}

void ftlv6_set_session_index(ftlv6_entry_t *entry, uint32_t session)
{
    entry->set_session_index(session);
}

void ftlv6_set_epoch(ftlv6_entry_t *entry, uint8_t val)
{
    entry->set_epoch(val);
}

void ftlv6_set_key(ftlv6_entry_t *entry,
                   uint8_t *sip,
                   uint8_t *dip,
                   uint8_t ip_proto,
                   uint16_t src_port,
                   uint16_t dst_port,
                   uint16_t lookup_id,
                   uint8_t key_type)
{   
    entry->set_key(sip, dip, ip_proto,
                   src_port, dst_port,
                   lookup_id, key_type);
}

uint32_t
ftlv6_get_session_id(ftlv6_entry_t *entry)
{
    return entry->get_session_index();
}

}
