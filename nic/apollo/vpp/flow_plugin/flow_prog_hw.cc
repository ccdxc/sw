/*
 *  {C} Copyright 2019 Pensando Systems Inc. All rights reserved.
 */

#include <stdint.h>
#include <cstddef>

#include <nic/sdk/include/sdk/table.hpp>
#include <cstring>
#include <nic/utils/ftl/ftlv4.hpp>
#include <nic/utils/ftl/ftlv6.hpp>
#include <nic/utils/ftl/ftl_structs.hpp>
#include <nic/sdk/include/sdk/ip.hpp>
#include <nic/sdk/include/sdk/base.hpp>
#include <nic/sdk/include/sdk/types.hpp>
#include <nic/sdk/include/sdk/platform.hpp>
#include <nic/sdk/platform/capri/csrint/csr_init.hpp>
#include <nic/sdk/platform/capri/capri_state.hpp>
#include <nic/sdk/lib/pal/pal.hpp>
#include <nic/sdk/asic/rw/asicrw.hpp>
#include <nic/sdk/lib/p4/p4_utils.hpp>
#include <nic/sdk/lib/p4/p4_api.hpp>
#ifdef APOLLO
#define PDS_PLATFORM "apollo"
#include <gen/p4gen/apollo/include/p4pd.h>
#elif ARTEMIS
#define PDS_PLATFORM "artemis"
#include <gen/p4gen/artemis/include/p4pd.h>
#endif
#include <nic/p4/common/defines.h>
#include <nic/sdk/platform/capri/capri_p4.hpp>
#include <nic/sdk/platform/capri/capri_tbl_rw.hpp>
#include <nic/sdk/asic/pd/pd.hpp>
#include "flow_prog_hw.h"

using namespace sdk;
using namespace sdk::table;
using namespace sdk::platform;

typedef char* (*key2str_t)(void *key);
typedef char* (*appdata2str_t)(void *data);

