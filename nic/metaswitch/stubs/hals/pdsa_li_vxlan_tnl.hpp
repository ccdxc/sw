//---------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// LI VXLAN Tunnel HAL integration
//---------------------------------------------------------------

#ifndef __PDSA_LI_VXLAN_TNL_HPP__
#define __PDSA_LI_VXLAN_TNL_HPP__

#include "nic/metaswitch/stubs/common/pdsa_cookie.hpp"
#include "nic/metaswitch/stubs/common/pdsa_util.hpp"
#include "nic/metaswitch/stubs/common/pdsa_state.hpp"
#include "nic/apollo/api/include/pds_tep.hpp"
#include "nic/apollo/api/include/pds_nexthop.hpp"
#include "nic/sdk/include/sdk/ip.hpp"

#include <nbase.h>
extern "C"
{
#include <a0build.h>
#include <a0glob.h>
#include <ntloffst.h>
#include <a0spec.h>
#include <o0mac.h>
#include "lipi.h"
}

namespace pdsa_stub {

class li_vxlan_tnl {
public:    
   bool handle_add_upd_ips(ATG_LIPI_VXLAN_ADD_UPDATE* vxlan_tnl_add_upd);
   bool handle_delete(NBB_ULONG vxlan_tnl_ifindex);

private:
    struct ips_info_t {
        ms_ifindex_t if_index;
        ip_addr_t    tep_ip;
        ip_addr_t    src_ip;
        pds_nexthop_group_id_t hal_uecmp_idx; 
    };

    struct store_info_t {
        tep_obj_t*   tep_obj;
        if_obj_t*    tun_if_obj;
    };

private:
    std::unique_ptr<cookie_t> cookie_uptr_;
    pds_batch_ctxt_guard_t bctxt_guard_;
    store_info_t  store_info_;
    ips_info_t  ips_info_;
    bool op_create_ = false;
    bool op_delete_ = false;

private:
    void cache_obj_in_cookie_for_create_op_(void); 
    bool cache_obj_in_cookie_for_update_op_(void);
    void cache_obj_in_cookie_for_delete_op_(void); 
    pds_batch_ctxt_guard_t make_batch_pds_spec_ (void);

    void parse_ips_info_(ATG_LIPI_VXLAN_ADD_UPDATE* vxlan_tnl_add_upd) {
        ips_info_.if_index = vxlan_tnl_add_upd->id.if_index;
        ATG_INET_ADDRESS& ms_dest_ip = vxlan_tnl_add_upd->vxlan_settings.dest_ip;
        pdsa_stub::convert_ipaddr_ms_to_pdsa(ms_dest_ip, &ips_info_.tep_ip);
        ATG_INET_ADDRESS& ms_src_ip = vxlan_tnl_add_upd->vxlan_settings.source_ip;
        pdsa_stub::convert_ipaddr_ms_to_pdsa(ms_src_ip, &ips_info_.src_ip);
        NBB_CORR_GET_VALUE(ips_info_.hal_uecmp_idx, vxlan_tnl_add_upd->id.hw_correlator);
    }

    void fetch_store_info_(pdsa_stub::state_t* state) {
        store_info_.tun_if_obj = state->if_store().get(ips_info_.if_index);
        if (op_delete_) {
            auto& tun_prop = store_info_.tun_if_obj->vxlan_tunnel_properties();
            store_info_.tep_obj = state->tep_store().get(tun_prop.tep_ip.addr.v4_addr);
        } else {
            store_info_.tep_obj = state->tep_store().get(ips_info_.tep_ip.addr.v4_addr);
        }
    }

    pds_tep_key_t make_pds_tep_key_(void) {
        pds_tep_key_t key; 
        auto& tep_prop = store_info_.tep_obj->properties();
        key.id = tep_prop.hal_tep_idx;
        return key;
    }

    pds_nexthop_group_key_t make_pds_nhgroup_key_(void) {
        pds_nexthop_group_key_t key; 
        auto& tep_prop = store_info_.tep_obj->properties();
        key.id = tep_prop.hal_oecmp_idx;
        return key;
    }

    pds_tep_spec_t make_pds_tep_spec_(void) {
        pds_tep_spec_t spec;
        auto& tep_prop = store_info_.tep_obj->properties();
        spec.key = make_pds_tep_key_();
        spec.remote_ip = tep_prop.tep_ip;
        spec.ip_addr = ips_info_.src_ip;
        spec.nh_type = PDS_NH_TYPE_UNDERLAY_ECMP;
        spec.nh_group.id = tep_prop.hal_uecmp_idx;
        spec.type = PDS_TEP_TYPE_WORKLOAD;
        spec.nat = false;
        return spec;
    }

    pds_nexthop_group_spec_t make_pds_nhgroup_spec_(void) {
        pds_nexthop_group_spec_t spec;
        auto& tep_prop = store_info_.tep_obj->properties();
        spec.key = make_pds_nhgroup_key_();
        spec.type = PDS_NHGROUP_TYPE_OVERLAY_ECMP;
        spec.num_nexthops = 1;
        spec.nexthops[0].key.id = 0; // Unused for NHs pointing to TEPs
        spec.nexthops[0].type = PDS_NH_TYPE_OVERLAY;
        // Use the TEP MS IfIndex as the TEP Index
        spec.nexthops[0].tep.id = tep_prop.hal_tep_idx;
        return spec;
    }
};

} // End namespace
#endif
