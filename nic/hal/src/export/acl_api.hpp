/* 
 * ----------------------------------------------------------------------------
 *
 * acl_api.hpp
 *
 * ACL APIs exported by PI to PD.
 *
 * ----------------------------------------------------------------------------
 */
#ifndef __ACL_API_HPP__
#define __ACL_API_HPP__


namespace hal {

using hal::acl_t;

// Acl APIs
uint32_t acl_get_acl_id(acl_t *pi_acl);
void acl_set_pd_acl(acl_t *pi_acl, void *pd_acl);
acl_match_spec_t *acl_get_match_spec(acl_t *pi_acl);
acl_action_spec_t *acl_get_action_spec(acl_t *pi_acl);
uint32_t acl_get_priority(acl_t *pi_acl);

} // namespace hal
#endif // __ACL_API_HPP__
