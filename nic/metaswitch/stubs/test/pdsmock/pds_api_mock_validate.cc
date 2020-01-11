//------------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//------------------------------------------------------------------------------

#include "nic/metaswitch/stubs/test/pdsmock/pds_api_mock_validate.hpp"

namespace pds_ms_test {

using namespace test::api; 
bool
pds_tep_mock_validate (const spec_t&  expected_pds,
                       const spec_t&  rcvd_pds)
{
    if (rcvd_pds.op == API_OP_DELETE) {
        if (expected_pds.tep.key != rcvd_pds.tep.key) {
            std::cout << "TEP delete key compare failed" << std::endl;
            std::cout << "Expected: " << expected_pds.tep.key.tostr() <<std::endl;
            std::cout << "Rcvd: " << rcvd_pds.tep.key.tostr() << std::endl;
            return false;
        }
        return true;;
    }
    test::api::tep_feeder feeder;
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
pds_nhgroup_mock_validate (const spec_t&  expected_pds,
                           const spec_t&  rcvd_pds)
{
    if (rcvd_pds.op == API_OP_DELETE) {
        if (expected_pds.nhgroup.key != rcvd_pds.nhgroup.key) {
            std::cout << "NHGroup delete key compare failed" << std::endl;
            std::cout << "Expected: " << expected_pds.nhgroup.key.tostr() <<std::endl;
            std::cout << "Rcvd: " << rcvd_pds.nhgroup.key.tostr() << std::endl;
            return false;
        }
        return true;;
    }
    test::api::nexthop_group_feeder feeder;
    feeder.spec = expected_pds.nhgroup;
    if (!feeder.spec_compare (&rcvd_pds.nhgroup)) {
        std::cout << "NH Group compare failed" << std::endl;
        std::cout << "Expected: " << &(feeder.spec) <<std::endl;
        std::cout << "Rcvd: " << &(rcvd_pds.nhgroup) << std::endl;
        return false;
    }
    return true;
}

bool
pds_if_mock_validate (const spec_t&  expected_pds,
                      const spec_t&  rcvd_pds)
{
    if (rcvd_pds.op == API_OP_DELETE) {
        if (expected_pds.intf.key.id != rcvd_pds.intf.key.id) {
            std::cout << "IF delete key compare failed" << std::endl;
            std::cout << "Expected: " << expected_pds.intf.key.id <<std::endl;
            std::cout << "Rcvd: " << rcvd_pds.intf.key.id << std::endl;
            return false;
        }
        return true;;
    }
    test::api::if_feeder feeder;
    feeder.spec_feeder = expected_pds.intf; 
    if (!feeder.spec_compare (&rcvd_pds.intf)) {
        std::cout << "IF compare failed" << std::endl;
        std::cout << "Expected: " << &(feeder.spec_feeder) <<std::endl;
        std::cout << "Rcvd: " << &(rcvd_pds.intf) << std::endl;
        return false;
    }
    return true;
}

bool
pds_subnet_mock_validate (const spec_t&  expected_pds,
                          const spec_t&  rcvd_pds)
{
    if (rcvd_pds.op == API_OP_DELETE) {
        if (expected_pds.subnet.key != rcvd_pds.subnet.key) {
            std::cout << "Subnet delete key compare failed" << std::endl;
            std::cout << "Expected: " << expected_pds.subnet.key.tostr() <<std::endl;
            std::cout << "Rcvd: " << rcvd_pds.subnet.key.tostr() << std::endl;
            return false;
        }
        return true;;
    }
    test::api::subnet_feeder feeder;
    feeder.spec = expected_pds.subnet; 
    if (!feeder.spec_compare (&rcvd_pds.subnet)) {
        std::cout << "Subnet compare failed" << std::endl;
        std::cout << "Expected: " << &(feeder.spec) <<std::endl;
        std::cout << "Rcvd: " << &(rcvd_pds.subnet) << std::endl;
        return false;
    }
    if (feeder.spec.host_ifindex != rcvd_pds.subnet.host_ifindex) {
        std::cout << "Subnet Host IfIndex failed" << std::endl;
        std::cout << "Expected: " << feeder.spec.host_ifindex <<std::endl;
        std::cout << "Rcvd: " << rcvd_pds.subnet.host_ifindex << std::endl;
        return false;
    }
    return true;
}

bool
pds_vpc_mock_validate (const spec_t&  expected_pds,
                          const spec_t&  rcvd_pds)
{
    if (rcvd_pds.op == API_OP_DELETE) {
        if (expected_pds.vpc.key != rcvd_pds.vpc.key) {
            std::cout << "VPC delete key compare failed" << std::endl;
            std::cout << "Expected: " << expected_pds.vpc.key.tostr() <<std::endl;
            std::cout << "Rcvd: " << rcvd_pds.vpc.key.tostr() << std::endl;
            return false;
        }
        return true;;
    }
#if 0
    test::api::vpc_feeder feeder;

    feeder.spec = expected_pds.vpc; 
    if (!feeder.spec_compare (&rcvd_pds.vpc)) {
        std::cout << "VPC compare failed" << std::endl;
        std::cout << "Expected: " << &(feeder.spec) <<std::endl;
        std::cout << "Rcvd: " << &(rcvd_pds.vpc) << std::endl;
        return false;
    }
#endif
    return true;
}

} // End namespace pds_ms_test
