// {C} Copyright 2019 Pensando Systems Inc. All rights reserved

#ifndef __PDSA_SUBNET_HPP__
#define __PDSA_SUBNET_HPP__

#include "nic/apollo/agent/core/subnet.hpp"
#include "nic/apollo/api/include/pds_vnic.hpp"

namespace pds_ms {
sdk_ret_t subnet_create (pds_subnet_spec_t *vpc_spec, pds_batch_ctxt_t bctxt);
sdk_ret_t subnet_delete (pds_subnet_spec_t *vpc_spec, pds_batch_ctxt_t bctxt);
sdk_ret_t subnet_update (pds_subnet_spec_t *spec, pds_batch_ctxt_t bctxt);
void subnet_vnic_bind (pds_vnic_spec_t *vnic_spec);
void subnet_vnic_unbind (pds_vnic_spec_t *vnic_spec);

};


#endif /*__PDSA_SUBNET_HPP__*/
