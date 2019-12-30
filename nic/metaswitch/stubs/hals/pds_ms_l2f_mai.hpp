//---------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// L2F MAI HAL integration
//---------------------------------------------------------------

#ifndef __PDS_MS_L2F_MAI_HPP__
#define __PDS_MS_L2F_MAI_HPP__

#include "nic/apollo/api/include/pds.hpp"
#include "nic/sdk/include/sdk/ip.hpp"

namespace pds_ms {

void l2f_local_mac_ip_add(pds_subnet_id_t subnet_id, const ip_addr_t& ip,
                      mac_addr_t mac, pds_ifindex_t lif_ifindex);
void l2f_local_mac_ip_del(pds_subnet_id_t subnet_id, const ip_addr_t& ip,
                          mac_addr_t mac);
} // End namespace
#endif
