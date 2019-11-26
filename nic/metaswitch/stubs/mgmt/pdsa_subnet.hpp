// {C} Copyright 2019 Pensando Systems Inc. All rights reserved

#ifndef __PDSA_SUBNET_HPP__
#define __PDSA_SUBNET_HPP__
#include "nic/metaswitch/stubs/mgmt/pdsa_mgmt_utils.hpp"
#include "nic/apollo/agent/core/subnet.hpp"
#include "nic/apollo/api/include/pds_vnic.hpp"

namespace pdsa_stub {
void pdsa_subnet_create (pds_subnet_spec_t *vpc_spec);
void pdsa_subnet_delete (pds_subnet_spec_t *vpc_spec);
void pdsa_subnet_vnic_bind (pds_vnic_spec_t *vnic_spec);
void pdsa_subnet_vnic_unbind (pds_vnic_spec_t *vnic_spec);

};


#endif /*__PDSA_SUBNET_HPP__*/
