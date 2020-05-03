//------------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//------------------------------------------------------------------------------
#ifndef __PDS_MS_TEST_UNDERLAY_ECMP_PARAMS_HPP__
#define __PDS_MS_TEST_UNDERLAY_ECMP_PARAMS_HPP__

#include "nic/metaswitch/stubs/test/hals/test_params.hpp"
#include "nic/metaswitch/stubs/common/pds_ms_util.hpp"
#include "nic/metaswitch/stubs/common/pds_ms_defs.hpp"

namespace pds_ms_test{

using pds_ms::ms_ifindex_t;
using pds_ms::mac_addr_wr_t;

struct nhinfo_t {
    ms_ifindex_t   l3_ifindex;
    mac_addr_wr_t  l3_dest_mac;
    nhinfo_t(ms_ifindex_t ifi, const std::string& mac) 
        : l3_ifindex(ifi) {
        mac_str_to_addr ((char*)mac.c_str(), l3_dest_mac.m_mac);
    }
};    

class underlay_ecmp_input_params_t : public test_input_base_t {
public:
    uint32_t                pathset_id;
    std::vector<nhinfo_t>   nexthops;
    std::vector<nhinfo_t>   deleted_nexthops;
    bool bh = false; // blackhole

   // These inputs are used to generate feeder inputs 
   // as well as output verifications 
   virtual void init(std::vector<nhinfo_t>&& nhs) {
       nexthops = std::move(nhs);
   }
   void set_bh(void) { bh = true; }
   void reset_bh(void) { bh = false; }
   void modify(void) override {
       // Move second half of nexthops to deleted list
       auto end = nexthops.size()/2;
       auto it = nexthops.begin();
       for (auto i = 0; i < end; ++i) {
           deleted_nexthops.emplace_back(nexthops[i]);
           ++it;
       }
       nexthops.erase(it, nexthops.end());
   } 
   void next(void) override {};
   virtual ~underlay_ecmp_input_params_t(void) {};
};

void load_underlay_ecmp_test_input(void);
void load_underlay_ecmp_test_output(void);

static inline void
load_underlay_ecmp_test (void) 
{
    load_underlay_ecmp_test_input();
    load_underlay_ecmp_test_output();
}

} // End namespace

#endif

