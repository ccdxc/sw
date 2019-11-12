//---------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// PDSA LI stub DP APIs
//---------------------------------------------------------------

#ifndef __PDSA_LI_HPP__
#define __PDSA_LI_HPP__

#include <nbase.h>
#include <ntl_remote_object.hpp>
extern "C"
{
#include <lipi.h>
}

namespace pdsa_stub {

class li_integ_subcomp_t {
public:
    ~li_integ_subcomp_t() {};
      // Physical interface 
    NBB_BYTE port_add_update (ATG_LIPI_PORT_ADD_UPDATE* port_add_upd);
    NBB_BYTE port_delete (NBB_ULONG port_ifindex);

    NBB_BYTE vrf_add_update(ATG_LIPI_VRF_ADD_UPDATE* vrf_add_upd);
    NBB_BYTE vrf_delete(const NBB_BYTE* vrf_name, NBB_ULONG vrf_len);

    NBB_BYTE vxlan_add_update(ATG_LIPI_VXLAN_ADD_UPDATE* vxlan_tnl_add_upd);
    NBB_BYTE vxlan_delete(NBB_ULONG vxlan_tnl_ifindex);

    NBB_BYTE vxlan_port_add_update(ATG_LIPI_VXLAN_PORT_ADD_UPD* vxlan_port_add_upd);
    NBB_BYTE vxlan_port_delete(NBB_ULONG vxlan_port_ifindex);

    NBB_BYTE irb_add_update(ATG_LIPI_IRB_ADD_UPDATE* irb_add_upd);
    NBB_BYTE irb_delete(NBB_ULONG irb_ifindex);
};

li_integ_subcomp_t* li_is (void);

} // End namespace

#endif
