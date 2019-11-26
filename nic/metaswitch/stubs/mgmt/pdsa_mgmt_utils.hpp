// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// Purpose: Common helper APIs header file for metaswitch stub programming 

#ifndef __PDSA_MGMT_UTILS_HPP__
#define __PDSA_MGMT_UTILS_HPP__
#include <nbase.h>
extern "C" {
#include <a0spec.h>
#include <o0mac.h>
#include <a0cust.h>
#include <a0glob.h>
#include <a0mib.h>
#include <ambips.h>
#include <a0stubs.h>
#include <a0cpif.h>
#include "smsiincl.h"
#include "smsincl.h"
}
#include "li_mgmt_if.h"
#include "lim_mgmt_if.h"
#include "psm_prod.h"
#include "ftm_mgmt_if.h"
#include "nrm_mgmt_if.h"
#include "nrm_prod.h"
#include "psm_mgmt_if.h"
#include "include/sdk/ip.hpp"
#include "nic/metaswitch/stubs/mgmt/pdsa_config.hpp"
#include "nic/metaswitch/stubs/mgmt/pdsa_ctm.hpp"
#include "gen/proto/types.pb.h"

#define PDSA_CTM_CORRELATOR 0x100

NBB_VOID  pdsa_convert_ip_addr_to_amb_ip_addr (ip_addr_t     pdsa_ip_addr, 
                                               NBB_LONG      *type, 
                                               NBB_ULONG     *len, 
                                               NBB_BYTE      *amb_ip_addr);

NBB_VOID pdsa_convert_long_to_pdsa_ipv4_addr (NBB_ULONG ip,
                                              ip_addr_t *pdsa_ip_addr); 


NBB_VOID pdsa_set_address_oid(NBB_ULONG *oid,
                              const NBB_CHAR  *tableName,
                              const NBB_CHAR  *fieldName,
                              const types::IPAddress &addr);

NBB_VOID pdsa_set_address_field(AMB_GEN_IPS *mib_msg,
                                const NBB_CHAR  *tableName,
                                const NBB_CHAR  *fieldName,
                                NBB_VOID        *dest,
                                const types::IPAddress &addr);

NBB_LONG pdsa_nbb_get_long(NBB_BYTE *byteVal);

types::IPAddress* pdsa_get_address(const NBB_CHAR  *tableName,
                                  const NBB_CHAR  *fieldName,
                                  NBB_VOID    *src);

namespace pdsa_stub {
NBB_VOID pdsa_row_update_l2f (pdsa_config_t *conf);
NBB_VOID pdsa_row_update_li (pdsa_config_t *conf);
NBB_VOID pdsa_row_update_li_mj (pdsa_config_t *conf,
                                NBB_ULONG   interface_id,
                                NBB_ULONG   partner_type,
                                NBB_ULONG   partner_index,
                                NBB_ULONG   sub_index);
NBB_VOID pdsa_row_update_lim (pdsa_config_t *conf);
NBB_VOID pdsa_row_update_lim_mj (pdsa_config_t *conf,
                                 NBB_ULONG   interface_id,
                                 NBB_ULONG   partner_type,
                                 NBB_ULONG   partner_index,
                                 NBB_ULONG   sub_index);
NBB_VOID pdsa_row_update_sck (pdsa_config_t *conf);
NBB_VOID pdsa_row_update_smi (pdsa_config_t *conf);
NBB_VOID pdsa_row_update_ftm (pdsa_config_t *conf);
NBB_VOID pdsa_row_update_hals (pdsa_config_t *conf);
NBB_VOID pdsa_row_update_nar (pdsa_config_t *conf);
NBB_VOID pdsa_row_update_nrm (pdsa_config_t *conf);
NBB_VOID pdsa_row_update_psm (pdsa_config_t *conf);
NBB_VOID pdsa_row_update_ftm_mj (pdsa_config_t *conf,
                                 NBB_ULONG   interface_id,
                                 NBB_ULONG   partner_type,
                                 NBB_ULONG   partner_index,
                                 NBB_ULONG   sub_index);
NBB_VOID pdsa_row_update_nrm_mj (pdsa_config_t *conf,
                                 NBB_ULONG   interface_id,
                                 NBB_ULONG   partner_type,
                                 NBB_ULONG   partner_index,
                                 NBB_ULONG   sub_index);
NBB_VOID pdsa_row_update_psm_mj (pdsa_config_t *conf,
                                 NBB_ULONG   interface_id,
                                 NBB_ULONG   partner_type,
                                 NBB_ULONG   partner_index,
                                 NBB_ULONG   sub_index);
NBB_VOID pdsa_row_update_fts (pdsa_config_t *conf, NBB_LONG  admin_status);
NBB_VOID pdsa_row_update_rtm (pdsa_config_t *conf, NBB_LONG  admin_status);
NBB_VOID pdsa_row_update_rtm_mj (pdsa_config_t *conf, NBB_LONG slave_type);
NBB_VOID pdsa_rtm_redis_connected (pdsa_config_t *conf);
NBB_VOID pdsa_row_update_bgp_nm (pdsa_config_t *conf);
NBB_VOID pdsa_row_update_bgp_rm_afm_join (pdsa_config_t *conf,
                                          NBB_LONG  join,
                                          NBB_LONG  afi,
                                          NBB_LONG  safi);
NBB_VOID pdsa_row_update_evpn (pdsa_config_t *conf);
NBB_VOID pdsa_row_update_evpn_mj (pdsa_config_t *conf,
                                  NBB_ULONG   interface_id,
                                  NBB_ULONG   partner_type,
                                  NBB_ULONG   partner_index,
                                  NBB_ULONG   sub_index);
}

#endif /*__PDSA_MGMT_UTILS_HPP__*/
