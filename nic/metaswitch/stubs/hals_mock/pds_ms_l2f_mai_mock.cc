//---------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// MOCK APIs of Metaswitch L2F MAI stub
//---------------------------------------------------------------

#include "nic/apollo/api/include/pds.hpp"
namespace pds_ms {
void l2f_local_mac_ip_add(const pds_obj_key_t& subnet_key, const ip_addr_t& ip,
                          mac_addr_t mac, pds_ifindex_t lif_ifindex) {
    return;
}
void l2f_local_mac_ip_del(const pds_obj_key_t& subnet_key, const ip_addr_t& ip,
                          mac_addr_t mac) {
    return;
}
} // end namespace pds_ms
