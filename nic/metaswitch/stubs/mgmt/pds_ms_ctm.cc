// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// Purpose: CTM helper APIs to communicate with metaswitch from mgmt stub 


#include <iostream>
#include "nic/metaswitch/stubs/mgmt/pds_ms_ctm.hpp"
#include "nic/metaswitch/stubs/mgmt/pds_ms_mgmt_utils.hpp"
#include "nic/metaswitch/stubs/mgmt/pds_ms_mgmt_state.hpp"
#include "nic/metaswitch/stubs/hals/pds_ms_hal_init.hpp"
#include "nic/sdk/lib/thread/thread.hpp"

using namespace std;

#define SHARED_DATA_TYPE SMS_SHARED_LOCAL

NBB_VOID 
pds_ms_ctm_send_transaction_start (NBB_ULONG correlator)
{
    // Local variables
    ATG_CPI_TRANSACTION_START   *trans_start = NULL;

    trans_start = (ATG_CPI_TRANSACTION_START *) NBB_GET_BUFFER ( NBB_NULL_HANDLE,
                                                sizeof(ATG_CPI_TRANSACTION_START),
                                                0,
                                                NBB_NORETRY_ABORT);

    NBB_ASSERT_PTR_NE (trans_start, NULL);
    NBB_ZERO_IPS (trans_start);
    trans_start->ips_hdr.ips_type = IPS_ATG_CPI_TRANSACTION_START;
    trans_start->transaction_type = ATG_CPI_TRANSACTION_UPDATE;
    NBB_CORR_PUT_VALUE (trans_start->trans_correlator, correlator);

    // Send the IPS
    NBB_TRC_DETAIL ((NBB_FORMAT "Send Transaction Start"));
    NBB_SEND_IPS (SHARED.css_pid, USER_TO_CPI_Q, trans_start);
}

NBB_VOID 
pds_ms_ctm_send_transaction_abort (NBB_ULONG correlator)
{
    // Local variables
    ATG_CPI_TRANSACTION_ABORT   *trans_abort = NULL;

    trans_abort = (ATG_CPI_TRANSACTION_ABORT *) NBB_GET_BUFFER ( NBB_NULL_HANDLE,
                                                sizeof(ATG_CPI_TRANSACTION_ABORT),
                                                0,
                                                NBB_NORETRY_ABORT);
    NBB_ASSERT_PTR_NE (trans_abort, NULL);
    NBB_ZERO_IPS (trans_abort);
    trans_abort->ips_hdr.ips_type = IPS_ATG_CPI_TRANSACTION_ABORT;
    NBB_CORR_PUT_VALUE (trans_abort->trans_correlator, correlator);

    // Send the IPS
    NBB_TRC_DETAIL ((NBB_FORMAT "Send Transaction Abort"));
    NBB_SEND_IPS (SHARED.css_pid, USER_TO_CPI_Q, trans_abort);
}

NBB_VOID 
pds_ms_ctm_send_transaction_end (NBB_ULONG correlator)
{
    // Local variables
    ATG_CPI_TRANSACTION_END   *trans_end = NULL;

    trans_end = (ATG_CPI_TRANSACTION_END *) NBB_GET_BUFFER ( NBB_NULL_HANDLE,
                                            sizeof(ATG_CPI_TRANSACTION_END),
                                            0,
                                            NBB_NORETRY_ABORT);

    NBB_ASSERT_PTR_NE (trans_end, NULL);
    NBB_ZERO_IPS (trans_end);
    trans_end->ips_hdr.ips_type = IPS_ATG_CPI_TRANSACTION_END;
    NBB_CORR_PUT_VALUE (trans_end->trans_correlator, correlator);

    // Send the IPS
    NBB_TRC_DETAIL ((NBB_FORMAT "Send Transaction End"));
    NBB_SEND_IPS (SHARED.css_pid, USER_TO_CPI_Q, trans_end);
}

