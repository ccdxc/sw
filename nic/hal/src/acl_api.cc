#include <acl.hpp>
#include <pd.hpp>
#include <acl_api.hpp>

namespace hal {

// ----------------------------------------------------------------------------
// Acl API: Get Acl's ID
// ----------------------------------------------------------------------------
uint32_t
acl_get_acl_id(acl_t *pi_acl)
{
    return pi_acl->acl_id;
}

// ----------------------------------------------------------------------------
// Acl API: Set PD Acl in PI Acl
// ----------------------------------------------------------------------------
void 
acl_set_pd_acl(acl_t *pi_acl, void *pd_acl)
{
    pi_acl->pd_acl = pd_acl;
}

// ----------------------------------------------------------------------------
// Acl API: Get PD Acl from PI Acl
// ----------------------------------------------------------------------------
void*
acl_get_pd_acl(acl_t *pi_acl)
{
    return pi_acl->pd_acl;
}

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
