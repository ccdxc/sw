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
    ms_txn_guard_t(uint32_t pid, NBB_ULONG correlator);
    void end_txn(void);
    ~ms_txn_guard_t();
private:
    NBB_SAVED_CONTEXT saved_context_;
    NBB_ULONG   correlator_ = 0;
    bool end_txn_ = false;
};

class ms_thr_ctxt_guard_t {
public:
    ms_thr_ctxt_guard_t(uint32_t pid);
    ~ms_thr_ctxt_guard_t();
private:
    NBB_SAVED_CONTEXT saved_context_;
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
