#include "nic/hal/src/acl.hpp"
#include "nic/include/pd.hpp"
#include "nic/include/acl_api.hpp"

namespace hal {

// ----------------------------------------------------------------------------
// Acl API: Get Match spec in PI Acl
// ----------------------------------------------------------------------------
acl_match_spec_t *
acl_get_match_spec(acl_t *pi_acl) 
{
    return &pi_acl->match_spec;
}

// ----------------------------------------------------------------------------
// Acl API: Get Action spec in PI Acl
// ----------------------------------------------------------------------------
acl_action_spec_t *
acl_get_action_spec(acl_t *pi_acl)
{
    return &pi_acl->action_spec;
}

// ----------------------------------------------------------------------------
// Acl API: Get priority in PI Acl
// ----------------------------------------------------------------------------
uint32_t
acl_get_priority(acl_t *pi_acl)
{
    return pi_acl->priority;
}
} // namespace hal
