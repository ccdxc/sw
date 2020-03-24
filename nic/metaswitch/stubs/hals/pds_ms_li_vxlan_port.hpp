//---------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// LI VXLAN Port HAL integration
//---------------------------------------------------------------

#ifndef __PDS_MS_LI_VXLAN_PORT_HPP__
#define __PDS_MS_LI_VXLAN_PORT_HPP__

#include "nic/metaswitch/stubs/common/pds_ms_cookie.hpp"
#include "nic/metaswitch/stubs/common/pds_ms_state.hpp"
#include "nic/apollo/api/include/pds_tep.hpp"
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

#define PDS_MS_UECMP_INVALID_INDEX 0

class li_vxlan_port {
public:    
   NBB_BYTE handle_add_upd_ips(ATG_LIPI_VXLAN_PORT_ADD_UPD* vxlan_tnl_add_upd);
   void handle_delete(ms_ifindex_t vxlan_port_ifindex);
   void add_pds_tep_spec(pds_batch_ctxt_t bctxt, if_obj_t* vxp_if_obj,
                         tep_obj_t* tep_obj, bool op_create);

private:
    struct ips_info_t {
        ms_ifindex_t   if_index;
        ip_addr_t      tep_ip;
        ip_addr_t      src_ip;
        pds_vnid_id_t  vni;
    };

    struct store_info_t {
        tep_obj_t*   tep_obj;
        if_obj_t*    vxp_if_obj;
    };

private:
    std::unique_ptr<cookie_t> cookie_uptr_;
    pds_batch_ctxt_guard_t bctxt_guard_;
    store_info_t  store_info_;
    ips_info_t  ips_info_;
    bool op_create_ = false;
    bool op_delete_ = false;

private:
    void cache_obj_in_cookie_(void); 
    pds_batch_ctxt_guard_t make_batch_pds_spec_ (void);
    void fetch_store_info_(pds_ms::state_t* state);

    void parse_ips_info_(ATG_LIPI_VXLAN_PORT_ADD_UPD* vxlan_port_add_upd);
    pds_tep_spec_t make_pds_tep_spec_(void);
    pds_nexthop_group_spec_t make_pds_nhgroup_spec_(void);

    pds_obj_key_t make_pds_tep_key_
        (const if_obj_t::vxlan_port_properties_t& vxp_prop) {
        return pds_obj_key_t(vxp_prop.hal_tep_idx);
    }
};

} // End namespace
#endif

