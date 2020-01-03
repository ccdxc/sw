
/**MOD+***********************************************************************/
/* Module:      smsproc.c                                                    */
/*                                                                           */
/* Purpose:     MIB Stub create, destroy, receive and clean_up procedures.   */
/*              These constitute the N-BASE functional interface into the    */
/*              MIB stub.  These functions are registered with the N-BASE    */
/*              from smsinit.c.                                              */
/*                                                                           */
/* (C) COPYRIGHT METASWITCH NETWORKS 2000 - 2016                             */
/*                                                                           */
/* $Revision::                      $ $Date::                             $  */
/*                                                                           */
/**MOD-***********************************************************************/

#define SHARED_DATA_TYPE SMS_SHARED_LOCAL

#include "nbase.h"
#include "nic/sdk/lib/logger/logger.hpp"

/*****************************************************************************/
/* Global Variables.  Use #define to include definitions from header file    */
/* here.  Other files will pick up the extern defines.                       */
/*****************************************************************************/
#define SMS_DEFINE_VARS
extern "C" {
#include "smsiincl.h"
#include "smsincl.h"
}

/**PROC+**********************************************************************/
/* Name:      sms_create_proc                                                */
/*                                                                           */
/* Purpose:   MIB Stub Create Procedure                                      */
/*                                                                           */
/* Returns:   TRUE if created successfully, otherwise FALSE                  */
/*                                                                           */
/* Params:    IN     our_pid      - process ID                               */
/*            IN     create_parms_rcvd                                       */
/*                                - MIB Stub create parameters               */
/*                                                                           */
/* Operation: Stores create parameters, sets process priority and receive    */
/*            procedure, opens N-BASE and MIB queues.                        */
/*                                                                           */
/**PROC-**********************************************************************/

NBB_BOOL sms_create_proc(NBB_PROC_ID our_pid,
                         NBB_VOID *create_parms_rcvd
                         NBB_CCXT_T NBB_CXT)
{
  /***************************************************************************/
  /* Local Variables                                                         */
  /***************************************************************************/
  ASMB_CREATE_PARMS *create_parms = (ASMB_CREATE_PARMS *)create_parms_rcvd;
  NBB_BOOL ok = FALSE;

  NBB_TRC_ENTRY("sms_create_proc");

  /***************************************************************************/
  /* Get the lock for the SHARED LOCAL data.                                 */
  /***************************************************************************/
  NBS_GET_SHARED_DATA();

  /***************************************************************************/
  /* Assert create parameters are valid                                      */
  /***************************************************************************/
  NBB_ASSERT_NUM_GE(create_parms->length, sizeof(ASMB_CREATE_PARMS));
  NBB_ASSERT_NUM_NE(our_pid, NBB_NULL_PROC_ID);

  /***************************************************************************/
  /* Initialize Local data                                                   */
  /***************************************************************************/
  SHARED.our_pid = our_pid;
  sms_our_pid = our_pid;
  SHARED.sm_pid = create_parms->sm_pid;
  SHARED.css_pid = create_parms->cust_info.css_pid;

  SHARED.action = SMS_ACT_START;
  SHARED.wait_issue_get = NULL;
  SHARED.wait_check_get = NULL;
  SHARED.saved_ips_for_multi_tnnl = NULL;
  SHARED.saved_ips_for_get_try_again = NULL;
  SHARED.saved_ips = NULL;
  SHARED.wait_outstanding_gets = 0;
  SHARED.bgp_session_established = FALSE;
  SHARED.rtm_redist_index = 0;
  NBB_MEMSET(SHARED.remote_addr, 0, sizeof(SHARED.remote_addr));
  NBB_MEMSET(SHARED.local_addr, 0, sizeof(SHARED.local_addr));
  NBB_MEMSET(SHARED.evpn_ip_addr, 0, sizeof(SHARED.evpn_ip_addr));
  NBB_MEMSET(SHARED.evpn_mac_addr, 0, sizeof(SHARED.evpn_mac_addr));
  SHARED.i3_ei_flags = 0;
  SHARED.i3_ei_next_hop_addr_type = 0;
  SHARED.i3_ei_next_hop_addr_len = 0;
  NBB_MEMSET(SHARED.i3_ei_next_hop_addr,
             0,
             sizeof(SHARED.i3_ei_next_hop_addr));

  /***************************************************************************/
  /* Set N-BASE priority for the MIB stub.                                   */
  /***************************************************************************/
  NBB_SET_PRIORITY(SMS_NBASE_PRIORITY);

  /***************************************************************************/
  /* Set receive queue                                                       */
  /***************************************************************************/
  NBB_SET_RECEIVE(sms_receive_proc);

  /***************************************************************************/
  /* Initialize and enable queues.  Only two queues are required for the MIB */
  /* stub.                                                                   */
  /*                                                                         */
  /* - AMB_MIB_Q.  Receives all MIB IPSs from System Manager                 */
  /*                                                                         */
  /* - NBB_SEND_Q.  Receives N-BASE IPSs (including posted buffers)          */
  /***************************************************************************/
  NBB_OPEN_QUEUES(AMB_MIB_Q | NBB_SEND_Q | CPI_TO_USER_Q);
  NBB_ENABLE_QUEUES(AMB_MIB_Q | NBB_SEND_Q | CPI_TO_USER_Q);

  ok = TRUE;

  /***************************************************************************/
  /* Release the lock on the SHARED LOCAL data.                              */
  /***************************************************************************/
  NBS_RELEASE_SHARED_DATA();

  NBB_TRC_EXIT();

  return(ok);

} /* sms_create_proc */

