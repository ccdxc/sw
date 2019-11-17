//------------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//------------------------------------------------------------------------------
#ifndef __VXLAN_IPS_FEEDER_HPP__
#define __VXLAN_IPS_FEEDER_HPP__

#include "nic/metaswitch/stubs/test/hals/vxlan_test_params.hpp"
#include "nic/metaswitch/stubs/common/pdsa_util.hpp"
#include "nic/metaswitch/stubs/hals/pdsa_li.hpp"
#include "nic/apollo/test/utils/utils.hpp"

namespace pdsa_test {

class vxlan_ips_feeder_t final : public vxlan_input_params_t {
public:
   void init(const char *sip, const char* dip) override {
       vxlan_input_params_t::init(sip,dip);
       // Mock TunnelIfIndex and Underlay Nexthop 
       // for the IPS feeder case
       tnl_ifindex = 1000;
       unh_dp_idx = 100;
    }

    ATG_LIPI_VXLAN_ADD_UPDATE generate_add_upd_ips(void) {
        ATG_LIPI_VXLAN_ADD_UPDATE add_upd;
      // generate_ips_header (add_upd); 
        add_upd.id.if_index = tnl_ifindex;
        pdsa_stub::convert_ipaddr_pdsa_to_ms (source_ip, 
                                              &(add_upd.vxlan_settings.source_ip));
        pdsa_stub::convert_ipaddr_pdsa_to_ms (dest_ip, 
                                              &(add_upd.vxlan_settings.dest_ip));
        NBB_CORR_PUT_VALUE(add_upd.id.hw_correlator, unh_dp_idx);
        return add_upd;
    }

    void trigger_create(void) override {
        auto add_upd = generate_add_upd_ips();
        pdsa_stub::li_is()->vxlan_add_update(&add_upd);
    }

    void trigger_delete(void) override {
        pdsa_stub::li_is()->vxlan_delete(tnl_ifindex);
    }

    void trigger_update(void) override {
        auto add_upd = generate_add_upd_ips();
        pdsa_stub::li_is()->vxlan_add_update(&add_upd);
    }
    void modify(void) override {
        unh_dp_idx += 5;
    }
    void next(void) override {
        tnl_ifindex += 1;
        api_test::increment_ip_addr (&dest_ip);
    }

};

} // End Namespace

#endif
