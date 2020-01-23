// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//Purpose: Helper APIs for metaswitch EVPN component

#include "nic/metaswitch/stubs/mgmt/pds_ms_mgmt_utils.hpp"
#include "nic/apollo/api/include/pds.hpp"
#include "nic/metaswitch/stubs/mgmt/pds_ms_mgmt_state.hpp"
#include "nic/metaswitch/stubs/mgmt/pds_ms_uuid_obj.hpp"
#include "evpn_mgmt_if.h"
using namespace pds_ms;

namespace pds {
NBB_VOID
evpn_evi_pre_set (EvpnEviSpec  &req,
                  NBB_LONG     row_status,
                  NBB_ULONG    test_correlator)
{
    // Local variables
    pds_obj_key_t uuid = {0};
    pds_obj_key_t subnet_uuid = {0};

    // get uuids
    pds_ms_get_uuid (&uuid, req.id());
    pds_ms_get_uuid (&subnet_uuid, req.subnetid());

    if (uuid ==  subnet_uuid) {
        // spec uuid is same as subnet uuid
        auto mgmt_ctxt = mgmt_state_t::thread_context();
        auto uuid_obj = mgmt_ctxt.state()->lookup_uuid(subnet_uuid);

        if (uuid_obj == nullptr) {
            SDK_TRACE_ERR ("EVPN EVI request with unknown key %s", subnet_uuid.str());
            return;
        } 
        if (uuid_obj->obj_type() == uuid_obj_type_t::SUBNET) {
           auto subnet_uuid_obj = (subnet_uuid_obj_t *)uuid_obj;
           req.set_eviid(subnet_uuid_obj->ms_id()); 
           SDK_TRACE_DEBUG("EVPN EVI request: %s evi-index: %d",
                            uuid.str(), subnet_uuid_obj->ms_id());
        } else {
            SDK_TRACE_ERR ("EVPN EVI request with non-matching UUID type %s",
                            uuid_obj_type_str(uuid_obj->obj_type()));
        }
    } else {
        // TODO: spec uuid doesnt match with subnet uuid. need to key-map
    }
}

NBB_VOID
evpn_evi_rt_pre_set (EvpnEviRtSpec  &req,
                     NBB_LONG       row_status,
                     NBB_ULONG      test_correlator)
{
    // Local variables
    pds_obj_key_t uuid = {0};
    pds_obj_key_t subnet_uuid = {0};
    
    // get uuids
    pds_ms_get_uuid (&uuid, req.id());
    pds_ms_get_uuid (&subnet_uuid, req.subnetid());

    if (strncmp (uuid.id, subnet_uuid.id, PDS_MAX_KEY_LEN) == 0) {
        // Venice case
        // spec uuid is same as subnet uuid
        auto mgmt_ctxt = mgmt_state_t::thread_context();
        auto uuid_obj = mgmt_ctxt.state()->lookup_uuid(subnet_uuid);

        if (uuid_obj == nullptr) {
            SDK_TRACE_ERR ("EVPN EVI RT request with unknown key %s", subnet_uuid.str());
            return;
        } else if (uuid_obj->obj_type() == uuid_obj_type_t::SUBNET) {
           auto subnet_uuid_obj = (subnet_uuid_obj_t *)uuid_obj;
           req.set_eviid(subnet_uuid_obj->ms_id()); 
           SDK_TRACE_DEBUG("EVPN EVI RT request: %s evi-index: %d",
                            uuid.str(), subnet_uuid_obj->ms_id());
        } else {
            SDK_TRACE_ERR ("EVPN EVI RT request with non-matching UUID type %s",
                            uuid_obj_type_str(uuid_obj->obj_type()));
        }
    } else {
        // Non Venice case
        // TODO: spec uuid doesnt match with subnet uuid. need to key-map
    }
}

NBB_VOID 
evpn_ip_vrf_pre_set (EvpnIpVrfSpec &req,
                     NBB_LONG      row_status,
                     NBB_ULONG     test_correlator) 
{
    // Local variables
    pds_obj_key_t uuid = {0};
    pds_obj_key_t vpc_uuid = {0};

    
    // get uuids
    pds_ms_get_uuid (&uuid, req.id());
    pds_ms_get_uuid (&vpc_uuid, req.vpcid());

    if (strncmp (uuid.id, vpc_uuid.id, PDS_MAX_KEY_LEN) == 0) {
        // Venice case
        // spec uuid is same as vpcuuid
        auto mgmt_ctxt = mgmt_state_t::thread_context();
        auto uuid_obj = mgmt_ctxt.state()->lookup_uuid(vpc_uuid);

        if (uuid_obj == nullptr) {
            SDK_TRACE_ERR ("EVPN IP VRF request with unknown VPC reference %s",
                            vpc_uuid.str());
            return;
        } 
        if (uuid_obj->obj_type() == uuid_obj_type_t::VPC) {
            auto vpc_uuid_obj = (vpc_uuid_obj_t *)uuid_obj;
            std::string vrf_name = std::to_string (vpc_uuid_obj->ms_id());
            req.set_vrfname(vrf_name);
            SDK_TRACE_DEBUG("EVPN IP VRF request: %s vrf-id: %d, vrf-name:%s",
                            uuid.str(), vpc_uuid_obj->ms_id(), vrf_name.c_str());
        } else {
            SDK_TRACE_ERR ("EVPN IP VRF request with non-matching reference type %s",
                            uuid_obj_type_str(uuid_obj->obj_type()));
        }
    } else {
        // Non-Venice case
        // TODO: spec uuid doesnt match with VPC uuid. need to key-map
    }
}

NBB_VOID 
evpn_ip_vrf_rt_pre_set (EvpnIpVrfRtSpec &req,
                        NBB_LONG        row_status,
                        NBB_ULONG       test_correlator) 
{
    // Local variables
    pds_obj_key_t uuid = {0};
    pds_obj_key_t vpc_uuid = {0};

    // get uuids
    pds_ms_get_uuid (&uuid, req.id());
    pds_ms_get_uuid (&vpc_uuid, req.vpcid());

    if (strncmp (uuid.id, vpc_uuid.id, PDS_MAX_KEY_LEN) == 0) {
        // Venice case
        // spec uuid is same as vpcuuid
        auto mgmt_ctxt = mgmt_state_t::thread_context();
        auto uuid_obj = mgmt_ctxt.state()->lookup_uuid(vpc_uuid);

        if (uuid_obj == nullptr) {
            SDK_TRACE_ERR ("EVPN IP VRF RT request with unknown VPC reference %s",
                            vpc_uuid.str());
            return;
        } 
        if (uuid_obj->obj_type() == uuid_obj_type_t::VPC) {
            auto vpc_uuid_obj = (vpc_uuid_obj_t *)uuid_obj;
            std::string vrf_name = std::to_string (vpc_uuid_obj->ms_id());
            req.set_vrfname(vrf_name);
            SDK_TRACE_DEBUG
                ("EVPN IP RT VRF request: %s vrf-id: %d, vrf-name:%s",
                 uuid.str(), vpc_uuid_obj->ms_id(), vrf_name.c_str());
        } else {
            SDK_TRACE_ERR ("EVPN IP VRF RT request with non-matching reference type %s",
                            uuid_obj_type_str(uuid_obj->obj_type()));
        }
    } else {
        // Non-Venice case
        // TODO: spec uuid doesnt match with VPC uuid. need to key-map
    }
}

NBB_VOID
evpn_evi_get_fill_func (EvpnEviSpec&    req,
                        NBB_ULONG*       oid)
{
    oid[AMB_EVPN_EVI_ENTITY_IX_INDEX] = PDS_MS_EVPN_ENT_INDEX;
}

NBB_VOID
evpn_evi_set_fill_func (EvpnEviSpec&    req,
                    AMB_GEN_IPS     *mib_msg,
                    AMB_EVPN_EVI    *data,
                    NBB_LONG        row_status)
{
    // Local variables
    NBB_ULONG *oid = (NBB_ULONG *)((NBB_BYTE *)mib_msg + mib_msg->oid_offset);

    data->entity_index                = PDS_MS_EVPN_ENT_INDEX;
    oid[AMB_EVPN_EVI_ENTITY_IX_INDEX] = PDS_MS_EVPN_ENT_INDEX;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_EVPN_EVI_ENTITY_IX);
}

