/**MOD+***********************************************************************/
/* Module:      smsrecv.c                                                    */
/*                                                                           */
/* Purpose:     MIB Stub Receive procedure.  This procedure receives all     */
/*              N-BASE IPSs sent to us.                                      */
/*                                                                           */
/* (C) COPYRIGHT METASWITCH NETWORKS 2000 - 2015                             */
/*                                                                           */
/* $Revision::                      $ $Date::                             $  */
/*                                                                           */
/**MOD-***********************************************************************/

#define SHARED_DATA_TYPE SMS_SHARED_LOCAL

#include "nbase.h"
#include "nbbstub.h"
#include "nic/metaswitch/stubs/mgmt/pds_ms_mgmt_utils.hpp"
extern "C" {
#include "smsiincl.h"
#include "smsincl.h"
}

NBB_VOID sms_rcv_amb_ips(NBB_IPS *ips NBB_CCXT_T NBB_CXT);

/**PROC+**********************************************************************/
/* Name:      sms_receive_proc                                               */
/*                                                                           */
/* Purpose:   MIB Stub receive procedure.                                    */
/*                                                                           */
/* Returns:   Nothing.                                                       */
/*                                                                           */
/* Params:    IN     ips          - Received IPS.                            */
/*            IN     queue_id     - Queue on which IPS was received.         */
/*                                                                           */
/* Operation: Fan out the IPS by queue.                                      */
/*                                                                           */
/**PROC-**********************************************************************/
NBB_VOID sms_receive_proc(NBB_IPS *ips,
                          NBB_QUEUE_ID queue_id
                          NBB_CCXT_T NBB_CXT)
{
  /***************************************************************************/
  /* Local Variables                                                         */
  /***************************************************************************/

  NBB_TRC_ENTRY("sms_receive_proc");

  /***************************************************************************/
  /* Get the lock for the SHARED LOCAL data.                                 */
  /***************************************************************************/
  NBS_GET_SHARED_DATA();

  /***************************************************************************/
  /* Fan out the IPS by queue type.                                          */
  /***************************************************************************/
  switch (queue_id)
  {
    case AMB_MIB_Q:
      /***********************************************************************/
      /* IPS on the MIB Queue.                                               */
      /***********************************************************************/
      NBB_TRC_FLOW((NBB_FORMAT "AMB_MIB_Q"));
      sms_rcv_amb_ips(ips NBB_CCXT);
      break;

    case NBB_SEND_Q:
      /***********************************************************************/
      /* IPS from N-BASE                                                     */
      /***********************************************************************/
      NBB_TRC_FLOW((NBB_FORMAT "NBB_SEND_Q"));
      sms_rcv_nbase_ips(ips NBB_CCXT);
      break;

   case CPI_TO_USER_Q:
      /***********************************************************************/
      /* IPS from N-BASE                                                     */
      /***********************************************************************/
      NBB_TRC_FLOW((NBB_FORMAT "CPI_TO_USER_Q"));
      pds_ms_ctm_rcv_ips (ips NBB_CCXT);
      NBB_FREE_BUFFER(ips);
      break;

    default:
      /***********************************************************************/
      /* Invalid queue                                                       */
      /***********************************************************************/
      NBB_TRC_FLOW((NBB_FORMAT "Invalid queue %lx (IPS type %lx)",
                    (NBB_ULONG)queue_id,
                    ips->ips_type));
      NBB_FREE_BUFFER(ips);
      NBB_ASSERT_INVALID_BRANCH;
      break;
  }

  OS_FFLUSH(OS_STDOUT);

  /***************************************************************************/
  /* Release the lock on the SHARED LOCAL data.                              */
  /***************************************************************************/
  NBS_RELEASE_SHARED_DATA();

  NBB_TRC_EXIT();

  return;

} /* sms_receive_proc */

