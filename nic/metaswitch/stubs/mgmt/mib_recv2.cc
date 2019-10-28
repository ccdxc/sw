/**MOD+***********************************************************************/
/* Module:      smsrecv2.c                                                   */
/*                                                                           */
/* Purpose:     Additional procedures for handling received messages.        */
/*                                                                           */
/* (C) COPYRIGHT METASWITCH NETWORKS 2002 - 2016                             */
/*                                                                           */
/* $Revision::                      $ $Date::                             $  */
/*                                                                           */
/**MOD-***********************************************************************/

#define SHARED_DATA_TYPE SMS_SHARED_LOCAL

#include "nbase.h"
#include "nbbstub.h"
extern "C" {
#include "smsiincl.h"
#include "smsincl.h"
}

NBB_VOID sms_rcv_nbase_ips(NBB_IPS *ips NBB_CCXT_T NBB_CXT)
{
  /***************************************************************************/
  /* Local Variables                                                         */
  /***************************************************************************/

  NBB_TRC_ENTRY("sms_rcv_nbase_ips");

  switch (ips->ips_type)
  {
    case IPS_NBB_POSTED_BUFFER:
      /***********************************************************************/
      /* Received NBB_POSTED_BUFFER signal from the N-BASE, inform the       */
      /* buffer handler that a buffer has been posted.                       */
      /***********************************************************************/
      NBB_TRC_FLOW((NBB_FORMAT "Received NBB_POSTED_BUFFER"));
      NBB_TRC_FLOW((NBB_FORMAT "Call NTL_BUF_AVAILABLE."));

      /***********************************************************************/
      /* Release the lock on the SHARED data while calling the               */
      /* NTL_BUF_AVAILABLE function. This function implements callbacks to   */
      /* the MIB Stub.  The way the callback is made is not known so the     */
      /* lock must be released here and specifically obtained again in the   */
      /* callback functions.                                                 */
      /***********************************************************************/
      NBS_RELEASE_SHARED_DATA();
      NTL_BUF_AVAILABLE(ips);
      NBS_GET_SHARED_DATA();
      break;

    default:
      /***********************************************************************/
      /* Signal is not NBB_POSTED_BUFFER: unexpected.                        */
      /***********************************************************************/
      NBB_TRC_FLOW((NBB_FORMAT "Received unexpected signal from the N-BASE."));
      NBB_ASSERT_INVALID_BRANCH;
      break;
  }

  NBB_TRC_EXIT();

  return;

} /* sms_rcv_nbase_ips */

/**PROC+**********************************************************************/
/* Name:      sms_rcv_amb_test                                               */
/*                                                                           */
/* Purpose:   Process a received MIB TEST signal from SM.                    */
/*                                                                           */
/* Returns:   Nothing.                                                       */
/*                                                                           */
/* Params:    IN    ips           - Received IPS                             */
/*                                                                           */
/* Operation: If AMB_TEST succeeded                                          */
/*              Issue AMB_SET to perform required MIB operation.             */
/*            Else                                                           */
/*              Issue error message to user.                                 */
/*              Spin to "complete SMS operation".                            */
/*            Endif.                                                         */
/*                                                                           */
/**PROC-**********************************************************************/

NBB_VOID sms_rcv_amb_test(NBB_IPS *ips NBB_CCXT_T NBB_CXT)
{
  NBB_TRC_ENTRY("sms_rcv_amb_test");
  NBB_TRC_EXIT();
  return;
} /* sms_rcv_amb_test */

/**PROC+**********************************************************************/
/* Name:      sms_rcv_amb_trap                                               */
/*                                                                           */
/* Purpose:   Process a received MIB SET signal from SM.                     */
/*                                                                           */
/* Returns:   Nothing.                                                       */
/*                                                                           */
/* Params:    IN     v_amb_trap   - Received MIB trap                        */
/*                                                                           */
/* Operation: Print out information contained in trap.                       */
/*                                                                           */
/**PROC-**********************************************************************/

NBB_VOID sms_rcv_amb_trap(AMB_TRAP *v_amb_trap NBB_CCXT_T NBB_CXT)
{
  NBB_TRC_ENTRY("sms_rcv_amb_trap");
  NBB_TRC_EXIT();
  return;
} /* sms_rcv_amb_trap */

