//---------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// PDSA SMI HW description
//---------------------------------------------------------------

#include "nic/metaswitch/stubs/hals/pdsa_smi_hw_desc.hpp"
#include "nic/metaswitch/stubs/common/pdsa_util.hpp"
#include <lipi.h>
#include <lim_smi_mac.hpp>
#include "nic/apollo/api/pds_state.hpp"
#include "nic/sdk/include/sdk/eth.hpp"
#include "nic/sdk/include/sdk/base.hpp"
#include "nic/sdk/include/sdk/if.hpp"
#include "nic/sdk/lib/logger/logger.hpp"
#include "nic/sdk/platform/fru/fru.hpp"

//-------------------------------------------------------------------------
// Shared function used by LIM and SMI for MAC handling.
// Used to initialize LIM hardware manager MAC address pool.
// LIM allocates MAC address from this pool for 
// System MAC, IRB interfaces and Loopback interfaces
// The system MAC will be used as Routers MAC in Type-5 routes generated
// unless it is overridden by setting evpnIpVrfRoutersMac when creating VRF
//-------------------------------------------------------------------------
NBB_ULONG get_initial_mac_address(NBB_BYTE (&initial_mac)[ATG_L2_MAC_ADDR_LEN],
                                  NBB_BOOL system_mac_requested,
                                  NBB_ULONG suggested_number)
{
    std::string value;
    auto ret = sdk::platform::readFruKey(NUMMACADDR_KEY, value);
    if (ret == -1) {
        SDK_TRACE_ERR("Could not read FRU num-mac-address");
        return 0;
    }
    NBB_ULONG total_num_macs = atoi (value.c_str());

    ret = sdk::platform::readFruKey(MACADDRESS_KEY, value);
    if (ret == -1) {
        SDK_TRACE_ERR("Could not read FRU mac-address");
        return 0;
    }
    mac_str_to_addr((char*) value.c_str(), initial_mac);

    return total_num_macs;
}

namespace smi
{
// Replace the MS stock smi::create_hw_desc function 
// with our PDSA version that creates instance of our class
HwDesc*
create_hw_desc(void)
{
    NBB_BYTE initial_mac_address[ATG_L2_MAC_ADDR_LEN];
    NBB_ULONG total_macs = get_initial_mac_address(initial_mac_address,
                                                   false, 0);

    static pdsa_stub::pdsa_smi_hw_desc_t g_hw_desc_(initial_mac_address, total_macs);
    return &g_hw_desc_;
}
} // End namespace

namespace pdsa_stub 
{
pdsa_smi_hw_desc_t::pdsa_smi_hw_desc_t(NBB_BYTE (&mac_address)[ATG_L2_MAC_ADDR_LEN],
                     NBB_ULONG total_macs) :
    mac_pool(mac_address, total_macs)
{
    reset();
}

bool pdsa_smi_hw_desc_t::create_ports(std::vector <smi::PortData> &port_config)
{
    SDK_TRACE_INFO("Creating uplinks ...");

    ATG_SMI_PORT_SETTINGS settings = {};
    create_default_port_settings(settings);

    for (uint32_t port = 1;
         port <= api::g_pds_state.catalogue()->num_fp_ports(); port++) {
        // Using the HAL L3 IfIndex as the Metaswitch IfIndex avoids 
        // the need for IfIndex translation in the MS HAL Stubs
        uint32_t  ifindex = IFINDEX(IF_TYPE_L3, 
                                    api::g_pds_state.catalogue()->slot(),
                                    port, ETH_IF_DEFAULT_CHILD_PORT);

        smi::PortData temp_port = smi::PortData();
        temp_port.id.module_id = 1;
        temp_port.id.slot_id = api::g_pds_state.catalogue()->slot();
        long_to_3_byte(temp_port.id, ifindex);

        // Hardcoding the interface name to be same as Linux interface names avoids
        // the need for translation in FT-stub when programming routes in Linux
        // uplink 1 - inb_mnic0
        // uplink 2 - inb_mnic1
        // TODO: This needs to be changed to use L3 interface name
        // And also if linux interface name for the uplink is not fixed then 
        // need a way to derive dynamically here or in the FT-Stub
        std::string if_name = "inb_mnic";
        if_name += std::to_string(port-1);
        strncpy (temp_port.id.if_name, if_name.c_str(), AMB_LIM_NAME_MAX_LEN);

        // Fetch Linux MAC address
        temp_port.settings = settings;
        auto& mac_address = temp_port.settings.mac_address;
        if (!get_interface_mac_address (if_name, mac_address)) {
            // Set a dummy MAC
            // Need to fetch it later in LI stub when LIM interface is created
            ATG_SET_ZERO_MAC(mac_address);
            SDK_TRACE_DEBUG ("Could not find Linux interface %s. Using dummy MAC", 
                             port);
            mac_address[5] = port;
        }

        SDK_TRACE_INFO ("Metaswitch SMI port add id=%d name=%s ifindex=0x%x MAC=%s", 
                        port, if_name, ifindex, macaddr2str(mac_address));

        port_config.push_back(temp_port);
    }
    return true;
}

bool pdsa_smi_hw_desc_t::create_default_port_settings(ATG_SMI_PORT_SETTINGS &settings)
{
    settings.enabled = true;
    settings.support_autoneg = true;
    settings.duplex_mode = ATG_LIPI_DUPLEX_FULL;
    settings.is_if_pointtopoint = true;

    //-------------------------------------------------------------------------
    // Port speeds and rate limits are set to 0, which means "maximum".
    //-------------------------------------------------------------------------
    settings.port_speed = 0;
    settings.peak_info_rate = 0;
    settings.peak_burst_size = 0;
    settings.mcast_rate_limit = 0;
    settings.bcast_rate_limit = 0;
    settings.uucast_rate_limit = 0;
    settings.mcast_burst_limit = 0;
    settings.bcast_burst_limit = 0;
    settings.uucast_burst_limit = 0;

    //-------------------------------------------------------------------------
    // Default modes for Int hardware.
    //-------------------------------------------------------------------------
    settings.ethernet_max_frame_size = ATG_L2_DEFAULT_MTU;
    settings.color_coding_mode = ATG_LIPI_PORT_COLOR_DEI;
    settings.pcp_mode = ATG_LIPI_PCP_8P0D;
    settings.hqos_supported = ATG_NO;
    settings.tagging_mode = ATG_LIPI_PORT_TAGGING_NONE;
    settings.loopback_mode = ATG_LIPI_PORT_LOOPBACK_NONE;
    settings.vlan_id = 0;
    settings.untagged_priority = ATG_LIPI_PCP_MIN;
    settings.tpid = 0x8100;;
    settings.support_stats = true;
    settings.pause_tx = false;
    settings.pause_rx = false;
    settings.phy_info = 0;

    //-------------------------------------------------------------------------
    // Default bandwidth.
    //-------------------------------------------------------------------------
    NBB_UINT64_ASSIGN(settings.bandwidth, ATG_L2_ETH_BANDWIDTH_1G);

    return true;
}

} // End namespace

