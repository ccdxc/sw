//------------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//------------------------------------------------------------------------------
#ifndef __ROUTE_IPS_FEEDER_HPP__
#define __ROUTE_IPS_FEEDER_HPP__

#include "nic/metaswitch/stubs/test/hals/route_test_params.hpp"
#include "nic/metaswitch/stubs/common/pds_ms_util.hpp"
#include "nic/metaswitch/stubs/common/pds_ms_ifindex.hpp"
#include "nic/metaswitch/stubs/common/pds_ms_defs.hpp"
#include <hals_c_includes.hpp>
#include "nic/metaswitch/stubs/hals/pds_ms_hals_l3.hpp"
#include "nic/metaswitch/stubs/hals/pds_ms_hals_route.hpp"

namespace pds_ms_test {
using pds_ms::ms_ifindex_t;

class route_ips_feeder_t final : public route_input_params_t {
public:
    pds_ms::hals_l3_integ_subcomp_t hals_is; 
    ms_ifindex_t   prev_if_bind = 0;

    ATG_ROPI_UPDATE_ROUTE generate_add_upd_ips(void) {
        ATG_ROPI_UPDATE_ROUTE add_upd = {0};
        int len = strlen(vrf_name_route);
        strncpy((char *)add_upd.route_id.vrf_name, vrf_name_route, len);
        add_upd.route_id.vrf_name_len = len;
        if (op_update_) {
            // Update the dp correlator
            add_upd.route_properties.dp_pathset_correlator.correlator1 = 200;
        } else {
            add_upd.route_properties.dp_pathset_correlator.correlator1 = 100;
        }
        pds_ms::pds_to_ms_ipaddr(route_ip, 
                                    &(add_upd.route_id.destination_address));
        add_upd.route_id.prefix_length = pfxlen;
        return add_upd;
    }

    ATG_ROPI_ROUTE_ID generate_del_ips(void) {
        auto add_upd = generate_add_upd_ips();
        return (add_upd.route_id);
    }

    void trigger_create(void) override {
        auto add_upd = generate_add_upd_ips();
        hals_is.ropi_update_route(&add_upd);
    }

    void trigger_update(void) override {
        op_update_ = true;
        auto add_upd = generate_add_upd_ips();
        hals_is.ropi_update_route(&add_upd);
        op_update_ = false;
    }

    void trigger_delete(void) override {
        auto del = generate_del_ips();
        hals_is.ropi_delete_route(del);
    }

    void modify(void) override { }
    void next(void) override { }
    bool ips_mock() override {return true;}

private:
    bool op_update_ = false;
};

} // End Namespace

#endif
