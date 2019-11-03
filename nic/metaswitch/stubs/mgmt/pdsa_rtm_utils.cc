// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// Purpose: Helper APIs for metaswitch RTM component

#include "pdsa_mgmt_utils.hpp"
#include "qc0rtmib.h"


// Fill rtmEntityTable: AMB_CIPR_RTM_ENTITY 
NBB_VOID 
pdsa_fill_amb_cipr_rtm (AMB_GEN_IPS         *mib_msg,
                        AMB_CIPR_RTM_ENTITY *v_amb_rtm,
                        NBB_ULONG           fte_index,
                        NBB_LONG            row_status,
                        NBB_LONG            admin_status,
                        NBB_LONG            addr_family,
                        NBB_ULONG           i3_index)
{
  NBB_TRC_ENTRY ("pdsa_fill_amb_cipr_rtm");

  // Set all fields absentt
  AMB_SET_ALL_FIELDS_NOT_PRESENT (mib_msg);

  // Set all incoming fields
  v_amb_rtm->row_status = row_status;
  AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_QCR_ENT_ROW_STATUS);
  
  v_amb_rtm->admin_stat = admin_status;
  AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_QCR_ENT_ADMIN_STAT);

  if (row_status != AMB_ROW_DESTROY)
  {
    NBB_TRC_FLOW ((NBB_FORMAT "Not destroying DC-RTM: fill in fields"));
    v_amb_rtm->fte_index = fte_index;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_QCR_ENT_FTE_INDEX);

    v_amb_rtm->addr_family = addr_family;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_QCR_ENT_ADDR_FAM);

    v_amb_rtm->i3_index = i3_index;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_QCR_ENT_I3_INDEX);
  }

  NBB_TRC_EXIT();
  return;
} 


// Fill rtmMjTable: AMB_CIPR_RTM_MJ
NBB_VOID
pdsa_fill_amb_cipr_rtm_mj (AMB_GEN_IPS      *mib_msg,
                           AMB_CIPR_RTM_MJ  *v_amb_rtm,
                           NBB_ULONG        rtm_index,
                           NBB_ULONG        slave_entity_index,
                           NBB_ULONG        slave_type,
                           NBB_LONG         row_status,
                           NBB_LONG         admin_status)
{
    NBB_TRC_ENTRY ("sms_fill_amb_cipr_rtm_mj");

    // Set all fields absent
    AMB_SET_ALL_FIELDS_NOT_PRESENT (mib_msg);

    // Set all incoming fields
    v_amb_rtm->row_status = row_status;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_QRPM_MJ_ROW_STATUS);

    v_amb_rtm->admin_status = admin_status;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_QRPM_MJ_ADMIN_STATUS);

    if (row_status != AMB_ROW_DESTROY)
    {
        NBB_TRC_FLOW ((NBB_FORMAT "Not DC-RTM: fill in fields"));
        v_amb_rtm->rtm_fte_index = rtm_index;
        AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_QRPM_MJ_RTM_FTE_INDEX);

        v_amb_rtm->slave_fte_id = slave_entity_index;
        AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_QRPM_MJ_SLAVE_FTE_ID);

        v_amb_rtm->slave_type = slave_type;
        AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_QRPM_MJ_SLAVE_TYPE);
    }

    NBB_TRC_EXIT();
    return;

} 
