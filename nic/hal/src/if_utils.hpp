#ifndef __IF_UTILS_HPP__
#define __IF_UTILS_HPP__

#include "nic/hal/src/tenant.hpp"
#include "nic/hal/src/l2segment.hpp"
#include "nic/hal/src/session.hpp"
#include "nic/hal/src/interface.hpp"
#include "nic/hal/src/network.hpp"
#include "nic/hal/pd/common/pd_api.hpp"
#include "nic/hal/src/lif.hpp"
#include "nic/gen/proto/hal/types.pb.h"
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
hal_ret_t tenant_handle_nwsec_update (tenant_t *tenant, 
                                      nwsec_profile_t *nwsec_prof);

// Adding IFs to lif 
hal_ret_t lif_add_if (lif_t *lif, if_t *hal_if);
hal_ret_t lif_del_if (lif_t *lif, if_t *hal_if);

// Adding ifs to l2seg
hal_ret_t l2seg_add_if (l2seg_t *l2seg, if_t *hal_if);
hal_ret_t l2seg_del_if (l2seg_t *l2seg, if_t *hal_if);

// Adding l2segs to if
hal_ret_t if_add_l2seg (if_t *hal_if, l2seg_t *l2seg);
hal_ret_t if_del_l2seg (if_t *hal_if, l2seg_t *l2seg);

// Adding tenants to nwsec profile
hal_ret_t nwsec_prof_add_tenant (nwsec_profile_t *nwsec, 
                                 tenant_t *tenant);
hal_ret_t nwsec_prof_del_tenant (nwsec_profile_t *nwsec, 
                                 tenant_t *tenant);

// Adding sessions & l2segs to network
hal_ret_t network_add_l2seg (network_t *nw, l2seg_t *l2seg);
hal_ret_t network_del_l2seg (network_t *nw, l2seg_t *l2seg);
hal_ret_t network_add_session (network_t *nw, session_t *sess);
hal_ret_t network_del_session (network_t *nw, session_t *sess);

// Add enicifs to uplink
hal_ret_t uplink_add_enicif (if_t *uplink, if_t *enic_if);
hal_ret_t uplink_del_enicif (if_t *uplink, if_t *enic_if);

// Handle lif update in IF
hal_ret_t if_handle_lif_update (pd::pd_if_lif_upd_args_t *args);

hal_ret_t add_nw_to_security_group(uint32_t sg_id, hal_handle_t nw_handle_id);
hal_ret_t del_nw_from_security_group(uint32_t sg_id, hal_handle_t nw_handle_id);


}    // namespace hal

#endif    // __IF_UTILS_HPP__

