//---------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// L2F BD HAL integration
//---------------------------------------------------------------

#ifndef __PDS_MS_L2F_BD_HPP__
#define __PDS_MS_L2F_BD_HPP__

#include "nic/metaswitch/stubs/common/pds_ms_defs.hpp"
#include "nic/metaswitch/stubs/common/pds_ms_cookie.hpp"
#include "nic/metaswitch/stubs/common/pds_ms_state.hpp"
#include "nic/sdk/include/sdk/ip.hpp"
#include <nbase.h>
extern "C"
{
#include <bdpi.h>
}

namespace pds_ms {

using pds_ms::ms_ifindex_t;
using pds_ms::ms_bd_id_t;
using pds_ms::cookie_t;
using pds_ms::pds_batch_ctxt_guard_t;
using pds_ms::bd_obj_t;
using pds_ms::state_t;

class l2f_bd_t {
public:    
   void handle_add_upd_ips(ATG_BDPI_UPDATE_BD* bd_add_upd);
   void handle_delete(NBB_ULONG bd_id);
   void handle_add_if(NBB_ULONG bd_id, ms_ifindex_t ifindex);
   void handle_del_if(NBB_ULONG bd_id, ms_ifindex_t ifindex);
   // Synchronous HAL update completion
   sdk_ret_t update_pds_synch(state_t::context_t&& state_ctxt,
                              bd_obj_t* bd_obj);

private:
    struct ips_info_t {
        ms_bd_id_t    bd_id;
        pds_vnid_id_t vnid;
    };
    struct store_info_t {
        bd_obj_t*    bd_obj = nullptr;
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
    void parse_ips_info_(ATG_BDPI_UPDATE_BD* bd_add_upd);
    pds_subnet_spec_t make_pds_subnet_spec_(void);
    pds_subnet_key_t make_pds_subnet_key_(void);
    pds_batch_ctxt_guard_t prepare_pds(state_t::context_t& state_ctxt,
                                       bool async);
};

// API to update HAL directly bypassing Metaswitch
// Synchronous HAL update completion
sdk_ret_t l2f_bd_update_pds_synch(state_t::context_t&& state_ctxt,
                                  bd_obj_t* bd_obj);

} // End namespace
#endif
