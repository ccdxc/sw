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

    //--------------------------------------
    // Physical port (Front panel port)
    //--------------------------------------
    NBB_BYTE port_add_update(ATG_LIPI_PORT_ADD_UPDATE* port_add_upd_ips);
    NBB_BYTE port_delete(NBB_ULONG port_ifindex);

    //----------------------------------
    // VRF
    //---------------------------------
    NBB_BYTE vrf_add_update(ATG_LIPI_VRF_ADD_UPDATE* vrf_add_upd_ips);
    NBB_BYTE vrf_delete(const NBB_BYTE* vrf_name, NBB_ULONG vrf_len);

    //----------------------------------
    // VXLAN tunnel (TEP)
    //---------------------------------
    NBB_BYTE vxlan_add_update(ATG_LIPI_VXLAN_ADD_UPDATE* vxlan_tnl_add_upd_ips);
    NBB_BYTE vxlan_delete(NBB_ULONG vxlan_tnl_ifindex);

    //----------------------------------
    // VXLAN port (TEP, VNI)
    //---------------------------------
    NBB_BYTE vxlan_port_add_update(ATG_LIPI_VXLAN_PORT_ADD_UPD* vxlan_port_add_upd_ips);
    NBB_BYTE vxlan_port_delete(NBB_ULONG vxlan_port_ifindex);

    //----------------------------------
    // IRB (SVI for overlay BD)
    //---------------------------------
    NBB_BYTE irb_add_update(ATG_LIPI_IRB_ADD_UPDATE* irb_add_upd_ips);
    NBB_BYTE irb_delete(NBB_ULONG irb_ifindex);

    //--------------------------------------------------
    // Software interface (Loopback and Dummy LIFs)
    //-------------------------------------------------
    NBB_BYTE softwif_add_update(ATG_LIPI_SOFTWIF_ADD_UPDATE* softwif_add_upd_ips);
    NBB_BYTE softwif_delete(NBB_ULONG softwif_ifindex, 
                            const NBB_CHAR (&if_name) [ATG_LIPI_NAME_MAX_LEN], 
                            NBB_ULONG softw_iftype);

    //--------------------------------------------------
    // Software interface IP (Loopback)
    //-------------------------------------------------
    NBB_BYTE softwif_addr_set(const NBB_CHAR *if_name,
                              ATG_LIPI_L3_IP_ADDR *ip_addr,
                              NBB_BYTE *vrf_name);
    NBB_BYTE softwif_addr_del(const NBB_CHAR *if_name,
                              ATG_LIPI_L3_IP_ADDR *ip_addr,
                              NBB_BYTE *vrf_name);
    NBB_BYTE softwif_addr_clear(const NBB_CHAR *if_name) {
        if_name = if_name; return ATG_OK;
    };

};

li_integ_subcomp_t* li_is (void);

} // End namespace

#endif
