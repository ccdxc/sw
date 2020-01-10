//------------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//------------------------------------------------------------------------------
#ifndef __PDS_MS_TEST_BD_PARAMS_HPP__
#define __PDS_MS_TEST_BD_PARAMS_HPP__

#include "nic/metaswitch/stubs/test/hals/test_params.hpp"
#include "nic/metaswitch/stubs/mgmt/pds_ms_subnet.hpp"
#include "nic/metaswitch/stubs/common/pds_ms_state.hpp"
#include "nic/apollo/api/include/pds_subnet.hpp"
#include "nic/sdk/include/sdk/if.hpp"

namespace pds_ms_test{

class bd_input_params_t : public test_input_base_t {
public:
    uint32_t           bd_id;
    uint32_t           vrf_id;
    pds_subnet_spec_t  subnet_spec = {0};
    bool test_if_bind = false;
    bool test_if_unbind = false;

   // These inputs are used to generate feeder inputs 
   // as well as output verifications 
   virtual void init() {
    bd_id = 1; vrf_id = 1;
    //subnet_spec.key = pds_ms::msidx2pdsobjkey(bd_id);
    subnet_spec.key.id = bd_id;
    subnet_spec.vpc = pds_ms::msidx2pdsobjkey(vrf_id);
    subnet_spec.v4_prefix.len = 24; 
    str2ipv4addr("23.3.10.1", &subnet_spec.v4_prefix.v4_addr);
    subnet_spec.v4_vr_ip = subnet_spec.v4_prefix.v4_addr;
    mac_str_to_addr((char*) "04:06:03:09:00:03", subnet_spec.vr_mac);
    subnet_spec.num_ing_v4_policy = 2;
    subnet_spec.ing_v4_policy[0].id = 1;
    subnet_spec.ing_v4_policy[1].id = 2;
    subnet_spec.num_egr_v4_policy = 3;
    subnet_spec.egr_v4_policy[0].id = 5;
    subnet_spec.egr_v4_policy[1].id = 6;
    subnet_spec.egr_v4_policy[2].id = 7;
    subnet_spec.fabric_encap.type = PDS_ENCAP_TYPE_VXLAN;
    subnet_spec.fabric_encap.val.vnid  = 100;
    subnet_spec.host_ifindex = 0;
    subnet_spec.dhcp_policy.id = 10;
    subnet_spec.tos = 5;

    auto state_ctxt = pds_ms::state_t::thread_context(); 
    state_ctxt.state()->subnet_store().add_upd (bd_id,
                                            new pds_ms::subnet_obj_t(subnet_spec));
   }

   void modify(void) override {
   }

   void modify_fast_fields(void) {
       str2ipv4addr("23.4.10.1", &subnet_spec.v4_prefix.v4_addr);
   }
   void modify_slow_fields(void) {
       subnet_spec.fabric_encap.val.vnid += 100;
   }
   void add_if_bind(void) {
       subnet_spec.host_ifindex = LIF_IFINDEX(1);
       test_if_bind = true;
   }
   void del_if_bind(void) {
       subnet_spec.host_ifindex = 0;
       test_if_unbind = true;
   }
   void next(void) override { 
       //subnet_spec.key = pds_ms::msidx2pdsobjkey(++bd_id);
       subnet_spec.key.id = ++bd_id;
       str2ipv4addr("24.4.10.1", &subnet_spec.v4_prefix.v4_addr);
       subnet_spec.fabric_encap.val.vnid  += 100;
       auto state_ctxt = pds_ms::state_t::thread_context(); 
       state_ctxt.state()->subnet_store().add_upd (bd_id,
                                               new pds_ms::subnet_obj_t(subnet_spec));
   }
   void trigger_delete(void) override { 
       auto state_ctxt = pds_ms::state_t::thread_context(); 
       state_ctxt.state()->subnet_store().erase (vrf_id);
   }
   virtual ~bd_input_params_t(void) {};
   virtual void init_direct_update() {
       // Set an initial subnet spec in the BD store 
       //subnet_spec.key.id = pds_ms::msidx2pdsobjkey(++bd_id);
       subnet_spec.key.id = ++bd_id;
       subnet_spec.fabric_encap.val.vnid  += 100;
       str2ipv4addr("33.3.10.1", &subnet_spec.v4_prefix.v4_addr);
       auto state_ctxt = pds_ms::state_t::thread_context(); 
       state_ctxt.state()->subnet_store().add_upd (bd_id,
                                               new pds_ms::subnet_obj_t(subnet_spec));

       // And then change it to simulate Direct Update
       subnet_spec.num_ing_v4_policy = 3;
       subnet_spec.ing_v4_policy[0].id = 4;
       subnet_spec.ing_v4_policy[1].id = 2;
       subnet_spec.ing_v4_policy[2].id = 3;
       subnet_spec.num_egr_v4_policy = 1;
       subnet_spec.egr_v4_policy[0].id = 9;
   }
   virtual void send_direct_update() {
       pds_ms::subnet_update(&subnet_spec, 0);
   }
   void modify_direct_update(void) {
       subnet_spec.num_ing_v4_policy = 1;
       subnet_spec.ing_v4_policy[0].id = 10;
       subnet_spec.num_egr_v4_policy = 2;
       subnet_spec.egr_v4_policy[0].id = 11;
       subnet_spec.egr_v4_policy[1].id = 13;
   }
};

void load_bd_test_input(void);
void load_bd_test_output(void);

static inline void
load_bd_test (void) 
{
    load_bd_test_input();
    load_bd_test_output();
}

} // End namespace

#endif
