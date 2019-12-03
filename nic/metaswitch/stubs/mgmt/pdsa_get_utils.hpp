// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// Purpose: Helper APIs header file to help fill-up GET header for metaswitch 
// from mgmt stub 

#ifndef __PDSA_GET_UTILS_HPP__
#define __PDSA_GET_UTILS_HPP__

#include "nic/metaswitch/stubs/mgmt/pdsa_config.hpp"
#include "include/sdk/ip.hpp"

AMB_GET *pdsa_amb_get_bulk_common(NBB_LONG bulk_size,
                                     NBB_LONG data_len,
                                     NBB_LONG oid_len,
                                     NBB_ULONG *oid
                                     NBB_CCXT_T NBB_CXT);

#endif /*__PDSA_GET_UTILS_HPP__*/