/**PROC+**********************************************************************/
/* Name:      sms_rcv_amb_ips                                                */
/*                                                                           */
/* Purpose:   Receive a MIB signal from System Manager.                      */
/*                                                                           */
/* Returns:   Nothing.                                                       */
/*                                                                           */
/* Params:    IN    ips           - Received IPS                             */
/*                                                                           */
/* Operation: Route MIB response to appropriate handler function.            */
/*                                                                           */
/**PROC-**********************************************************************/
NBB_VOID sms_rcv_amb_ips(NBB_IPS *ips NBB_CCXT_T NBB_CXT)
{
  NBB_TRC_ENTRY("sms_rcv_amb_ips");
  switch (ips->ips_type)
  {
    case IPS_AMB_GET:
      NBB_TRC_FLOW((NBB_FORMAT "Received AMB_GET"));
      if (!sms_rcv_amb_get(ips NBB_CCXT))
      {
        NBB_TRC_FLOW((NBB_FORMAT "Free the buffer"));
        if (SHARED.saved_ips == ips)
        {
          NBB_TRC_FLOW((NBB_FORMAT "Clear SHARED.saved_ips"));
          SHARED.saved_ips = NULL;
        }
        NBB_FREE_BUFFER(ips);
      }
      break;

    case IPS_AMB_CANCEL:
      NBB_TRC_FLOW((NBB_FORMAT "Received AMB_CANCEL"));
      NBB_FREE_BUFFER(ips);
      break;

    case IPS_AMB_TEST:
      NBB_TRC_FLOW((NBB_FORMAT "Received AMB_TEST"));
      break;

    case IPS_AMB_SET:
      NBB_TRC_FLOW((NBB_FORMAT "Received AMB_SET"));
      break;

    case IPS_AMB_TRAP:
      NBB_TRC_FLOW((NBB_FORMAT "Received AMB_TRAP"));
      sms_rcv_amb_trap((AMB_TRAP *)ips NBB_CCXT);
      NBB_FREE_BUFFER(ips);
      break;

    default:
      /***********************************************************************/
      /* Received unexpected signal from AMB Queue.                          */
      /***********************************************************************/
      NBB_TRC_FLOW((NBB_FORMAT "Received unexpected signal from AMB_Q."));
      NBB_ASSERT_INVALID_BRANCH;
      NBB_FREE_BUFFER(ips);
      break;
  }

  NBB_TRC_EXIT();

  return;

} /* sms_rcv_amb_ips */

/**PROC+**********************************************************************/
/* Name:      sms_rcv_amb_get                                                */
/*                                                                           */
/* Purpose:   Process a received MIB GET signal from SM.                     */
/*                                                                           */
/* Returns:   TRUE if the buffer has been freed or reused by this routine.   */
/*            FALSE if the caller is responsible for the buffer.             */
/*                                                                           */
/* Params:    IN    ips           - Received IPS                             */
/*                                                                           */
/* Operation: Update product status based on AMB_GET response.               */
/*            Issue AMB_GET processor function to determine what to do next. */
/*                                                                           */
/**PROC-**********************************************************************/
NBB_BOOL sms_rcv_amb_get(NBB_IPS *ips NBB_CCXT_T NBB_CXT)
{
  NBB_TRC_ENTRY("sms_rcv_amb_get");
  SHARED.saved_ips = ips;
  SHARED.wait_check_get(NBB_CXT);
  NBB_TRC_EXIT();
  return(TRUE);

} /* sms_rcv_amb_get */

/**PROC+**********************************************************************/
/* Name:      sms_delay_for_multiple_lsps                                    */
/*                                                                           */
/* Purpose:   Start timer to wait between last AMB_SET and next AMB_TEST for */
/*            LSP setup.                                                     */
/*                                                                           */
/* Returns:   Nothing.                                                       */
/*                                                                           */
/* Params:    None                                                           */
/*                                                                           */
/* Operation: Initializes and starts timer for LSP delay.                    */
/*                                                                           */
/**PROC-**********************************************************************/
NBB_VOID sms_delay_for_multiple_lsps(NBB_CXT_T NBB_CXT)
{
  NBB_TRC_ENTRY("sms_delay_for_multiple_lsps");

  NBB_TRC_EXIT();

  return;

} /* sms_delay_for_multiple_lsps */

/**PROC+**********************************************************************/
/* Name:      sms_lsp_delay                                                  */
/*                                                                           */
/* Purpose:   Send next AMB_TEST.                                            */
/*                                                                           */
/* Returns:   None.                                                          */
/*                                                                           */
/* Params:    IN     timer_data   - Ignored.                                 */
/*                                                                           */
/* Operation: Timer has popped send AMB_TEST for next LSP.                   */
/*                                                                           */
/**PROC-**********************************************************************/
NBB_VOID sms_lsp_delay(NBB_TIMER_DATA *timer_data  NBB_CCXT_T NBB_CXT)
{
  NBB_TRC_ENTRY("sms_lsp_delay");
  NBB_TRC_EXIT();
  return;

} /* sms_lsp_delay */

/**PROC+**********************************************************************/
/* Name:        sms_wait_get_try_again                                       */
/*                                                                           */
/* Purpose:     Start a timer to wait before trying a MIB GET request again. */
/*                                                                           */
/* Returns:     Nothing.                                                     */
/*                                                                           */
/* Params:      None.                                                        */
/*                                                                           */
/* Operation:   Initialize and start the timer.                              */
/*                                                                           */
/**PROC-**********************************************************************/
NBB_VOID sms_wait_get_try_again(NBB_CXT_T NBB_CXT)
{
  NBB_TRC_ENTRY("sms_wait_get_try_again");
  NBB_TRC_EXIT();
  return;

} /* sms_wait_get_try_again */