static NBB_VOID
pds_ms_ctm_rcv_transaction_done (ATG_CPI_TRANSACTION_DONE *trans_done)
{
    NBB_ASSERT_PTR_NE (trans_done, NULL);
    types::ApiStatus status;
    switch (trans_done->return_code)
    {
        case ATG_CPI_RC_OK:
            NBB_TRC_FLOW((NBB_FORMAT "ATG_CPI_RC_OK"));
            status = types::ApiStatus::API_STATUS_OK;
            break;

        case ATG_CPI_RC_INTERNAL_FAILURE:
            NBB_TRC_FLOW((NBB_FORMAT "ATG_CPI_RC_INTERNAL_FAILURE"));
            status = types::ApiStatus::API_STATUS_ERR;
            break;

        case ATG_CPI_RC_APPLY_CFG_FAILURE:
            NBB_TRC_FLOW((NBB_FORMAT "ATG_CPI_RC_APPLY_CFG_FAILURE"));
            if (trans_done->return_subcode == AMB_RC_RESOURCE_UNAVAILABLE)
            {
                NBB_TRC_FLOW((NBB_FORMAT "AMB_RC_RESOURCE_UNAVAILABLE"));
                status = types::ApiStatus::API_STATUS_OUT_OF_RESOURCE;
            }
            else if ((trans_done->return_subcode == AMB_RC_WRONG_VALUE) ||
                    (trans_done->return_subcode == AMB_RC_INCONSISTENT_VALUE))
            {
                NBB_TRC_FLOW((NBB_FORMAT "Wrong/inconsistent value"));
                status = types::ApiStatus::API_STATUS_INVALID_ARG;
            }
            else
            {
                NBB_TRC_FLOW((NBB_FORMAT "Config injection failure %d",
                              trans_done->return_subcode));
                status = types::ApiStatus::API_STATUS_OPERATION_NOT_ALLOWED;
            }
            break;

        case ATG_CPI_RC_INCONSISTENT_STATE:
            NBB_TRC_FLOW((NBB_FORMAT "ATG_CPI_RC_INCONSISTENT_STATE"));
            status = types::ApiStatus::API_STATUS_INVALID_ARG;
            break;

        default:
            NBB_TRC_FLOW((NBB_FORMAT "Unexpected return code %lu",
                        trans_done->return_code));
            NBB_ASSERT_INVALID_BRANCH;
            status = types::ApiStatus::API_STATUS_OPERATION_NOT_ALLOWED;
            break;
    }

    if (trans_done->trans_correlator.correlator1 == PDS_MS_CTM_GRPC_CORRELATOR) {
        // Unblock the GRPC thread which is waiting for the response
        pds_ms::mgmt_state_t::ms_response_ready(status);
    } else if (trans_done->trans_correlator.correlator1 ==
                                PDS_MS_CTM_STUB_INIT_CORRELATOR) {
        SDK_ABORT_TRACE((trans_done->return_code == ATG_CPI_RC_OK),
                                "Nbase stubs init txn failed! Aborting!");
        // Stubs initialization transaction is successful
        // We can now signal that the nbase thread is ready
        PDS_TRACE_INFO("Nbase thread is ready");
        cout << "N-Base thread is ready\n";
        auto ctx = pds_ms::mgmt_state_t::thread_context();
        ctx.state()->nbase_thread()->set_ready(true);
    }
    
    return;
}

static NBB_VOID 
pds_ms_ctm_fill_mib_msg_defaults (AMB_GEN_IPS   *mib_msg)
{
    //Fill in the mib_msg defaults
    mib_msg->ips_hdr.ips_type       = IPS_AMB_SET;
    mib_msg->ips_hdr.sender_handle  = NBB_NULL_HANDLE;
    mib_msg->ips_hdr.receiver_handle= NBB_NULL_HANDLE;
    mib_msg->is_linked              = ATG_NO;
    mib_msg->last_of_linked         = ATG_NO;
    mib_msg->linked_id              = 0;
    mib_msg->in_progress            = ATG_NO;
    mib_msg->create_delete_type     = AMB_CD_TYPE_NONE;
    mib_msg->fte_create             = ATG_NO;
    mib_msg->fte_delete             = ATG_NO;
    mib_msg->source_address_type    = AMB_ADDRESS_LOCAL;
    mib_msg->dest_address_type      = AMB_ADDRESS_LOCAL;
    mib_msg->exact                  = ATG_YES;
    mib_msg->get_next_object        = ATG_NO;
    mib_msg->first_cumulative_get   = ATG_NO;
    mib_msg->oid_incremented        = ATG_NO;
    mib_msg->error_index            = 0;
    mib_msg->reset_stats            = ATG_NO;
    mib_msg->reserved_handle        = NBB_NULL_HANDLE;
    mib_msg->reserved_pid           = NBB_NULL_PROC_ID;
    mib_msg->is_structured_msg      = ATG_YES;
    mib_msg->bulk_requested         = 0;
    mib_msg->bulk_returned          = 0;

    // Set the return PID.
    mib_msg->return_pid             = SHARED.our_pid;

}


