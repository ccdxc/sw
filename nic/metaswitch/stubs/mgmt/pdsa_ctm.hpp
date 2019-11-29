// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// Purpose: CTM helper APIs header file to communicate with metaswitch 
// from mgmt stub 

#ifndef __PDSA_CTM_HPP__
#define __PDSA_CTM_HPP__

#include "nic/metaswitch/stubs/mgmt/pdsa_config.hpp"
#include "include/sdk/ip.hpp"

typedef NBB_VOID (*pdsa_amb_fill_fnptr_t)(AMB_GEN_IPS *, 
                                          pdsa_stub::pdsa_config_t *);

NBB_VOID pdsa_ctm_send_transaction_start (NBB_ULONG correlator);
NBB_VOID pdsa_ctm_send_transaction_abort (NBB_ULONG correlator);
NBB_VOID pdsa_ctm_send_transaction_end (NBB_ULONG correlator);
NBB_VOID pdsa_ctm_rcv_ips (NBB_IPS *ips NBB_CCXT NBB_CXT);
ATG_CPI_ROW_UPDATE *
pdsa_ctm_bld_row_update_common (AMB_GEN_IPS    **mib,
                                 NBB_LONG        data_len,
                                 NBB_LONG        oid_len,
                                 NBB_LONG        row_status,
                                 NBB_ULONG       correlator);
NBB_VOID pdsa_ctm_send_row_update_common (pdsa_stub::pdsa_config_t  *conf, 
                                          pdsa_amb_fill_fnptr_t     fill_api);

#define PDSA_START_TXN(correlator) NBB_CREATE_THREAD_CONTEXT \
    NBB_TRC_ENTRY("PDSA_START_TXN"); \
    NBS_ENTER_SHARED_CONTEXT(sms_our_pid); \
    NBS_GET_SHARED_DATA(); \
    NBB_TRC_FLOW ((NBB_FORMAT "Start CTM Transaction")); \
    pdsa_ctm_send_transaction_start (correlator);

#define PDSA_END_TXN(correlator) pdsa_ctm_send_transaction_end (correlator); \
    NBS_RELEASE_SHARED_DATA(); \
    NBS_EXIT_SHARED_CONTEXT(); \
    NBB_TRC_EXIT(); \
    NBB_DESTROY_THREAD_CONTEXT
#endif /*__PDSA_CTM_HPP__*/
