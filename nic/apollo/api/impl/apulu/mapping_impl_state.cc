//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// mapping datapath database handling
///
//----------------------------------------------------------------------------

#include "nic/sdk/lib/p4/p4_api.hpp"
#include "nic/sdk/include/sdk/ip.hpp"
#include "nic/sdk/include/sdk/eth.hpp"
#include "nic/sdk/lib/utils/utils.hpp"
#include "nic/sdk/lib/table/memhash/mem_hash.hpp"
#include "nic/apollo/api/include/pds_mapping.hpp"
#include "nic/apollo/api/impl/apulu/pds_impl_state.hpp"
#include "nic/apollo/api/impl/apulu/mapping_impl.hpp"
#include "nic/apollo/api/impl/apulu/apulu_impl.hpp"
#include "nic/apollo/api/impl/apulu/vpc_impl.hpp"
#include "nic/apollo/api/impl/apulu/subnet_impl.hpp"
#include "nic/apollo/api/include/pds_debug.hpp"
#include "nic/apollo/api/include/pds.hpp"
#include "nic/apollo/p4/include/apulu_defines.h"
#include "gen/p4gen/apulu/include/p4pd.h"
#include <arpa/inet.h>
#include <netinet/ether.h>
using sdk::table::sdk_table_factory_params_t;

