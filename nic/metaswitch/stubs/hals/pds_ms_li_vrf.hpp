//---------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// LI VRF HAL integration
//---------------------------------------------------------------

#ifndef __PDS_MS_LI_VRF_HPP__
#define __PDS_MS_LI_VRF_HPP__

#include "nic/metaswitch/stubs/common/pds_ms_defs.hpp"
#include "nic/metaswitch/stubs/common/pds_ms_cookie.hpp"
#include "nic/metaswitch/stubs/common/pds_ms_state.hpp"
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

class li_vrf_t {
public:    
   void handle_add_upd_ips(ATG_LIPI_VRF_ADD_UPDATE* vrf_add_upd);
   void handle_delete(const NBB_BYTE* vrf_name, NBB_ULONG vrf_name_len);
   // Synchronous HAL update completion
   sdk_ret_t update_pds_synch(state_t::context_t&& state_ctxt,
                              vpc_obj_t* bd_obj);

private:
    struct ips_info_t {
        uint32_t   vrf_id;
    };
    struct store_info_t {
        vpc_obj_t*    vpc_obj = nullptr;
    };

private:
    std::unique_ptr<cookie_t> cookie_uptr_;
    pds_batch_ctxt_guard_t bctxt_guard_;
    ips_info_t  ips_info_;
    store_info_t  store_info_;
    bool op_create_ = false;
    bool op_delete_ = false;

private:
    pds_batch_ctxt_guard_t make_batch_pds_spec_(bool async);
    void fetch_store_info_(pds_ms::state_t* state);
    void parse_ips_info_(ATG_LIPI_VRF_ADD_UPDATE* vrf_add_upd);
    pds_vpc_spec_t make_pds_vpc_spec_(void);
    pds_vpc_key_t make_pds_vpc_key_(void);
    pds_batch_ctxt_guard_t prepare_pds(state_t::context_t& state_ctxt,
                                       bool async);
};

// API to update HAL directly bypassing Metaswitch
// Synchronous HAL update completion
sdk_ret_t li_vrf_update_pds_synch(state_t::context_t&& state_ctxt,
                                  vpc_obj_t* vpc_obj);

} // End namespace
#endif
