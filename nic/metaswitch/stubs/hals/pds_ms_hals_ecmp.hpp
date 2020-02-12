//---------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// NHPI ECMP HAL integration
//---------------------------------------------------------------

#ifndef __PDS_MS_HALS_ECMP_HPP__
#define __PDS_MS_HALS_ECMP_HPP__

#include "nic/metaswitch/stubs/common/pds_ms_cookie.hpp"
#include "nic/metaswitch/stubs/common/pds_ms_util.hpp"
#include "nic/metaswitch/stubs/common/pds_ms_defs.hpp"
#include "nic/metaswitch/stubs/common/pds_ms_state.hpp"
#include "nic/apollo/api/include/pds_nexthop.hpp"
#include "nic/sdk/include/sdk/eth.hpp"
#include <nbase.h>
extern "C"
{
#include <a0build.h>
#include <a0glob.h>
#include <a0spec.h>
#include <o0mac.h>
#include <nhpi.h>
}
namespace pds_ms {

using pds_ms::ms_ifindex_t;
using pds_ms::mac_addr_wr_t;
using pds_ms::cookie_t;
using pds_ms::pds_batch_ctxt_guard_t;

class hals_ecmp_t {
public:    
    void handle_add_upd_ips(ATG_NHPI_ADD_UPDATE_ECMP* add_upd_ecmp_ips);
    void handle_delete(NBB_CORRELATOR pathset_id);

private:
    struct nh_info_t {
        ms_ifindex_t   ms_ifindex;
        mac_addr_wr_t  mac_addr;
        nh_info_t(ms_ifindex_t ifi, const mac_addr_t& mac) 
            : ms_ifindex(ifi), mac_addr(mac) {};
    };
    struct ips_info_t {
        uint32_t        pathset_id;
        std::vector<nh_info_t> nexthops;
        uint32_t       num_added_nh = 0; 
        uint32_t       num_deleted_nh = 0; 
        pds_nexthop_group_type_t  pds_nhgroup_type = PDS_NHGROUP_TYPE_NONE;
    };
    struct store_info_t {
        pathset_obj_t*   pathset_obj = nullptr;
        pds_batch_ctxt_t  bctxt = 0;
    };

private:
    std::unique_ptr<cookie_t> cookie_uptr_;
    ips_info_t  ips_info_;
    store_info_t  store_info_;
    bool op_create_ = false;
    bool op_delete_ = false;

private:
    pds_batch_ctxt_guard_t make_batch_pds_spec_(state_t::context_t& state_ctxt);
    bool parse_ips_info_(ATG_NHPI_ADD_UPDATE_ECMP* nh_add_upd);
    void fetch_store_info_(state_t* state);
    pds_nexthop_group_spec_t make_pds_nhgroup_spec_(state_t::context_t& state_ctxt);
    void make_pds_underlay_nhgroup_spec_ (pds_nexthop_group_spec_t&,
                                          state_t::context_t& state_ctxt);
    void make_pds_overlay_nhgroup_spec_ (pds_nexthop_group_spec_t&,
                                         state_t::context_t& state_ctxt);
    pds_obj_key_t make_pds_nhgroup_key_(void);
};

} // End namespace
#endif
