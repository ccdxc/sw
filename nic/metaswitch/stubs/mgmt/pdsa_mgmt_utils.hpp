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
#include "include/sdk/ip.hpp"
#include "nic/metaswitch/stubs/mgmt/pdsa_config.hpp"
#include "nic/metaswitch/stubs/mgmt/pdsa_ctm.hpp"
#include "gen/proto/types.pb.h"

NBB_VOID  pdsa_convert_ip_addr_to_amb_ip_addr (ip_addr_t     pdsa_ip_addr, 
                                               NBB_LONG      *type, 
                                               NBB_ULONG     *len, 
                                               NBB_BYTE      *amb_ip_addr);

NBB_VOID pdsa_convert_long_to_pdsa_ipv4_addr (NBB_ULONG ip,
                                              ip_addr_t *pdsa_ip_addr); 

int parse_json_config(pdsa_config_t *conf);

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

#endif /*__PDSA_MGMT_UTILS_HPP__*/
