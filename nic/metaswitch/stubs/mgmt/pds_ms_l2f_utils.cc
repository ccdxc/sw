// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// Purpose: Helper APIs for metaswitch L2F stub programming 

#include "nic/metaswitch/stubs/mgmt/pds_ms_mgmt_utils.hpp"
#include "nic/metaswitch/stubs/mgmt/pds_ms_destip_track.hpp"
#include "nic/metaswitch/stubs/hals/pds_ms_l2f_mai.hpp"
#include "l2f_mgmt_if.h"
#include <iostream>

using namespace std;
#define SHARED_DATA_TYPE SMS_SHARED_LOCAL

namespace pds_ms {

// Fill l2fEntTable: AMB_STUBS_L2F_ENT
NBB_VOID
pds_ms_fill_amb_l2f_ent (AMB_GEN_IPS *mib_msg, pds_ms_config_t *conf)
{ 
    // Local variables
    NBB_ULONG           *oid = NULL; 
    AMB_STUBS_L2F_ENT   *data = NULL;

    // Get oid and data offset 
    oid     = (NBB_ULONG *)((NBB_BYTE *)mib_msg + mib_msg->oid_offset);
    data    = (AMB_STUBS_L2F_ENT *)((NBB_BYTE *)mib_msg + mib_msg->data_offset); 

    NBB_TRC_ENTRY ("pds_ms_fill_amb_l2f_ent");

    // Set all fields absent
    AMB_SET_ALL_FIELDS_NOT_PRESENT (mib_msg);

    // Set OID len and family
    oid[0] = AMB_L2F_ENT_OID_LEN;
    oid[1] = AMB_FAM_STUBS_L2F_ENT;

    // Set all incoming fields
    data->entity_index              = conf->entity_index;
    oid[AMB_L2F_ENT_ENT_IX_INDEX]   = conf->entity_index;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_L2F_ENT_ENTITY_INDEX);

    data->row_status = conf->row_status;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_L2F_ENT_ROW_STATUS);

    if (conf->row_status != AMB_ROW_DESTROY)
    {
        NBB_TRC_FLOW ((NBB_FORMAT "Not destroying L2F Ent: fill in fields"));
        data->stateful = conf->stateful;
        AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_L2F_ENT_STATEFUL);
    }

    NBB_TRC_EXIT ();
    return;
}

NBB_VOID
pds_ms_row_update_l2f (pds_ms_config_t *conf)
{
    NBB_TRC_ENTRY ("pds_ms_row_update_l2f");

    conf->oid_len       = AMB_L2F_ENT_OID_LEN;
    conf->data_len      = sizeof (AMB_STUBS_L2F_ENT);

    pds_ms_ctm_send_row_update_common (conf, pds_ms_fill_amb_l2f_ent); 

    NBB_TRC_EXIT();
    return;
}

NBB_VOID 
pds_ms_l2f_stub_create (pds_ms_config_t *conf)
{
    NBB_TRC_ENTRY ("pds_ms_l2f_stub_create");

    // l2fEntTable
    conf->entity_index  = PDS_MS_L2F_ENT_INDEX;
    conf->stateful      = AMB_FALSE;
    pds_ms_row_update_l2f (conf);

    NBB_TRC_EXIT();
    return;
}

types::ApiStatus
l2f_test_local_mac_ip_add (const CPL2fTestCreateSpec *req, CPL2fTestResponse *resp)
{
    ip_addr_t   ip_addr;
    mac_addr_t  mac_addr = {0};
    pds_obj_key_t subnet_uuid = {0};

    pds_ms_get_uuid (&subnet_uuid, req->subnetid());
    ip_addr_spec_to_ip_addr (req->ipaddr(), &ip_addr);
    NBB_MEMCPY(mac_addr, req->macaddr().c_str(), req->macaddr().length());
    
    pds_ms::l2f_local_mac_ip_add (subnet_uuid, ip_addr, mac_addr,
                                  req->ifid());

    return types::ApiStatus::API_STATUS_OK;
}

types::ApiStatus
l2f_test_local_mac_ip_del (const CPL2fTestDeleteSpec *req, CPL2fTestResponse *resp)
{
    ip_addr_t   ip_addr;
    mac_addr_t  mac_addr = {0};
    pds_obj_key_t subnet_uuid = {0};

    pds_ms_get_uuid (&subnet_uuid, req->subnetid());
    ip_addr_spec_to_ip_addr (req->ipaddr(), &ip_addr);
    NBB_MEMCPY(mac_addr, req->macaddr().c_str(), req->macaddr().length());
    
    pds_ms::l2f_local_mac_ip_del (subnet_uuid, ip_addr, mac_addr);

    return types::ApiStatus::API_STATUS_OK;
}

} // end namespace pds_ms
