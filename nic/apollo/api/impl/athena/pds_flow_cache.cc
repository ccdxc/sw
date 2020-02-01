//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// athena flow cache implementation
///
//----------------------------------------------------------------------------

#include "nic/sdk/include/sdk/base.hpp"
#include "nic/sdk/include/sdk/platform.hpp"
#include "nic/sdk/include/sdk/table.hpp"
#include "nic/sdk/platform/capri/capri_tbl_rw.hpp"
#include "nic/sdk/lib/p4/p4_api.hpp"
#include "nic/sdk/lib/p4/p4_utils.hpp"
#include "nic/apollo/core/trace.hpp"
#include "nic/apollo/api/include/athena/pds_flow_cache.h"
#include "nic/apollo/api/include/athena/pds_flow_session.h"
#include "nic/apollo/p4/include/athena_defines.h"
#include "nic/apollo/p4/include/athena_table_sizes.h"
#include "ftl_wrapper.h"
#include "gen/p4gen/athena/include/p4pd.h"

using namespace sdk;
using namespace sdk::table;

extern "C" {

// Per thread address of table object
static thread_local ftl_base *ftl_table;

uint32_t ftl_entry_count;
thread_local bool ftl_entry_valid;

typedef struct pds_flow_read_cbdata_s {
    pds_flow_key_t *key;
    pds_flow_info_t *info;
} pds_flow_read_cbdata_t;

static char *
pds_flow6_key2str (void *key)
{
    static char str[256] = { 0 };
    flow_swkey_t *k = (flow_swkey_t *)key;
    char srcstr[INET6_ADDRSTRLEN + 1];
    char dststr[INET6_ADDRSTRLEN + 1];
    uint16_t vnic_id;

    if (k->key_metadata_ktype == KEY_TYPE_IPV6) {
        inet_ntop(AF_INET6, k->key_metadata_src, srcstr, INET6_ADDRSTRLEN);
        inet_ntop(AF_INET6, k->key_metadata_dst, dststr, INET6_ADDRSTRLEN);
    } else {
        inet_ntop(AF_INET, k->key_metadata_src, srcstr, INET_ADDRSTRLEN);
        inet_ntop(AF_INET, k->key_metadata_dst, dststr, INET_ADDRSTRLEN);
    }
    sprintf(str, "SMAC:%lu DMAC:%lu "
            "Src:%s Dst:%s Dport:%u Sport:%u Proto:%u "
            "Ktype:%u VNICID:%hu",
            (*(uint64_t *)k->key_metadata_smac) & 0xFFFFFFFFFFFF,
            (*(uint64_t *)k->key_metadata_dmac) & 0xFFFFFFFFFFFF,
            srcstr, dststr,
            k->key_metadata_dport, k->key_metadata_sport,
            k->key_metadata_proto, k->key_metadata_ktype,
            k->key_metadata_vnic_id);
    return str;
}

static char *
pds_flow6_appdata2str (void *appdata)
{
    static char str[512] = { 0 };
    flow_appdata_t *d = (flow_appdata_t *)appdata;
    sprintf(str, "id_type:%u id:%u", d->index_type, d->index);
    return str;
}

static sdk_ret_t
flow_cache_entry_setup_key (flow_hash_entry_t *entry,
                            pds_flow_key_t *key)
{
    if (!entry) {
        PDS_TRACE_ERR("entry is null");
        return SDK_RET_INVALID_ARG;
    }

    if (key->ip_addr_family == IP_AF_IPV4) { 
        ftlv6_set_key_ktype(entry, KEY_TYPE_IPV4);
    } else {
        ftlv6_set_key_ktype(entry, KEY_TYPE_IPV6);
    }
    ftlv6_set_key_vnic_id(entry, key->vnic_id);
    ftlv6_set_key_dst_ip(entry, key->ip_daddr);
    ftlv6_set_key_src_ip(entry, key->ip_saddr);
    ftlv6_set_key_proto(entry, key->ip_proto);
    ftlv6_set_key_sport(entry, key->l4.tcp_udp.sport);
    ftlv6_set_key_dport(entry, key->l4.tcp_udp.dport);
    ftlv6_set_key_dest_mac(entry, key->dmac);
    ftlv6_set_key_src_mac(entry, key->smac);

    return SDK_RET_OK;
}

sdk_ret_t
pds_flow_cache_create (uint32_t core_id)
{
    sdk_table_factory_params_t factory_params = { 0 };

    factory_params.table_id = P4TBL_ID_FLOW;
    factory_params.num_hints = 4;
    factory_params.max_recircs = 8;
    factory_params.key2str = pds_flow6_key2str;
    factory_params.appdata2str = pds_flow6_appdata2str;
    factory_params.thread_id = core_id;
    // TODO: Remove this later
    factory_params.entry_trace_en = true;
    factory_params.entry_alloc_cb = flow_hash_entry_t::alloc;

    if ((ftl_table = flow_hash::factory(&factory_params)) == NULL) {
        PDS_TRACE_ERR("Table creation failed in thread %u", core_id);
        return SDK_RET_OOM;
    }
    return SDK_RET_OK;
}

sdk_ret_t
pds_flow_cache_entry_create (pds_flow_spec_t *spec)
{
    sdk_ret_t ret;
    sdk_table_api_params_t params = { 0 };
    flow_hash_entry_t entry;
    uint32_t index;
    pds_flow_spec_index_type_t index_type;

    if (!spec) {
        PDS_TRACE_ERR("spec is null");
        return SDK_RET_INVALID_ARG;
    }
    index = spec->data.index;
    index_type = spec->data.index_type;
    if (index > PDS_FLOW_SESSION_INFO_ID_MAX) {
        PDS_TRACE_ERR("session id %u is invalid", index);
        return SDK_RET_INVALID_ARG;
    }

    entry.clear();
    if ((ret = flow_cache_entry_setup_key(&entry, &spec->key))
             != SDK_RET_OK)
         return ret;
    ftlv6_set_index(&entry, index);
    ftlv6_set_index_type(&entry, index_type);
    params.entry = &entry;
    params.entry_size = flow_hash_entry_t::entry_size();
    return ftl_table->insert(&params);
}

static void
flow_cache_entry_find_cb (sdk_table_api_params_t *params)
{
    flow_hash_entry_t *hwentry = (flow_hash_entry_t *)params->entry;
    pds_flow_read_cbdata_t *cbdata = (pds_flow_read_cbdata_t *)params->cbdata;

    // Iterate only when entry valid and if another entry is not found already
    if (hwentry->entry_valid && (ftl_entry_valid == false)) {
        // TODO: Optimize this
        if ((hwentry->key_metadata_ktype == KEY_TYPE_IPV6) &&
            (cbdata->key->ip_addr_family == IP_AF_IPV4)) {
            return;
        } else if ((hwentry->key_metadata_ktype == KEY_TYPE_IPV4) &&
            (cbdata->key->ip_addr_family == IP_AF_IPV6)) {
            return;
        }
        if ((hwentry->get_key_metadata_vnic_id() == cbdata->key->vnic_id) &&
            (!memcmp(hwentry->key_metadata_dst, cbdata->key->ip_daddr, IP6_ADDR8_LEN)) &&
            (!memcmp(hwentry->key_metadata_src, cbdata->key->ip_saddr, IP6_ADDR8_LEN)) &&
            (hwentry->key_metadata_proto == cbdata->key->ip_proto) &&
            (hwentry->key_metadata_sport == cbdata->key->l4.tcp_udp.sport) &&
            (hwentry->key_metadata_dport == cbdata->key->l4.tcp_udp.dport) &&
            (hwentry->key_metadata_dmac == cbdata->key->dmac) &&
            (hwentry->key_metadata_smac == cbdata->key->smac)) {
            // Key matching with index, so fill data
            cbdata->info->spec.data.index = hwentry->index;
            cbdata->info->spec.data.index_type =
                (pds_flow_spec_index_type_t)hwentry->index_type;
            ftl_entry_valid = true;
        }
        return;
    }
    return;
}

sdk_ret_t
pds_flow_cache_entry_read (pds_flow_key_t *key,
                           pds_flow_info_t *info)
{
    sdk_ret_t ret;
    sdk_table_api_params_t params = { 0 };
    flow_hash_entry_t entry;
    pds_flow_read_cbdata_t cbdata = { 0 };

    if (!key || !info) {
        PDS_TRACE_ERR("key/info is null");
        return SDK_RET_INVALID_ARG;
    }

    entry.clear();
    ftl_entry_valid = false;
    if ((ret = flow_cache_entry_setup_key(&entry, key))
             != SDK_RET_OK)
         return ret;
    params.entry = &entry;
    params.entry_size = flow_hash_entry_t::entry_size();
    params.itercb = flow_cache_entry_find_cb;
    cbdata.key = key;
    cbdata.info = info;
    params.cbdata = &cbdata;
    ret = ftl_table->iterate(&params);
    if (ftl_entry_valid == false)
        return SDK_RET_ENTRY_NOT_FOUND;
    else
        return SDK_RET_OK;
}

sdk_ret_t
pds_flow_cache_entry_update (pds_flow_spec_t *spec)
{
    sdk_ret_t ret;
    sdk_table_api_params_t params = { 0 };
    flow_hash_entry_t entry;

    if (!spec) {
        PDS_TRACE_ERR("spec is null");
        return SDK_RET_INVALID_ARG;
    }

    entry.clear();
    if ((ret = flow_cache_entry_setup_key(&entry, &spec->key))
             != SDK_RET_OK)
         return ret;
    params.entry = &entry;
    params.entry_size = flow_hash_entry_t::entry_size();
    return ftl_table->update(&params);
}

sdk_ret_t
pds_flow_cache_entry_delete (pds_flow_key_t *key)
{
    sdk_ret_t ret;
    sdk_table_api_params_t params = { 0 };
    flow_hash_entry_t entry;

    if (!key) {
        PDS_TRACE_ERR("key is null");
        return SDK_RET_INVALID_ARG;
    }

    entry.clear();
    if ((ret = flow_cache_entry_setup_key(&entry, key))
             != SDK_RET_OK)
         return ret;
    params.entry = &entry;
    params.entry_size = flow_hash_entry_t::entry_size();
    return ftl_table->remove(&params);
}

}
