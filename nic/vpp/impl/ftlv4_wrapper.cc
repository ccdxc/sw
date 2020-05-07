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
#include "nic/operd/decoders/vpp/flow_decoder.h"
#include <nat.h>
#include <pds_table.h>

// TODO: Move to a common header in nat
#define PDS_DYNAMIC_NAT_START_INDEX 1000

using namespace sdk;
using namespace sdk::table;
using namespace sdk::platform;

extern "C" {

extern session_update_cb g_ses_cb;

typedef struct ftlv4_cache_s {
    ipv4_flow_hash_entry_t ip4_flow[MAX_FLOW_ENTRIES_PER_BATCH];
    ipv4_flow_hash_entry_t ip4_last_read_flow;
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
              uint32_t *pindex, uint32_t *sindex, uint8_t update)
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
        goto done;
    }

    if (SDK_RET_OK != obj->insert(&params)) {
        return -1;
    }

done:
    if (params.handle.svalid()) {
        *pindex = (uint32_t) (~0L);
        *sindex = params.handle.sindex();
    } else {
        *pindex = params.handle.pindex();
    }
    return 0;
}

static void
ftlv4_move_cb (base_table_entry_t *entry, handle_t old_handle, 
               handle_t new_handle, bool move_complete)
{
    ipv4_flow_hash_entry_t *v4entry = (ipv4_flow_hash_entry_t *)entry;
    uint32_t ses_id = v4entry->get_session_index();
    uint32_t new_pindex, new_sindex = ~0;

    if (new_handle.svalid()) {
        new_pindex = (uint32_t) (~0L);
        new_sindex = new_handle.sindex();
    } else {
        new_pindex = new_handle.pindex();
    }

    if (v4entry->get_flow_role() == TCP_FLOW_INITIATOR) {
        g_ses_cb (ses_id, new_pindex, new_sindex, true, 
                  move_complete);
    } else {
        g_ses_cb (ses_id, new_pindex, new_sindex, false,
                  move_complete);
    }
}

int
ftlv4_remove (ftlv4 *obj, ipv4_flow_hash_entry_t *entry, uint32_t hash)
{
    sdk_table_api_params_t params = {0};

    if (get_skip_ftl_program()) {
        return 0;
    }

    if (hash) {
        params.hash_32b = hash;
        params.hash_valid = 1;
    }
    params.movecb = ftlv4_move_cb;
    params.entry = entry;
    if (SDK_RET_OK != obj->remove(&params)) {
        return -1;
    }
    return 0;
}

int
ftlv4_get_with_handle (ftlv4 *obj, uint32_t index, bool primary)
{
    sdk_table_api_params_t params = {0};
    ipv4_flow_hash_entry_t *v4entry = &g_ip4_flow_cache.ip4_last_read_flow;

    v4entry->clear();

    if (get_skip_ftl_program()) {
        return 0;
    }

    if (primary) {
        params.handle.pindex(index);
    } else {
        params.handle.sindex(index);
    }
    params.entry = v4entry;

    if (SDK_RET_OK != obj->get_with_handle(&params)) {
        return -1;
    }

    return 0;
}

int
ftlv4_update_cached_entry (ftlv4 *obj)
{
    uint32_t pindex;
    uint32_t sindex;
    return ftlv4_insert(obj, &g_ip4_flow_cache.ip4_last_read_flow, 0,
                        &pindex, &sindex, 1);
}

int
ftlv4_remove_cached_entry (ftlv4 *obj)
{
    return ftlv4_remove(obj, &g_ip4_flow_cache.ip4_last_read_flow, 0);
}

static inline sdk::sdk_ret_t
ftlv4_read_with_handle (ftlv4 *obj, uint32_t index, bool primary,
                        ipv4_flow_hash_entry_t &entry)
{
    sdk::sdk_ret_t ret;
    sdk_table_api_params_t params = {0};

    if (primary) {
        params.handle.pindex(index);
    } else {
        params.handle.sindex(index);
    }
    params.entry = &entry;

    ret = obj->get_with_handle(&params);
    return ret;
}

