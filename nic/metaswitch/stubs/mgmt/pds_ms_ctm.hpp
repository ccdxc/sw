// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// Purpose: CTM helper APIs header file to communicate with metaswitch 
// from mgmt stub 

#ifndef __PDS_MS_CTM_HPP__
#define __PDS_MS_CTM_HPP__

#include "nic/metaswitch/stubs/mgmt/pds_ms_config.hpp"
#include "include/sdk/ip.hpp"

typedef NBB_VOID (*pds_ms_amb_fill_fnptr_t)(AMB_GEN_IPS *, 
                                          pds_ms_stub::pds_ms_config_t *);

NBB_VOID pds_ms_ctm_send_transaction_start (NBB_ULONG correlator);
NBB_VOID pds_ms_ctm_send_transaction_abort (NBB_ULONG correlator);
NBB_VOID pds_ms_ctm_send_transaction_end (NBB_ULONG correlator);
NBB_VOID pds_ms_ctm_rcv_ips (NBB_IPS *ips NBB_CCXT NBB_CXT);
ATG_CPI_ROW_UPDATE *
pds_ms_ctm_bld_row_update_common (AMB_GEN_IPS    **mib,
                                 NBB_LONG        data_len,
                                 NBB_LONG        oid_len,
                                 NBB_LONG        row_status,
                                 NBB_ULONG       correlator);
NBB_VOID pds_ms_ctm_send_row_update_common (pds_ms_stub::pds_ms_config_t  *conf, 
                                          pds_ms_amb_fill_fnptr_t     fill_api);

#define PDS_MS_START_TXN(correlator) \
    NBB_CREATE_THREAD_CONTEXT \
    NBS_ENTER_SHARED_CONTEXT(sms_our_pid); \
    NBS_GET_SHARED_DATA(); \
    NBB_TRC_FLOW ((NBB_FORMAT "Start CTM Transaction")); \
    pds_ms_ctm_send_transaction_start (correlator);

#define PDS_MS_END_TXN(correlator) \
    pds_ms_ctm_send_transaction_end (correlator); \
    NBS_RELEASE_SHARED_DATA(); \
    NBS_EXIT_SHARED_CONTEXT(); \
    NBB_DESTROY_THREAD_CONTEXT

#define PDS_MS_GET_SHARED_START() \
    NBB_CREATE_THREAD_CONTEXT \
    NBS_ENTER_SHARED_CONTEXT(sms_our_pid); \
    NBS_GET_SHARED_DATA(); \
    NBB_TRC_FLOW ((NBB_FORMAT "Start PDS_MS_GET_SHARED_START"));

#define PDS_MS_GET_SHARED_END() \
    NBS_RELEASE_SHARED_DATA(); \
    NBS_EXIT_SHARED_CONTEXT(); \
    NBB_DESTROY_THREAD_CONTEXT
#endif /*__PDS_MS_CTM_HPP__*/
