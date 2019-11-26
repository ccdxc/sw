#ifndef __PDSA_TEST_INIT_HPP__
#define __PDSA_TEST_INIT_HPP__
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// Purpose: defines prototypes and typedefs for  metaswitch temp init seq APIs

#include "nic/metaswitch/stubs/mgmt/pdsa_mgmt_utils.hpp"
#include "nic/metaswitch/stubs/mgmt/pdsa_ctm.hpp"
#include "li_mgmt_if.h"
#include "lim_mgmt_if.h"
#include "psm_prod.h"
#include "ftm_mgmt_if.h"
#include "nrm_mgmt_if.h"
#include "nrm_prod.h"
#include "psm_mgmt_if.h"

namespace pdsa_stub {

// Prototypes
NBB_VOID pdsa_test_row_update_lim_if_cfg (pdsa_config_t *conf,
                                          NBB_LONG   if_index,
                                          NBB_LONG   ipv4_enabled,
                                          NBB_LONG   ipv4_forwarding,
                                          NBB_LONG   ipv6_enabled,
                                          NBB_LONG   ipv6_forwarding,
                                          NBB_LONG   forwarding_mode);
NBB_VOID pdsa_test_row_update_lim_if_addr (pdsa_config_t *conf);
NBB_VOID pdsa_test_row_update_l2f_mac_ip_cfg (pdsa_config_t *conf);

NBB_VOID pdsa_test_row_update_bgp_rm (pdsa_config_t *conf);
NBB_VOID pdsa_test_row_update_bgp_rm_afi_safi (pdsa_config_t *conf);
NBB_VOID pdsa_test_row_update_bgp_nm_listen (pdsa_config_t *conf);
NBB_VOID pdsa_test_row_update_bgp_peer (pdsa_config_t *conf);
}

#endif /*__PDSA_TEST_INIT_HPP__*/