NBB_VOID
evpn_ip_vrf_get_fill_func (EvpnIpVrfSpec&   req,
                       NBB_ULONG*        oid)
{
    oid[AMB_EVPN_IP_VRF_ENT_INDEX]  = PDS_MS_EVPN_ENT_INDEX;
}

NBB_VOID
evpn_ip_vrf_set_fill_func (EvpnIpVrfSpec&   req,
                       AMB_GEN_IPS      *mib_msg,
                       AMB_EVPN_IP_VRF  *data,
                       NBB_LONG         row_status)
{
    // Local variables
    NBB_ULONG *oid = (NBB_ULONG *)((NBB_BYTE *)mib_msg + mib_msg->oid_offset);

    data->entity_index              = PDS_MS_EVPN_ENT_INDEX;
    oid[AMB_EVPN_IP_VRF_ENT_INDEX]  = PDS_MS_EVPN_ENT_INDEX;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_EVPN_IP_VRF_ENT_IX);
}

NBB_VOID
evpn_evi_rt_get_fill_func (EvpnEviRtSpec&   req,
                           NBB_ULONG*        oid)
{
    oid[AMB_EVPN_EVI_RT_ENT_INDEX]  = PDS_MS_EVPN_ENT_INDEX;
}

NBB_VOID
evpn_evi_rt_set_fill_func (EvpnEviRtSpec&   req,
                       AMB_GEN_IPS      *mib_msg,
                       AMB_EVPN_EVI_RT  *data,
                       NBB_LONG         row_status)
{
    // Local variables
    NBB_ULONG *oid = (NBB_ULONG *)((NBB_BYTE *)mib_msg + mib_msg->oid_offset);

    data->entity_index              = PDS_MS_EVPN_ENT_INDEX;
    oid[AMB_EVPN_EVI_RT_ENT_INDEX]  = PDS_MS_EVPN_ENT_INDEX;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_EVPN_EVI_RT_ENT_IX);
}