/**PROC+**********************************************************************/
/* Name:      sms_destroy_proc                                               */
/*                                                                           */
/* Purpose:   MIB Stub destroy procedure.                                    */
/*                                                                           */
/* Returns:   Nothing.                                                       */
/*                                                                           */
/* Params:    None.                                                          */
/*                                                                           */
/* Operation: Frees all resources owned by the MIB Stub.  Note that this     */
/*            must complete synchronously.                                   */
/*                                                                           */
/**PROC-**********************************************************************/

NBB_VOID sms_destroy_proc(NBB_CXT_T NBB_CXT)
{
  /***************************************************************************/
  /* Local Variables                                                         */
  /***************************************************************************/

  NBB_TRC_ENTRY("sms_destroy_proc");

  /***************************************************************************/
  /* Get the lock for the SHARED LOCAL data.                                 */
  /***************************************************************************/
  NBS_GET_SHARED_DATA();

  /***************************************************************************/
  /* Release the lock on the SHARED LOCAL data.                              */
  /***************************************************************************/
  NBS_RELEASE_SHARED_DATA();

  NBB_TRC_EXIT();

  return;

} /* sms_destroy_proc */

/**PROC+**********************************************************************/
/* Name:        sms_verify_proc                                              */
/*                                                                           */
/* Purpose:     MIB Stub verify procedure.                                   */
/*                                                                           */
/* Returns:     Nothing.                                                     */
/*                                                                           */
/* Params:      None.                                                        */
/*                                                                           */
/* Operation:   The verify proc is only called when the debug N-BASE         */
/*              scheduler is running (i.e.  an N-BASE compiled with the      */
/*              NBB_DEBUG macro set on.  This function is called after each  */
/*              scheduling loop.  It should verify any memory that has been  */
/*              allocated within the MIB stub using the NBB_VERIFY functions */
/*              defined in \code\nbase\base\nbbxinfc.h.                      */
/*                                                                           */
/*              The debug N-BASE then checks whether this is memory that has */
/*              already been freed, whether this memory has been verified    */
/*              twice (indicating two components have a pointer to this      */
/*              memory), whether this memory was allocated from the buffer   */
/*              or memory manager and not verified (indicating that the      */
/*              memory has been lost.  The Metaswitch development teams use  */
/*              this function from day one of testing.                       */
/*                                                                           */
/*              This function does not need to implemented.  If a            */
/*              verify_proc is not specified in smsinit.c then the verify    */
/*              function is not called and the N-BASE will not fail          */
/*              validation for resource owned by the MIB stub.  Look at the  */
/*              equivalent functions in DC-RSVP and DC-LMGR for more         */
/*              examples of how validation is done.                          */
/*                                                                           */
/**PROC-**********************************************************************/

