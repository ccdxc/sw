//---------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// LI VXLAN Tunnel HAL integration
//---------------------------------------------------------------

#ifndef __PDS_MS_LI_VXLAN_TNL_HPP__
#define __PDS_MS_LI_VXLAN_TNL_HPP__

#include "nic/metaswitch/stubs/common/pds_ms_cookie.hpp"
#include "nic/metaswitch/stubs/common/pds_ms_state.hpp"
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
#include <lipi.h>
}

namespace pds_ms {

using pds_ms::ms_ifindex_t;
using pds_ms::mac_addr_wr_t;
using pds_ms::cookie_t;
using pds_ms::pds_batch_ctxt_guard_t;
using pds_ms::tep_obj_t;
using pds_ms::if_obj_t;

class li_vxlan_tnl {
public:    
   void handle_add_upd_ips(ATG_LIPI_VXLAN_ADD_UPDATE* vxlan_tnl_add_upd);
   void handle_delete(NBB_ULONG vxlan_tnl_ifindex);

private:
    struct ips_info_t {
        ms_ifindex_t if_index;
        ip_addr_t    tep_ip;
        ip_addr_t    src_ip;
        pds_nexthop_group_id_t hal_uecmp_idx; 
        std::string  tep_ip_str;
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
    void fetch_store_info_(pds_ms::state_t* state);

    void parse_ips_info_(ATG_LIPI_VXLAN_ADD_UPDATE* vxlan_tnl_add_upd);
    pds_tep_spec_t make_pds_tep_spec_(void);
    pds_nexthop_group_spec_t make_pds_nhgroup_spec_(void);

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
};

} // End namespace
#endif