int
ftlv4_dump_entry_with_handle (ftlv4 *obj, uint32_t index, bool primary,
                              v4_flow_info_t *flow_info)
{
    sdk::sdk_ret_t ret;
    ipv4_flow_hash_entry_t entry;

    ret = ftlv4_read_with_handle(obj, index, primary, entry);
    if (ret != SDK_RET_OK) {
        return -1;
    }

    flow_info->key_metadata_ipv4_src = entry.key_metadata_ipv4_src;
    flow_info->key_metadata_ipv4_dst = entry.key_metadata_ipv4_dst;
    flow_info->key_metadata_flow_lkp_id = ftlv4_get_key_lookup_id(&entry);
    flow_info->key_metadata_dport = entry.key_metadata_dport;
    flow_info->key_metadata_sport = entry.key_metadata_sport;
    flow_info->key_metadata_proto = entry.key_metadata_proto;
    flow_info->epoch = ftlv4_get_entry_epoch(&entry);
    flow_info->session_index = entry.session_index;
    flow_info->nexthop_valid = ftlv4_get_entry_nexthop_valid(&entry);
    flow_info->nexthop_type = ftlv4_get_entry_nexthop_type(&entry);
    flow_info->nexthop_id = ftlv4_get_entry_nexthop_id(&entry);
    flow_info->nexthop_priority = ftlv4_get_entry_nexthop_priority(&entry);
    return 0;
}

static inline int
ftlv4_read_snat_info (uint32_t session_id, bool host_origin,
                      const operd_flow_key_v4_t &flow_key,
                      operd_flow_nat_data_t &nat_data)
{
    uint16_t rx_xlate_id, tx_xlate_id;
    uint16_t rx_xlate_id2, tx_xlate_id2;
    int ret;

    pds_session_get_xlate_ids(session_id, &rx_xlate_id, &tx_xlate_id,
                               &rx_xlate_id2, &tx_xlate_id2);

    if (host_origin) {
        if (tx_xlate_id) {
            ret = pds_snat_tbl_read_ip4(tx_xlate_id, &nat_data.src_nat_addr,
                                        &nat_data.src_nat_port);
            if (ret != 0) {
                return -1;
            }
        }

        if (tx_xlate_id2) {
            ret = pds_snat_tbl_read_ip4(tx_xlate_id2, &nat_data.dst_nat_addr,
                                        &nat_data.dst_nat_port);
            if (ret != 0) {
                return -1;
            }
        }
    } else {
        if (rx_xlate_id) {
            ret = pds_snat_tbl_read_ip4(rx_xlate_id, &nat_data.dst_nat_addr,
                                        &nat_data.dst_nat_port);
            if (ret != 0) {
                return -1;
            }
        }

        if (rx_xlate_id2) {
            ret = pds_snat_tbl_read_ip4(rx_xlate_id2, &nat_data.src_nat_addr,
                                        &nat_data.src_nat_port);
            if (ret != 0) {
                return -1;
            }
        }
    }
    return 0;
}

