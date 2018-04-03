/* 
 * ----------------------------------------------------------------------------
 *
 * qos_api.hpp
 *
 * QOS APIs exported by PI to PD.
 *
 * ----------------------------------------------------------------------------
 */
#ifndef __QOS_API_HPP__
#define __QOS_API_HPP__


namespace hal {

using hal::qos_class_t;

// Qos-class APIs
void qos_class_set_pd_qos_class(qos_class_t *pi_qos_class, pd::pd_qos_class_t *pd_qos_class);
qos_group_t qos_class_get_qos_group(qos_class_t *pi_qos_class);
bool qos_class_is_no_drop(qos_class_t *pi_qos_class);

// Copp APIs
void copp_set_pd_copp(copp_t *pi_copp, pd::pd_copp_t *pd_copp);
} // namespace hal
#endif // __QOS_API_HPP__
