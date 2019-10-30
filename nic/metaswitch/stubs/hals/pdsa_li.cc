//---------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// Implementation of Metaswitch LI stub integration 
//---------------------------------------------------------------
 
#include "pdsa_li.hpp"
#include "pdsa_util.hpp"

namespace pdsa_stub {

NBB_BYTE LiIntegSubcomponent::vrf_add_update(ATG_LIPI_VRF_ADD_UPDATE* vrf_add_upd)
{
    auto vrf_id = vrfname_2_vrfid(vrf_add_upd->vrf_name, vrf_add_upd->vrf_name_len);
    vrf_id = vrf_id;
    return ATG_OK;
}
NBB_BYTE LiIntegSubcomponent::vrf_delete(const NBB_BYTE* vrf_name, NBB_ULONG vrf_len)
{
    return ATG_OK;
}

NBB_BYTE LiIntegSubcomponent::vxlan_add_update(ATG_LIPI_VXLAN_ADD_UPDATE* vxlan_tnl_add_upd)
{
    return ATG_OK;
}
NBB_BYTE LiIntegSubcomponent::vxlan_delete(NBB_ULONG vxlan_tnl_ifindex)
{
    return ATG_OK;
}

NBB_BYTE LiIntegSubcomponent::vxlan_port_add_update(ATG_LIPI_VXLAN_PORT_ADD_UPD* vxlan_port_add_upd)
{
    return ATG_OK;
}
NBB_BYTE LiIntegSubcomponent::vxlan_port_delete(NBB_ULONG vxlan_port_ifindex)
{
    return ATG_OK;
}

NBB_BYTE LiIntegSubcomponent::irb_add_update(ATG_LIPI_IRB_ADD_UPDATE* irb_add_upd)
{
    return ATG_OK;
}
NBB_BYTE LiIntegSubcomponent::irb_delete(NBB_ULONG irb_ifindex)
{
    return ATG_OK;
}
}
