// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//Purpose: Helper APIs for metaswitch EVPN component

#include "pdsa_mgmt_utils.hpp"
#include "evpn_mgmt_if.h"

// Fill evpnEntTable: AMB_EVPN_ENT 
NBB_VOID
pdsa_fill_amb_evpn_ent (AMB_GEN_IPS     *mib_msg,
                        AMB_EVPN_ENT    *v_amb_evpn_ent,
                        NBB_ULONG       entity_index,
                        NBB_LONG        row_status)
{ 
    NBB_TRC_ENTRY ("pdsa_fill_amb_evpn");

    // Set all fields absent
    AMB_SET_ALL_FIELDS_NOT_PRESENT (mib_msg);

    // Set all incoming fields
    v_amb_evpn_ent->entity_index = entity_index;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_EVPN_ENT_ENTITY_INDEX);

    v_amb_evpn_ent->row_status = row_status;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_EVPN_ENT_ROW_STATUS);

    NBB_TRC_EXIT ();
    return;
}

// Fill evpnMjTable:AMB_EVPN_MJ 
NBB_VOID
pdsa_fill_amb_evpn_mj (AMB_GEN_IPS   *mib_msg,
                       AMB_EVPN_MJ   *v_amb_evpn_mj,
                       NBB_ULONG     ent_entity_index,
                       NBB_LONG      interface_id,
                       NBB_ULONG     partner_type,
                       NBB_ULONG     partner_index,
                       NBB_ULONG     sub_index,
                       NBB_LONG      row_status)
{ 
    NBB_TRC_ENTRY ("pdsa_fill_amb_evpn_mj");

    // Set all fields absent
    AMB_SET_ALL_FIELDS_NOT_PRESENT (mib_msg);

    // Set all incoming fields
    v_amb_evpn_mj->ent_entity_index = ent_entity_index;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_EVPN_MJ_ENT_ENTITY_IX);

    v_amb_evpn_mj->interface_id = interface_id;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_EVPN_MJ_INTERFACE_ID);

    v_amb_evpn_mj->partner_type = partner_type;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_EVPN_MJ_PARTNER_TYPE);

    v_amb_evpn_mj->partner_index = partner_index;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_EVPN_MJ_PARTNER_INDEX);

    v_amb_evpn_mj->sub_index = sub_index;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_EVPN_MJ_SUB_INDEX);

    v_amb_evpn_mj->row_status = row_status;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_EVPN_MJ_ROW_STATUS);

    NBB_TRC_EXIT ();
    return;
}


// Fill evpnEviTable: AMB_EVPN_EVI
NBB_VOID
pdsa_fill_amb_evpn_evi (AMB_GEN_IPS     *mib_msg,
                        AMB_EVPN_EVI    *v_amb_evpn_evi,
                        NBB_ULONG       entity_index,
                        NBB_ULONG       index,
                        NBB_LONG        rd_cfg_or_auto,
                        NBB_BYTE        *cfg_rd,
                        NBB_LONG        row_status,
                        NBB_LONG        encapsulation)
{ 
    NBB_TRC_ENTRY ("pdsa_fill_amb_evpn_evi");

    // Set all fields absent
    AMB_SET_ALL_FIELDS_NOT_PRESENT (mib_msg);

    // Set all incoming fields
    v_amb_evpn_evi->entity_index = entity_index;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_EVPN_EVI_ENTITY_IX);

    v_amb_evpn_evi->index = index;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_EVPN_EVI_INDEX);

    v_amb_evpn_evi->row_status = row_status;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_EVPN_EVI_ROW_STATUS);

    if (row_status != AMB_ROW_DESTROY)
    {

        NBB_TRC_FLOW ((NBB_FORMAT "Not destroying EVPN EVI: fill in fields"));
        v_amb_evpn_evi->rd_cfg_or_auto = rd_cfg_or_auto;
        AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_EVPN_EVI_RD_CFG_AUTO);

        if (rd_cfg_or_auto == AMB_EVPN_CONFIGURED)
        {
            NBB_MEMCPY (v_amb_evpn_evi->cfg_rd, cfg_rd,  
                        AMB_EVPN_EXT_COMM_LENGTH);
            AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_EVPN_EVI_CFG_RD);
        }

        v_amb_evpn_evi->encapsulation = encapsulation;
        AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_EVPN_EVI_ENCAPS);
    }

    NBB_TRC_EXIT ();
    return;
}

