#include "nic/hal/src/aclqos/qos.hpp"
#include "nic/include/pd.hpp"
#include "nic/include/qos_api.hpp"

namespace hal {

// ----------------------------------------------------------------------------
// Qos-class API: Set PD Qos-class in PI Qos-class
// ----------------------------------------------------------------------------
void 
qos_class_set_pd_qos_class(qos_class_t *pi_qos_class, pd::pd_qos_class_t *pd_qos_class)
{
    pi_qos_class->pd = pd_qos_class;
}

qos_group_t 
qos_class_get_qos_group (qos_class_t *pi_qos_class)
{
    return pi_qos_class->key.qos_group;
}

bool 
qos_class_is_no_drop (qos_class_t *pi_qos_class)
{
    return pi_qos_class->no_drop;
}

// ----------------------------------------------------------------------------
// Copp API: Set PD Copp in PI Copp
// ----------------------------------------------------------------------------
void 
copp_set_pd_copp(copp_t *pi_copp, pd::pd_copp_t *pd_copp)
{
    pi_copp->pd = pd_copp;
}

} // namespace hal
