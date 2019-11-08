//---------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// Implementation of Metaswitch LI stub integration 
//---------------------------------------------------------------
 
#include "nic/metaswitch/stubs/hals/pdsa_li.hpp"
#include "nic/metaswitch/stubs/common/pdsa_cookie.hpp"
#include "nic/metaswitch/stubs/common/pdsa_util.hpp"
#include "nic/metaswitch/stubs/common/pdsa_state.hpp"
#include "nic/apollo/api/include/pds_tep.hpp"
#include "nic/apollo/api/include/pds_nexthop.hpp"
#include "nic/sdk/include/sdk/ip.hpp"
#include "nic/sdk/lib/logger/logger.hpp"
#include "nic/metaswitch/stubs/hals/pdsa_li_vxlan_tnl.hpp"

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

NBB_BYTE 
LiIntegSubcomponent::vxlan_add_update(ATG_LIPI_VXLAN_ADD_UPDATE* vxlan_tnl_add_upd)
{
    try {
        pdsa_stub::li_vxlan_tnl vxtnl;
        if (vxtnl.handle_add_upd_ips (vxlan_tnl_add_upd)) {
            return ATG_ASYNC_COMPLETION;
        }
    } catch (Error& e) {
        SDK_TRACE_ERR ("Vxlan Tunnel Add Update processing failed ", e.what());
        return ATG_UNSUCCESSFUL;
    }
    return ATG_OK;
}
     

NBB_BYTE LiIntegSubcomponent::vxlan_delete(NBB_ULONG vxlan_tnl_ifindex)
{
    try {
        pdsa_stub::li_vxlan_tnl vxtnl;
        vxtnl.handle_delete (vxlan_tnl_ifindex);
        // TODO: Need to change this API to include the IPS 
        // TODO: For now send synchronous response to MS for deletes
    } catch (Error& e) {
        SDK_TRACE_ERR ("Vxlan Tunnel Add Update processing failed ", e.what());
        return ATG_UNSUCCESSFUL;
    }
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