int
ftlv4_export_with_entry (ipv4_flow_hash_entry_t *iv4entry,
                         ipv4_flow_hash_entry_t *rv4entry,
                         uint8_t reason, bool host_origin)
{
    operd_flow_t flow = {0};
    pds_session_stats_t session_stats;

    flow.v4.src = iv4entry->get_key_metadata_ipv4_src();
    flow.v4.sport = iv4entry->get_key_metadata_sport();
    flow.v4.dst = rv4entry->get_key_metadata_ipv4_src();
    flow.v4.dport = rv4entry->get_key_metadata_sport();
    flow.v4.proto = iv4entry->get_key_metadata_proto();
    flow.v4.lookup_id = ftlv4_get_key_lookup_id(iv4entry);

    flow.session_id = iv4entry->get_session_index();

    if (0 != ftlv4_read_snat_info(flow.session_id, host_origin, flow.v4,
                                  flow.nat_data)) {
        return -1;
    }

    if (FLOW_EXPORT_REASON_ADD != reason) {
        // don't read stats for add. they would be zero anyway
        if (0 != pds_session_stats_read(flow.session_id, &session_stats)) {
            return -1;
        }
        memcpy(&flow.stats, &session_stats, sizeof(pds_session_stats_t));
    }

    switch(reason) {
    case FLOW_EXPORT_REASON_ADD:
        flow.logtype = OPERD_FLOW_LOGTYPE_ADD;
        break;
    case FLOW_EXPORT_REASON_DEL:
        flow.logtype = OPERD_FLOW_LOGTYPE_DEL;
        break;
    default:
    case FLOW_EXPORT_REASON_ACTIVE:
        flow.logtype = OPERD_FLOW_LOGTYPE_ACTIVE;
        break;
    }

    flow.type = OPERD_FLOW_TYPE_IP4;
    flow.action = OPERD_FLOW_ACTION_ALLOW;

    pds_operd_export_flow_ip4(&flow);

    return 0;
}

