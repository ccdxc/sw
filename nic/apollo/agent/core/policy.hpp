//------------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// -----------------------------------------------------------------------------

#ifndef __CORE_VPC_HPP__
#define __CORE_VPC_HPP__

#include "nic/apollo/api/include/pds_policy.hpp"

namespace core {

typedef void (*policy_get_cb_t)(const pds_policy_info_t *spec, void *ctxt);

typedef struct policy_db_cb_ctxt_s {
    policy_get_cb_t cb;
    void         *ctxt;
} policy_db_cb_ctxt_t;

sdk_ret_t policy_create(pds_policy_key_t *key, pds_policy_spec_t *spec);
sdk_ret_t policy_delete(pds_policy_key_t *key);
sdk_ret_t policy_get(pds_policy_key_t *key, pds_policy_info_t *info);
sdk_ret_t policy_get_all(policy_get_cb_t policy_get_cb, void *ctxt);

}    // namespace core

#endif    // __CORE_VPC_HPP__