NBB_VOID
evpn_ip_vrf_rt_get_fill_func (EvpnIpVrfRtSpec&      req,
                          NBB_ULONG*              oid)
{
    oid[AMB_EVPN_IP_VRF_RT_ENT_INDEX]   = PDS_MS_EVPN_ENT_INDEX;
}

NBB_VOID
evpn_ip_vrf_rt_set_fill_func (EvpnIpVrfRtSpec&      req,
                          AMB_GEN_IPS           *mib_msg,
                          AMB_EVPN_IP_VRF_RT    *data,
                          NBB_LONG              row_status)
{
    // Local variables
    NBB_ULONG *oid = (NBB_ULONG *)((NBB_BYTE *)mib_msg + mib_msg->oid_offset);

    data->entity_index                  = PDS_MS_EVPN_ENT_INDEX;
    oid[AMB_EVPN_IP_VRF_RT_ENT_INDEX]   = PDS_MS_EVPN_ENT_INDEX;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_EVPN_IV_RT_ENT_IX);
}


NBB_VOID
evpn_mac_ip_get_fill_func (EvpnMacIpSpec& req, NBB_ULONG *oid)
{
    oid[AMB_EVPN_MAC_IP_ENTITY_IX_INDEX] = PDS_MS_EVPN_ENT_INDEX;
}
} // End namespace pds

