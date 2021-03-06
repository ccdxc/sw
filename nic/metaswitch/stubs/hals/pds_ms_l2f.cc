//---------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// PDS Implementation of Metaswitch L2F stub integration
//---------------------------------------------------------------
#include <l2f_c_includes.hpp>
#include "nic/metaswitch/stubs/hals/pds_ms_l2f.hpp"
#include "nic/metaswitch/stubs/hals/pds_ms_l2f_bd.hpp"
#include "nic/metaswitch/stubs/hals/pds_ms_l2f_mai.hpp"
#include "nic/metaswitch/stubs/hals/pds_ms_l2f_utils.hpp"
#include "nic/metaswitch/stubs/hals/pds_ms_rtr_mac.hpp"
#include "nic/metaswitch/stubs/common/pds_ms_state.hpp"
#include "nic/metaswitch/stubs/common/pds_ms_util.hpp"
#include "nic/sdk/lib/logger/logger.hpp"
#include <l2f_fte.hpp>

namespace pds_ms {

// Class that implements the L2F Integration subcomponent interface 
NBB_BYTE l2f_integ_subcomp_t::add_upd_bd(ATG_BDPI_UPDATE_BD *update_bd_ips) {
    NBB_BYTE rc;
    try {
        l2f_bd_t bd;
        rc = bd.handle_add_upd_ips(update_bd_ips);
    } catch (Error& e) {
        PDS_TRACE_ERR ("BD Add Update processing failed %s", e.what());
        rc = ATG_UNSUCCESSFUL;
    }
    return rc;
}

void l2f_integ_subcomp_t::delete_bd(const ATG_L2_BD_ID *bd_id,
                                    NBB_CORRELATOR dp_bd_correlator) {
    try {
        l2f_bd_t bd;
        bd.handle_delete(bd_id->bd_id);
    } catch (Error& e) {
        PDS_TRACE_ERR ("BD Del processing failed %s", e.what());
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
        PDS_TRACE_ERR ("BD If Add processing failed %s", e.what());
    }
}

void l2f_integ_subcomp_t::delete_bd_if(const ATG_L2_BD_ID *bd_id,
                                       NBB_CORRELATOR dp_bd_correlator,
                                       ATG_BDPI_INTERFACE_BIND *if_bind) {
    try {
        l2f_bd_t bd;
        bd.handle_del_if(bd_id->bd_id, if_bind->if_index);
    } catch (Error& e) {
        PDS_TRACE_ERR ("BD If Del processing failed %s", e.what());
    }
}

NBB_BYTE l2f_integ_subcomp_t::add_upd_fdb_mac(ATG_BDPI_UPDATE_FDB_MAC *update_fdb_mac) {
    NBB_BYTE rc;
    try {
        l2f_mai_t mai;
        rc = mai.handle_add_upd_mac(update_fdb_mac);
    } catch (Error& e) {
        rc = ATG_UNSUCCESSFUL;
        PDS_TRACE_ERR ("BDPI Remote MAC Add/Upd failed %s", e.what());
    }
    return rc;
}

void l2f_integ_subcomp_t::delete_fdb_mac(l2f::FdbMacKey *key) {
    try {
        l2f_mai_t mai;
        mai.handle_delete_mac(key);
    } catch (Error& e) {
        PDS_TRACE_ERR ("BDPI Remote MAC Delete failed %s", e.what());
    }
}

NBB_ULONG l2f_integ_subcomp_t::add_upd_mac_ip(ATG_MAI_UPDATE_MAC_IP *mac_ip_id,
                                              NBB_BYTE sticky)
{
    if (mac_ip_id->mac_ip_id.ip_address.length ==0) {
        return ATG_OK;
    }
    try {
        l2f_mai_t mai;
        mai.handle_add_upd_ip(mac_ip_id);
    } catch (Error& e) {
        PDS_TRACE_ERR ("BDPI Remote MAC Add/Upd failed %s", e.what());
    }
    return ATG_OK;
}

void l2f_integ_subcomp_t::delete_mac_ip(const ATG_MAI_MAC_IP_ID *mac_ip_id,
                                        bool programmed)
{
    if (mac_ip_id->ip_address.length ==0) {
        return;
    }
    try {
        l2f_mai_t mai;
        mai.handle_delete_ip(mac_ip_id);
    } catch (Error& e) {
        PDS_TRACE_ERR ("BDPI Remote MAC Add/Upd failed %s", e.what());
    }
    return;
}

NBB_BYTE l2f_integ_subcomp_t::add_upd_vrf_arp_entry(const ATG_MAI_MAC_IP_ID *mac_ip_id,
                                                    const char *vrf_name)
{
    return ATG_OK;
}

void l2f_integ_subcomp_t::delete_vrf_arp_entry(const ATG_MAI_MAC_IP_ID *mac_ip_id,
                                               const char *vrf_name)
{
}

NBB_BYTE l2f_integ_subcomp_t::update_routers_mac(ATG_INET_ADDRESS *ip_address,
                                                 NBB_BYTE *mac_addr,
                                                 NBB_ULONG if_index,
                                                 const char *vrf_name)
{
    ip_addr_t  tep_ip;
    ms_to_pds_ipaddr(*ip_address, &tep_ip);
    PDS_TRACE_INFO("Add Router's MAC for TEP %s MAC %s IfIndex 0x%x VRF %s ---- ",
                    ipaddr2str(&tep_ip), macaddr2str(mac_addr), if_index, vrf_name);
    rtr_mac_update (ip_address, mac_addr, if_index, vrf_name,
                    false /* create */);
    return ATG_OK;
}

void l2f_integ_subcomp_t::delete_routers_mac(ATG_INET_ADDRESS *ip_address,
                        NBB_BYTE *mac_addr,
                        NBB_ULONG if_index,
                        const char *vrf_name)
{
    ip_addr_t  tep_ip;
    ms_to_pds_ipaddr(*ip_address, &tep_ip);
    PDS_TRACE_INFO("Delete Router's MAC for TEP %s MAC %s IfIndex 0x%x VRF %s",
                    ipaddr2str(&tep_ip), macaddr2str(mac_addr), if_index, vrf_name);
    rtr_mac_update (ip_address, mac_addr, if_index, vrf_name,
                    true /* delete */);
}

} // End namespace