ATG_CPI_ROW_UPDATE *
pds_ms_ctm_bld_row_update_common (AMB_GEN_IPS    **mib,
                                 NBB_LONG        data_len,
                                 NBB_LONG        oid_len,
                                 NBB_LONG        row_status,
                                 NBB_ULONG       correlator)
{
    // Local variables
    NBB_ULONG   oid_offset = 0, data_offset = 0, amb_set_size = 0;
    NBB_ULONG   cpi_buf_size = 0;
    NBB_BUF_SIZE cpi_offset_array[OFL_ATG_CPI_ROW_UPDATE + 1];
    AMB_GEN_IPS *mib_msg = NULL;
    ATG_CPI_ROW_UPDATE  *row_update = NULL;

    // Set up the size info and fill in the buffer
    oid_offset      = sizeof (AMB_GEN_IPS);
    data_offset     = oid_offset + (oid_len * sizeof (NBB_ULONG));
    amb_set_size    = data_offset + data_len + 1;

    cpi_offset_array[0] = sizeof(ATG_CPI_ROW_UPDATE);
    cpi_offset_array[1] = data_offset + data_len + 1;
    cpi_offset_array[2] = NTL_OFF_SIZE_ARRAY_TERMINATOR;
    cpi_buf_size        = NTL_OFF_CALC_STRUCT_SIZE(cpi_offset_array);


    row_update = (ATG_CPI_ROW_UPDATE *) NBB_GET_BUFFER ( NBB_NULL_HANDLE,
                                                         cpi_buf_size,
                                                         0,
                                                         NBB_NORETRY_ABORT);

    if (row_update == NULL)
    {
        NBB_TRC_FLOW((NBB_FORMAT "Failed to get buffer - exit"));
        goto EXIT_LABEL;
    }

    NBB_ASSERT_NUM_GE(row_update->ips_hdr.ctrl_size, cpi_buf_size);                                                        
    NBB_ZERO_IPS(row_update);
    NTL_OFF_INIT_STRUCT(row_update, cpi_offset_array, off_atg_cpi_row_update);
    row_update->ips_hdr.ips_type = IPS_ATG_CPI_ROW_UPDATE;
    NBB_CORR_PUT_VALUE(row_update->trans_correlator, correlator);

    // Get pointer to mib message
    mib_msg = (AMB_GEN_IPS *)NTL_OFF_GET_POINTER(row_update,
            &row_update->row_update);
    NBB_ASSERT_PTR_NE(mib_msg, NULL);

    // Fill in the defaults
    pds_ms_ctm_fill_mib_msg_defaults (mib_msg);

    // Set control portion of the buffer
    mib_msg->ips_hdr.ctrl_size      = amb_set_size;

    // Set the OID offset, data offset and data length
    mib_msg->oid_offset     = oid_offset;
    mib_msg->data_offset    = data_offset;
    mib_msg->data_len       = data_len;

    // Set MIB create_delete_type
    if (row_status == AMB_ROW_DESTROY)
    {
        NBB_TRC_FLOW((NBB_FORMAT "Row status destroy - type delete"));
        mib_msg->create_delete_type = AMB_CD_TYPE_DELETE;
    }
    else if ((row_status == AMB_ROW_CREATE_AND_GO) ||
            (row_status == AMB_ROW_ACTIVE) ||
            (row_status == AMB_ROW_CREATE_AND_WAIT))
    {
        NBB_TRC_FLOW((NBB_FORMAT "Row status create - type create"));
        mib_msg->create_delete_type = AMB_CD_TYPE_CREATE;
    }
    else
    {
        NBB_TRC_FLOW((NBB_FORMAT "Row status not create or destroy - type none"));
        mib_msg->create_delete_type = AMB_CD_TYPE_NONE;
    }

    // Fill in the correlator
    mib_msg->correlator.handle = correlator;

EXIT_LABEL:
    *mib = mib_msg;
    return row_update; 
}

