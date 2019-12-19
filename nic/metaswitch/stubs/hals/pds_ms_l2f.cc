//---------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// PDS Implementation of Metaswitch L2F stub integration
//---------------------------------------------------------------
#include <l2f_c_includes.hpp>
#include "nic/metaswitch/stubs/hals/pds_ms_l2f.hpp"
#include "nic/metaswitch/stubs/hals/pds_ms_l2f_bd.hpp"
#include "nic/metaswitch/stubs/common/pdsa_state.hpp"
#include "nic/metaswitch/stubs/common/pdsa_util.hpp"
#include "nic/sdk/lib/logger/logger.hpp"

namespace pds_ms {
using pdsa_stub::Error;

// Class that implements the L2F Integration subcomponent interface 
void l2f_integ_subcomp_t::add_upd_bd(ATG_BDPI_UPDATE_BD *update_bd_ips) {
    try {
        l2f_bd_t bd;
        bd.handle_add_upd_ips(update_bd_ips);
    } catch (Error& e) {
        SDK_TRACE_ERR ("BD Add Update processing failed %s", e.what());
        update_bd_ips->return_code = ATG_UNSUCCESSFUL;
    }
    // Always return ATG_OK - fill the actual return code in the IPS
}

void l2f_integ_subcomp_t::delete_bd(const ATG_L2_BD_ID *bd_id,
                                    NBB_CORRELATOR dp_bd_correlator) {
    try {
        l2f_bd_t bd;
        bd.handle_delete(bd_id->bd_id);
    } catch (Error& e) {
        SDK_TRACE_ERR ("BD Del processing failed %s", e.what());
    }
}

void l2f_integ_subcomp_t::add_upd_bd_if(const ATG_L2_BD_ID *bd_id,
                                        NBB_CORRELATOR dp_bd_correlator,
                                        NBB_LONG bd_service_type,
                                        ATG_BDPI_INTERFACE_BIND *if_bind,
                                        const l2f::BdpiSlaveJoin &join) {
    try {
        l2f_bd_t bd;
        bd.handle_add_if(bd_id->bd_id, if_bind->if_index);
    } catch (Error& e) {
        SDK_TRACE_ERR ("BD If Add processing failed %s", e.what());
    }
}

void l2f_integ_subcomp_t::delete_bd_if(const ATG_L2_BD_ID *bd_id,
                                       NBB_CORRELATOR dp_bd_correlator,
                                       ATG_BDPI_INTERFACE_BIND *if_bind) {
    try {
        l2f_bd_t bd;
        bd.handle_del_if(bd_id->bd_id, if_bind->if_index);
    } catch (Error& e) {
        SDK_TRACE_ERR ("BD If Del processing failed %s", e.what());
    }
}

void l2f_integ_subcomp_t::add_upd_fdb_mac(ATG_BDPI_UPDATE_FDB_MAC *update_fdb_mac) {
    SDK_TRACE_INFO("L2F FDB ADD BD %d MAC %s", update_fdb_mac->bd_id.bd_id,
                   macaddr2str(update_fdb_mac->mac_address));
}
void l2f_integ_subcomp_t::delete_fdb_mac(l2f::FdbMacKey *key) {
    SDK_TRACE_INFO("L2F FDB DEL BD %d MAC %s", key->bd_id.bd_id,
                   macaddr2str(key->mac_address));
}
NBB_ULONG l2f_integ_subcomp_t::add_upd_mac_ip(ATG_MAI_MAC_IP_ID *mac_ip_id,
                                              NBB_BYTE sticky)
{
    if (mac_ip_id->ip_address.length ==0) {
        SDK_TRACE_INFO("L2F MAC IP ADD BD %d MAC only %s",
                       mac_ip_id->bd_id.bd_id, macaddr2str(mac_ip_id->mac_address));
        return ATG_OK;
    }
    ip_addr_t ip;
    pdsa_stub::ms_to_pds_ipaddr(mac_ip_id->ip_address, &ip);
    SDK_TRACE_INFO("L2F MAC IP ADD BD %d IP %s MAC %s", mac_ip_id->bd_id.bd_id,
                   ipaddr2str(&ip), macaddr2str(mac_ip_id->mac_address));
    return ATG_OK;
}

void l2f_integ_subcomp_t::delete_mac_ip(const ATG_MAI_MAC_IP_ID *mac_ip_id,
                                        bool programmed)
{
    if (mac_ip_id->ip_address.length ==0) {
        SDK_TRACE_INFO("L2F MAC IP DEL BD %d MAC only %s", mac_ip_id->bd_id.bd_id,
                       macaddr2str(mac_ip_id->mac_address));
        return;
    }
    ip_addr_t ip;
    pdsa_stub::ms_to_pds_ipaddr(mac_ip_id->ip_address, &ip);
    SDK_TRACE_INFO("L2F MAC IP DEL BD %d IP %s MAC %s", mac_ip_id->bd_id.bd_id,
                   ipaddr2str(&ip), macaddr2str(mac_ip_id->mac_address));
}
NBB_BYTE l2f_integ_subcomp_t::add_upd_vrf_arp_entry(const ATG_MAI_MAC_IP_ID *mac_ip_id,
                                                    const char *vrf_name)
{
    ip_addr_t ip;
    pdsa_stub::ms_to_pds_ipaddr(mac_ip_id->ip_address, &ip);
    SDK_TRACE_INFO("L2F VRF MAC IP ADD BD %d IP %s MAC %s VRF %s", mac_ip_id->bd_id.bd_id,
                   ipaddr2str(&ip), macaddr2str(mac_ip_id->mac_address), vrf_name);
    return ATG_OK;
}

void l2f_integ_subcomp_t::delete_vrf_arp_entry(const ATG_MAI_MAC_IP_ID *mac_ip_id,
                                               const char *vrf_name)
{
    ip_addr_t ip;
    pdsa_stub::ms_to_pds_ipaddr(mac_ip_id->ip_address, &ip);
    SDK_TRACE_INFO("L2F VRF MAC IP DEL BD %d IP %s MAC %s VRF %s", mac_ip_id->bd_id.bd_id,
                   ipaddr2str(&ip), macaddr2str(mac_ip_id->mac_address), vrf_name);
}

} // End namespace
