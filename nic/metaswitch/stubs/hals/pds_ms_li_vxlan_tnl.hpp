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

class li_vxlan_tnl {
public:
   NBB_BYTE handle_add_upd_ips(ATG_LIPI_VXLAN_ADD_UPDATE* vxlan_tnl_add_upd);
   void handle_delete(NBB_ULONG vxlan_tnl_ifindex);
   NBB_BYTE handle_upathset_update(state_t::context_t&& state_ctxt,
                                   tep_obj_t* tep_obj,
                                   ms_hw_tbl_id_t ms_upathset_dpcorr,
                                   cookie_uptr_t&& cookie_uptr);

private:
    struct ips_info_t {
        ms_ifindex_t if_index;
        ip_addr_t    tep_ip;
        ip_addr_t    src_ip;
        std::string  tep_ip_str;
        ms_ps_id_t   ms_upathset; // Underlay Pathset
        ms_hw_tbl_id_t ms_upathset_dpcorr = 0;
        ATG_LIPI_VXLAN_ADD_UPDATE* vxlan_tnl_add_upd_ips = nullptr;
    };

    struct store_info_t {
        tep_obj_t*   tep_obj = nullptr;
        if_obj_t*    tun_if_obj = nullptr;
        ms_hw_tbl_id_t  ms_upathset_dpcorr = 0;
    };

private:
    std::unique_ptr<cookie_t> cookie_uptr_;
    store_info_t  store_info_;
    ips_info_t  ips_info_;
    bool op_create_ = false;
    bool op_delete_ = false;

private:
    void create_obj_(void);
    bool update_obj_(void);
    pds_batch_ctxt_guard_t make_batch_pds_spec_(state_t::context_t& state_ctxt);
    void fetch_store_info_(pds_ms::state_t* state);
    void populate_cookie_ips_response_(void);
    NBB_BYTE handle_add_upd_(state_t::context_t&& state_ctxt,
                             bool ext_trigger);

    void parse_ips_info_(ATG_LIPI_VXLAN_ADD_UPDATE* vxlan_tnl_add_upd);
    pds_tep_spec_t make_pds_tep_spec_(void);
    pds_nexthop_group_spec_t make_pds_onhgroup_spec_(void);

    pds_obj_key_t make_pds_tep_key_(void) {
        auto& tep_prop = store_info_.tep_obj->properties();
        return msidx2pdsobjkey(tep_prop.hal_tep_idx);
    }
    pds_obj_key_t make_pds_onhgroup_key_(void) {
        return msidx2pdsobjkey(store_info_.tep_obj->hal_oecmp_idx_guard->idx());
    }
};

} // End namespace
#endif
