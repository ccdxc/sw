//------------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//------------------------------------------------------------------------------
#ifndef __VRF_IPS_FEEDER_HPP__
#define __VRF_IPS_FEEDER_HPP__

#include "nic/metaswitch/stubs/test/hals/vrf_test_params.hpp"
#include "nic/metaswitch/stubs/common/pds_ms_util.hpp"
#include "nic/metaswitch/stubs/common/pds_ms_ifindex.hpp"
#include "nic/metaswitch/stubs/hals/pds_ms_li.hpp"

namespace pds_ms_test {

class vrf_ips_feeder_t final : public vrf_input_params_t {
public:
   void init() override {
       vrf_input_params_t::init();
    }

    ATG_LIPI_VRF_ADD_UPDATE generate_add_upd_ips(void) {
        ATG_LIPI_VRF_ADD_UPDATE add_upd {0};
      // generate_ips_header (add_upd); 
        auto vrf_str = std::to_string(vrf_id);
        memcpy(add_upd.vrf_name, vrf_str.c_str(), vrf_str.length());
        add_upd.vrf_name_len = vrf_str.length();
        return add_upd;
    }

    void trigger_create(void) override {
        pds_ms::vpc_create(&vpc_spec, 0);
    //    auto add_upd = generate_add_upd_ips();
    //    pds_ms::li_is()->vrf_add_update(&add_upd);
    }

    void trigger_delete(void) override {
        // TODO Fix - VPC delete is currently not calling MS HAL stub VRF delete
       vrf_input_params_t::trigger_delete();
       auto vrf_str = std::to_string(vrf_id);
       pds_ms::li_is()->vrf_delete((const NBB_BYTE*) vrf_str.c_str(), vrf_str.length());
        pds_ms::vpc_delete(vpc_spec.key, 0);
    }

    void trigger_update(void) override {
      //  auto add_upd = generate_add_upd_ips();
      //  pds_ms::li_is()->vrf_add_update(&add_upd);
        pds_ms::vpc_update(&vpc_spec, 0);
    }
    bool ips_mock() override {return true;}
};

} // End Namespace

#endif
