//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// learn thread's API processing interface
///
//----------------------------------------------------------------------------

#ifndef __LEARN__LEARN_API_HPP__
#define __LEARN__LEARN_API_HPP__

#include "nic/sdk/include/sdk/base.hpp"
#include "nic/apollo/api/include/pds.hpp"
#include "nic/apollo/framework/api.h"
#include "nic/apollo/learn/learn_ctxt.hpp"

namespace learn {

/// \brief commit API batch to learn thread
sdk_ret_t api_batch_commit(pds_batch_ctxt_t bctxt);

typedef union {
    pds_mapping_key_t    *skey;
    pds_mapping_spec_t   *spec;
} mapping_key_spec_t;

sdk_ret_t process_mapping_api(mapping_key_spec_t key_spec, api_op_t op,
                              pds_batch_ctxt_t bctxt,
                              learn_batch_ctxt_t *lbctxt);
}   // namespace learn

#endif  // __LEARN__LEARN_API_HPP__
