//---------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// PDS-MS mock SMI HW ports, MAC description for Pegasus
//---------------------------------------------------------------

#ifndef PDS_MS_SMI_HPP_INCLUDED
#define PDS_MS_SMI_HPP_INCLUDED

#include <smi_c_includes.hpp>
#include <smi.h>
#include <hw_desc/smi_hw_desc.hpp>

#include <vector>

namespace pds_ms
{
// Pegasus implementation of the SMI Hardware Description class.
class mock_smi_hw_desc_t : public smi::HwDesc
{
public:
    //-------------------------------------------------------------------------
    // Constructor.
    //-------------------------------------------------------------------------
    mock_smi_hw_desc_t () {};
    ~mock_smi_hw_desc_t () {};
    void verify (void) {};

    //-------------------------------------------------------------------------
    // Reset the hardware description.
    //-------------------------------------------------------------------------
    void reset() {};

    //-------------------------------------------------------------------------
    // Create the hardware ports.
    //-------------------------------------------------------------------------
    bool create_ports(std::vector<smi::PortData> &) {return true;}

    //-------------------------------------------------------------------------
    // Allocate and free mac addresses from the pool.
    // This is called from MS LIM component whenever 
    //-------------------------------------------------------------------------
    bool allocate_mac_address(NBB_BYTE (&mac)[ATG_L2_MAC_ADDR_LEN])
                                 { return true; }
    void free_mac_address(NBB_BYTE (&mac)[ATG_L2_MAC_ADDR_LEN])
                                            { };

    bool create_default_port_settings(ATG_SMI_PORT_SETTINGS &) {return true;}
};

} // End namespace

#endif