NBB_VOID sms_verify_proc(NBB_CXT_T NBB_CXT)
{
  /***************************************************************************/
  /* Local Variables                                                         */
  /***************************************************************************/

  NBB_TRC_ENTRY("sms_verify_proc");

  /***************************************************************************/
  /* Get the lock for the SHARED LOCAL data.                                 */
  /***************************************************************************/
  NBS_GET_SHARED_DATA();

  if (SHARED.saved_ips_for_multi_tnnl != NULL)
  {
    NBB_TRC_FLOW((NBB_FORMAT "Verify buffer for stored AMB_TEST/SET"));
    NBB_VERIFY_BUFFER((NBB_IPS *)SHARED.saved_ips_for_multi_tnnl);
  }

  /***************************************************************************/
  /* In the delivered SMS the SHARED.saved_ips field is only used within a   */
  /* single dispatch.  Therefore assert that it is always NULL and not       */
  /* pointing to anything that might be freed memory or an IPS that has been */
  /* sent to SM.                                                             */
  /*                                                                         */
  /* CUSTOMIZE ME.                                                           */
  /* If a customer wishes to use this field to share MIB replies between     */
  /* their SMS N-BASE process and their SMS initiator, they will need to     */
  /* replace the assert below with the following.                            */
  /*                                                                         */
  /* if (SHARED.saved_ips != NULL)                                           */
  /* {                                                                       */
  /*   NBB_TRC_FLOW((NBB_FORMAT "Verify buffer for saved IPS"));             */
  /*   NBB_VERIFY_BUFFER((NBB_IPS *)SHARED.saved_ips);                       */
  /* }                                                                       */
  /*                                                                         */
  /***************************************************************************/
  NBB_ASSERT_PTR_EQ(SHARED.saved_ips, NULL);

  /***************************************************************************/
  /* If there is a stored buffer because a GET request needs to be retried,  */
  /* the stored buffer needs verifying and the timer for retrying the GET    */
  /* request must be running.                                                */
  /***************************************************************************/
  if (SHARED.saved_ips_for_get_try_again != NULL)
  {
    NBB_TRC_FLOW((NBB_FORMAT "Verify buffer for stored AMB_GET"));
    NBB_VERIFY_BUFFER((NBB_IPS *)SHARED.saved_ips_for_get_try_again);
    NBB_ASSERT_TRUE(SHARED.activate_timer.pending);
  }

  /***************************************************************************/
  /* Release the lock on the SHARED LOCAL data.                              */
  /***************************************************************************/
  NBS_RELEASE_SHARED_DATA();

  NBB_TRC_EXIT();

  return;

} /* sms_verify_proc */

/**PROC+**********************************************************************/
/* Name:      sms_cleanup_proc                                               */
/*                                                                           */
/* Purpose:   MIB Stub Cleanup procedure.                                    */
/*                                                                           */
/* Returns:   Nothing.                                                       */
/*                                                                           */
/* Params:    None.                                                          */
/*                                                                           */
/* Operation: This function (if registered) is called before the stub is     */
/*            destroyed.  It allows the stub to do any asynchronous cleanup  */
/*            (cleanup from within the destroy proc must be synchronous).    */
/*            When asynchronous cleanup is complete the                      */
/*            NBS_CLEANUP_COMPLETE() call is made, and the stub should see a */
/*            call to it's destroy proc either immediately or some time      */
/*            later.                                                         */
/*                                                                           */
/**PROC-**********************************************************************/

NBB_VOID sms_cleanup_proc(NBB_CXT_T NBB_CXT)
{
  /***************************************************************************/
  /* Local Variables                                                         */
  /***************************************************************************/

  NBB_TRC_ENTRY("sms_cleanup_proc");

  /***************************************************************************/
  /* Get the lock on the SHARED LOCAL data.                                  */
  /***************************************************************************/
  NBS_GET_SHARED_DATA();

  NBS_CLEANUP_COMPLETE(SHARED.our_pid);

  /***************************************************************************/
  /* Release the lock on the SHARED LOCAL data.                              */
  /***************************************************************************/
  NBS_RELEASE_SHARED_DATA();

  NBB_TRC_EXIT();

  return;

} /* sms_cleanup_proc */



