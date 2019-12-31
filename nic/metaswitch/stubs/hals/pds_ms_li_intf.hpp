//---------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// LI VXLAN Tunnel HAL integration
//---------------------------------------------------------------

#ifndef __PDS_MS_LI_INTF_HPP__
#define __PDS_MS_LI_INTF_HPP__

#include "nic/metaswitch/stubs/common/pds_ms_cookie.hpp"
#include "nic/apollo/api/include/pds_if.hpp"
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

class li_intf_t {
public:    
   void handle_add_upd_ips(ATG_LIPI_PORT_ADD_UPDATE* port_add_upd);
   void handle_delete(NBB_ULONG phy_port_ifindex);

private:
    struct ips_info_t {
        ms_ifindex_t ifindex;
        bool         admin_state;
        bool         admin_state_updated;
        bool         switchport;
        bool         switchport_updated;
        char*        if_name = nullptr;
    };
    struct store_info_t {
        if_obj_t*    phy_port_if_obj;
    };

private:
    std::unique_ptr<cookie_t> cookie_uptr_;
    pds_batch_ctxt_guard_t bctxt_guard_;
    ips_info_t  ips_info_;
    store_info_t  store_info_;
    bool op_create_ = false;
    bool op_delete_ = false;

private:
    bool cache_new_obj_in_cookie_(void);
    pds_batch_ctxt_guard_t make_batch_pds_spec_(void);
    void fetch_store_info_(pds_ms::state_t* state);
    void parse_ips_info_(ATG_LIPI_PORT_ADD_UPDATE* port_add_upd);
    pds_if_spec_t make_pds_if_spec_(void);
    pds_if_key_t make_pds_if_key_(void);
};

} // End namespace
#endif
