//
// {C} Copyright 2018 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This file deals with base object definition for all API objects
///
//----------------------------------------------------------------------------

#include "nic/apollo/api/pds_state.hpp"
#include "nic/apollo/framework/api_base.hpp"
#include "nic/apollo/framework/api_ctxt.hpp"

namespace api {

api_base *
api_base::factory(api_ctxt_t *api_ctxt) {
    api_base    *obj;

    switch (api_ctxt->obj_id) {
    case OBJ_ID_DEVICE:
        obj = device_entry::factory(&api_ctxt->api_params->device_spec);
        return obj;

    case OBJ_ID_VCN:
        obj = vcn_entry::factory(&api_ctxt->api_params->vcn_spec);
        return obj;

    case OBJ_ID_SUBNET:
        obj = subnet_entry::factory(&api_ctxt->api_params->subnet_spec);
        return obj;

    case OBJ_ID_TEP:
        obj = tep_entry::factory(&api_ctxt->api_params->tep_spec);
        return obj;

    case OBJ_ID_VNIC:
        obj = vnic_entry::factory(&api_ctxt->api_params->vnic_spec);
        return obj;

    case OBJ_ID_MAPPING:
        obj = mapping_entry::factory(&api_ctxt->api_params->mapping_spec);
        return obj;

    case OBJ_ID_ROUTE_TABLE:
        obj = route_table::factory(&api_ctxt->api_params->route_table_spec);
        return obj;

    case OBJ_ID_POLICY:
        obj = policy::factory(&api_ctxt->api_params->policy_spec);
        return obj;

    default:
        break;
    }
    return NULL;
}

api_base *
api_base::find_obj(api_ctxt_t *api_ctxt, bool ignore_dirty) {
    api_base    *obj;

    switch (api_ctxt->obj_id) {
    case OBJ_ID_DEVICE:
        obj = device_db()->find();
        return obj;

    case OBJ_ID_VCN:
        if (api_ctxt->api_op == API_OP_DELETE) {
            obj = vcn_db()->find(&api_ctxt->api_params->vcn_key);
        } else {
            obj = vcn_db()->find(&api_ctxt->api_params->vcn_spec.key);
        }
        return obj;

    case OBJ_ID_SUBNET:
        if (api_ctxt->api_op == API_OP_DELETE) {
            obj = subnet_db()->find(&api_ctxt->api_params->subnet_key);
        } else {
            obj = subnet_db()->find(&api_ctxt->api_params->subnet_spec.key);
        }
        return obj;

    case OBJ_ID_TEP:
        if (api_ctxt->api_op == API_OP_DELETE) {
            obj = tep_db()->find(&api_ctxt->api_params->tep_key);
        } else {
            obj = tep_db()->find(&api_ctxt->api_params->tep_spec.key);
        }
        return obj;

    case OBJ_ID_VNIC:
        if (api_ctxt->api_op == API_OP_DELETE) {
            obj = vnic_db()->vnic_find(&api_ctxt->api_params->vnic_key);
        } else {
            obj = vnic_db()->vnic_find(&api_ctxt->api_params->vnic_spec.key);
        }
        return obj;

    case OBJ_ID_MAPPING:
        return NULL;

    case OBJ_ID_ROUTE_TABLE:
        if (api_ctxt->api_op == API_OP_DELETE) {
            obj = route_table_db()->find(&api_ctxt->api_params->route_table_key);
        } else {
            obj = route_table_db()->find(&api_ctxt->api_params->route_table_spec.key);
        }
        return obj;

    case OBJ_ID_POLICY:
        if (api_ctxt->api_op == API_OP_DELETE) {
            obj = policy_db()->policy_find(&api_ctxt->api_params->policy_key);
        } else {
            obj = policy_db()->policy_find(&api_ctxt->api_params->policy_spec.key);
        }
        return obj;

    default:
        break;
    }
    return NULL;
}

}    // namespace api