int
ftlv4_export_with_handle (ftlv4 *obj, uint32_t iflow_index, bool iflow_primary,
                          uint32_t rflow_index, bool rflow_primary,
                          uint8_t reason, bool host_origin)
{
    ipv4_flow_hash_entry_t iv4entry, rv4entry;
    sdk_ret_t ret;

    if (get_skip_ftl_program()) {
        return 0;
    }

    ret = ftlv4_read_with_handle(obj, iflow_index, iflow_primary, iv4entry);
    if (SDK_RET_OK != ret) {
        return -1;
    }
    ret = ftlv4_read_with_handle(obj, rflow_index, rflow_primary, rv4entry);
    if (SDK_RET_OK != ret) {
        return -1;
    }

    ftlv4_export_with_entry(&iv4entry, &rv4entry, reason, host_origin);

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

static int
ftlv4_read_hw_entry (ftlv4 *obj, uint32_t src, uint32_t dst,
                     uint8_t ip_proto, uint16_t sport,
                     uint16_t dport, uint16_t lookup_id,
                     ipv4_flow_hash_entry_t *entry)
{
    sdk_ret_t ret;
    sdk_table_api_params_t params = {0};
    int retcode = 0;

    entry->clear();
    ftlv4_set_key(entry, src, dst, ip_proto, sport, dport, lookup_id);
    params.entry = entry;

    ret = obj->get(&params);
    if (ret != SDK_RET_OK) {
        retcode = -1;
    }

    return retcode;
}

int
ftlv4_read_session_index (ftlv4 *obj, uint32_t src, uint32_t dst,
                          uint8_t ip_proto, uint16_t sport,
                          uint16_t dport, uint16_t lookup_id,
                          uint32_t *ses_id)
{
    int retcode = 0;
    ipv4_flow_hash_entry_t entry;

    retcode = ftlv4_read_hw_entry(obj, src, dst, ip_proto, sport, dport, lookup_id,
                                  &entry);
    if (retcode != -1) {
        *ses_id = entry.session_index;
    }
    return retcode;
}

int
ftlv4_dump_hw_entry (ftlv4 *obj, uint32_t src, uint32_t dst,
                     uint8_t ip_proto, uint16_t sport,
                     uint16_t dport, uint16_t lookup_id,
                     char *buf, int max_len)
{
    int retcode = 0;
    ipv4_flow_hash_entry_t entry;

    retcode = ftlv4_read_hw_entry(obj, src, dst, ip_proto, sport, dport, lookup_id,
                                  &entry);
    if (retcode != -1) {
        entry.tostr(buf, max_len);
    }
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
ftlv4_set_flow_role(ipv4_flow_hash_entry_t *entry, uint8_t flow_role)
{
    entry->set_flow_role(flow_role);
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
ftlv4_cache_set_napt_flag (uint8_t napt)
{
    g_ip4_flow_cache.flags[g_ip4_flow_cache.count].napt = napt;
}

uint8_t
ftlv4_cache_get_napt_flag (int id)
{
    return g_ip4_flow_cache.flags[id].napt;
}

void
ftlv4_cache_set_host_origin (uint8_t host_origin)
{
    g_ip4_flow_cache.flags[g_ip4_flow_cache.count].host_origin = host_origin & 1;
}

void
ftlv4_cache_set_flow_miss_hit (uint8_t val)
{
    ftlv4_set_entry_flow_miss_hit(g_ip4_flow_cache.ip4_flow + g_ip4_flow_cache.count,
                                  val);
}

void
ftlv4_cache_set_flow_role(uint8_t flow_role)
{
    ftlv4_set_flow_role(g_ip4_flow_cache.ip4_flow + g_ip4_flow_cache.count,
                        flow_role);
}

void
ftlv4_cache_set_counter_index (uint8_t ctr_idx)
{
    g_ip4_flow_cache.flags[g_ip4_flow_cache.count].ctr_idx = ctr_idx;
}

uint8_t
ftlv4_cache_get_counter_index (int id)
{
    return g_ip4_flow_cache.flags[id].ctr_idx;
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
                         uint8_t nh_valid,
                         uint8_t priority)
{
   ftlv4_set_entry_nexthop(g_ip4_flow_cache.ip4_flow + g_ip4_flow_cache.count,
                           nhid, nhtype, nh_valid, priority);
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
                        g_ip4_flow_cache.ip4_hash[id], pindex, sindex,
                        g_ip4_flow_cache.flags[id].update);
}

int
ftlv4_cache_delete_index (ftlv4 *obj, uint16_t id)
{
    return ftlv4_remove(obj, g_ip4_flow_cache.ip4_flow + id,
                        g_ip4_flow_cache.ip4_hash[id]);
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

int
ftlv4_cache_log_session(uint16_t iid, uint16_t rid, uint8_t reason)
{
    if ((!g_ip4_flow_cache.flags[iid].log) &&
        (!g_ip4_flow_cache.flags[rid].log)) {
        return 0;
    }
    return ftlv4_export_with_entry(g_ip4_flow_cache.ip4_flow + iid,
                                   g_ip4_flow_cache.ip4_flow + rid, reason,
                                   g_ip4_flow_cache.flags[iid].host_origin);
}

void
ftlv4_cache_set_epoch (uint8_t val)
{
    ftlv4_set_epoch(g_ip4_flow_cache.ip4_flow + g_ip4_flow_cache.count, val);
}

/*
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
*/

void
ftlv4_get_last_read_session_info (uint32_t *sip, uint32_t *dip, uint16_t *sport,
                                  uint16_t *dport, uint16_t *lkd_id)
{
    ipv4_flow_hash_entry_t *v4entry = &g_ip4_flow_cache.ip4_last_read_flow;
    *sip = v4entry->get_key_metadata_ipv4_src();
    *dip = v4entry->get_key_metadata_ipv4_dst();
    *sport = v4entry->get_key_metadata_sport();
    *dport = v4entry->get_key_metadata_dport();
    *lkd_id = ftlv4_get_key_lookup_id(v4entry);
}

void
ftlv4_set_last_read_entry_epoch (uint8_t epoch)
{
    ipv4_flow_hash_entry_t *v4entry = &g_ip4_flow_cache.ip4_last_read_flow;
    ftlv4_set_epoch(v4entry, epoch);
}

void
ftlv4_set_last_read_entry_miss_hit (uint8_t flow_miss)
{
    ipv4_flow_hash_entry_t *v4entry = &g_ip4_flow_cache.ip4_last_read_flow;
    ftlv4_set_entry_flow_miss_hit(v4entry, flow_miss);
}

void
ftlv4_set_thread_id (ftlv4 *obj, uint32_t thread_id)
{
    obj->set_thread_id(thread_id);
    return;
}

}
