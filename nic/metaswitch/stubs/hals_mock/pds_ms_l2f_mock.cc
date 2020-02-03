//---------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// Mock implementation of Metaswitch L2F stub integration
//---------------------------------------------------------------

#include <l2f_c_includes.hpp>
#include "nic/metaswitch/stubs/hals/pds_ms_l2f.hpp"
#include "nic/metaswitch/stubs/common/pds_ms_state.hpp"
#include "nic/sdk/include/sdk/base.hpp"

namespace pds_ms {

using pds_ms::bd_obj_t;
using pds_ms::state_t;

void l2f_integ_subcomp_t::add_upd_bd(ATG_BDPI_UPDATE_BD *update_bd_ips)
{
}

void l2f_integ_subcomp_t::delete_bd(const ATG_L2_BD_ID *bd_id,
                                    NBB_CORRELATOR dp_bd_correlator)
{
}

void l2f_integ_subcomp_t::add_upd_bd_if(const ATG_L2_BD_ID *bd_id,
                                        NBB_CORRELATOR dp_bd_correlator,
                                        NBB_LONG bd_service_type,
                                        ATG_BDPI_INTERFACE_BIND *if_bind,
                                        const l2f::BdpiSlaveJoin &join)
{
}

void l2f_integ_subcomp_t::delete_bd_if(const ATG_L2_BD_ID *bd_id,
                                       NBB_CORRELATOR dp_bd_correlator,
                                       ATG_BDPI_INTERFACE_BIND *if_bind)
{
}

void l2f_integ_subcomp_t::add_upd_fdb_mac(ATG_BDPI_UPDATE_FDB_MAC *update_fdb_mac) {
}
void l2f_integ_subcomp_t::delete_fdb_mac(l2f::FdbMacKey *key) {
}
NBB_ULONG l2f_integ_subcomp_t::add_upd_mac_ip(ATG_MAI_MAC_IP_ID *mac_ip_id,
                                              NBB_BYTE sticky)
{
    return ATG_OK;
}

void l2f_integ_subcomp_t::delete_mac_ip(const ATG_MAI_MAC_IP_ID *mac_ip_id,
                                        bool programmed)
{
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
    return ATG_OK;
}

void l2f_integ_subcomp_t::delete_routers_mac(ATG_INET_ADDRESS *ip_address,
                                                 NBB_BYTE *mac_addr,
                                                 NBB_ULONG if_index,
                                                 const char *vrf_name)
{
}

sdk_ret_t
l2f_bd_update_pds_synch (state_t::context_t&& state_ctxt, uint32_t bd_id,
                         subnet_obj_t* subnet_obj)
{
    return SDK_RET_OK;
}

} // End namespace