// Fill evpnBdTable: AMB_EVPN_BD
NBB_VOID
pdsa_fill_amb_evpn_bd (AMB_GEN_IPS  *mib_msg,
                       AMB_EVPN_BD  *v_amb_evpn_bd,
                       NBB_ULONG    entity_index,
                       NBB_ULONG    evi_index,
                       NBB_LONG     row_status,
                       NBB_ULONG    vni)
{ 
    NBB_TRC_ENTRY ("pdsa_fill_amb_evpn_bd");

    // Set all fields absent
    AMB_SET_ALL_FIELDS_NOT_PRESENT (mib_msg);

    // Set all incoming fields
    v_amb_evpn_bd->entity_index = entity_index;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_EVPN_BD_ENT_ENTITY_IX);

    v_amb_evpn_bd->evi_index = evi_index;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_EVPN_BD_EVI_INDEX);

    v_amb_evpn_bd->row_status = row_status;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_EVPN_BD_ROW_STATUS);

    if (row_status != AMB_ROW_DESTROY)
    {
        NBB_TRC_FLOW ((NBB_FORMAT "Not destroying EVPN BD: fill in fields"));
        v_amb_evpn_bd->vni = vni;
        AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_EVPN_BD_VNI);
    }

    NBB_TRC_EXIT ();
    return;
}


// Fill evpnIfBindCfgTable: AMB_EVPN_IF_BIND_CFG
NBB_VOID
pdsa_fill_amb_evpn_if_bind_cfg (AMB_GEN_IPS             *mib_msg,
                                AMB_EVPN_IF_BIND_CFG    *v_amb_evpn_if_bind_cfg,
                                NBB_ULONG               entity_index,
                                NBB_ULONG               if_index,
                                NBB_LONG                row_status,
                                NBB_ULONG               evi_index)
{ 
    NBB_TRC_ENTRY ("pdsa_fill_amb_evpn_if_bind_cfg");

    // Set all fields absent
    AMB_SET_ALL_FIELDS_NOT_PRESENT (mib_msg);

    // Set all incoming fields
    v_amb_evpn_if_bind_cfg->entity_index = entity_index;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_EVPN_IBC_ENT_IX);

    v_amb_evpn_if_bind_cfg->if_index = if_index;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_EVPN_IBC_IF_INDEX);

    v_amb_evpn_if_bind_cfg->evi_index = evi_index;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_EVPN_IBC_EVI_IX);

    v_amb_evpn_if_bind_cfg->row_status = row_status;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_EVPN_IBC_ROW_STATUS);

    NBB_TRC_EXIT ();
    return;
}

// Fill evpnIpVrfTable: AMB_EVPN_IP_VRF
NBB_VOID
pdsa_fill_amb_evpn_ip_vrf (AMB_GEN_IPS      *mib_msg,
                           AMB_EVPN_IP_VRF  *v_amb_evpn_ip_vrf,
                           NBB_ULONG        entity_index,
                           NBB_LONG         vrf_name_len,
                           NBB_BYTE         *vrf_name,
                           NBB_LONG         row_status,
                           NBB_ULONG        vni,
                           NBB_BYTE         *route_distinguisher)
{ 
    NBB_TRC_ENTRY ("pdsa_fill_amb_evpn_ip_vrf");

    // Set all fields absent
    AMB_SET_ALL_FIELDS_NOT_PRESENT (mib_msg);

    // Set all incoming fields
    v_amb_evpn_ip_vrf->entity_index = entity_index;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_EVPN_IP_VRF_ENT_IX);

    v_amb_evpn_ip_vrf->vrf_name_len = vrf_name_len;
    NBB_MEMCPY (v_amb_evpn_ip_vrf->vrf_name, vrf_name, vrf_name_len);
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_EVPN_IP_VRF_NAME);

    v_amb_evpn_ip_vrf->row_status = row_status;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_EVPN_IP_VRF_ROW_STATUS);

    if (row_status != AMB_ROW_DESTROY)
    {
        NBB_TRC_FLOW ((NBB_FORMAT "Not destroying IP VRF Entry: fill in fields"));
        v_amb_evpn_ip_vrf->vni = vni;
        AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_EVPN_IP_VRF_VNI);

        NBB_MEMCPY (v_amb_evpn_ip_vrf->route_distinguisher, route_distinguisher, 
                    AMB_EVPN_EXT_COMM_LENGTH);
        AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_EVPN_IP_VRF_RD);
    }

    NBB_TRC_EXIT ();
    return;
}

// TODO: Do we need AMB_EVPN_MAC_IP (evpnMacIpTable)?