namespace pds_ms {
// Fill evpnEntTable: AMB_EVPN_ENT 
NBB_VOID
pds_ms_fill_amb_evpn_ent (AMB_GEN_IPS  *mib_msg, pds_ms_config_t *conf)
{ 
    // Local variables
    NBB_ULONG       *oid = NULL; 
    AMB_EVPN_ENT    *data= NULL;

    NBB_TRC_ENTRY ("pds_ms_fill_amb_evpn");

    // Get oid and data offset 
    oid     = (NBB_ULONG *)((NBB_BYTE *)mib_msg + mib_msg->oid_offset);
    data    = (AMB_EVPN_ENT *)((NBB_BYTE *)mib_msg + mib_msg->data_offset); 

    // Set all fields absent
    AMB_SET_ALL_FIELDS_NOT_PRESENT (mib_msg);

    // Set OID len and family
    oid[0] = AMB_EVPN_ENT_OID_LEN;
    oid[1] = AMB_FAM_EVPN_ENT;

    // Set all incoming fields
    oid[AMB_EVPN_ENT_ENTITY_INDEX_INDEX]    = conf->entity_index;
    data->entity_index                      = conf->entity_index;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_EVPN_ENT_ENTITY_INDEX);

    data->row_status = conf->row_status;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_EVPN_ENT_ROW_STATUS);

    if (conf->row_status != AMB_ROW_DESTROY) {
        // Reduce the interval required to turn around and issue MAI Delete
        // in case of the EVPN Remote to Local MAC/IP move scenario
        data->mac_withdraw_delay = AMB_EVPN_MIN_MAC_DELAY;
        AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_EVPN_ENT_MAC_DELAY);

        data->irb_mode = AMB_EVPN_IRB_ASYMMETRIC;
        AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_EVPN_ENT_IRB_MODE);
        data->use_hal = AMB_TRUE;
        AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_EVPN_ENT_USE_HAL);

    }
    NBB_TRC_EXIT ();
    return;
}

// Fill evpnMjTable:AMB_EVPN_MJ 
NBB_VOID
pds_ms_fill_amb_evpn_mj (AMB_GEN_IPS  *mib_msg, pds_ms_config_t *conf)
{ 
    // Local variables
    NBB_ULONG   *oid = NULL; 
    AMB_EVPN_MJ *data= NULL;

    NBB_TRC_ENTRY ("pds_ms_fill_amb_evpn_mj");

    // Get oid and data offset 
    oid     = (NBB_ULONG *)((NBB_BYTE *)mib_msg + mib_msg->oid_offset);
    data    = (AMB_EVPN_MJ *)((NBB_BYTE *)mib_msg + mib_msg->data_offset); 

    // Set all fields absent
    AMB_SET_ALL_FIELDS_NOT_PRESENT (mib_msg);

    // Set OID len and family
    oid[0] = AMB_EVPN_MJ_OID_LEN;
    oid[1] = AMB_FAM_EVPN_MJ;

    // Set all incoming fields
    oid[AMB_EVPN_MJ_ENTITY_IX_INDEX]    = conf->entity_index;
    data->ent_entity_index              = conf->entity_index;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_EVPN_MJ_ENT_ENTITY_IX);

    oid[AMB_EVPN_MJ_INTERFACE_ID_INDEX] = conf->interface_id;
    data->interface_id                  = conf->interface_id;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_EVPN_MJ_INTERFACE_ID);

    oid[AMB_EVPN_MJ_PARTNER_TYPE_INDEX] = conf->partner_type;
    data->partner_type                  = conf->partner_type;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_EVPN_MJ_PARTNER_TYPE);

    oid[AMB_EVPN_MJ_PARTNER_INDEX_INDEX]    = conf->partner_index;
    data->partner_index                     = conf->partner_index;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_EVPN_MJ_PARTNER_INDEX);

    oid[AMB_EVPN_MJ_SUB_INDEX_INDEX]    = conf->sub_index;
    data->sub_index                     = conf->sub_index;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_EVPN_MJ_SUB_INDEX);

    data->row_status = conf->row_status;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_EVPN_MJ_ROW_STATUS);

    NBB_TRC_EXIT ();
    return;
}