/**PROC+**********************************************************************/
/* Name:        pds_ms_register_mib_notify_sink                              */
/*                                                                           */
/* Purpose:     Send a message to register the Mgmt stub as a sink for MIB   */
/*              notifications.                                               */
/*                                                                           */
/* Returns:     TRUE if successful, FALSE if error.                          */
/*                                                                           */
/* Params:      None.                                                        */
/*                                                                           */
/**PROC-**********************************************************************/

NBB_BOOL pds_ms_register_mib_notify_sink(NBB_CXT_T NBB_CXT)
{
  /***************************************************************************/
  /* Local Variables                                                         */
  /***************************************************************************/
  AMB_REG_MIB_NOTIFY_SINK *reg_mib_notify_sink;
  NBB_BOOL rc = FALSE;

  /***************************************************************************/
  /* Get the lock for the SHARED LOCAL data.                                 */
  /***************************************************************************/
  NBS_ENTER_SHARED_CONTEXT(sms_our_pid);
  NBS_GET_SHARED_DATA();

  NBB_TRC_ENTRY("pds_ms_register_mib_notify_sink");

  /***************************************************************************/
  /* Allocate a buffer to use for registering the JSON stub as a sink for    */
  /* MIB notifications.                                                      */
  /***************************************************************************/
  reg_mib_notify_sink = (AMB_REG_MIB_NOTIFY_SINK *)
                        NBB_GET_BUFFER(NBB_NULL_PROC_ID,
                        sizeof(AMB_REG_MIB_NOTIFY_SINK),
                        0,
                        NBB_NORETRY_ACT);
  if (reg_mib_notify_sink == NULL)
  {
    /*************************************************************************/
    /* Failed to allocate buffer - exit.                                     */
    /*************************************************************************/
    SDK_TRACE_ERR ("Failed to allocate buffer\n");
    goto EXIT_LABEL;
  }

  /***************************************************************************/
  /* Fill in the buffer.                                                     */
  /*                                                                         */
  /* The master_mib_pid must be set to NULL because we are only interested   */
  /* in receiving MIB traps at the JSON stub.                                */
  /***************************************************************************/
  NBB_ZERO_IPS(reg_mib_notify_sink);
  reg_mib_notify_sink->ips_hdr.ips_type = IPS_AMB_REG_MIB_NOTIFY_SINK;
  reg_mib_notify_sink->ips_hdr.sender_handle = NBB_NULL_HANDLE;
  reg_mib_notify_sink->ips_hdr.receiver_handle = NBB_NULL_HANDLE;
  reg_mib_notify_sink->ips_hdr.sender_fti_main_pid = sms_our_pid;
  reg_mib_notify_sink->master_mib_pid = NBB_NULL_PROC_ID;
  reg_mib_notify_sink->mib_trap_pid = sms_our_pid;
  reg_mib_notify_sink->return_pid = sms_our_pid;
  reg_mib_notify_sink->return_qid = AMB_MIB_Q;
  reg_mib_notify_sink->return_code = ATG_OK;

  /***************************************************************************/
  /* Send the message.                                                       */
  /***************************************************************************/
  NBB_SEND_IPS(SHARED.sm_pid,
               AMB_MIB_Q,
               reg_mib_notify_sink);
  reg_mib_notify_sink = NULL;

  /***************************************************************************/
  /* If we have got this far return success.                                 */
  /***************************************************************************/
  rc = TRUE;

EXIT_LABEL:
  NBB_TRC_DETAIL((NBB_FORMAT "Returning %B", rc));
  NBB_TRC_EXIT();

  /***************************************************************************/ 
  /* Release the lock on the SHARED LOCAL data.                              */
  /***************************************************************************/
  NBS_RELEASE_SHARED_DATA();
  NBS_EXIT_SHARED_CONTEXT();

  return(rc);

} /* amj_register_mib_notify_sink */
