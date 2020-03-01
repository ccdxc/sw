//---------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// PDS-MS stub Remote VXLAN Tunnel Endpoint store
//---------------------------------------------------------------

#ifndef __PDS_MS_TEP_STORE_HPP__
#define __PDS_MS_TEP_STORE_HPP__

#include "nic/metaswitch/stubs/common/pds_ms_defs.hpp"
#include "nic/metaswitch/stubs/common/pds_ms_util.hpp"
#include "nic/metaswitch/stubs/common/pds_ms_ecmp_idx_guard.hpp"
#include "nic/metaswitch/stubs/common/pds_ms_object_store.hpp"
#include "nic/metaswitch/stubs/common/pds_ms_slab_object.hpp"
#include "nic/third-party/metaswitch/code/comn/ntlpp/ntl_comparators.hpp"
#include "nic/apollo/api/include/pds.hpp"
#include "nic/sdk/include/sdk/ip.hpp"
#include "nic/sdk/lib/slab/slab.hpp"
#include <cstdint>
#include <map>

static inline bool operator==(const ip_addr_t& a,
                              const ip_addr_t& b) {
    return (IPADDR_EQ(&a, &b));
}

namespace pds_ms {

class tep_obj_t : public slab_obj_t<tep_obj_t>,
                  public base_obj_t {
public:
    struct properties_t {
        ip_addr_t            tep_ip;
        ip_addr_t            src_ip;
        ms_ps_id_t           uecmp_ps;
        ms_hw_tbl_id_t       hal_uecmp_idx;
        ms_hw_tbl_id_t       hal_tep_idx;

        properties_t(const ip_addr_t& tep_ip_, const ip_addr_t& src_ip_, 
                     const ms_hw_tbl_id_t hal_uecmp_idx,
                     const ms_hw_tbl_id_t hal_tep_idx_) 
            : tep_ip(tep_ip_), src_ip(src_ip_), hal_uecmp_idx(hal_uecmp_idx),
              hal_tep_idx(hal_tep_idx_) {}
    };

    // TODO: Type 2 Overlay ECMP support - Needs list of back-pointers.
    // Without overlay ECMP support simplify to allocate 1-1 Overlay ECMP
    // entry for each TEP in PDS HAL. This avoids the need for ECMP Mgmt
    // and ref-counting based on MAC-IP create / delete. 
    // Overlay ECMP entries for Type-5 routes are managed independently by
    // Metaswitch HAL ROPI stub.

    // PDS HAL Overlay ECMP table entries are created from both the
    // MS VXLAN Tunnel(Type2) and MS Pathset(Type5) HAL stub objects
    // requiring index mgmt.
    // Index Guard provides automatic alloc/free of Overlay ECMP index
    // in the ctor/dtor.
    // Needs to be shared_ptr to avoid premature freeing of index
    // in destructor as part of update
    std::shared_ptr<ecmp_idx_guard_t> hal_oecmp_idx_guard;

    tep_obj_t(const ip_addr_t& tep_ip_, const ip_addr_t& src_ip_,
              ms_hw_tbl_id_t hal_uecmp_idx,
              ms_hw_tbl_id_t hal_tep_idx_);
    properties_t& properties(void) {return prop_;}
    const properties_t& properties(void) const {return prop_;}
    void set_properties(const properties_t& prop) {prop_ = prop;}

    ip_addr_t key(void) const {return prop_.tep_ip;}

    void update_store(state_t* state, bool op_delete) override;

    void print_debug_str(void) override {
        PDS_TRACE_DEBUG ("  - TEP: %s ", ipaddr2str (&(prop_.tep_ip)));
    }

    void add_l3_vxlan_port(ms_ifindex_t vxp_ifindex) {
        PDS_TRACE_DEBUG ("TEP %s Add L3 Vxlan Port 0x%x",
                         ipaddr2str (&(prop_.tep_ip)), vxp_ifindex);
        l3_vxlan_ports_.push_back(vxp_ifindex);
    }
    void del_l3_vxlan_port(ms_ifindex_t vxp_ifindex) {
        PDS_TRACE_DEBUG ("TEP %s Delete L3 Vxlan Port 0x%x",
                         ipaddr2str (&(prop_.tep_ip)), vxp_ifindex);
        l3_vxlan_ports_.erase(std::remove(l3_vxlan_ports_.begin(), l3_vxlan_ports_.end(),
                                          vxp_ifindex));
    }
    void walk_l3_vxlan_ports(std::function<bool(ms_ifindex_t)> cb) {
        for(auto vxp_ifi: l3_vxlan_ports_) {
            if (cb(vxp_ifi)) {
                return;
            }
        }
    }

private:
    properties_t  prop_;
    // List of L3 VXLAN ports created for this TEP
    // Assuming no duplicates - else need to change to std::set
    std::vector<ms_ifindex_t> l3_vxlan_ports_;
};

class tep_store_t : public obj_store_t<ip_addr_t, tep_obj_t, ip_hash> {
};

void tep_slab_init (slab_uptr_t slabs[], sdk::lib::slab_id_t slab_id);
}

#endif
