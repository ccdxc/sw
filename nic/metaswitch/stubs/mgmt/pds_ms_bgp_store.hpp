//---------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// PDS Metaswitch stub Subnet Spec store used by Mgmt
//---------------------------------------------------------------

#ifndef __PDS_MS_BGP_STORE_HPP__
#define __PDS_MS_BGP_STORE_HPP__

#include "nic/metaswitch/stubs/common/pds_ms_defs.hpp"
#include "include/sdk/ip.hpp"

namespace pds_ms {
// bgp peer info
struct bgp_peer_obj_t {
    ip_addr_t local_addr;
    ip_addr_t peer_addr;
    bgp_peer_obj_t (const ip_addr_t& l, const ip_addr_t& p) :
        local_addr(l), peer_addr(p) {}
};

// structure to store pending peer info
struct bgp_peer_pend_obj_t {
    ip_addr_t local_addr;
    ip_addr_t peer_addr;
    bool      add_oper;
    bgp_peer_pend_obj_t (const ip_addr_t& l, const ip_addr_t& p, bool add) :
        local_addr(l), peer_addr(p), add_oper(add) {}
};

// bgp peer store
class bgp_peer_store_t {
public:
    void add (const ip_addr_t& l, const ip_addr_t& p) {
        SDK_TRACE_DEBUG ("bgp_peer_store: add %s/%s",
                          ipaddr2str(&l), ipaddr2str(&p));
        list_.emplace_back(l, p);
    }
    void del (const ip_addr_t& l, const ip_addr_t& p) {
        SDK_TRACE_DEBUG ("bgp_peer_store: del %s/%s",
                          ipaddr2str(&l), ipaddr2str(&p));
        list_.erase (std::remove_if(list_.begin(), list_.end(),
                                    [l, p](bgp_peer_obj_t& obj) {
                                    return (IPADDR_EQ(&l, &obj.local_addr) &&
                                    IPADDR_EQ(&p, &obj.peer_addr)); }),
                     list_.end());
    }
    bool find (const ip_addr_t& l, const ip_addr_t& p) {
        for (auto& obj: list_) {
            if (IPADDR_EQ(&l, &obj.local_addr) &&
                IPADDR_EQ(&p, &obj.peer_addr)) { return true; }
        }
        return false;
    }

private:  
    std::vector<bgp_peer_obj_t> list_;
};

} // End namespace

#endif
