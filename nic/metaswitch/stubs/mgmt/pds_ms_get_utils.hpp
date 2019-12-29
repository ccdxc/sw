// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// Purpose: Helper APIs header file to help fill-up GET header for metaswitch 
// from mgmt stub 

#ifndef __PDS_MS_GET_UTILS_HPP__
#define __PDS_MS_GET_UTILS_HPP__

#include "nic/metaswitch/stubs/mgmt/pds_ms_config.hpp"
#include "include/sdk/ip.hpp"

AMB_GET *pds_ms_amb_get_bulk_common(NBB_LONG bulk_size,
                                    NBB_LONG data_len,
                                    NBB_LONG oid_len,
                                    NBB_ULONG *oid
                                    NBB_CCXT_T NBB_CXT);

AMB_GET *pds_ms_amb_get_common(NBB_LONG data_len,
                               NBB_LONG oid_len,
                               NBB_ULONG *oid
                               NBB_CCXT_T NBB_CXT);

#endif /*__PDS_MS_GET_UTILS_HPP__*/
