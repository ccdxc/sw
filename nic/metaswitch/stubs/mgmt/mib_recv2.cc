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

#include "nic/metaswitch/stubs/mgmt/pds_ms_mgmt_utils.hpp"
#include "nbase.h"
#include "nbbstub.h"
#include "qbnmdef.h"
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

static const char *
ms_bgp_conn_fsm_state_str (NBB_ULONG fsm_state)
{
    switch (fsm_state)
    {
        case QBNM_CONN_IDLE:
            return "conn_idle";
        case QBNM_CONN_CONNECT:
            return "conn_connect";
        case QBNM_CONN_ACTIVE:
            return "conn_active";
        case QBNM_CONN_OPEN_PEND:
            return "conn_open_pending";
        case QBNM_CONN_OPEN_SENT:
            return "conn_open_sent";
        case QBNM_CONN_OPEN_CONFIRM:
            return "conn_open_confirm";
        case QBNM_CONN_ESTABLISHED:
            return "conn_established";
    }
    return "invalid_state";
}

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
  /***************************************************************************/
  /* Local Variables                                                         */
  /***************************************************************************/
  ip_addr_t remote_ip;
  AMB_BGP_TRAP_DATA *trap_data;

  NBB_TRC_ENTRY("sms_rcv_amb_trap");

  switch (v_amb_trap->trap_type)
  {
    case AMB_BGP_TRAP_ESTABLISHED:
      /***********************************************************************/
      /* BGP session established trap.                                       */
      /***********************************************************************/
      NBB_TRC_FLOW((NBB_FORMAT "AMB_BGP_TRAP_ESTABLISHED"));
      trap_data = (AMB_BGP_TRAP_DATA *)((NBB_BYTE *)v_amb_trap +
                                                    v_amb_trap->data_offset);
      pds_ms_convert_amb_ip_addr_to_ip_addr (trap_data->remote_addr,
                                             trap_data->remote_addr_type,
                                             trap_data->remote_addr_len,
                                             &remote_ip);
      // ipaddr2str expects v4 addr in host order
      if (remote_ip.af == IP_AF_IPV4) {
        remote_ip.addr.v4_addr = ntohl (remote_ip.addr.v4_addr);
      }
      SDK_TRACE_INFO ("BGP session to %s is Established\n", ipaddr2str(&remote_ip));
      break;

    case AMB_BGP_TRAP_BACKWARD:
      /***********************************************************************/
      /* BGP session failed trap.                                            */
      /***********************************************************************/

      trap_data = (AMB_BGP_TRAP_DATA *)((NBB_BYTE *)v_amb_trap +
                                                    v_amb_trap->data_offset);
      // Log message only if the status is changing from established to Backward 
      if (trap_data->old_fsm_state == QBNM_CONN_ESTABLISHED) 
      {
          NBB_TRC_FLOW((NBB_FORMAT "AMB_BGP_TRAP_BACKWARD"));
          pds_ms_convert_amb_ip_addr_to_ip_addr (trap_data->remote_addr,
                                                 trap_data->remote_addr_type,
                                                 trap_data->remote_addr_len,
                                                 &remote_ip);
          // ipaddr2str expects v4 addr in host order
          if (remote_ip.af == IP_AF_IPV4) {
              remote_ip.addr.v4_addr = ntohl (remote_ip.addr.v4_addr);
          }
          SDK_TRACE_INFO ("BGP session to %s is Failed, State: %s\n",
                           ipaddr2str(&remote_ip),
                           ms_bgp_conn_fsm_state_str(trap_data->fsm_state));
      }
      break;

    default:
      /***********************************************************************/
      /* Unrecognized trap type.                                             */
      /***********************************************************************/
      NBB_TRC_FLOW((NBB_FORMAT "Unrecognized trap type %lx",
                                                       v_amb_trap->trap_type));
      break;
  }

  NBB_TRC_EXIT();
  return;
} /* sms_rcv_amb_trap */
