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

static ftl_base *ftl_table;

uint32_t ftl_entry_count;
thread_local bool ftl_entry_valid;

typedef struct pds_flow_read_cbdata_s {
    pds_flow_key_t *key;
    pds_flow_info_t *info;
} pds_flow_read_cbdata_t;

typedef struct pds_flow_iterate_cbdata_s {
    pds_flow_iter_cb_t        iter_cb;
    pds_flow_iter_cb_arg_t    *iter_cb_arg;
} pds_flow_iter_cbdata_t;

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
    sprintf(str,
            "Src:%s Dst:%s Dport:%u Sport:%u Proto:%u "
            "Ktype:%u VNICID:%hu",
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

    ftlv6_set_key_ktype(entry, key->key_type);
    ftlv6_set_key_vnic_id(entry, key->vnic_id);
    ftlv6_set_key_dst_ip(entry, key->ip_daddr);
    ftlv6_set_key_src_ip(entry, key->ip_saddr);
    ftlv6_set_key_proto(entry, key->ip_proto);
    ftlv6_set_key_sport(entry, key->l4.tcp_udp.sport);
    ftlv6_set_key_dport(entry, key->l4.tcp_udp.dport);

    return SDK_RET_OK;
}

sdk_ret_t
pds_flow_cache_create ()
{
    sdk_table_factory_params_t factory_params = { 0 };

    factory_params.table_id = P4TBL_ID_FLOW;
    factory_params.num_hints = 5;
    factory_params.max_recircs = 8;
    factory_params.key2str = pds_flow6_key2str;
    factory_params.appdata2str = pds_flow6_appdata2str;
    // TODO: Remove this later
    factory_params.entry_trace_en = true;

    if ((ftl_table = flow_hash::factory(&factory_params)) == NULL) {
        PDS_TRACE_ERR("Table creation failed.");
        return SDK_RET_OOM;
    }
    return SDK_RET_OK;
}

void
pds_flow_cache_set_core_id (unsigned int core_id)
{
    ftl_table->set_thread_id(core_id);
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
    if (spec->key.key_type == KEY_TYPE_INVALID ||
        spec->key.key_type >= KEY_TYPE_MAX) {
        PDS_TRACE_ERR("Key type %u invalid", spec->key.key_type);
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
    uint16_t vnic_id;

    // Iterate only when entry valid and if another entry is not found already
    if (hwentry->entry_valid && (ftl_entry_valid == false)) {
        if (hwentry->key_metadata_ktype != cbdata->key->key_type) {
            return;
        }
        vnic_id = (hwentry->key_metadata_vnic_id_sbit0_ebit7 << 1) | hwentry->key_metadata_vnic_id_sbit8_ebit8;
        if ((vnic_id == cbdata->key->vnic_id) &&
            (!memcmp(hwentry->key_metadata_dst, cbdata->key->ip_daddr, IP6_ADDR8_LEN)) &&
            (!memcmp(hwentry->key_metadata_src, cbdata->key->ip_saddr, IP6_ADDR8_LEN)) &&
            (hwentry->key_metadata_proto == cbdata->key->ip_proto) &&
            (hwentry->key_metadata_sport == cbdata->key->l4.tcp_udp.sport) &&
            (hwentry->key_metadata_dport == cbdata->key->l4.tcp_udp.dport)) {
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
    if (key->key_type == KEY_TYPE_INVALID ||
        key->key_type >= KEY_TYPE_MAX) {
        PDS_TRACE_ERR("Key type %u invalid", key->key_type);
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

static void
flow_cache_entry_iterate_cb (sdk_table_api_params_t *params)
{
    flow_hash_entry_t *hwentry = (flow_hash_entry_t *)params->entry;
    pds_flow_iter_cbdata_t *cbdata = (pds_flow_iter_cbdata_t *)params->cbdata;
    pds_flow_key_t *key;
    pds_flow_data_t *data;

    if (hwentry->entry_valid) {
        ftl_entry_count++;
        key = &cbdata->iter_cb_arg->flow_key;
        data = &cbdata->iter_cb_arg->flow_appdata;
        key->key_type = (pds_key_type_t)ftlv6_get_key_ktype(hwentry);
        key->vnic_id = ftlv6_get_key_vnic_id(hwentry);
        ftlv6_get_key_dst_ip(hwentry, key->ip_daddr);
        ftlv6_get_key_src_ip(hwentry, key->ip_saddr);
        key->ip_proto = ftlv6_get_key_proto(hwentry);
        key->l4.tcp_udp.sport = ftlv6_get_key_sport(hwentry);
        key->l4.tcp_udp.dport = ftlv6_get_key_dport(hwentry);
        data->index = ftlv6_get_index(hwentry);
        data->index_type =
            (pds_flow_spec_index_type_t)ftlv6_get_index_type(hwentry);
        cbdata->iter_cb(cbdata->iter_cb_arg);
    }
    return;
}

sdk_ret_t
pds_flow_cache_entry_iterate (pds_flow_iter_cb_t iter_cb,
                              pds_flow_iter_cb_arg_t *iter_cb_arg)
{
    sdk_table_api_params_t params = { 0 };
    pds_flow_iter_cbdata_t cbdata = { 0 };

    if (iter_cb == NULL || iter_cb_arg == NULL) {
        PDS_TRACE_ERR("itercb or itercb_arg is null");
        return SDK_RET_INVALID_ARG;
     }

    cbdata.iter_cb = iter_cb;
    cbdata.iter_cb_arg = iter_cb_arg;
    params.itercb = flow_cache_entry_iterate_cb;
    params.cbdata = &cbdata;
    params.force_hwread = false;
    params.entry_size = flow_hash_entry_t::entry_size();
    ftl_entry_count = 0;
    return ftl_table->iterate(&params);
}

}
