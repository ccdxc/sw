//------------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//------------------------------------------------------------------------------
#ifndef __PDS_API_MOCK_HPP__
#define __PDS_API_MOCK_HPP__

#include "nic/apollo/test/utils/device.hpp"
#include "nic/apollo/test/utils/nexthop.hpp"
#include "nic/apollo/test/utils/nexthop_group.hpp"
#include "nic/apollo/test/utils/policy.hpp"
#include "nic/apollo/test/utils/route.hpp"
#include "nic/apollo/test/utils/subnet.hpp"
#include "nic/apollo/test/utils/svc_mapping.hpp"
#include "nic/apollo/test/utils/tag.hpp"
#include "nic/apollo/test/utils/tep.hpp"
#include "nic/apollo/test/utils/vnic.hpp"
#include "nic/apollo/test/utils/vpc.hpp"
#include "nic/apollo/test/utils/vpc_peer.hpp"
#include "nic/apollo/framework/api.h"
#include "nic/metaswitch/stubs/test/hals/test_params.hpp"

namespace pdsa_test {

struct spec_t {
    obj_id_t               obj_id; 
    api_op_t               op;
    bool                   mock_fail = false;
    union {
        pds_device_spec_t       device;
        pds_nexthop_group_spec_t nhgroup;
        pds_subnet_spec_t       subnet;
        pds_tep_spec_t          tep;
        pds_vnic_spec_t         vnic;
        pds_vpc_spec_t          vpc;
    };
    spec_t (obj_id_t oid, api_op_t o) : obj_id(oid), op(o) {};
};

using batch_spec_t = std::vector<spec_t>;

class pds_mock_t : public test_output_base_t {
public:
    bool          sim;
    void*         cookie = nullptr;
    batch_spec_t  expected_pds;
    batch_spec_t  rcvd_pds;

    pds_mock_t(bool sim_=false) : sim(sim_) {};

    // Following need to be overridden by one of the xxx_pds_mock files
    // to set expected results specific to that feature
    void expect_create() override {};
    void expect_update() override {};
    void expect_delete() override {};
    void validate() override {}; 
    void expect_create_pds_async_fail() override {};

    void cleanup(void) override {
        clear_batches();
    }
    void expect_pds_spec_op_fail(void) override {
       mock_pds_spec_op_fail_ = true;
    }
    void expect_pds_batch_commit_fail(void) override {
       mock_pds_batch_commit_fail_ = true;
    }

    bool   pds_ret_status = false; // Mock Async status returned by PDS API
    bool   mock_pds_batch_async_fail_ = false;
    bool   mock_pds_batch_commit_fail_ = false;
    bool   mock_pds_spec_op_fail_ = false;

protected:    
    bool   op_delete_ = false;
    bool   op_create_ = true;

    void clear_batches(void) {
        expected_pds.clear();
        rcvd_pds.clear();
    }
   void reset_mock_fail(void) {
       mock_pds_batch_commit_fail_ = false;
       mock_pds_batch_async_fail_ = false;
       mock_pds_spec_op_fail_ = false;
   }
};

} // End namespace pdsa_test

#endif
