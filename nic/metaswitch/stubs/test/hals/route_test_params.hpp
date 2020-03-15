//------------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//------------------------------------------------------------------------------
#ifndef __PDS_MS_TEST_ROUTE_TEST_PARAMS_HPP__
#define __PDS_MS_TEST_ROUTE_TEST_PARAMS_HPP__

#include "nic/metaswitch/stubs/test/hals/test_params.hpp"
#include "nic/metaswitch/stubs/mgmt/pds_ms_vpc.hpp"
#include "nic/metaswitch/stubs/common/pds_ms_state.hpp"
#include "nic/apollo/api/include/pds_vpc.hpp"
#include "nic/sdk/include/sdk/if.hpp"

namespace pds_ms_test{

class route_input_params_t : public test_input_base_t {
public:
    ip_addr_t       route_ip;
    uint32_t        pfxlen;
    char            vrf_name_route[5] = "1";

   // These inputs are used to generate feeder inputs 
   // as well as output verifications 
   virtual void init(const char *rip, uint32_t len) {
        test::extract_ip_addr (rip, &route_ip);
        pfxlen = len;
        pds_vpc_spec_t  vpc_spec = {0};

       // Create VPC store entry
        int vrf_id = 1;
        pds_obj_key_t route_table_key = pds_ms::msidx2pdsobjkey(vrf_id);
        vpc_spec.key = pds_ms::msidx2pdsobjkey(vrf_id);
        vpc_spec.type = PDS_VPC_TYPE_TENANT;
        vpc_spec.v4_prefix.len = 24; 
        str2ipv4addr("23.1.10.1", &vpc_spec.v4_prefix.v4_addr);
        mac_str_to_addr((char*) "04:06:03:09:00:03", vpc_spec.vr_mac);
        vpc_spec.v4_route_table = route_table_key;
        vpc_spec.fabric_encap.type = PDS_ENCAP_TYPE_VXLAN;
        vpc_spec.fabric_encap.val.vnid  = 100;
        vpc_spec.tos = 5;
        auto state_ctxt = pds_ms::state_t::thread_context(); 
        state_ctxt.state()->vpc_store().add_upd (vrf_id,
                                                 new pds_ms::vpc_obj_t(vrf_id, vpc_spec));
        state_ctxt.state()->route_table_store().add_upd(route_table_key,
                            new pds_ms::route_table_obj_t(route_table_key, IP_AF_IPV4));
   }
   virtual ~route_input_params_t(void) {};
};

void load_route_test_input(void);
void load_route_test_output(void);

static inline void
load_route_test (void) 
{
    load_route_test_input();
    load_route_test_output();
}

} // End namespace

#endif

