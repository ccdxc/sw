//---------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// PDSA stub Remote VXLAN Tunnel Endpoint store
//---------------------------------------------------------------

#ifndef __PDSA_TEP_STORE_HPP__
#define __PDSA_TEP_STORE_HPP__

#include "nic/metaswitch/stubs/common/pdsa_ms_defs.hpp"
#include "nic/metaswitch/stubs/common/pdsa_util.hpp"
#include "nic/metaswitch/stubs/common/pdsa_object_store.hpp"
#include "nic/metaswitch/stubs/common/pdsa_slab_object.hpp"
#include "nic/third-party/metaswitch/code/comn/ntlpp/ntl_comparators.hpp"
#include "nic/apollo/api/include/pds.hpp"
#include "nic/sdk/include/sdk/ip.hpp"
#include "nic/sdk/lib/slab/slab.hpp"
#include <cstdint>
#include <map>

namespace pdsa_stub {

struct tep_dmac_t {
    pds_tep_id_t hal_tep_idx;
    // TODO: Overlay ECMP support - Change to list of back-pointers.
    // Refer below.
    pds_nexthop_group_id_t hal_oecmp_idx;
};
class tep_store_t;

class tep_obj_t : public slab_obj_t<tep_obj_t>,
                  public base_obj_t {
public:
    struct properties_t {
        ip_addr_t               tep_ip;
        ms_ps_id_t              ms_ps_id;
        pds_nexthop_group_id_t  hal_uecmp_idx;
        pds_tep_id_t            hal_tep_idx;
        // TODO: Overlay ECMP support - Change to list of back-pointers.
        // Without overlay ECMP support simplify to tie the life of the 
        // ECMP entry for each TEP in HAL to the life of the TEP itself.
        // This avoids the need for ECMP Mgmt and ref-counting 
        // based on MAC-IP create / delete. 
        // ECMP entries for Type-5 routes are managed independently by 
        // Metaswitch ROPI stub.
        pds_nexthop_group_id_t  hal_oecmp_idx; 

        properties_t(ip_addr_t tep_ip_, pds_nexthop_group_id_t hal_uecmp_idx_, pds_tep_id_t hal_tep_idx_, 
                     pds_nexthop_group_id_t hal_oecmp_idx_) 
            : tep_ip(tep_ip_), hal_uecmp_idx(hal_uecmp_idx_), hal_tep_idx(hal_tep_idx_),
              hal_oecmp_idx(hal_oecmp_idx_) {};
    };

    tep_obj_t(const properties_t& prop) : prop_(prop) {};

    properties_t& properties(void) {return prop_;}
    const properties_t& properties(void) const {return prop_;}
    void set_properties(const properties_t& prop) {prop_ = prop;}

    bool add_upd_dmac(const mac_addr_wr_t& mac, const tep_dmac_t& tep_dmac) {
        dmacs_.emplace(mac,tep_dmac);
        return true;
    }
    void del_dmac(const mac_addr_wr_t& mac) {dmacs_.erase(mac);}
    const tep_dmac_t* dmac_info(const mac_addr_wr_t& mac) const;
    // return copy
    bool dmac_info(const mac_addr_wr_t& mac, tep_dmac_t* tep_dmac) const;
    uint32_t key(void) const {return prop_.tep_ip.addr.v4_addr;}

    void update_store(state_t* state, bool op_delete) override;

private:
    properties_t  prop_;
    // This list is incase the same TEP advertises multiple Router MACs
    // Since we are mostly aniticipating only a single MAC this is kept 
    // as a map of tep_dmac_t rather than pointers to tep_dmac_t
    std::map<mac_addr_wr_t, tep_dmac_t> dmacs_;  
};

// TODO: Need hash function for ip_addr_t to support ipv6
class tep_store_t : public obj_store_t<uint32_t, tep_obj_t> 
{
};

void tep_slab_init (slab_uptr_t slabs[], sdk::lib::slab_id_t slab_id);
}

#endif
