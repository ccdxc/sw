//---------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// Router MAC integration
//---------------------------------------------------------------

#include "nic/metaswitch/stubs/hals/pds_ms_rtr_mac.hpp"
#include <nar_fte.hpp>
#include <nar_arp_nl.hpp>

namespace pds_ms {
void rtr_mac_update(ATG_INET_ADDRESS *ip_address,
                        NBB_BYTE *mac_addr,
                        NBB_ULONG if_index,
                        const char *vrf_name,
                        bool is_delete)
{
    NBS_ENTER_SHARED_CONTEXT(nar::Fte::proc_id());
    NBS_GET_SHARED_DATA();

    auto& nar_fte = nar::Fte::get();
    nar::ArpNetlink *arp_netlink = nar_fte.get_arp_netlink();
    arp_netlink->evpn_router_mac_arp_event(ip_address, mac_addr, if_index,
                                           is_delete);

    NBS_RELEASE_SHARED_DATA();
    NBS_EXIT_SHARED_CONTEXT();
}

}