extern "C" {

p4pd_table_properties_t g_session_tbl_ctx;

int
initialize_pds(void)
{
    pal_ret_t    pal_ret;
    p4pd_error_t p4pd_ret;
    capri_cfg_t  capri_cfg;
    sdk_ret_t    ret;

    p4pd_cfg_t p4pd_cfg = {
        .table_map_cfg_file  = PDS_PLATFORM "/capri_p4_table_map.json",
        .p4pd_pgm_name       = PDS_PLATFORM "_p4",
        .p4pd_rxdma_pgm_name = PDS_PLATFORM "_rxdma",
        .p4pd_txdma_pgm_name = PDS_PLATFORM "_txdma",
        .cfg_path = std::getenv("HAL_CONFIG_PATH")
    };
    p4pd_cfg_t p4pd_rxdma_cfg = {
        .table_map_cfg_file  = PDS_PLATFORM "/capri_rxdma_table_map.json",
        .p4pd_pgm_name       = PDS_PLATFORM "_p4",
        .p4pd_rxdma_pgm_name = PDS_PLATFORM "_rxdma",
        .p4pd_txdma_pgm_name = PDS_PLATFORM "_txdma",
        .cfg_path = std::getenv("HAL_CONFIG_PATH")
    };
    p4pd_cfg_t p4pd_txdma_cfg = {
        .table_map_cfg_file  = PDS_PLATFORM "/capri_txdma_table_map.json",
        .p4pd_pgm_name       = PDS_PLATFORM "_p4",
        .p4pd_rxdma_pgm_name = PDS_PLATFORM "_rxdma",
        .p4pd_txdma_pgm_name = PDS_PLATFORM "_txdma",
        .cfg_path = std::getenv("HAL_CONFIG_PATH")
    };

    /* initialize PAL */
    pal_ret = sdk::lib::pal_init(platform_type_t::PLATFORM_TYPE_HW);
    SDK_ASSERT(pal_ret == sdk::lib::PAL_RET_OK);

    memset(&capri_cfg, 0, sizeof(capri_cfg_t));
    capri_cfg.cfg_path = std::string(std::getenv("HAL_CONFIG_PATH"));
    std::string mpart_json = capri_cfg.cfg_path + "/" + PDS_PLATFORM + "/hbm_mem.json";
    capri_cfg.mempartition =
        sdk::platform::utils::mpartition::factory(mpart_json.c_str());

    /* do capri_state_pd_init needed by sdk capri
     * csr init is done inside capri_state_pd_init */
    sdk::platform::capri::capri_state_pd_init(&capri_cfg);

    /* do apollo specific initialization */
    p4pd_ret = p4pd_init(&p4pd_cfg);
    SDK_ASSERT(p4pd_ret == P4PD_SUCCESS);

    p4pd_ret = p4pluspd_rxdma_init(&p4pd_rxdma_cfg);
    SDK_ASSERT(p4pd_ret == P4PD_SUCCESS);

    p4pd_ret = p4pluspd_txdma_init(&p4pd_txdma_cfg);
    SDK_ASSERT(p4pd_ret == P4PD_SUCCESS);

    ret = sdk::asic::pd::asicpd_program_hbm_table_base_addr();
    SDK_ASSERT(ret == SDK_RET_OK);

    capri::capri_table_csr_cache_inval_init();

    p4pd_ret = p4pd_table_properties_get(P4TBL_ID_SESSION, &g_session_tbl_ctx);
    SDK_ASSERT(p4pd_ret == P4PD_SUCCESS);

    return 0;
}

void
session_insert(uint32_t ses_id, void *ses_info)
{
    uint64_t entry_addr = (ses_id * g_session_tbl_ctx.hbm_layout.entry_width);
    uint64_t *src_addr = (uint64_t *)ses_info;
    uint64_t *dst_addr = (uint64_t *)
                         (g_session_tbl_ctx.base_mem_va + entry_addr);
    for (int i = 0;
         i < (g_session_tbl_ctx.hbm_layout.entry_width / sizeof(uint64_t)); i++) {
        dst_addr[i] = src_addr[i];
    }
    capri::capri_hbm_table_entry_cache_invalidate(g_session_tbl_ctx.cache,
                                                  entry_addr,
                                                  g_session_tbl_ctx.hbm_layout.entry_width,
                                                  g_session_tbl_ctx.base_mem_pa);
}

void
session_get_addr(uint32_t ses_id, uint8_t **ses_addr, uint32_t *entry_size)
{
    *entry_size = g_session_tbl_ctx.hbm_layout.entry_width;
    *ses_addr = (uint8_t *) (g_session_tbl_ctx.base_mem_va +
                (ses_id * (*entry_size)));
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
                    "Tbl_insert %u, Tbl_remove %u, Tbl_read %u, Tbl_write %u\n"
                    "Current HW Entries %u\n",
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
                    table_stats->read, table_stats->write, table_stats->entries);
    for (int i= 0; i < 8; i++) {
        cur += snprintf(cur, buf + max_len - cur,
                        "Tbl_lvl %u, Tbl_insert %u, Tbl_remove %u\n",
                        i, table_stats->insert_lvl[i], table_stats->remove_lvl[i]);
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

    if (hwentry->entry_valid) {
        ftlv4_entry_count++;
        hwentry->tofile(fp, ftlv4_entry_count);
    }
}

static void
ftlv4_dump_hw_entry_detail_iter_cb(sdk_table_api_params_t *params)
{
    ftlv4_entry_t *hwentry =  (ftlv4_entry_t *) params->entry;
    FILE *fp = (FILE *)params->cbdata;
    uint8_t *entry;
    uint32_t size;

    if (hwentry->entry_valid) {
        ftlv4_dump_hw_entry_iter_cb(params);
        session_get_addr(hwentry->session_index, &entry, &size);
        fprintf(fp, "Session Id - %u\nSession data:\n", hwentry->session_index);
        for (uint32_t i = 0; i < size; i++) {
            fprintf(fp, "%x", entry[i]);
        }
        fprintf(fp, "\n");
    }
}

int
ftlv4_dump_hw_entries(ftlv4 *obj, char *logfile, uint8_t detail)
{
    if (!detail) {
        return obj->hwentries_dump(logfile);
    }
    sdk_ret_t ret;
    sdk_table_api_params_t params = {0};
    FILE *logfp = fopen(logfile, "a");
    int retcode = -1;

    if (logfp == NULL) {
        goto end;
    }
    params.itercb = ftlv4_dump_hw_entry_detail_iter_cb;
    params.cbdata = logfp;
    ftlv4_entry_count = 0;

    fprintf(logfp, "*******FTLv4 Table:*******\n");
    fprintf(logfp, "%8s\t%16s\t%16s\t%5s\t%5s\t%3s\t%4s\n",
            "Entry", "SrcIP", "DstIP", "SrcPort", "DstPort", "Proto", "Vnic");
    fprintf(logfp, "%8s\t%16s\t%16s\t%5s\t%5s\t%3s\t%4s\n",
            "-----", "-----", "-----", "-------", "-------", "-----", "----");
    ret = obj->iterate(&params, TRUE);
    if (ret != SDK_RET_OK) {
        retcode = -1;
    } else {
        retcode = ftlv4_entry_count;
    }
    fprintf(logfp, "%8s\t%16s\t%16s\t%5s\t%5s\t%3s\t%4s\n",
            "Entry", "SrcIP", "DstIP", "SrcPort", "DstPort", "Proto", "Vnic");
    fprintf(logfp, "%8s\t%16s\t%16s\t%5s\t%5s\t%3s\t%4s\n",
            "-----", "-----", "-----", "-------", "-------", "-----", "----");
    fclose(logfp);

end:
    return retcode;
}

void
ftlv4_dump_stats(ftlv4 *obj, char *buf, int max_len, bool force_hwread)
{
    sdk_table_api_stats_t api_stats;
    sdk_table_stats_t table_stats;

    obj->stats_get(&api_stats, &table_stats, force_hwread);
    ftl_print_stats(&api_stats, &table_stats, buf, max_len);
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

    if (hwentry->entry_valid) {
        ftlv6_entry_count++;
        hwentry->tofile(fp, ftlv6_entry_count);
    }
}

static void
ftlv6_dump_hw_entry_detail_iter_cb(sdk_table_api_params_t *params)
{
    ftlv6_entry_t *hwentry =  (ftlv6_entry_t *) params->entry;
    FILE *fp = (FILE *)params->cbdata;
    uint8_t *entry;
    uint32_t size;

    if (hwentry->entry_valid) {
        ftlv6_dump_hw_entry_iter_cb(params);
        session_get_addr(hwentry->session_index, &entry, &size);
        fprintf(fp, "Session Id - %u\nSession data:\n", hwentry->session_index);
        for (uint32_t i = 0; i < size; i++) {
            fprintf(fp, "%x", entry[i]);
        }
        fprintf(fp, "\n");
    }
}

int
ftlv6_dump_hw_entries(ftlv6 *obj, char *logfile, uint8_t detail)
{
    if (!detail) {
        return obj->hwentries_dump(logfile);
    }
    sdk_ret_t ret;
    sdk_table_api_params_t params = {0};
    FILE *logfp = fopen(logfile, "a");
    int retcode = -1;

    if (logfp == NULL) {
        goto end;
    }
    params.itercb = ftlv6_dump_hw_entry_detail_iter_cb;
    params.cbdata = logfp;
    ftlv6_entry_count = 0;

    fprintf(logfp, "*******FTLv6 Table:*******\n");
    fprintf(logfp, "%8s\t%16s\t%16s\t%5s\t%5s\t%3s\t%4s\n",
            "Entry", "SrcIP", "DstIP", "SrcPort", "DstPort", "Proto", "Vnic");
    fprintf(logfp, "%8s\t%16s\t%16s\t%5s\t%5s\t%3s\t%4s\n",
            "-----", "-----", "-----", "-------", "-------", "-----", "----");
    ret = obj->iterate(&params, TRUE);
    if (ret != SDK_RET_OK) {
        retcode = -1;
    } else {
        retcode = ftlv6_entry_count;
    }
    fprintf(logfp, "%8s\t%16s\t%16s\t%5s\t%5s\t%3s\t%4s\n",
            "Entry", "SrcIP", "DstIP", "SrcPort", "DstPort", "Proto", "Vnic");
    fprintf(logfp, "%8s\t%16s\t%16s\t%5s\t%5s\t%3s\t%4s\n",
            "-----", "-----", "-----", "-------", "-------", "-----", "----");
    fclose(logfp);

end:
    return retcode;
}

void
ftlv6_dump_stats(ftlv6 *obj, char *buf, int max_len, bool force_hwread)
{
    sdk_table_api_stats_t api_stats;
    sdk_table_stats_t table_stats;

    obj->stats_get(&api_stats, &table_stats, force_hwread);
    ftl_print_stats(&api_stats, &table_stats, buf, max_len);
}

}
