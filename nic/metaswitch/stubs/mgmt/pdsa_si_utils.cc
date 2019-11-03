// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// Purpose: Helper APIs for metaswitch SI component

#include "pdsa_mgmt_utils.hpp"
#include "y0mib.h"


// Fill siNode: AMB_YSS_SI_NODE
NBB_VOID
pdsa_fill_amb_si_node (AMB_GEN_IPS      *mib_msg,
                       AMB_YSS_SI_NODE  *v_amb_si_node,
                       NBB_ULONG        node_index,
                       NBB_LONG         sm_css_replay,
                       NBB_LONG         location_capable,
                       NBB_LONG         ft_capable,
                       NBB_LONG         row_status)
{
    NBB_TRC_ENTRY ("pdsa_fill_amb_si_node");

    // Set all fields absent
    AMB_SET_ALL_FIELDS_NOT_PRESENT (mib_msg);

    // Set all incoming fields
    v_amb_si_node->node_index = node_index;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_YSS_SI_NODE_NODE_INDEX);

    v_amb_si_node->sm_css_replay = sm_css_replay;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_YSS_SI_SM_CSS_REPLAY);

    v_amb_si_node->location_capable = location_capable;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_YSS_SI_NODE_LOC_CAPABLE);

    v_amb_si_node->ft_capable = ft_capable;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_YSS_SI_NODE_FT_CAPABLE);

    v_amb_si_node->row_status = row_status;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_YSS_SI_NODE_FT_CAPABLE);

    NBB_TRC_EXIT();
    return;
}
