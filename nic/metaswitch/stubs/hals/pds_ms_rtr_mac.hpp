//---------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// Router MAC integration
//---------------------------------------------------------------

#ifndef __PDS_MS_RTR_MAC_HPP__
#define __PDS_MS_RTR_MAC_HPP__

#include <nbase.h>
extern "C"
{
#include <a0build.h>
#include <a0glob.h>
#include <a0spec.h>
#include <o0mac.h>
}

namespace pds_ms {
void rtr_mac_update(ATG_INET_ADDRESS *ip_address, NBB_BYTE *mac_addr,
                        NBB_ULONG if_index, const char *vrf_name,
                        bool is_delete);

}

#endif
