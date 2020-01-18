// {C} Copyright 2019 Pensando Systems Inc. All rights reserved

#ifndef __PDS_MS_INTERFACE_HPP__
#define __PDS_MS_INTERFACE_HPP__

#include "nic/apollo/agent/core/interface.hpp"
#include "nic/apollo/api/include/pds_if.hpp"

namespace pds_ms {
#define LOOPBACK_IF_ID  0
sdk_ret_t interface_create(pds_if_spec_t *spec, pds_batch_ctxt_t bctxt);
sdk_ret_t interface_delete(pds_if_spec_t *spec, pds_batch_ctxt_t bctxt);
sdk_ret_t interface_update(pds_if_spec_t *spec, pds_batch_ctxt_t bctxt);

};

#endif    // __PDS_MS_INTERFACE_HPP__
