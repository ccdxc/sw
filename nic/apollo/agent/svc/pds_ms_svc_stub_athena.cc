// {C} Copyright 2020 Pensando Systems Inc. All rights reserved

#include "nic/apollo/api/include/pds_device.hpp"
#include "nic/apollo/api/include/pds_vpc.hpp"
#include "nic/apollo/api/include/pds_subnet.hpp"
#include "nic/apollo/api/include/pds.hpp"

namespace pds_ms {

void device_create (pds_device_spec_t *spec) {}
void subnet_create (pds_subnet_spec_t *spec, pds_batch_ctxt_t bctxt) {}
void subnet_update (pds_subnet_spec_t *spec, pds_batch_ctxt_t bctxt) {}
void subnet_delete(pds_obj_key_t &key, pds_batch_ctxt_t bctxt) {}
void vpc_create (pds_vpc_spec_t *spec, pds_batch_ctxt_t bctxt) {}
void vpc_update (pds_vpc_spec_t *spec, pds_batch_ctxt_t bctxt) {}
void vpc_delete(pds_vpc_spec_t *spec, pds_batch_ctxt_t bctxt) {}

} // End namespace
