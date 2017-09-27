#ifndef __IF_UTILS_HPP__
#define __IF_UTILS_HPP__

#include "nic/hal/src/tenant.hpp"
#include "nic/hal/src/l2segment.hpp"
#include "nic/hal/src/interface.hpp"
#include "nic/proto/types.pb.h"
#include "nic/include/ip.h"

namespace hal {

hal_ret_t pltfm_get_port_from_front_port_num(uint32_t fp_num, 
                                             uint32_t *port_num);

// APIs
hal_ret_t tenant_add_l2seg (tenant_t *tenant, l2seg_t *l2seg);
hal_ret_t tenant_del_l2seg (tenant_t *tenant, l2seg_t *l2seg);

hal_ret_t l2seg_handle_nwsec_update (l2seg_t *l2seg, 
                                     nwsec_profile_t *nwsec_prof);

hal_ret_t if_handle_nwsec_update (l2seg_t *l2seg, if_t *hal_if, 
                                  nwsec_profile_t *nwsec_prof);

}    // namespace hal

#endif    // __IF_UTILS_HPP__