namespace api {
namespace impl {

/// \defgroup PDS_MAPPING_IMPL_STATE - mapping database functionality
/// \ingroup PDS_MAPPING
/// \@{

mapping_impl_state::mapping_impl_state(pds_state *state) {
    p4pd_table_properties_t       tinfo;
    sdk_table_factory_params_t    tparams;
    dhcpctl_status                dhcpctl = 0;

    // instantiate P4 tables for bookkeeping
    bzero(&tparams, sizeof(tparams));
    tparams.max_recircs = 8;
    tparams.entry_trace_en = false;
    tparams.key2str = NULL;
    tparams.appdata2str = NULL;

    // LOCAL_MAPPING table bookkeeping
    tparams.table_id = P4TBL_ID_LOCAL_MAPPING;
    tparams.num_hints = P4_LOCAL_MAPPING_NUM_HINTS_PER_ENTRY;
    local_mapping_tbl_ = mem_hash::factory(&tparams);
    SDK_ASSERT(local_mapping_tbl_ != NULL);

    // MAPPING table bookkeeping
    tparams.table_id = P4TBL_ID_MAPPING;
    tparams.num_hints = P4_MAPPING_NUM_HINTS_PER_ENTRY;
    mapping_tbl_ = mem_hash::factory(&tparams);
    SDK_ASSERT(mapping_tbl_ != NULL);

    // instantiate indexer for binding table
    p4pd_table_properties_get(P4TBL_ID_IP_MAC_BINDING, &tinfo);
    ip_mac_binding_idxr_ = rte_indexer::factory(tinfo.tabledepth, true, true);
    SDK_ASSERT(ip_mac_binding_idxr_ != NULL);

    // create a slab for mapping impl entries
    mapping_impl_slab_ = slab::factory("mapping-impl", PDS_SLAB_ID_MAPPING_IMPL,
                                       sizeof(mapping_impl), 8192, true, true,
                                       true, NULL);
    SDK_ASSERT(mapping_impl_slab_!= NULL);

    dhcp_connection_ = NULL;
    dhcpctl = dhcpctl_initialize();
    SDK_ASSERT(dhcpctl == ISC_R_SUCCESS);
}

mapping_impl_state::~mapping_impl_state() {
    mem_hash::destroy(local_mapping_tbl_);
    mem_hash::destroy(mapping_tbl_);
    rte_indexer::destroy(ip_mac_binding_idxr_);
    slab::destroy(mapping_impl_slab_);
}

mapping_impl *
mapping_impl_state::alloc(void) {
    return ((mapping_impl *)mapping_impl_slab_->alloc());
}

void
mapping_impl_state::free(mapping_impl *impl) {
    mapping_impl_slab_->free(impl);
}

sdk_ret_t
mapping_impl_state::table_transaction_begin(void) {
    local_mapping_tbl_->txn_start();
    mapping_tbl_->txn_start();
    return SDK_RET_OK;
}

sdk_ret_t
mapping_impl_state::table_transaction_end(void) {
    local_mapping_tbl_->txn_end();
    mapping_tbl_->txn_end();
    return SDK_RET_OK;
}

sdk_ret_t
mapping_impl_state::table_stats(debug::table_stats_get_cb_t cb, void *ctxt) {
    pds_table_stats_t stats;
    p4pd_table_properties_t tinfo;

    memset(&stats, 0, sizeof(pds_table_stats_t));
    p4pd_table_properties_get(P4TBL_ID_LOCAL_MAPPING, &tinfo);
    stats.table_name = tinfo.tablename;
    local_mapping_tbl_->stats_get(&stats.api_stats, &stats.table_stats);
    cb(&stats, ctxt);

    memset(&stats, 0, sizeof(pds_table_stats_t));
    p4pd_table_properties_get(P4TBL_ID_MAPPING, &tinfo);
    stats.table_name = tinfo.tablename;
    mapping_tbl_->stats_get(&stats.api_stats, &stats.table_stats);
    cb(&stats, ctxt);

    return SDK_RET_OK;
}

/// \brief     function to print local mapping header
/// \param[in] fd   file descriptor to print to
static void
local_mapping_print_header (int fd)
{
    dprintf(fd, "%s\n", std::string(148, '-').c_str());
    dprintf(fd, "%-7s%-40s%-18s%-40s%-7s%-10s%-10s%-16s\n",
            "VpcID", "OverlayIP", "OverlayMAC", "PublicIP",
            "VnicID", "NhType", "Tunnel", "FabricEncap");
    dprintf(fd, "%s\n", std::string(148, '-').c_str());
}

/// \brief     callback function to dump local mapping entries
/// \param[in] params   sdk_table_api_params_t structure
void
local_mapping_dump_cb (sdk_table_api_params_t *params)
{
    mapping_swkey_t         mapping_key;
    mapping_appdata_t       mapping_data;
    local_mapping_swkey_t   *key;
    local_mapping_appdata_t *data;
    uint16_t                vpc_id;
    sdk_table_api_params_t  api_params;
    int                     fd = *(int *)(params->cbdata);
    p4pd_error_t            p4pd_ret;
    sdk_ret_t               ret;
    bd_actiondata_t         bd_data;
    ip_addr_t               public_ip = { 0 }, private_ip;
    mac_addr_t              overlay_mac;
    pds_encap_t             encap;
    string                  nexthop_type;

    key = (local_mapping_swkey_t *)(params->key);
    data = (local_mapping_appdata_t *)(params->appdata);
    vpc_id = key->key_metadata_local_mapping_lkp_id;

    // skip l2 mappings
    if (key->key_metadata_local_mapping_lkp_type == KEY_TYPE_MAC) {
        return;
    }

    // read remote mapping
    mapping_key.p4e_i2e_mapping_lkp_id = key->key_metadata_local_mapping_lkp_id;
    mapping_key.p4e_i2e_mapping_lkp_type = key->key_metadata_local_mapping_lkp_type;
    memcpy(mapping_key.p4e_i2e_mapping_lkp_addr,
           key->key_metadata_local_mapping_lkp_addr,
           IP6_ADDR8_LEN);
    PDS_IMPL_FILL_TABLE_API_PARAMS(&api_params, &mapping_key, NULL,
                                   &mapping_data, 0,
                                   sdk::table::handle_t::null());
    ret = mapping_impl_db()->mapping_tbl()->get(&api_params);
    if (ret != SDK_RET_OK) {
        return;
    }

    // TODO: read NAT table for public ip address
    private_ip.af = (key->key_metadata_local_mapping_lkp_type == KEY_TYPE_IPV6) ?
                        IP_AF_IPV6 : IP_AF_IPV4;
    public_ip.af = private_ip.af;
    if (private_ip.af == IP_AF_IPV4) {
        private_ip.addr.v4_addr = *(uint32_t *)key->key_metadata_local_mapping_lkp_addr;
    } else {
        sdk::lib::memrev(private_ip.addr.v6_addr.addr8,
                         key->key_metadata_local_mapping_lkp_addr,
                         IP6_ADDR8_LEN);
    }
    sdk::lib::memrev(overlay_mac, mapping_data.dmaci, ETH_ADDR_LEN);

    // read bd id table
    p4pd_ret = p4pd_global_entry_read(P4TBL_ID_BD, mapping_data.egress_bd_id,
                                      NULL, NULL, &bd_data);
    if (p4pd_ret != P4PD_SUCCESS) {
        return;
    }
    encap.val.vnid = bd_data.bd_info.vni;
    if (encap.val.vnid) {
        encap.type = PDS_ENCAP_TYPE_VXLAN;
    } else {
        encap.type = PDS_ENCAP_TYPE_NONE;
    }
    nexthop_type_to_string(nexthop_type, mapping_data.nexthop_type);

    dprintf(fd, "%-7u%-40s%-18s%-40s%-7u%-10s%-10u%-16s\n",
            vpc_id, ipaddr2str(&private_ip), macaddr2str(overlay_mac),
            ipaddr2str(&public_ip),
            data->vnic_id, nexthop_type.c_str(),
            mapping_data.nexthop_id,
            pds_encap2str(&encap));
}

/// \brief     function to print l3 mapping header
/// \param[in] fd   file descriptor to print to
static void
remote_mapping_print_l3_header (int fd)
{
    dprintf(fd, "%s\n", std::string(82, '-').c_str());
    dprintf(fd, "%-7s%-40s%-18s%-10s%-7s\n",
            "VpcID", "OverlayIP", "OverlayMAC",
            "NhType", "Tunnel");
    dprintf(fd, "%s\n", std::string(82, '-').c_str());
}

/// \brief     callback function to dump remote l3 mapping entries
/// \param[in] params   sdk_table_api_params_t structure
static void
remote_l3_mapping_dump_cb (sdk_table_api_params_t *params)
{
    mapping_swkey_t         *mapping_key;
    mapping_appdata_t       *mapping_data;
    uint16_t                vpc_id;
    int                     fd = *(int *)(params->cbdata);
    p4pd_error_t            p4pd_ret;
    sdk_ret_t               ret;
    bd_actiondata_t         bd_data;
    ip_addr_t               private_ip;
    mac_addr_t              overlay_mac;
    pds_encap_t             encap;
    string                  nexthop_type;

    mapping_key = (mapping_swkey_t *)(params->key);
    mapping_data = (mapping_appdata_t *)(params->appdata);

    // skip l2 mappings
    if (mapping_key->p4e_i2e_mapping_lkp_type == KEY_TYPE_MAC) {
        return;
    }
    // skip local mappings
    if (mapping_data->nexthop_type == NEXTHOP_TYPE_NEXTHOP) {
        return;
    }

    vpc_id = mapping_key->p4e_i2e_mapping_lkp_id;
    private_ip.af = (mapping_key->p4e_i2e_mapping_lkp_type == KEY_TYPE_IPV6) ?
                    IP_AF_IPV6 : IP_AF_IPV4;
    if (private_ip.af == IP_AF_IPV4) {
        private_ip.addr.v4_addr = *(uint32_t *)mapping_key->p4e_i2e_mapping_lkp_addr;
    } else {
        sdk::lib::memrev(private_ip.addr.v6_addr.addr8,
                         mapping_key->p4e_i2e_mapping_lkp_addr,
                         IP6_ADDR8_LEN);
    }
    sdk::lib::memrev(overlay_mac, mapping_data->dmaci, ETH_ADDR_LEN);

    // read bd id table
    p4pd_ret = p4pd_global_entry_read(P4TBL_ID_BD, mapping_data->egress_bd_id,
                                      NULL, NULL, &bd_data);
    if (p4pd_ret != P4PD_SUCCESS) {
        return;
    }
    encap.val.vnid = bd_data.bd_info.vni;
    if (encap.val.vnid) {
        encap.type = PDS_ENCAP_TYPE_VXLAN;
    } else {
        encap.type = PDS_ENCAP_TYPE_NONE;
    }
    nexthop_type_to_string(nexthop_type, mapping_data->nexthop_type);

    dprintf(fd, "%-7u%-40s%-18s%-10s%-7u\n",
            vpc_id,  ipaddr2str(&private_ip),
            macaddr2str(overlay_mac),
            nexthop_type.c_str(),
            mapping_data->nexthop_id);
}

/// \brief     function to print l2 mapping header
/// \param[in] fd   file descriptor to print to
static void
remote_mapping_print_l2_header (int fd)
{
    dprintf(fd, "%s\n", std::string(44, '-').c_str());
    dprintf(fd, "%-9s%-18s%-10s%-7s\n",
            "SubnetID", "OverlayMAC", "NhType", "Tunnel");
    dprintf(fd, "%s\n", std::string(44, '-').c_str());
}

/// \brief     callback function to dump remote l2 mapping entries
/// \param[in] params   sdk_table_api_params_t structure
static void
remote_l2_mapping_dump_cb (sdk_table_api_params_t *params)
{
    mapping_swkey_t         *mapping_key;
    mapping_appdata_t       *mapping_data;
    uint16_t                subnet_id;
    int                     fd = *(int *)(params->cbdata);
    p4pd_error_t            p4pd_ret;
    sdk_ret_t               ret;
    bd_actiondata_t         bd_data;
    mac_addr_t              mac;
    string                  nexthop_type;

    mapping_key = (mapping_swkey_t *)(params->key);
    mapping_data = (mapping_appdata_t *)(params->appdata);

    // skip l3 mappings
    if (mapping_key->p4e_i2e_mapping_lkp_type != KEY_TYPE_MAC) {
        return;
    }
    // skip local mappings
    if (mapping_data->nexthop_type == NEXTHOP_TYPE_NEXTHOP) {
        return;
    }

    subnet_id = mapping_key->p4e_i2e_mapping_lkp_id;
    sdk::lib::memrev(mac, mapping_key->p4e_i2e_mapping_lkp_addr, ETH_ADDR_LEN);

    nexthop_type_to_string(nexthop_type, mapping_data->nexthop_type);

    dprintf(fd, "%-9u%-18s%-10s%-7u\n",
            subnet_id, macaddr2str(mac),
            nexthop_type.c_str(),
            mapping_data->nexthop_id);
}

sdk_ret_t
mapping_impl_state::mapping_dump(int fd, cmd_args_t *args) {
    sdk_table_api_params_t api_params = { 0 };
    mapping_dump_type_t    type;

    if (!args->mapping_dump.key_valid) {
        type = args->mapping_dump.type;
        if (type == MAPPING_DUMP_TYPE_LOCAL) {
            local_mapping_print_header(fd);
            api_params.itercb = local_mapping_dump_cb;
            api_params.cbdata = &fd;
            local_mapping_tbl_->iterate(&api_params);
        }
        // TODO: skip local entries from remote table
        if (type == MAPPING_DUMP_TYPE_REMOTE_L3) {
            remote_mapping_print_l3_header(fd);
            api_params.itercb = remote_l3_mapping_dump_cb;
            api_params.cbdata = &fd;
            mapping_tbl_->iterate(&api_params);
        }
        if (type == MAPPING_DUMP_TYPE_REMOTE_L2) {
            remote_mapping_print_l2_header(fd);
            api_params.itercb = remote_l2_mapping_dump_cb;
            api_params.cbdata = &fd;
            mapping_tbl_->iterate(&api_params);
        }
    } else {
        mapping_dump_args_t *mapping_args = &args->mapping_dump;
        type = mapping_args->type;

        if (type == MAPPING_DUMP_TYPE_LOCAL) {
            local_mapping_swkey_t       local_ip_mapping_key;
            local_mapping_appdata_t     local_ip_mapping_data;
            sdk_ret_t                   ret;

            PDS_IMPL_FILL_LOCAL_IP_MAPPING_SWKEY(&local_ip_mapping_key,
                                                 mapping_args->skey.vpc,
                                                 &mapping_args->skey.ip_addr);
            PDS_IMPL_FILL_TABLE_API_PARAMS(&api_params, &local_ip_mapping_key,
                                           NULL, &local_ip_mapping_data, 0,
                                           sdk::table::handle_t::null());
            api_params.cbdata = &fd;

            ret = local_mapping_tbl_->get(&api_params);
            if (ret == SDK_RET_OK) {
                local_mapping_print_header(fd);
                local_mapping_dump_cb(&api_params);
            }
        }
        if (type == MAPPING_DUMP_TYPE_REMOTE_L3) {
            mapping_swkey_t       mapping_key = { 0 };
            mapping_appdata_t     mapping_data = { 0 };
            sdk_ret_t             ret;

            PDS_IMPL_FILL_IP_MAPPING_SWKEY(&mapping_key,
                                           mapping_args->skey.vpc,
                                           &mapping_args->skey.ip_addr);
            PDS_IMPL_FILL_TABLE_API_PARAMS(&api_params, &mapping_key, NULL,
                                           &mapping_data, 0,
                                           sdk::table::handle_t::null());
            api_params.cbdata = &fd;
            ret = mapping_tbl_->get(&api_params);
            if (ret == SDK_RET_OK) {
                remote_mapping_print_l3_header(fd);
                remote_l3_mapping_dump_cb(&api_params);
            }
        }
        if (type == MAPPING_DUMP_TYPE_REMOTE_L2) {
            mapping_swkey_t       mapping_key = { 0 };
            mapping_appdata_t     mapping_data = { 0 };
            sdk_ret_t             ret;

            PDS_IMPL_FILL_L2_MAPPING_SWKEY(&mapping_key,
                                           mapping_args->skey.subnet,
                                           mapping_args->skey.mac_addr);
            PDS_IMPL_FILL_TABLE_API_PARAMS(&api_params, &mapping_key, NULL,
                                           &mapping_data, 0,
                                           sdk::table::handle_t::null());
            api_params.cbdata = &fd;
            ret = mapping_tbl_->get(&api_params);
            if (ret == SDK_RET_OK) {
                remote_mapping_print_l2_header(fd);
                remote_l2_mapping_dump_cb(&api_params);
            }
        }
    }
    return SDK_RET_OK;
}

sdk_ret_t
mapping_impl_state::slab_walk(state_walk_cb_t walk_cb, void *ctxt) {
    walk_cb(mapping_impl_slab_, ctxt);
    return SDK_RET_OK;
}

const char *k_dhcp_ctl_ip = "127.0.0.1";
const int k_dhcp_ctl_port = 7911;
sdk_ret_t
mapping_impl_state::insert_dhcp_binding(const pds_mapping_spec_t *spec) {
    dhcpctl_status ret;
    dhcpctl_data_string ipaddr = NULL;
    dhcpctl_data_string mac = NULL;
    dhcpctl_status waitstatus = 0;
    dhcpctl_handle host = NULL;

    if (spec->skey.ip_addr.af != IP_AF_IPV4) {
        // No V6 support for now
        return SDK_RET_OK;
    }

    if (!dhcp_connection_) {
        ret = dhcpctl_connect(&dhcp_connection_, k_dhcp_ctl_ip, k_dhcp_ctl_port,
                              dhcpctl_null_handle);
        if (ret != ISC_R_SUCCESS) {
            PDS_TRACE_ERR("Failed to connect to dhcpd, err %u", ret);
            return SDK_RET_ERR;
        }
    }

    ret = dhcpctl_new_object(&host, dhcp_connection_, "host");
    if (ret != ISC_R_SUCCESS) {
        // If the connection fails for some reason(dhcpd restarted?) try to
        // establish the channel again.
        ret = dhcpctl_connect(&dhcp_connection_, k_dhcp_ctl_ip, k_dhcp_ctl_port,
                              dhcpctl_null_handle);
        if (ret != ISC_R_SUCCESS) {
            PDS_TRACE_ERR("Failed to connect to dhcpd, err %u", ret);
            return SDK_RET_ERR;
        }

        ret = dhcpctl_new_object(&host, dhcp_connection_, "host");
        if (ret != ISC_R_SUCCESS) {
            PDS_TRACE_ERR("Failed to allocate host object, err %u", ret);
            return SDK_RET_ERR;
        }
    }

    memset(&ipaddr, 0, sizeof ipaddr);
    ret = omapi_data_string_new (&ipaddr, IP4_ADDR8_LEN, MDL);
    if (ret != ISC_R_SUCCESS) {
        PDS_TRACE_ERR("Failed to allocate data string, err %u", ret);
        return SDK_RET_OOM;
    }
    sdk::lib::memrev(ipaddr->value, (uint8_t *)&spec->skey.ip_addr.addr.v4_addr, IP4_ADDR8_LEN);
    dhcpctl_set_value (host, ipaddr, "ip-address");

    memset(&mac, 0, sizeof(mac));
    ret = omapi_data_string_new(&mac, sizeof(spec->overlay_mac), MDL);
    if (ret != ISC_R_SUCCESS) {
        PDS_TRACE_ERR("Failed to allocate data string, err %u", ret);
        return SDK_RET_OOM;
    }
    memcpy(mac->value, spec->overlay_mac, sizeof(spec->overlay_mac));
    dhcpctl_set_value(host, mac, "hardware-address");

    dhcpctl_set_string_value(host, spec->key.str(), "name");
    dhcpctl_set_int_value(host, 1, "hardware-type");

    ret = dhcpctl_open_object(host, dhcp_connection_,
                              DHCPCTL_CREATE|DHCPCTL_EXCL);
    if (ret != ISC_R_SUCCESS) {
        PDS_TRACE_ERR("dhcp open object failure, err %u", ret);
        return SDK_RET_ERR;
    }

    ret = dhcpctl_wait_for_completion(host, &waitstatus);
    if (ret != ISC_R_SUCCESS) {
        PDS_TRACE_ERR("dhcp wait for completion failure, err %u", ret);
        return SDK_RET_ERR;
    }
    omapi_data_string_dereference(&ipaddr, MDL);
    omapi_data_string_dereference(&mac, MDL);
    omapi_object_dereference(&host, MDL);
    return SDK_RET_OK;
}

sdk_ret_t
mapping_impl_state::remove_dhcp_binding(const char *hostname) {
    dhcpctl_status ret = 0;
    dhcpctl_status waitstatus = 0;
    dhcpctl_handle host = NULL;

    if (!dhcp_connection_) {
        ret = dhcpctl_connect(&dhcp_connection_, k_dhcp_ctl_ip, k_dhcp_ctl_port,
                              dhcpctl_null_handle);
        if (ret != ISC_R_SUCCESS) {
            PDS_TRACE_ERR("Failed to connect to dhcpd, err %u", ret);
            return SDK_RET_ERR;
        }
    }

    memset(&host, 0, sizeof(host));
    ret = dhcpctl_new_object(&host, dhcp_connection_, "host");
    if (ret != ISC_R_SUCCESS) {
        ret = dhcpctl_connect(&dhcp_connection_, k_dhcp_ctl_ip, k_dhcp_ctl_port,
                              dhcpctl_null_handle);
        if (ret != ISC_R_SUCCESS) {
            PDS_TRACE_ERR("Failed to connect to dhcpd, err %u", ret);
            return SDK_RET_ERR;
        }

        // Try to create the object again.
        ret = dhcpctl_new_object(&host, dhcp_connection_, "host");
        if (ret != ISC_R_SUCCESS) {
            PDS_TRACE_ERR("Failed to allocate host object, err %u", ret);
            return SDK_RET_ERR;
        }
    }

    ret = dhcpctl_set_string_value(host, hostname, "name");
    if (ret != ISC_R_SUCCESS) {
        PDS_TRACE_ERR("Failed to allocate data string, err %u", ret);
        return SDK_RET_ERR;
    }

    ret = dhcpctl_open_object(host, dhcp_connection_, 0);
    if (ret != ISC_R_SUCCESS) {
        PDS_TRACE_ERR("dhcp open object failed, err %u", ret);
        return SDK_RET_ENTRY_NOT_FOUND;
    }

    ret = dhcpctl_wait_for_completion(host, &waitstatus);
    if (ret != ISC_R_SUCCESS) {
        PDS_TRACE_ERR("dhcp wait for completion failure, err %u", ret);
        return SDK_RET_ERR;
    }

    ret = dhcpctl_object_remove( dhcp_connection_, host );
    if (ret != ISC_R_SUCCESS) {
        PDS_TRACE_ERR("dhcp object remove failure, err %u", ret);
        return SDK_RET_ERR;
    }

    ret = dhcpctl_wait_for_completion(host, &waitstatus);
    if (ret != ISC_R_SUCCESS) {
        PDS_TRACE_ERR("dhcp wait for completion failure, err %u", ret);
        return SDK_RET_ERR;
    }
    omapi_object_dereference(&host, MDL);
    return SDK_RET_OK;
}

/// \@}

}    // namespace impl
}    // namespace api
