// {C} Copyright 2019 Pensando Systems Inc. All rights reserved

#ifndef __PDSA_VPC_HPP__
#define __PDSA_VPC_HPP__
#include "nic/metaswitch/stubs/mgmt/pdsa_mgmt_utils.hpp"
#include "nic/apollo/agent/core/vpc.hpp"

namespace pdsa_stub {
void pdsa_vpc_create (pds_vpc_spec_t *vpc_spec);
void pdsa_vpc_delete (pds_vpc_spec_t *vpc_spec);

};


#endif /*__PDSA_VPC_HPP__*/
