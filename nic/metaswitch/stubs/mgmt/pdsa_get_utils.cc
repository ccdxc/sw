// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// Purpose: Helper APIs header file to help fill-up GET header for metaswitch
// from mgmt stub


#include <iostream>
#include "nic/metaswitch/stubs/mgmt/pdsa_get_utils.hpp"
#include <iostream>

using namespace std;

#define SHARED_DATA_TYPE SMS_SHARED_LOCAL

NBB_VOID pdsa_amb_gen_common(AMB_GEN_IPS *amb_gen,
                             NBB_LONG row_status
                             NBB_CCXT_T NBB_CXT)
{
  /***************************************************************************/
  /* Local Variables                                                         */
  /***************************************************************************/

  NBB_TRC_ENTRY("pdsa_amb_gen_common");

  /***************************************************************************/
  /* Fill in common fields.                                                  */
  /***************************************************************************/
  amb_gen->ips_hdr.sender_handle = NBB_NULL_HANDLE;
  amb_gen->ips_hdr.receiver_handle = NBB_NULL_HANDLE;
  amb_gen->correlator.handle = 0x100;
  amb_gen->is_linked = ATG_NO;
  amb_gen->in_progress = ATG_NO;
  amb_gen->return_pid = SHARED.our_pid;
  amb_gen->ret_code = AMB_RC_INCOMPLETE;
  amb_gen->source_address_type = AMB_ADDRESS_LOCAL;
  NBB_MEMSET(amb_gen->source_address, 0, AMB_ADDRESS_LEN);
  amb_gen->dest_address_type = AMB_ADDRESS_LOCAL;
  NBB_MEMSET(amb_gen->dest_address, 0, AMB_ADDRESS_LEN);
  amb_gen->exact = ATG_YES;
  amb_gen->get_next_object = ATG_NO;
  amb_gen->reset_stats = ATG_NO;

  /***************************************************************************/
  /* If the row status is AMB_ROW_DESTROY, we need to also set the field     */
  /* create_delete_type to AMB_CD_TYPE_DELETE.                               */
  /***************************************************************************/
  if (row_status == AMB_ROW_DESTROY)
  {
    NBB_TRC_FLOW((NBB_FORMAT "Row status destroy - type delete"));
    amb_gen->create_delete_type = AMB_CD_TYPE_DELETE;
  }
  else if ((row_status == AMB_ROW_CREATE_AND_GO) ||
           (row_status == AMB_ROW_ACTIVE) ||
           (row_status == AMB_ROW_CREATE_AND_WAIT))
  {
    NBB_TRC_FLOW((NBB_FORMAT "Row status create - type create"));
    amb_gen->create_delete_type = AMB_CD_TYPE_CREATE;
  }
  else
  {
    NBB_TRC_FLOW((NBB_FORMAT "Row status not create or destroy - type none"));
    amb_gen->create_delete_type = AMB_CD_TYPE_NONE;
  }

  /***************************************************************************/
  /* The reserved_handle field must be set to NBB_NULL_HANDLE when using the */
  /* original MIB Manager.                                                   */
  /***************************************************************************/
  amb_gen->reserved_handle = NBB_NULL_HANDLE;

  amb_gen->registration_pid = NBB_NULL_PROC_ID;
  amb_gen->bulk_requested = 0;
  amb_gen->bulk_returned = 0;

  NBB_TRC_EXIT();

  return;

} /* pdsa_amb_gen_common */

