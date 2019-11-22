//------------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//------------------------------------------------------------------------------

#include "nic/metaswitch/stubs/test/pdsmock/pds_api_mock_validate.hpp"

namespace pdsa_test {

bool
pds_tep_mock_validate(const spec_t&  expected_pds,
                      const spec_t&  rcvd_pds)
{
    if (rcvd_pds.op == API_OP_DELETE) {
        if (expected_pds.tep.key.id != rcvd_pds.tep.key.id) {
            std::cout << "TEP delete key compare failed" << std::endl;
            std::cout << "Expected: " << expected_pds.tep.key.id <<std::endl;
            std::cout << "Rcvd: " << rcvd_pds.tep.key.id << std::endl;
            return false;
        }
    }
    api_test::tep_feeder feeder;
    feeder.spec = expected_pds.tep; 
    if (!feeder.spec_compare (&rcvd_pds.tep)) {
        std::cout << "TEP compare failed" << std::endl;
        std::cout << "Expected: " << &(feeder.spec) <<std::endl;
        std::cout << "Rcvd: " << &(rcvd_pds.tep) << std::endl;
        return false;
    }
    return true;
}

bool
pds_nhgroup_mock_validate(const spec_t&  expected_pds,
                          const spec_t&  rcvd_pds)
{
    if (rcvd_pds.op == API_OP_DELETE) {
        if (expected_pds.nhgroup.key.id != rcvd_pds.nhgroup.key.id) {
            std::cout << "NHGroup delete key compare failed" << std::endl;
            std::cout << "Expected: " << expected_pds.nhgroup.key.id <<std::endl;
            std::cout << "Rcvd: " << rcvd_pds.nhgroup.key.id << std::endl;
            return false;
        }
    }
    api_test::nexthop_group_feeder feeder;
    feeder.spec = expected_pds.nhgroup;
    if (!feeder.spec_compare (&rcvd_pds.nhgroup)) {
        std::cout << "NH Group compare failed" << std::endl;
        std::cout << "Expected: " << &(feeder.spec) <<std::endl;
        std::cout << "Rcvd: " << &(rcvd_pds.nhgroup) << std::endl;
        return false;
    }
    return true;
}

} // End namespace pdsa_test
