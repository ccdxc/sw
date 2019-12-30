//---------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// Mock implementation of Metaswitch LI stub integration 
//---------------------------------------------------------------
 
#include "nic/metaswitch/stubs/hals/pds_ms_li.hpp"
#include "nic/metaswitch/stubs/common/pds_ms_state.hpp"
#include "nic/sdk/include/sdk/base.hpp"

namespace pds_ms {

using pds_ms::vpc_obj_t;
using pds_ms::state_t;

li_integ_subcomp_t* li_is () 
{
    return nullptr;
}

NBB_BYTE li_integ_subcomp_t::port_add_update(ATG_LIPI_PORT_ADD_UPDATE* port_add_upd)
{
    return ATG_OK;
}

NBB_BYTE li_integ_subcomp_t::port_delete(NBB_ULONG port_ifindex)
{
    return ATG_OK;
}

NBB_BYTE li_integ_subcomp_t::vrf_add_update(ATG_LIPI_VRF_ADD_UPDATE* vrf_add_upd)
{
    return ATG_OK;
}

NBB_BYTE li_integ_subcomp_t::vrf_delete(const NBB_BYTE* vrf_name, NBB_ULONG vrf_len)
{
    return ATG_OK;
}

NBB_BYTE li_integ_subcomp_t::vxlan_add_update(ATG_LIPI_VXLAN_ADD_UPDATE* vxlan_tnl_add_upd)
{
    return ATG_OK;
}
     
NBB_BYTE li_integ_subcomp_t::vxlan_delete(NBB_ULONG vxlan_tnl_ifindex)
{
    return ATG_OK;
}

NBB_BYTE li_integ_subcomp_t::vxlan_port_add_update(ATG_LIPI_VXLAN_PORT_ADD_UPD* vxlan_port_add_upd)
{
    return ATG_OK;
}
NBB_BYTE li_integ_subcomp_t::vxlan_port_delete(NBB_ULONG vxlan_port_ifindex)
{
    return ATG_OK;
}

NBB_BYTE li_integ_subcomp_t::irb_add_update(ATG_LIPI_IRB_ADD_UPDATE* irb_add_upd)
{
    return ATG_OK;
}
NBB_BYTE li_integ_subcomp_t::irb_delete(NBB_ULONG irb_ifindex)
{
    return ATG_OK;
}

NBB_BYTE li_integ_subcomp_t::softwif_add_update(ATG_LIPI_SOFTWIF_ADD_UPDATE* softwif_add_upd) {
    return ATG_OK;
}

NBB_BYTE li_integ_subcomp_t::softwif_delete(NBB_ULONG softwif_ifindex, 
                                            const NBB_CHAR (&if_name) [ATG_LIPI_NAME_MAX_LEN], 
                                            NBB_ULONG softw_iftype) {
    return ATG_OK;
}

NBB_BYTE li_integ_subcomp_t::softwif_addr_set(const NBB_CHAR *if_name,
                                              ATG_LIPI_L3_IP_ADDR *ip_addr,
                                              NBB_BYTE *vrf_name) {
    return ATG_OK;
}

NBB_BYTE li_integ_subcomp_t::softwif_addr_del(const NBB_CHAR *if_name,
                                              ATG_LIPI_L3_IP_ADDR *ip_addr,
                                              NBB_BYTE *vrf_name) {
    return ATG_OK;
}

sdk_ret_t
li_vrf_update_pds_synch (state_t::context_t&& state_ctxt,
                         vpc_obj_t* vpc_obj)
{
    return SDK_RET_OK;
}
} // End namespace