NBB_VOID 
pds_ms_ctm_send_row_update_common (pds_ms::pds_ms_config_t   *conf,
                                 pds_ms_amb_fill_fnptr_t      fill_api)
{
    ATG_CPI_ROW_UPDATE  *row_update = NULL; 
    AMB_GEN_IPS         *mib_msg = NULL;

    NBB_ASSERT_PTR_NE (conf, NULL);

    // Build row update
    row_update = pds_ms_ctm_bld_row_update_common ( &mib_msg,
                                                  conf->data_len, 
                                                  conf->oid_len,
                                                  conf->row_status,
                                                  conf->correlator);
    NBB_ASSERT_PTR_NE (row_update, NULL);
    NBB_ASSERT_PTR_NE (mib_msg, NULL);

    fill_api (mib_msg, conf);

    // Send the Row Update request to CSS
    NBB_SEND_IPS (SHARED.css_pid, USER_TO_CPI_Q, row_update);
    return;
}

NBB_VOID
pds_ms_ctm_rcv_ips (NBB_IPS *ips NBB_CCXT NBB_CXT)
{
    NBB_ASSERT_NUM_EQ (ips->ips_type, IPS_ATG_CPI_TRANSACTION_DONE);

    NBB_TRC_FLOW ((NBB_FORMAT "Received IPS_ATG_CPI_TRANSACTION_DONE"));
    pds_ms_ctm_rcv_transaction_done ((ATG_CPI_TRANSACTION_DONE *)ips NBB_CCXT);
    
    return;
}

ms_txn_guard_t::ms_txn_guard_t(uint32_t pid, NBB_ULONG correlator) {
    if (nbb_thread_global_data == nullptr) {
        nbb_thread_global_data = nbb_alloc_tgd();
    }
    nbs_enter_shared_context(pid, &saved_context_ NBB_CCXT);
    NBS_GET_SHARED_DATA();
    NBB_TRC_FLOW ((NBB_FORMAT "Start CTM Transaction"));
    correlator_ = correlator;
    pds_ms_ctm_send_transaction_start (correlator_);
}

void ms_txn_guard_t::end_txn(void) {
    pds_ms_ctm_send_transaction_end (correlator_);
    correlator_ = 0;
    end_txn_ = true;
}

ms_txn_guard_t::~ms_txn_guard_t(void) {
    if (!end_txn_) {
        {
            // Release any pending UUID if transaction aborted
            // This is to avoid accidentally committing the pending UUID
            // in the ms_response_ready callback for the CTM abort below
            auto mgmt_ctxt = pds_ms::mgmt_state_t::thread_context();
            mgmt_ctxt.state()->release_pending_uuid();
        }
        pds_ms_ctm_send_transaction_abort (correlator_);
    }
    correlator_ = 0;
    NBS_RELEASE_SHARED_DATA();
    nbs_exit_shared_context(&saved_context_  NBB_CCXT);
    if (nbb_thread_global_data != nullptr) {
        nbb_free_tgd(NBB_CXT);
    }
    if (!end_txn_) {
        // Await MS response for transaction abort
        pds_ms::mgmt_state_t::ms_response_wait();
    }
}

ms_thr_ctxt_guard_t::ms_thr_ctxt_guard_t(uint32_t pid) {
    if (nbb_thread_global_data == nullptr) {
        nbb_thread_global_data = nbb_alloc_tgd();
    }
    nbs_enter_shared_context(pid, &saved_context_ NBB_CCXT);
    NBB_TRC_FLOW ((NBB_FORMAT "Start PDS_MS_GET_SHARED_START"));
    NBS_GET_SHARED_DATA();
}

ms_thr_ctxt_guard_t::~ms_thr_ctxt_guard_t() {
    NBS_RELEASE_SHARED_DATA();
    nbs_exit_shared_context(&saved_context_  NBB_CCXT);
    if (nbb_thread_global_data != nullptr) {
        nbb_free_tgd(NBB_CXT);
    }
}
