//---------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// L2F MAI HAL integration
//---------------------------------------------------------------

#ifndef __PDS_MS_L2F_MAI_HPP__
#define __PDS_MS_L2F_MAI_HPP__

#include <l2f_c_includes.hpp>
#include "nic/metaswitch/stubs/common/pds_ms_defs.hpp"
#include "nic/metaswitch/stubs/common/pds_ms_state.hpp"
#include "nic/metaswitch/stubs/common/pds_ms_cookie.hpp"
#include "nic/apollo/api/include/pds_mapping.hpp"
#include "nic/apollo/api/include/pds.hpp"
#include "nic/sdk/include/sdk/eth.hpp"
#include "nic/sdk/include/sdk/ip.hpp"
#include <memory>

namespace pds_ms {

void l2f_local_mac_ip_add(const pds_obj_key_t& subnet_key, const ip_addr_t& ip,
                      mac_addr_t mac, pds_ifindex_t lif_ifindex);
void l2f_local_mac_ip_del(const pds_obj_key_t& subnet_key, const ip_addr_t& ip,
                          mac_addr_t mac);
void l2f_del_remote_macs_for_bd(state_t::context_t& state_ctxt, ms_bd_id_t bd_id,
                                pds_batch_ctxt_t bctxt);

class l2f_mai_t {
public:    
   NBB_BYTE handle_add_upd_mac(ATG_BDPI_UPDATE_FDB_MAC* fdb_add_upd);
   void handle_delete_mac(l2f::FdbMacKey *key);
   void handle_add_upd_ip(const ATG_MAI_MAC_IP_ID* ip_add_upd);
   void handle_delete_ip(const ATG_MAI_MAC_IP_ID* ip_add_upd);
   void batch_pds_mapping_del_spec(bd_obj_t* bd_obj, const mac_addr_t& mac,
                                   pds_batch_ctxt_t bctxt);

private:
    struct ips_info_t {
        ms_bd_id_t    bd_id;
        mac_addr_t    mac_address;
        ip_addr_t     ip_address;
        std::set<ip_addr_t> tep_ip_list;
        bool          has_ip = false; // is this a MAI IP 
                                      // update IPS
    };
    struct store_info_t {
        bd_obj_t*       bd_obj = nullptr;
        mac_obj_t*      mac_obj = nullptr;
        subnet_obj_t*   subnet_obj = nullptr;
        std::vector<tep_obj_t*>  tep_obj_list;
        ms_hw_tbl_id_t  hal_oecmp_idx; 
    };

private:
    std::unique_ptr<cookie_t> cookie_uptr_;
    pds_batch_ctxt_guard_t bctxt_guard_;
    ips_info_t  ips_info_;
    store_info_t  store_info_;
    bool op_create_ = false;
    bool op_delete_ = false;

private:
    pds_batch_ctxt_guard_t make_batch_pds_spec_();
    void fetch_store_info_(state_t* state);
    void resolve_teps_(state_t* state);
    void parse_ips_info_(ATG_BDPI_UPDATE_FDB_MAC* bd_add_upd);
    void parse_ips_info_(const ATG_MAI_MAC_IP_ID* mai_ip_id);
    void add_pds_mapping_spec_(pds_batch_ctxt_t bctxt);
    pds_remote_mapping_spec_t make_pds_mapping_spec_(void);
    pds_mapping_key_t make_pds_mapping_key_(void);
};


} // End namespace
#endif