// row_update for evpnEntTable
NBB_VOID
pds_ms_row_update_evpn (pds_ms_config_t *conf)
{
    NBB_TRC_ENTRY ("pds_ms_row_update_evpn");

    // Set params
    conf->oid_len       = AMB_EVPN_ENT_OID_LEN;
    conf->data_len      = sizeof (AMB_EVPN_ENT);

    // Convert to row_update and send
    pds_ms_ctm_send_row_update_common (conf, pds_ms_fill_amb_evpn_ent); 

    NBB_TRC_EXIT();
    return;
}

NBB_VOID
pds_ms_row_update_evpn_mj (pds_ms_config_t *conf)
{
    NBB_TRC_ENTRY ("pds_ms_row_update_evpn_mj");

    // Set params
    conf->oid_len       = AMB_EVPN_MJ_OID_LEN;
    conf->data_len      = sizeof (AMB_EVPN_MJ);

    // Convert to row_update and send
    pds_ms_ctm_send_row_update_common (conf, pds_ms_fill_amb_evpn_mj); 

    NBB_TRC_EXIT();
    return;
}

NBB_VOID
pds_ms_evpn_create (pds_ms_config_t *conf)
{
    NBB_TRC_ENTRY ("pds_ms_evpn_create");

    // evpnEntTable
    conf->entity_index  = PDS_MS_EVPN_ENT_INDEX;
    pds_ms_row_update_evpn (conf);

    // evpnMjTable - AMB_EVPN_IF_ATG_BDPI
    conf->interface_id   = AMB_EVPN_IF_ATG_BDPI;
    conf->partner_type   = AMB_EVPN_MJ_PARTNER_L2FST;
    conf->partner_index  = 1;
    conf->sub_index      = 0;
    pds_ms_row_update_evpn_mj (conf);

    // evpnMjTable - AMB_EVPN_IF_ATG_I3
    conf->interface_id   = AMB_EVPN_IF_ATG_I3;
    conf->partner_type   = AMB_EVPN_MJ_PARTNER_LIM;
    conf->partner_index  = 1;
    conf->sub_index      = 0;
    pds_ms_row_update_evpn_mj (conf);

    // evpnMjTable -AMB_EVPN_IF_ATG_TPI 
    conf->interface_id   = AMB_EVPN_IF_ATG_TPI;
    conf->partner_type   = AMB_EVPN_MJ_PARTNER_LIM;
    conf->partner_index  = 1;
    conf->sub_index      = 0;
    pds_ms_row_update_evpn_mj (conf);

    // evpnMjTable - AMB_EVPN_IF_ATG_MAI
    conf->interface_id   = AMB_EVPN_IF_ATG_MAI;
    conf->partner_type   = AMB_EVPN_MJ_PARTNER_L2FST;
    conf->partner_index  = 1;
    conf->sub_index      = 0;
    pds_ms_row_update_evpn_mj (conf);

    // evpnMjTable - PRI
    conf->interface_id   = AMB_EVPN_IF_ATG_PRI;
    conf->partner_type   = AMB_EVPN_MJ_PARTNER_PSM;
    conf->partner_index  = 1;
    conf->sub_index      = 0;
    pds_ms_row_update_evpn_mj (conf);

    NBB_TRC_EXIT();
    return;
}

void
pds_ms_evpn_rtm_join (pds_ms_config_t *conf, int rtm_entity_index)
{
    conf->entity_index  = PDS_MS_EVPN_ENT_INDEX;
    conf->interface_id   = AMB_EVPN_IF_ATG_RPI;
    conf->partner_type   = AMB_EVPN_MJ_PARTNER_DFLT;
    conf->partner_index  = rtm_entity_index;
    conf->sub_index      = AMB_INETWK_ADDR_TYPE_IPV4;
    pds_ms_row_update_evpn_mj (conf);
}

}
