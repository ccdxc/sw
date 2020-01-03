// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// Purpose: Helper APIs for metaswitch LIM stub programming 

#include "nic/metaswitch/stubs/mgmt/pds_ms_mgmt_utils.hpp"
#include "evpn_prod.h"
#include "nic/metaswitch/stubs/common/pds_ms_ifindex.hpp"
#include "lim_mgmt_if.h"

namespace pds {
NBB_VOID
lim_intf_addr_fill_func (CPInterfaceAddrSpec&   req,
                         AMB_GEN_IPS            *mib_msg,
                         AMB_LIM_L3_IF_ADDR     *data,
                         NBB_LONG               row_status)
{
    // Local variables
    NBB_ULONG if_type   = req.iftype();
    NBB_ULONG if_id     = req.ifid();
    NBB_ULONG if_index  = 0;
    NBB_ULONG *oid      = (NBB_ULONG *)((NBB_BYTE *)mib_msg + mib_msg->oid_offset);

    data->entity_index                = PDS_MS_LIM_ENT_INDEX;
    oid[AMB_LIM_L3_ADDR_ENT_IX_INDEX] = PDS_MS_LIM_ENT_INDEX;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_LIM_L3_ADDR_ENT_IX);

    // Convert Spec Interface ID to MS interface Index
    switch (if_type)
    {
        case CP_IF_TYPE_IRB:
            if_index = pds_ms::bd_id_to_ms_ifindex (if_id);
            break;

        case CP_IF_TYPE_ETH:
        case CP_IF_TYPE_LIF:
            if_index = pds_ms::pds_to_ms_ifindex (if_id, if_type);
            break;

        case CP_IF_TYPE_LOOPBACK:
            if_index = pds_ms::loopback_to_ms_ifindex (if_id);
            break;

        default:
           SDK_TRACE_ERR ("Invalid Interface Address Req %d\n", if_type); 
    }
    data->if_index                   = if_index;
    oid[AMB_LIM_L3_ADDR_IF_IX_INDEX] = if_index;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_LIM_L3_ADDR_IF_IX);
}

NBB_VOID
lim_sw_intf_fill_func (CPInterfaceSpec&    req,
                       AMB_GEN_IPS         *mib_msg,
                       AMB_LIM_SOFTWARE_IF *data,
                       NBB_LONG            row_status)
{
    // Local variables
    NBB_ULONG if_type       = req.iftype();
    NBB_ULONG ms_if_subtype = 0;
    NBB_ULONG *oid = (NBB_ULONG *)((NBB_BYTE *)mib_msg + mib_msg->oid_offset);

    // Set Entity Index
    data->entity_index                = PDS_MS_LIM_ENT_INDEX;
    oid[AMB_LIM_SOFTWIF_ENT_IX_INDEX] = PDS_MS_LIM_ENT_INDEX;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_LIM_SOFTWIF_ENT_IX);

    // Set SW interface type
    if (if_type == CP_IF_TYPE_LIF) {
        ms_if_subtype = AMB_LIM_SOFTWIF_DUMMY;
    } else if (if_type == CP_IF_TYPE_LOOPBACK) {
        ms_if_subtype = AMB_LIM_SOFTWIF_LOOPBACK;
    } else {
        SDK_TRACE_ERR ("Invalid Interface Creation Req %d\n", if_type);
    }
    data->type                   = ms_if_subtype;
    oid[AMB_LIM_SOFTWIF_IF_TYPE] = ms_if_subtype;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_LIM_SOFTWIF_TYPE);
}
} // end of namespace pds

namespace pds_ms {

// Fill limEntTable: AMB_LIM_ENT
NBB_VOID
pds_ms_fill_amb_lim_ent (AMB_GEN_IPS *mib_msg, pds_ms_config_t *conf)
{ 
    // Local variables
    NBB_ULONG   *oid = NULL; 
    AMB_LIM_ENT *data= NULL;

    NBB_TRC_ENTRY ("pds_ms_fill_amb_lim_ent");

    // Get oid and data offset 
    oid     = (NBB_ULONG *)((NBB_BYTE *)mib_msg + mib_msg->oid_offset);
    data    = (AMB_LIM_ENT *)((NBB_BYTE *)mib_msg + mib_msg->data_offset); 

    // Set all fields absent
    AMB_SET_ALL_FIELDS_NOT_PRESENT (mib_msg);

    // Set OID len and family
    oid[0] = AMB_LIM_ENT_OID_LEN;
    oid[1] = AMB_FAM_LIM_ENT;

    // Set all incoming fields
    data->entity_index              = conf->entity_index;
    oid[AMB_LIM_ENT_ENT_IX_INDEX]   = conf->entity_index;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_LIM_ENT_ENTITY_INDEX);

