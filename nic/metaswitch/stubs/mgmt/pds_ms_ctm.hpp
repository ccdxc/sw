// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// Purpose: CTM helper APIs header file to communicate with metaswitch 
// from mgmt stub 

#ifndef __PDS_MS_CTM_HPP__
#define __PDS_MS_CTM_HPP__

#include "nic/metaswitch/stubs/mgmt/pds_ms_config.hpp"
#include "include/sdk/ip.hpp"

typedef NBB_VOID (*pds_ms_amb_fill_fnptr_t)(AMB_GEN_IPS *, 
                                          pds_ms::pds_ms_config_t *);

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
NBB_VOID pds_ms_ctm_send_row_update_common (pds_ms::pds_ms_config_t  *conf, 
                                          pds_ms_amb_fill_fnptr_t     fill_api);

class ms_txn_guard_t {
public:
    ms_txn_guard_t(uint32_t pid, NBB_ULONG correlator) {
        if (nbb_thread_global_data == nullptr) {
            nbb_thread_global_data = nbb_alloc_tgd();
        }
        nbs_enter_shared_context(pid, &saved_context NBB_CCXT);
        NBS_GET_SHARED_DATA();
        NBB_TRC_FLOW ((NBB_FORMAT "Start CTM Transaction"));
        pds_ms_ctm_send_transaction_start (correlator);
    }
    void end_txn(void) {end_txn_ = true;}
    ~ms_txn_guard_t() {
        if (end_txn_) {
            pds_ms_ctm_send_transaction_end (correlator);
        } else {
            pds_ms_ctm_send_transaction_abort (correlator);
        }
        NBS_RELEASE_SHARED_DATA();
        nbs_exit_shared_context(&saved_context  NBB_CCXT);
        if (nbb_thread_global_data != nullptr) {
            nbb_free_tgd(NBB_CXT);
        }
    }
private:
    NBB_SAVED_CONTEXT saved_context;
    NBB_ULONG   correlator;
    bool end_txn_ = false;
};

class ms_thr_ctxt_guard_t {
public:
    ms_thr_ctxt_guard_t(uint32_t pid) {
        if (nbb_thread_global_data == nullptr) {
            nbb_thread_global_data = nbb_alloc_tgd();
        }
        nbs_enter_shared_context(pid, &saved_context NBB_CCXT);
        NBB_TRC_FLOW ((NBB_FORMAT "Start PDS_MS_GET_SHARED_START"));
        NBS_GET_SHARED_DATA();
    }
    ~ms_thr_ctxt_guard_t() {
        NBS_RELEASE_SHARED_DATA();
        nbs_exit_shared_context(&saved_context  NBB_CCXT);
        if (nbb_thread_global_data != nullptr) {
            nbb_free_tgd(NBB_CXT);
        }
    }
private:
    NBB_SAVED_CONTEXT saved_context;
};

#define PDS_MS_START_TXN(correlator) \
    { \
       ms_txn_guard_t ms_txn_guard(sms_our_pid, correlator); \

#define PDS_MS_END_TXN(correlator) \
       ms_txn_guard.end_txn();  \
    }    

#define PDS_MS_GET_SHARED_START() \
    { \
       ms_thr_ctxt_guard_t ms_thr_ctxt_guard(sms_our_pid); \

#define PDS_MS_GET_SHARED_END() \
    }    

#endif /*__PDS_MS_CTM_HPP__*/
