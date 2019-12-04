// {C} Copyright 2019 Pensando Systems Inc. All rights reserved

#ifndef __PDS_MS_VPC_HPP__
#define __PDS_MS_VPC_HPP__

#include "nic/apollo/agent/core/vpc.hpp"
#include "nic/apollo/api/include/pds_vpc.hpp"

namespace pds_ms {

sdk_ret_t vpc_create(pds_vpc_spec_t *spec, pds_batch_ctxt_t bctxt);
sdk_ret_t vpc_delete(pds_vpc_spec_t *spec, pds_batch_ctxt_t bctxt);
sdk_ret_t vpc_update(pds_vpc_spec_t *spec, pds_batch_ctxt_t bctxt);

};

#endif    //__PDS_MS_VPC_HPP__
