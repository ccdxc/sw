//---------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// PDSA SMI HW ports, MAC description
//---------------------------------------------------------------

#ifndef PDSA_SMI_HPP_INCLUDED
#define PDSA_SMI_HPP_INCLUDED

#include <ntl_mac_pool.hpp>
#include <hw_desc/smi_hw_desc.hpp>
#include <vector>

namespace pds_ms
{
// PDSA implementation of the SMI Hardware Description class.
class pdsa_smi_hw_desc_t : public smi::HwDesc
{
public:
    //-------------------------------------------------------------------------
    // Constructor.
    //-------------------------------------------------------------------------
    pdsa_smi_hw_desc_t (NBB_BYTE (&)[ATG_L2_MAC_ADDR_LEN], NBB_ULONG);
    ~pdsa_smi_hw_desc_t () {};
    void verify(void) {};

    //-------------------------------------------------------------------------
    // Reset the hardware description.
    //-------------------------------------------------------------------------
    void reset() { mac_pool.reset(); }

    //-------------------------------------------------------------------------
    // Create the hardware ports.
    //-------------------------------------------------------------------------
    bool create_ports(std::vector<smi::PortData> &);

    //-------------------------------------------------------------------------
    // Allocate and free mac addresses from the mock MAC address pool.
    // This doesn't seem to be used anywhere but is needed to keep MS build happy
    //-------------------------------------------------------------------------
    bool allocate_mac_address(NBB_BYTE (&mac)[ATG_L2_MAC_ADDR_LEN])
                                 { return mac_pool.allocate_mac_address(mac); }
    void free_mac_address(NBB_BYTE (&mac)[ATG_L2_MAC_ADDR_LEN])
                                            { mac_pool.free_mac_address(mac); }

    //-------------------------------------------------------------------------
    // Create default port settings.  Will also call to allocate MAC
    // address.
    //-------------------------------------------------------------------------
    bool create_default_port_settings(ATG_SMI_PORT_SETTINGS &);

  private:
    //-------------------------------------------------------------------------
    // Mock MAC address pool.
    //-------------------------------------------------------------------------
    ntl::MacPool mac_pool;

};

} // End namespace

#endif
