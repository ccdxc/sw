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

enum pds_ms_nh_type_t {
    PDS_MS_NH_DEST_PORT = 0,
    PDS_MS_NH_DEST_NH,
    PDS_MS_NH_DEST_BH,
    PDS_MS_NH_DEST_MAX
};

class hals_ecmp_t {
public:
    NBB_BYTE handle_add_upd_ips(ATG_NHPI_ADD_UPDATE_ECMP* add_upd_ecmp_ips);
    void handle_delete(NBB_CORRELATOR pathset_id);

    enum class op_type_e {
        IGNORE,
        CREATE,
        BH_TO_VALID,
        UPDATE_ADD,
        UPDATE_DEL,
        UPDATE_REPLACE,
        DELETE,
        VALID_TO_BH
    };
    enum class ps_type_e {
        UNKNOWN,
        DIRECT,
        INDIRECT,
        OVERLAY
    };
    struct nh_info_t {
        ms_ifindex_t   ms_ifindex;
        mac_addr_wr_t  mac_addr;
        nh_info_t(ms_ifindex_t ifi, const mac_addr_t& mac)
            : ms_ifindex(ifi), mac_addr(mac) {};
    };
    struct ips_info_t {
        uint32_t        pathset_id;
        std::vector<nh_info_t> nexthops;
        pds_nexthop_group_type_t  pds_nhgroup_type = PDS_NHGROUP_TYPE_NONE;
        uint32_t       direct_ps_dpcorr = 0;
        uint32_t       nh_count[PDS_MS_NH_DEST_MAX] = {0};
        uint32_t       nh_add_count[PDS_MS_NH_DEST_MAX] = {0};
        uint32_t       nh_del_count[PDS_MS_NH_DEST_MAX] = {0};
        ps_type_e      ps_type;
    };
    struct store_info_t {
        pathset_obj_t*   pathset_obj = nullptr;
        pds_batch_ctxt_t  bctxt = 0;
    };

private:
    std::unique_ptr<cookie_t> cookie_uptr_;
    ips_info_t  ips_info_;
    store_info_t  store_info_;
    op_type_e op_;

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
    NBB_BYTE handle_indirect_ps_update_(ATG_NHPI_ADD_UPDATE_ECMP* add_upd_ecmp_ips);
    void calculate_op_(void);
};

} // End namespace
#endif
