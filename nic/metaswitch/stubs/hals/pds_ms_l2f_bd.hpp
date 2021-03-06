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

struct subnet_batches_t {
    pds_batch_ctxt_guard_t remote_mac_batch;
    pds_batch_ctxt_guard_t subnet_batch;
};

class l2f_bd_t {
public:    
   NBB_BYTE handle_add_upd_ips(ATG_BDPI_UPDATE_BD* bd_add_upd);
   void handle_delete(NBB_ULONG bd_id);
   void handle_add_if(NBB_ULONG bd_id, ms_ifindex_t ifindex);
   void handle_del_if(NBB_ULONG bd_id, ms_ifindex_t ifindex);
   // Synchronous HAL update completion
   sdk_ret_t update_pds_synch(state_t::context_t&& state_ctxt,
                              uint32_t bd_id,
                              subnet_obj_t* subnet_obj);

private:
    struct ips_info_t {
        ms_bd_id_t    bd_id;
        pds_vnid_id_t vnid;
    };
    struct store_info_t {
        subnet_obj_t*    subnet_obj = nullptr;
        bd_obj_t*        bd_obj = nullptr;
    };

private:
    std::unique_ptr<cookie_t> cookie_uptr_;
    ips_info_t  ips_info_;
    store_info_t  store_info_;
    bool op_create_ = false;
    bool op_delete_ = false;

private:
    subnet_batches_t make_batch_pds_spec_(state_t::context_t& state_ctxt,
                                                bool async);
    void fetch_store_info_(state_t* state);
    void parse_ips_info_(ATG_BDPI_UPDATE_BD* bd_add_upd);
    pds_subnet_spec_t make_pds_subnet_spec_(void);
    pds_obj_key_t  make_pds_subnet_key_(void);
    subnet_batches_t prepare_pds(state_t::context_t& state_ctxt,
                                       bool async);
};

// API to update HAL directly bypassing Metaswitch
// Synchronous HAL update completion
sdk_ret_t l2f_bd_update_pds_synch(state_t::context_t&& state_ctxt,
                                  uint32_t bd_id,      
                                  subnet_obj_t* subnet_obj);

} // End namespace
#endif