    data->row_status = conf->row_status;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_LIM_ENT_ROW_STATUS);

    if (data->row_status != AMB_ROW_DESTROY) {
        // Enable IF status change traps by default
        data->if_status_traps = AMB_TRUE;
        AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_LIM_ENT_IF_STATUS_TRAPS);
    }

    NBB_TRC_EXIT ();
    return;
}

// Fill limMjTable: AMB_LIM_MJ
NBB_VOID
pds_ms_fill_amb_lim_mj (AMB_GEN_IPS *mib_msg, pds_ms_config_t *conf)
{ 
    // Local variables
    NBB_ULONG   *oid = NULL; 
    AMB_LIM_MJ  *data= NULL;

    NBB_TRC_ENTRY ("pds_ms_fill_amb_lim_mj");

    // Get oid and data offset 
    oid     = (NBB_ULONG *)((NBB_BYTE *)mib_msg + mib_msg->oid_offset);
    data    = (AMB_LIM_MJ *)((NBB_BYTE *)mib_msg + mib_msg->data_offset); 

    // Set all fields absent
    AMB_SET_ALL_FIELDS_NOT_PRESENT (mib_msg);

    // Set OID len and family
    oid[0] = AMB_LIM_MJ_OID_LEN;
    oid[1] = AMB_FAM_LIM_MJ;

    // Set all incoming fields
    data->lim_entity_index              = conf->entity_index;
    oid[AMB_LIM_MJ_APPL_INDEX_INDEX]    = conf->entity_index;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_LIM_MJ_ENTITY_INDEX);

    data->interface_id                  = conf->interface_id;
    oid[AMB_LIM_MJ_IF_TYPE_INDEX]       = conf->interface_id;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_LIM_MJ_INTERFACE);

    data->partner_type                  = conf->partner_type;
    oid[AMB_LIM_MJ_PARTNER_TYPE_INDEX]  = conf->partner_type;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_LIM_MJ_PARTNER_TYPE);

    data->partner_index                 = conf->partner_index;
    oid[AMB_LIM_MJ_PARTNER_INDEX_INDEX] = conf->partner_index;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_LIM_MJ_PARTNER_INDEX);

    data->sub_index                     = conf->sub_index;
    oid[AMB_LIM_MJ_SUB_INDEX_INDEX]     = conf->sub_index;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_LIM_MJ_SUB_INDEX);

    data->row_status = conf->row_status;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_LIM_MJ_ROW_STATUS);

    NBB_TRC_EXIT ();
    return;
}

NBB_VOID
pds_ms_row_update_lim (pds_ms_config_t *conf)
{
    NBB_TRC_ENTRY ("pds_ms_row_update_lim");

    // Set params
    conf->oid_len       = AMB_LIM_ENT_OID_LEN;
    conf->data_len      = sizeof (AMB_LIM_ENT);

    // Convert to row_update and send
    pds_ms_ctm_send_row_update_common (conf, pds_ms_fill_amb_lim_ent); 

    NBB_TRC_EXIT();
    return;
}

NBB_VOID
pds_ms_row_update_lim_mj (pds_ms_config_t  *conf)
{
    NBB_TRC_ENTRY ("pds_ms_row_update_lim_mj");

    // Set params
    conf->oid_len       = AMB_LIM_ENT_OID_LEN;
    conf->data_len      = sizeof (AMB_LIM_ENT);
    
    // Convert to row_update and send
    pds_ms_ctm_send_row_update_common (conf, pds_ms_fill_amb_lim_mj); 

    NBB_TRC_EXIT();
    return;
}

NBB_VOID
pds_ms_lim_create (pds_ms_config_t *conf)
{
    NBB_TRC_ENTRY ("pds_ms_lim_create");

    // limEntTable
    conf->entity_index  = PDS_MS_LIM_ENT_INDEX;
    pds_ms_row_update_lim (conf);

    // limMjTable - LIPI
    conf->interface_id   = AMB_LIM_IF_ATG_LIPI;
    conf->partner_type   = AMB_LIM_MJ_PARTNER_LI;
    conf->partner_index  = 1;
    conf->sub_index      = 0;
    pds_ms_row_update_lim_mj (conf);

    // limMjTable - SMI
    conf->interface_id   = AMB_LIM_IF_ATG_SMI;
    conf->partner_type   = AMB_LIM_MJ_PARTNER_SMI;
    conf->partner_index  = 1;
    conf->sub_index      = 0;
    pds_ms_row_update_lim_mj (conf);

    // limMjTable - BDII to EVPN
    conf->interface_id   = AMB_LIM_IF_ATG_BDII;
    conf->partner_type   = AMB_LIM_MJ_PARTNER_EVPN;
    conf->partner_index  = 1;
    conf->sub_index      = 0;
    pds_ms_row_update_lim_mj (conf);

    NBB_TRC_EXIT();
    return;
}
}
