//---------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// PDS MS EVPN Type 2 remote MAC store
//---------------------------------------------------------------

#ifndef __PDS_MS_MAC_STORE_HPP__
#define __PDS_MS_MAC_STORE_HPP__

#include "nic/metaswitch/stubs/common/pds_ms_defs.hpp"
#include "nic/metaswitch/stubs/common/pds_ms_object_store.hpp"
#include "nic/metaswitch/stubs/common/pds_ms_slab_object.hpp"
#include "nic/metaswitch/stubs/common/pds_ms_util.hpp"
#include "nic/sdk/include/sdk/eth.hpp"
#include "nic/sdk/include/sdk/ip.hpp"
#include "nic/sdk/lib/slab/slab.hpp"
#include <set>

// For the std::set of IP addresses
static inline bool operator<(const ip_addr_t& a, const ip_addr_t& b) {
    return IPADDR_LT(&a, &b);
}

namespace pds_ms {

// Holds MAC addresses received in EVPN Type 2 routes from other TEPs
class mac_obj_t : public slab_obj_t<mac_obj_t>,
                  public base_obj_t {
public:
    struct key_t {
        ms_bd_id_t bd_id;
        mac_addr_t mac;  
        key_t(ms_bd_id_t in_bd_id, const mac_addr_t& in_mac)
            : bd_id(in_bd_id) {
            MAC_ADDR_COPY(mac, in_mac);
        }
        bool operator==(const mac_obj_t::key_t& k) const {
            if (bd_id != k.bd_id) return false;
            if (memcmp(mac, k.mac, ETH_ADDR_LEN) != 0) return false;
            return true;
        }
    } __PACK__; // Need to pack to get consistent hash code
    key_t  key;
    bool hal_created = false;
    // Temporary list of Type 2 IP addresses associated with this MAC
    // that MS programmed out-of-seq before the MAC itself
    std::vector<ip_addr_t> out_of_seq_ip;
    // Using a Set for easier comparison when Overlay ECMP is supported
    std::set<ip_addr_t> tep_ip_list;
    mac_obj_t(const key_t& k) : key(k) {};
};

class mac_hash {
public:
    std::size_t operator()(const mac_obj_t::key_t& mac_key) const {
        return hash_algo::fnv_hash((void *)&mac_key, sizeof(mac_key));
    }
};

class mac_store_t : public obj_store_t <mac_obj_t::key_t, mac_obj_t,
                                            mac_hash> 
{
};
void mac_slab_init (slab_uptr_t slabs[], sdk::lib::slab_id_t slab_id);

} // End namespace

#endif

