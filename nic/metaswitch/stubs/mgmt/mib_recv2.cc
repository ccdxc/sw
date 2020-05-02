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
#include "nic/metaswitch/stubs/common/pds_ms_ifindex.hpp"
#include "nic/metaswitch/stubs/mgmt/pds_ms_mgmt_state.hpp"
#include "nic/apollo/core/trace.hpp"
#include "nic/operd/alerts/alerts.hpp"
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
            return "Idle";
        case QBNM_CONN_CONNECT:
            return "Connect";
        case QBNM_CONN_ACTIVE:
            return "Active";
        case QBNM_CONN_OPEN_PEND:
            return "OpenPending";
        case QBNM_CONN_OPEN_SENT:
            return "OpenSent";
        case QBNM_CONN_OPEN_CONFIRM:
            return "OpenConfirm";
        case QBNM_CONN_ESTABLISHED:
            return "Established";
    }
    return "Invalid";
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
  ip_addr_t                 remote_ip;
  AMB_BGP_TRAP_DATA         *trap_data = NULL;
  AMB_LIM_IF_STATUS_CHANGE  *if_trap_data = NULL;
  uint32_t                   if_index = 0;
  bool                       rr_mode = false;

  NBB_TRC_ENTRY("sms_rcv_amb_trap");

  {
    auto mgmt_ctxt = pds_ms::mgmt_state_t::thread_context();
    rr_mode = mgmt_ctxt.state()->rr_mode();
  }
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
        remote_ip.addr.v4_addr = remote_ip.addr.v4_addr;
      }
      PDS_TRACE_INFO ("BGP session to %s is Established",
                       ipaddr2str(&remote_ip));
      if (!rr_mode) {
        // Raise an event
        operd::alerts::operd_alerts_t alert;
        alert = operd::alerts::BGP_SESSION_ESTABLISHED;
        operd::alerts::alert_recorder::get()->alert(alert, "Peer %s",
                                                      ipaddr2str(&remote_ip));
      }
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
              remote_ip.addr.v4_addr = remote_ip.addr.v4_addr;
          }
          PDS_TRACE_INFO ("BGP session to %s is Failed, State: %s",
                           ipaddr2str(&remote_ip),
                           ms_bgp_conn_fsm_state_str(trap_data->fsm_state));
          if (!rr_mode) {
            // Raise an event
            operd::alerts::operd_alerts_t alert;
            alert = operd::alerts::BGP_SESSION_DOWN;
            operd::alerts::alert_recorder::get()->alert(alert,"Peer %s, State %s",
                                                      ipaddr2str(&remote_ip), 
                            ms_bgp_conn_fsm_state_str(trap_data->fsm_state));
          }
      }
      break;

    case AMB_TRAP_LIM_IF_STATUS_CHANGE:
      /***********************************************************************/
      /* IF Oper Status Change                                               */
      /***********************************************************************/
      if_trap_data = (AMB_LIM_IF_STATUS_CHANGE*)((NBB_BYTE *)v_amb_trap +
                                                  v_amb_trap->data_offset);
      if (pds_ms::ms_ifindex_to_pds_type(if_trap_data->if_index) == IF_TYPE_L3)
      {
          // Interested only in L3 Interface status changes
          if_index = pds_ms::ms_to_pds_ifindex (if_trap_data->if_index);
          PDS_TRACE_INFO 
                 ("Interface %s [index: 0x%X] Oper Status changed to %s",
                 if_trap_data->name, if_index,
                 (if_trap_data->oper_status == AMB_IF_OPER_UP) ? "UP" : "DOWN");
      }
      break;

    case AMB_CSS_TRAP_STATUS:
      /***********************************************************************/
      /* CSS Status Trap, this is coming without any registration            */
      /***********************************************************************/
      NBB_TRC_FLOW((NBB_FORMAT "AMB_CSS_TRAP_STATUS"));
      break;

    default:
      /***********************************************************************/
      /* Unrecognized trap type.                                             */
      /***********************************************************************/
      PDS_TRACE_DEBUG ("Unrecognized trap type 0x%lx\n", v_amb_trap->trap_type);
  }

  NBB_TRC_EXIT();
  return;
} /* sms_rcv_amb_trap */
