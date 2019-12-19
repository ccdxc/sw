//---------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// PDS Implementation of Metaswitch L2F MAI stub integration
//---------------------------------------------------------------
#include <l2f_c_includes.hpp>
#include "nic/metaswitch/stubs/common/pdsa_util.hpp"
#include "nic/metaswitch/stubs/common/pdsa_ms_defs.hpp"
#include "nic/metaswitch/stubs/common/pds_ms_ifindex.hpp"
#include "nic/sdk/lib/logger/logger.hpp"
#include <l2f_integ_api.hpp>

extern int l2f_proc_id;
namespace pds_ms {
using pdsa_stub::ms_ifindex_t;

void 
l2f_local_mac_ip_add (pds_subnet_id_t subnet_id, const ip_addr_t& ip,
                      mac_addr_t mac, pds_ifindex_t lif_ifindex)
{
    NBB_CREATE_THREAD_CONTEXT
    NBS_ENTER_SHARED_CONTEXT(l2f_proc_id);
    NBS_GET_SHARED_DATA();

    ms_ifindex_t ms_lif_index = pds_to_ms_ifindex(lif_ifindex, IF_TYPE_LIF);
    if (ip_addr_is_zero(&ip)) {
        SDK_TRACE_DEBUG("Advertise MAC learn for BD %d MAC %s LIF 0x%x MS-LIF 0x%x",
                        subnet_id, macaddr2str(mac), lif_ifindex, ms_lif_index);
    } else {
        SDK_TRACE_DEBUG("Advertise IP-MAC learn for BD %d IP %s MAC %s LIF 0x%x MS-LIF 0x%x",
                        subnet_id, ipaddr2str(&ip), macaddr2str(mac), lif_ifindex, ms_lif_index);
    }

    ATG_MAI_MAC_IP_ID mac_ip_id = {0};
    mac_ip_id.bd_id.bd_type = ATG_L2_BRIDGE_DOMAIN_EVPN;
    mac_ip_id.bd_id.bd_id = subnet_id;
    memcpy(mac_ip_id.mac_address, mac, ETH_ADDR_LEN);

    // Notify MAC only first
    auto ret = l2f::l2f_cc_is_mac_add_update(&mac_ip_id, ms_lif_index);
    if (ret != ATG_OK) {
        SDK_TRACE_ERR("Adding local MAC to MS failed for BD %d MAC %s",
                      subnet_id, macaddr2str(mac));
    }

    if (!ip_addr_is_zero(&ip)) {
        // Then Notify IP along with MAC
        pdsa_stub::pds_to_ms_ipaddr(ip, &mac_ip_id.ip_address);
        ret = l2f::l2f_cc_is_mac_add_update(&mac_ip_id, ms_lif_index);
        if (ret != ATG_OK) {
            SDK_TRACE_ERR("Adding local IP-MAC to MS failed for BD %d IP %s MAC %s",
                          subnet_id, ipaddr2str(&ip), macaddr2str(mac));
        }
    }

    NBS_RELEASE_SHARED_DATA();
    NBS_EXIT_SHARED_CONTEXT();
    NBB_DESTROY_THREAD_CONTEXT
}

void 
l2f_local_mac_ip_del (pds_subnet_id_t subnet_id, const ip_addr_t& ip,
                      mac_addr_t mac)
{
    NBB_CREATE_THREAD_CONTEXT
    NBS_ENTER_SHARED_CONTEXT(l2f_proc_id);
    NBS_GET_SHARED_DATA();

    if (ip_addr_is_zero(&ip)) {
        SDK_TRACE_DEBUG("Received MAC remove for BD %d MAC %s",
                        subnet_id, macaddr2str(mac));
    } else {
        SDK_TRACE_DEBUG("Received MAC-IP remove for BD %d IP %s MAC %s",
                        subnet_id, ipaddr2str(&ip), macaddr2str(mac));
    }

    ATG_MAI_MAC_IP_ID mac_ip_id = {0};
    mac_ip_id.bd_id.bd_type = ATG_L2_BRIDGE_DOMAIN_EVPN;
    mac_ip_id.bd_id.bd_id = subnet_id;
    memcpy(mac_ip_id.mac_address, mac, ETH_ADDR_LEN);
    if (!ip_addr_is_zero(&ip)) {
        pdsa_stub::pds_to_ms_ipaddr(ip, &mac_ip_id.ip_address);
    }
    l2f::l2f_cc_is_mac_delete(mac_ip_id);

    NBS_RELEASE_SHARED_DATA();
    NBS_EXIT_SHARED_CONTEXT();
    NBB_DESTROY_THREAD_CONTEXT
}
}
