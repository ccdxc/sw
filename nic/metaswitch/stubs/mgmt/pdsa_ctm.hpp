// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// Purpose: CTM helper APIs header file to communicate with metaswitch 
// from mgmt stub 

#ifndef __PDSA_CTM_HPP__
#define __PDSA_CTM_HPP__

#include "nic/metaswitch/stubs/mgmt/pdsa_config.hpp"
#include "include/sdk/ip.hpp"

typedef NBB_VOID (*pdsa_amb_fill_fnptr_t)(AMB_GEN_IPS *, pdsa_config_t *);

NBB_VOID pdsa_ctm_send_transaction_start (NBB_ULONG correlator);
NBB_VOID pdsa_ctm_send_transaction_abort (NBB_ULONG correlator);
NBB_VOID pdsa_ctm_send_transaction_end (NBB_ULONG correlator);
NBB_VOID pdsa_ctm_rcv_ips (NBB_IPS *ips NBB_CCXT NBB_CXT);
NBB_VOID pdsa_ctm_send_row_update_common (pdsa_config_t         *conf, 
                                          pdsa_amb_fill_fnptr_t fill_api);

#endif /*__PDSA_CTM_HPP__*/
