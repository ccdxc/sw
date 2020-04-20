//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// athena dnat mapping implementation
///
//----------------------------------------------------------------------------

#include "nic/sdk/include/sdk/base.hpp"
#include "nic/sdk/include/sdk/ip.hpp"
#include "nic/sdk/include/sdk/platform.hpp"
#include "nic/sdk/include/sdk/table.hpp"
#include "nic/sdk/lib/p4/p4_api.hpp"
#include "nic/sdk/lib/p4/p4_utils.hpp"
#include "nic/apollo/core/trace.hpp"
#include "nic/apollo/api/include/athena/pds_dnat.h"
#include "nic/apollo/api/include/athena/pds_vnic.h"
#include "nic/apollo/p4/include/athena_defines.h"
#include "nic/apollo/p4/include/athena_table_sizes.h"
#include "ftl_wrapper.h"
#include "gen/p4gen/athena/include/p4pd.h"

using namespace sdk;
using namespace sdk::table;

extern "C" {

static ftl_base *g_dnat_mapping_tbl;

uint32_t dnat_entry_count;

static pds_ret_t
dnat_map_entry_setup_key (dnat_entry_t *entry,
                          pds_dnat_mapping_key_t *key)
{
    if (!entry) {
        PDS_TRACE_ERR("entry is null");
        return PDS_RET_INVALID_ARG;
    }

    dnat_set_key_ktype(entry, key->key_type);
    dnat_set_key_vnic_id(entry, key->vnic_id);
    dnat_set_key_ip(entry, key->addr);
    return PDS_RET_OK;
}

pds_ret_t
pds_dnat_map_create ()
{
    sdk_table_factory_params_t tparams = { 0 };

    tparams.table_id = P4TBL_ID_DNAT;
    tparams.num_hints = 4;
    tparams.max_recircs = 8;
    tparams.key2str = NULL;
    tparams.appdata2str = NULL;
    // TODO: Remove this later
    tparams.entry_trace_en = true;

    if ((g_dnat_mapping_tbl = dnat::factory(&tparams)) == NULL) {
        PDS_TRACE_ERR("DNAT mapping table creation failed");
        return PDS_RET_OOM;
    }
    return PDS_RET_OK;
}

void
pds_dnat_map_set_core_id (uint32_t core_id)
{
    g_dnat_mapping_tbl->set_thread_id(core_id);
}

pds_ret_t
pds_dnat_map_delete ()
{
    if (!g_dnat_mapping_tbl) {
        PDS_TRACE_ERR("DNAT mapping table not yet created");
        return PDS_RET_ERR;
    }
    dnat::destroy(g_dnat_mapping_tbl);
    return PDS_RET_OK;
}

pds_ret_t
pds_dnat_map_entry_create (pds_dnat_mapping_spec_t *spec)
{
    pds_ret_t ret = PDS_RET_OK;
    sdk_table_api_params_t params = { 0 };
    dnat_entry_t entry;
    uint16_t vnic_id;

    if (!spec) {
        //PDS_TRACE_ERR("spec is null");
        return PDS_RET_INVALID_ARG;
    }
    vnic_id = spec->key.vnic_id;
    if (spec->key.vnic_id == 0 ||
        spec->key.vnic_id > PDS_VNIC_ID_MAX) {
        //PDS_TRACE_ERR("Vnic id %u invalid", vnic_id);
        return PDS_RET_INVALID_ARG;
    }
    if (spec->key.key_type != IP_AF_IPV4 &&
        spec->key.key_type != IP_AF_IPV6) {
        //PDS_TRACE_ERR("Key type %u invalid", spec->key.key_type);
        return PDS_RET_INVALID_ARG;
    }
    if (spec->data.addr_type != IP_AF_IPV4 &&
        spec->data.addr_type != IP_AF_IPV6) {
        //PDS_TRACE_ERR("Address type %u invalid in data",
        //              spec->data.addr_type);
        return PDS_RET_INVALID_ARG;
    }

    entry.clear();
    if ((ret = dnat_map_entry_setup_key(&entry, &spec->key))
             != PDS_RET_OK)
         return (pds_ret_t) ret;
    dnat_set_map_ip(&entry, spec->data.addr);
    dnat_set_map_addr_type(&entry, spec->data.addr_type);
    dnat_set_map_epoch(&entry, spec->data.epoch);
    params.entry = &entry;
    ret = (pds_ret_t)g_dnat_mapping_tbl->insert(&params);
    if (ret != PDS_RET_OK) {
        //PDS_TRACE_ERR("Failed to insert entry in DNAT mapping "
        //              "table for (vnic id %u, IP %s), err %u\n", spec->key.vnic_id,
        //              ipv6addr2str(*(ipv6_addr_t *)spec->key.addr), ret);
    }
    return (pds_ret_t)ret;
}

pds_ret_t
pds_dnat_map_entry_read (pds_dnat_mapping_key_t *key,
                         pds_dnat_mapping_info_t *info)
{
    pds_ret_t ret;
    sdk_table_api_params_t params = { 0 };
    dnat_entry_t entry;

    if (!key || !info) {
        PDS_TRACE_ERR("key/info is null");
        return PDS_RET_INVALID_ARG;
    }
    if (key->vnic_id == 0 ||
        key->vnic_id > PDS_VNIC_ID_MAX) {
        PDS_TRACE_ERR("Vnic id %u invalid", key->vnic_id);
        return PDS_RET_INVALID_ARG;
    }
    if (key->key_type != IP_AF_IPV4 &&
        key->key_type != IP_AF_IPV6) {
        PDS_TRACE_ERR("Key type %u invalid", key->key_type);
        return PDS_RET_INVALID_ARG;
    }

    entry.clear();
    if ((ret = dnat_map_entry_setup_key(&entry, key))
             != PDS_RET_OK)
         return (pds_ret_t)ret;
    params.entry = &entry;
    ret = (pds_ret_t)g_dnat_mapping_tbl->get(&params);
    if (ret == PDS_RET_OK) {
        dnat_get_map_ip(&entry, info->spec.data.addr);
        info->spec.data.addr_type = dnat_get_map_addr_type(&entry);
        info->spec.data.epoch = dnat_get_map_epoch(&entry);
    }
    else {
        return PDS_RET_ENTRY_NOT_FOUND;
    }
    return PDS_RET_OK;
}

pds_ret_t
pds_dnat_map_entry_update (pds_dnat_mapping_spec_t *spec)
{
    pds_ret_t ret;
    sdk_table_api_params_t params = { 0 };
    dnat_entry_t entry;

    if (!spec) {
        PDS_TRACE_ERR("spec is null");
        return PDS_RET_INVALID_ARG;
    }
    if (spec->key.vnic_id == 0 ||
        spec->key.vnic_id > PDS_VNIC_ID_MAX) {
        PDS_TRACE_ERR("Vnic id %u invalid", spec->key.vnic_id);
        return PDS_RET_INVALID_ARG;
    }
    if (spec->key.key_type != IP_AF_IPV4 &&
        spec->key.key_type != IP_AF_IPV6) {
        PDS_TRACE_ERR("Key type %u invalid", spec->key.key_type);
        return PDS_RET_INVALID_ARG;
    }
    if (spec->data.addr_type != IP_AF_IPV4 &&
        spec->data.addr_type != IP_AF_IPV6) {
        PDS_TRACE_ERR("Address type %u invalid in data", spec->key.key_type);
        return PDS_RET_INVALID_ARG;
    }

    entry.clear();
    if ((ret = dnat_map_entry_setup_key(&entry, &spec->key))
             != PDS_RET_OK)
         return (pds_ret_t) ret;
    dnat_set_map_ip(&entry, spec->data.addr);
    dnat_set_map_addr_type(&entry, spec->data.addr_type);
    dnat_set_map_epoch(&entry, spec->data.epoch);
    params.entry = &entry;
    ret = (pds_ret_t)g_dnat_mapping_tbl->update(&params);
    if (ret != PDS_RET_OK) {
        //PDS_TRACE_ERR("Failed to update entry in DNAT mapping "
        //              "table for (vnic id %u, IP %s), err %u\n",
        //              spec->key.vnic_id,
        //              ipv6addr2str(*(ipv6_addr_t *)spec->key.addr), ret);
    }
    return (pds_ret_t)ret;
}

pds_ret_t
pds_dnat_map_entry_delete (pds_dnat_mapping_key_t *key)
{
    pds_ret_t ret;
    sdk_table_api_params_t params = { 0 };
    dnat_entry_t entry;

    if (!key) {
        PDS_TRACE_ERR("key is null");
        return PDS_RET_INVALID_ARG;
    }
    if (key->vnic_id == 0 ||
        key->vnic_id > PDS_VNIC_ID_MAX) {
        PDS_TRACE_ERR("Vnic id %u invalid", key->vnic_id);
        return PDS_RET_INVALID_ARG;
    }
    if (key->key_type != IP_AF_IPV4 &&
        key->key_type != IP_AF_IPV6) {
        PDS_TRACE_ERR("Key type %u invalid", key->key_type);
        return PDS_RET_INVALID_ARG;
    }

    entry.clear();
    if ((ret = dnat_map_entry_setup_key(&entry, key))
             != PDS_RET_OK)
         return (pds_ret_t)ret;
    params.entry = &entry;
    ret = (pds_ret_t)g_dnat_mapping_tbl->remove(&params);
    if (ret != PDS_RET_OK) {
        //PDS_TRACE_ERR("Failed to delete entry in DNAT mapping "
        //              "table for (vnic id %u, IP %s), err %u\n", key->vnic_id,
        //              ipv6addr2str(*(ipv6_addr_t *)key->addr), ret);
    }
    return (pds_ret_t)ret;
}

}
