//---------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// PDSA stub Remote VXLAN Tunnel Endpoint store
//---------------------------------------------------------------

#ifndef __PDSA_TEP_STORE_HPP__
#define __PDSA_TEP_STORE_HPP__

// Somewhere in slab.hpp __FNAME__ is used that result in build failure 
#define __FNAME__ __FUNCTION__

#include "pdsa_ms_defs.hpp"
#include "pdsa_util.hpp"
#include "pdsa_object_store.hpp"
#include "pdsa_slab_object.hpp"
#include "ntl_comparators.hpp"
#include "apollo/api/include/pds.hpp"
#include "sdk/include/sdk/ip.hpp"
#include "sdk/lib/slab/slab.hpp"
#include <map>

namespace pdsa_stub {

struct tep_dmac_t {
     pds_tep_id_t hal_tep_idx;
     pds_nexthop_group_id_t hal_ov_ecmp_idx; // Need to be made list of back-pointers
                                             // when we support Overlay ecmp
};

class tep_obj_t : public slab_obj_t<tep_obj_t> {
public:
    struct properties_t {
        ip_addr_t      tep_ip;
        ms_ps_id_t     ms_ps_id;
        pds_tep_id_t   hal_tep_idx;
        // Need to be made list of back-pointers when we support Overlay ecmp
        // But for no overlay ECMP support case we can allocate 
        // the ECMP entry in HAL for each TEP at TEP create time
        // rather than MAC-IP mapping create.
        // This will avoid the need for ECMP Mgmt and ref-counting 
        // of VXLAN tunnel ECMP based on MAC-IP create / delete. 
        // Instead we can delete the ECMP entry as part of the TEP delete. 
        pds_nexthop_group_id_t  hal_ov_ecmp_idx; 

        properties_t(ip_addr_t tep_ip_, ms_ps_id_t ms_ps_id_, pds_tep_id_t hal_tep_idx_, 
                   pds_nexthop_group_id_t hal_ov_ecmp_idx_) 
            : tep_ip(tep_ip_), ms_ps_id(ms_ps_id_), hal_tep_idx(hal_tep_idx_),
              hal_ov_ecmp_idx(hal_ov_ecmp_idx_) {};
    };

    tep_obj_t(const properties_t& prop) : prop_(prop) {};

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

private:
    properties_t  prop_;
    // This list is incase the same TEP advertises multiple Router MACs
    // Since we are mostly aniticipating only a single MAC this is kept 
    // as a map of tep_dmac_t rather than pointers to tep_dmac_t
    std::map<mac_addr_wr_t, tep_dmac_t> dmacs_;  
};

// TODO: Need hash function for ip_addr_t to support ipv6
class tep_store_t : public obj_store_t <uint32_t, tep_obj_t> 
{
};

}

#endif
