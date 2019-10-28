
/**MOD+***********************************************************************/
/* Module:    pegasus_mib_init.cc                                                      */
/*                                                                           */
/* Purpose:   MIB Stub initialization code.                                  */
/*                                                                           */
/* $Revision::                      $ $Date::                             $  */
/*                                                                           */
/**MOD-***********************************************************************/

#include "nbase.h"
#include "nbbstub.h"
extern "C" {
#include "smsiincl.h"
#include "smsincl.h"
}

NBB_BOOL sms_initialize(NBB_CXT_T);

/*****************************************************************************/
/* N-BASE registration information for the MIB Stub.                         */
/*****************************************************************************/
NBB_PROCESS_TYPE sms_proc_type =
{
  sms_create_proc,
  sms_destroy_proc,
  sms_verify_proc,
  0,
  0,
  NULL,
  0
};

/*****************************************************************************/
/* N-BASE stub registration information for the MIB stub.                    */
/*****************************************************************************/
NBS_STUB_PROCESS_TYPE sms_stub_proc_type =
{
  sms_cleanup_proc,
  sizeof(SMS_SHARED_LOCAL),
  MEM_SMS_SHARED_LOCAL
};

/**PROC+**********************************************************************/
/* Name:      sms_initialize                                                 */
/*                                                                           */
/* Purpose:   Initialization Procedure for the Sample MIB stub.              */
/*                                                                           */
/* Returns:   TRUE if successful                                             */
/*                                                                           */
/* Params:    None                                                           */
/*                                                                           */
/**PROC-**********************************************************************/

NBB_BOOL sms_initialize(NBB_CXT_T NBB_CXT)
{
  /***************************************************************************/
  /* Local Variables                                                         */
  /***************************************************************************/
  NBB_BOOL ok = FALSE;

  /***************************************************************************/
  /* NBB_TRC_ENTRY("sms_initialize"); Tracing commented out because this     */
  /* code may execute before the N-BASE has been initialized - specifically  */
  /* in the WIN32 case.                                                      */
  /***************************************************************************/
  NBB_REFERENCE_CONTEXT;

  if (NBS_REGISTER(PCT_YAMB, &sms_proc_type))
  {
    /*************************************************************************/
    /* FLOW TRACING NOT REQUIRED Reason: Outside N-BASE                      */
    /*************************************************************************/
    ok = NBS_STUB_REGISTER(PCT_YAMB, &sms_stub_proc_type);
  }

  /***************************************************************************/
  /* NBB_TRC_EXIT();                                                         */
  /***************************************************************************/

  return(ok);

} /* sms_initialize */


