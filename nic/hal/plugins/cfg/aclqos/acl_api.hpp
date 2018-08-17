//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//
// ACL APIs exported by PI to PD.
//-----------------------------------------------------------------------------

#ifndef __ACL_API_HPP__
#define __ACL_API_HPP__

namespace hal {

using hal::acl_t;

// Acl APIs
acl_match_spec_t *acl_get_match_spec(acl_t *pi_acl);
acl_action_spec_t *acl_get_action_spec(acl_t *pi_acl);
uint32_t acl_get_priority(acl_t *pi_acl);

}    // namespace hal

#endif    // __ACL_API_HPP__

