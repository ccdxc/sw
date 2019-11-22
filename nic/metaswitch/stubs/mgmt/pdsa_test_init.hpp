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

// Prototypes
NBB_VOID pdsa_test_init();

NBB_VOID pdsa_test_row_update_si (pdsa_config_t *conf);
NBB_VOID pdsa_test_row_update_l2f (pdsa_config_t *conf);
NBB_VOID pdsa_test_row_update_li (pdsa_config_t *conf);
NBB_VOID pdsa_test_row_update_li_mj (pdsa_config_t *conf,
                                     NBB_ULONG   interface_id,
                                     NBB_ULONG   partner_type,
                                     NBB_ULONG   partner_index,
                                     NBB_ULONG   sub_index);
NBB_VOID pdsa_test_row_update_lim (pdsa_config_t *conf);
NBB_VOID pdsa_test_row_update_lim_mj (pdsa_config_t *conf,
                                      NBB_ULONG   interface_id,
                                      NBB_ULONG   partner_type,
                                      NBB_ULONG   partner_index,
                                      NBB_ULONG   sub_index);
NBB_VOID pdsa_test_row_update_lim_if_cfg (pdsa_config_t *conf,
                                          NBB_LONG   if_index,
                                          NBB_LONG   ipv4_enabled,
                                          NBB_LONG   ipv4_forwarding,
                                          NBB_LONG   ipv6_enabled,
                                          NBB_LONG   ipv6_forwarding,
                                          NBB_LONG   forwarding_mode);
NBB_VOID pdsa_test_row_update_lim_if_addr (pdsa_config_t *conf);
NBB_VOID pdsa_test_row_update_sck (pdsa_config_t *conf);
NBB_VOID pdsa_test_row_update_l2f_mac_ip_cfg (pdsa_config_t *conf);
NBB_VOID pdsa_test_row_update_smi (pdsa_config_t *conf);
NBB_VOID pdsa_test_row_update_ftm (pdsa_config_t *conf);
NBB_VOID pdsa_test_row_update_hals (pdsa_config_t *conf);
NBB_VOID pdsa_test_row_update_nar (pdsa_config_t *conf);
NBB_VOID pdsa_test_row_update_nrm (pdsa_config_t *conf);
NBB_VOID pdsa_test_row_update_psm (pdsa_config_t *conf);
NBB_VOID pdsa_test_row_update_ftm_mj (pdsa_config_t *conf,
                                      NBB_ULONG   interface_id,
                                      NBB_ULONG   partner_type,
                                      NBB_ULONG   partner_index,
                                      NBB_ULONG   sub_index);
NBB_VOID pdsa_test_row_update_nrm_mj (pdsa_config_t *conf,
                                      NBB_ULONG   interface_id,
                                      NBB_ULONG   partner_type,
                                      NBB_ULONG   partner_index,
                                      NBB_ULONG   sub_index);
NBB_VOID pdsa_test_row_update_psm_mj (pdsa_config_t *conf,
                                      NBB_ULONG   interface_id,
                                      NBB_ULONG   partner_type,
                                      NBB_ULONG   partner_index,
                                      NBB_ULONG   sub_index);
NBB_VOID pdsa_test_row_update_rtm (pdsa_config_t *conf, NBB_LONG  admin_status);
NBB_VOID pdsa_test_row_update_fts (pdsa_config_t *conf, NBB_LONG  admin_status);
NBB_VOID pdsa_test_row_update_rtm_mj (pdsa_config_t *conf, NBB_LONG slave_type);
NBB_VOID pdsa_rtm_redis_connected (pdsa_config_t *conf);

NBB_VOID pdsa_test_row_update_bgp_rm (pdsa_config_t *conf);
NBB_VOID pdsa_test_row_update_bgp_nm (pdsa_config_t *conf);
NBB_VOID pdsa_test_row_update_bgp_rm_afi_safi (pdsa_config_t *conf);
NBB_VOID pdsa_test_row_update_bgp_nm_listen (pdsa_config_t *conf);
NBB_VOID pdsa_test_row_update_bgp_rm_afm_join (pdsa_config_t *conf,
                                               NBB_LONG  join,
                                               NBB_LONG  afi,
                                               NBB_LONG  safi);
NBB_VOID pdsa_test_row_update_bgp_peer (pdsa_config_t *conf);
NBB_VOID pdsa_test_row_update_evpn (pdsa_config_t *conf);
NBB_VOID pdsa_test_row_update_evpn_mj (pdsa_config_t *conf,
                                       NBB_ULONG   interface_id,
                                       NBB_ULONG   partner_type,
                                       NBB_ULONG   partner_index,
                                       NBB_ULONG   sub_index);
NBB_VOID pdsa_test_row_update_evpn_bd (pdsa_config_t *conf);
NBB_VOID pdsa_test_row_update_evpn_if_bind_cfg (pdsa_config_t *conf);
NBB_VOID pdsa_test_row_update_evpn_evi (pdsa_config_t *conf);

#endif /*__PDSA_TEST_INIT_HPP__*/
