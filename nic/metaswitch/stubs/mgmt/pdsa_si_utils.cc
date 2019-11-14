// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// Purpose: Helper APIs for metaswitch SI component

#include "nic/metaswitch/stubs/mgmt/pdsa_mgmt_utils.hpp"
#include "y0mib.h"
#include <iostream>

using namespace std;

// Fill siNode: AMB_YSS_SI_NODE
NBB_VOID
pdsa_fill_amb_si_node (AMB_GEN_IPS      *mib_msg,
                       pdsa_config_t    *conf)
{
    // Local variables
    NBB_ULONG        *oid = NULL; 
    AMB_YSS_SI_NODE  *amb_si_node = NULL;

    // Get oid and data offset 
    oid          = (NBB_ULONG *)((NBB_BYTE *)mib_msg + mib_msg->oid_offset);
    amb_si_node  = (AMB_YSS_SI_NODE *)((NBB_BYTE *)mib_msg + mib_msg->data_offset); 

    NBB_TRC_ENTRY ("pdsa_fill_amb_si_node");

    // Set all fields absent
    AMB_SET_ALL_FIELDS_NOT_PRESENT (mib_msg);

    // Set OID len and MIB family
    oid[0] = AMB_YSS_SI_NODE_OID_LEN;
    oid[1] = AMB_FAM_YSS_SI_NODE;

    // Set all incoming fields
    amb_si_node->node_index = conf->entity_index;
    oid[AMB_YSS_SI_NODE_NODE_INDEX_IDX] = conf->entity_index;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_YSS_SI_NODE_NODE_INDEX);

    amb_si_node->sm_css_replay = conf->sm_css_replay;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_YSS_SI_SM_CSS_REPLAY);

    amb_si_node->location_capable = conf->location_capable;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_YSS_SI_NODE_LOC_CAPABLE);

    amb_si_node->ft_capable = conf->ft_capable;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_YSS_SI_NODE_FT_CAPABLE);

    amb_si_node->row_status = conf->row_status;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_YSS_SI_NODE_FT_CAPABLE);

    NBB_TRC_EXIT();
    return;
}

NBB_VOID 
pdsa_test_row_update_si (pdsa_config_t *conf)
{
    // Local variables

    NBB_TRC_ENTRY ("pdsa_test_row_update_si");

    conf->oid_len            = AMB_YSS_SI_NODE_OID_LEN;
    conf->data_len           = sizeof (AMB_YSS_SI_NODE);
    conf->row_status         = AMB_ROW_ACTIVE;
    conf->entity_index       = AMB_YSS_SI_INDEX_NUM;
    conf->sm_css_replay      = AMB_FALSE;
    conf->location_capable   = AMB_FALSE;
    conf->ft_capable         = AMB_FALSE;

    pdsa_ctm_send_row_update_common (conf, pdsa_fill_amb_si_node);

    NBB_TRC_EXIT();
    return;
}
