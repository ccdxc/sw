//------------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//------------------------------------------------------------------------------
#ifndef __BD_IPS_FEEDER_HPP__
#define __BD_IPS_FEEDER_HPP__

#include "nic/metaswitch/stubs/test/hals/bd_test_params.hpp"
#include "nic/metaswitch/stubs/common/pds_ms_util.hpp"
#include "nic/metaswitch/stubs/common/pds_ms_ifindex.hpp"
#include "nic/metaswitch/stubs/common/pds_ms_defs.hpp"
#include <l2f_c_includes.hpp>
#include "nic/metaswitch/stubs/hals/pds_ms_l2f.hpp"
#include "nic/metaswitch/stubs/hals/pds_ms_l2f_bd.hpp"
#include "nic/apollo/api/utils.hpp"
#include <unordered_set>

namespace pds_ms_test {
using pds_ms::ms_ifindex_t;

class bd_ips_feeder_t final : public bd_input_params_t {
public:
    pds_ms::l2f_integ_subcomp_t  l2f_is; 
    ms_ifindex_t   prev_if_bind = 0;
   void init() override {
       bd_input_params_t::init();
    }
   void trigger_init() override {
        pds_ms::vpc_create(&vpc_spec, 0);
   }

    ATG_BDPI_UPDATE_BD generate_add_upd_ips(void) {
        ATG_BDPI_UPDATE_BD add_upd {0};
      // generate_ips_header (add_upd); 
        add_upd.bd_id.bd_id = bd_id;
        add_upd.bd_properties.vni = subnet_spec.fabric_encap.val.vnid;
        return add_upd;
    }

    void trigger_create(void) override {
        // TODO: Fix Subnet-BD Gtest
        // For now generate both proto create and IPS create
        // since BD oper status since MS is not sending create to L2F
        pds_ms::subnet_create(&subnet_spec, 0);
        auto add_upd = generate_add_upd_ips();
        l2f_is.add_upd_bd(&add_upd);
        subnet_uuids.insert(subnet_spec.key);
    }

    void trigger_delete(void) override {
        bd_input_params_t::trigger_delete();
        pds_ms::subnet_delete(subnet_spec.key, 0);
        ATG_L2_BD_ID ms_bd_id = {ATG_L2_BRIDGE_DOMAIN_EVPN, bd_id, 0};
        l2f_is.delete_bd(&ms_bd_id, NBB_CORRELATOR{0});
        subnet_uuids.erase(subnet_spec.key);
    }

    void trigger_update(void) override {
        if (test_if_bind) {
            pds_ms::l2f_bd_t  bd;
            auto lifindex =  api::objid_from_uuid(subnet_spec.host_if);
            auto ms_ifindex =
                pds_ms::pds_to_ms_ifindex(lifindex, IF_TYPE_LIF);
            bd.handle_add_if(bd_id, ms_ifindex);
            prev_if_bind = ms_ifindex;
            test_if_bind = false;
            return;
        }
        if (test_if_unbind) {
            ATG_BDPI_INTERFACE_BIND if_bind;
            ATG_L2_BD_ID ms_bd_id; ms_bd_id.bd_id = bd_id;
            if_bind.if_index = prev_if_bind;
            l2f_is.delete_bd_if(&ms_bd_id, NBB_CORRELATOR(), &if_bind);
            test_if_unbind = false;
            return;
        }
        auto add_upd = generate_add_upd_ips();
        l2f_is.add_upd_bd(&add_upd);
    }

    void trigger_if_bind(void) {
    }
    void trigger_if_unbind(void) {
    }
    bool ips_mock() override {return true;}

    void cleanup() override {
        // Delete the VPC created as a pre-req
        std::cout << " ====== Cleanup ========" << std::endl;
        for (auto& subnet_uuid: subnet_uuids) {
            pds_subnet_spec_t subnet_spec = {0};
            subnet_spec.key = subnet_uuid;
            pds_ms::subnet_delete(subnet_spec.key, 0);
        }
        pds_ms::vpc_delete(vpc_spec.key, 0);
        // TODO Fix - currently VPC delete is not calling HAL stub VRF delete
       auto state_ctxt = pds_ms::state_t::thread_context(); 
       auto vpc_obj = state_ctxt.state()->vpc_store().get(vrf_id);
       if (vpc_obj->properties().spec_invalid) {
           std::cout << "Erasing VPC from store" << std::endl;
           state_ctxt.state()->vpc_store().erase(vrf_id);
           state_ctxt.state()->route_table_store()
               .erase(pds_ms::msidx2pdsobjkey(vrf_id));
       }
    }
private:
    std::unordered_set<pds_obj_key_t, pds_obj_key_hash> subnet_uuids;
};

} // End Namespace

#endif
