//------------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//------------------------------------------------------------------------------
#ifndef __PDS_MS_TEST_PHY_PORT_PARAMS_HPP__
#define __PDS_MS_TEST_PHY_PORT_PARAMS_HPP__

#include "nic/metaswitch/stubs/test/hals/test_params.hpp"
#include "nic/metaswitch/stubs/common/pds_ms_util.hpp"
#include "nic/apollo/api/include/pds_if.hpp"
#include "nic/apollo/test/base/utils.hpp"

namespace pds_ms_test{

class phy_port_input_params_t : public test_input_base_t {
public:
    uint32_t      phy_port;
    uint32_t      ms_ifindex;
    uint32_t      l3_if; // Dummy number used to generate L3If UUId
    bool          admin_state;

   // These inputs are used to generate feeder inputs 
   // as well as output verifications 
   virtual void init() {};
   void next(void) override {}; 
   virtual ~phy_port_input_params_t(void) {};
};

void load_phy_port_test_input(void);
void load_phy_port_test_output(void);

static inline void
load_phy_port_test (void) 
{
    load_phy_port_test_input();
    load_phy_port_test_output();
}

} // End namespace

#endif