AMB_GET *pdsa_amb_get_bulk_common(NBB_LONG bulk_size,
                                  NBB_LONG data_len,
                                  NBB_LONG oid_len,
                                  NBB_ULONG *oid
                                  NBB_CCXT_T NBB_CXT)
{
  /***************************************************************************/
  /* Local Variables                                                         */
  /***************************************************************************/
  NBB_USHORT ii;
  NBB_ULONG *get_oid;
  NBB_BUF_SIZE get_buf_size;
  AMB_GET *v_amb_get = NULL;

  NBB_TRC_ENTRY("pdsa_amb_get_bulk_common");

  /***************************************************************************/
  /* Get an internal buffer.                                                 */
  /***************************************************************************/
  get_buf_size = AMB_BULK_GET_CALC_SIZE(bulk_size, oid_len, data_len, 0);
  v_amb_get = (AMB_GET *)NBB_GET_BUFFER(NBB_NULL_HANDLE,
                                        get_buf_size,
                                        0,
                                        NBB_NORETRY_ABORT);
  NBB_ASSERT_PTR_NE(v_amb_get, NULL);

  /***************************************************************************/
  /* Clear out the body of the IPS.                                          */
  /***************************************************************************/
  NBB_ZERO_IPS(&(v_amb_get->ips_hdr));

  /***************************************************************************/
  /* Fill in all fields on the message.                                      */
  /***************************************************************************/
  v_amb_get->ips_hdr.ips_type = IPS_AMB_GET;
  pdsa_amb_gen_common((AMB_GEN_IPS *)v_amb_get, AMB_ROW_ACTIVE NBB_CCXT);
  AMB_BULK_GET_INIT_REQ(v_amb_get, bulk_size, oid_len, data_len, 0);

  /***************************************************************************/
  /* Copy the OID into the request.                                          */
  /***************************************************************************/
  get_oid = (NBB_ULONG *)((NBB_BYTE *)v_amb_get + v_amb_get->oid_offset);
  for (ii = 0; ii < oid_len; ii++)
  {
    /*************************************************************************/
    /* FLOW TRACING NOT REQUIRED    Reason: tight loop.                      */
    /*************************************************************************/
    get_oid[ii] = oid[ii];
  }

  NBB_TRC_EXIT();

  return(v_amb_get);

} /* pdsa_amb_get_bulk_common */

AMB_GET *pdsa_amb_get_common(NBB_LONG data_len,
                                NBB_LONG oid_len,
                                NBB_ULONG *oid
                                NBB_CCXT_T NBB_CXT)
{
  /***************************************************************************/
  /* Local Variables                                                         */
  /***************************************************************************/
  NBB_USHORT ii;
  NBB_ULONG *get_oid;
  NBB_BUF_SIZE ctrl_size = 0;
  AMB_GET *v_amb_get = NULL;

  NBB_TRC_ENTRY("pdsa_amb_get_common");

  /***************************************************************************/
  /* Get an internal buffer.                                                 */
  /***************************************************************************/
  ctrl_size = sizeof(AMB_GET) + data_len + (oid_len * sizeof(NBB_ULONG));
  v_amb_get = (AMB_GET *)NBB_GET_BUFFER(NBB_NULL_HANDLE,
                                        ctrl_size,
                                        0,
                                        NBB_NORETRY_ABORT);
  NBB_ASSERT_PTR_NE(v_amb_get, NULL);

  /***************************************************************************/
  /* Clear out the body of the IPS.                                          */
  /***************************************************************************/
  NBB_ZERO_IPS(&(v_amb_get->ips_hdr));

  /***************************************************************************/
  /* Fill in all fields on the message.                                      */
  /***************************************************************************/
  v_amb_get->ips_hdr.ips_type = IPS_AMB_GET;

  pdsa_amb_gen_common((AMB_GEN_IPS *)v_amb_get, AMB_ROW_ACTIVE NBB_CCXT);

  v_amb_get->oid_offset = sizeof(AMB_GET);
  v_amb_get->data_offset = sizeof(AMB_GET) + (oid_len * sizeof(NBB_ULONG));
  v_amb_get->data_len = data_len;

  get_oid = (NBB_ULONG *)((NBB_BYTE *)v_amb_get + v_amb_get->oid_offset);
  for (ii = 0; ii < oid_len; ii++)
  {
    /*************************************************************************/
    /* FLOW TRACING NOT REQUIRED    Reason: tight loop.                      */
    /*************************************************************************/
    get_oid[ii] = oid[ii];
  }

  NBB_TRC_EXIT();

  return(v_amb_get);

} /* pdsa_amb_get_common */
