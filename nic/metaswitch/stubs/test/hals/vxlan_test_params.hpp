//------------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//------------------------------------------------------------------------------
#ifndef __PDS_MS_TEST_VXLAN_TEST_PARAMS_HPP__
#define __PDS_MS_TEST_VXLAN_TEST_PARAMS_HPP__

#include "nic/metaswitch/stubs/test/hals/test_params.hpp"

namespace pds_ms_test{

class vxlan_input_params_t : public test_input_base_t {
public:
    uint32_t      tnl_ifindex;
    ip_addr_t     source_ip;
    ip_addr_t     dest_ip;
    uint32_t      unh_dp_idx; // Underlay NH index
    uint32_t      indirect_pathset;

   // These inputs are used to generate feeder inputs 
   // as well as output verifications 
   virtual void init(const char *sip, const char* dip) {
       test::extract_ip_addr (sip, &source_ip);
       test::extract_ip_addr (dip, &dest_ip);
   }
   void next(void) override {}; 
   virtual void trigger_create(bool parallel_uecmp_upd) = 0;
   virtual ~vxlan_input_params_t(void) {};
};

void load_vxlan_test_input(void);
void load_vxlan_test_output(void);

static inline void
load_vxlan_test (void) 
{
    load_vxlan_test_input();
    load_vxlan_test_output();
}

} // End namespace

#endif

