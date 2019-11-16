//
// {C} Copyright 2018 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This file deals with PDS batch related APIs
///
//----------------------------------------------------------------------------

#include "nic/apollo/framework/api_base.hpp"
#include "nic/apollo/framework/api_msg.hpp"
#include "nic/apollo/api/include/pds_batch.hpp"

pds_batch_ctxt_t
pds_batch_start (_In_ pds_batch_params_t *batch_params)
{
    return api::api_batch_start(batch_params);
}

sdk_ret_t
pds_batch_destroy (pds_batch_ctxt_t bctxt)
{
    return api::api_batch_destroy(bctxt);
}

sdk_ret_t
pds_batch_commit (pds_batch_ctxt_t bctxt)
{
    return api::api_batch_commit(bctxt);
}
