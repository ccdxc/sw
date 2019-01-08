/**
 * Copyright (c) 2018 Pensando Systems, Inc.
 *
 * @file    api_base.cc
 *
 * @brief   base object definition for all API objects
 */

#include "nic/hal/apollo/core/oci_state.hpp"
#include "nic/hal/apollo/framework/api_base.hpp"
#include "nic/hal/apollo/framework/api_ctxt.hpp"

namespace api {

/** @brief        factory method to instantiate an object
 *  @param[in]    api_ctxt API context carrying object related configuration
 */
api_base *
api_base::factory(api_ctxt_t *api_ctxt) {
    api_base    *obj;

    switch (api_ctxt->obj_id) {
    case OBJ_ID_SWITCHPORT:
        obj = switchport_entry::factory(&api_ctxt->api_params->switchport_info);
        return obj;

    case OBJ_ID_VCN:
        obj = vcn_entry::factory(&api_ctxt->api_params->vcn_info);
        return obj;

    case OBJ_ID_SUBNET:
        obj = subnet_entry::factory(&api_ctxt->api_params->subnet_info);
        return obj;

    case OBJ_ID_TEP:
        obj = tep_entry::factory(&api_ctxt->api_params->tep_info);
        return obj;
        break;

    case OBJ_ID_VNIC:
        obj = vnic_entry::factory(&api_ctxt->api_params->vnic_info);
        return obj;

    case OBJ_ID_MAPPING:
        obj = mapping_entry::factory(&api_ctxt->api_params->mapping_info);
        return obj;

    case OBJ_ID_ROUTE_TABLE:
        obj = route_table::factory(&api_ctxt->api_params->route_table_info);
        break;

    case OBJ_ID_SECURITY_RULES:
        break;

    default:
        break;
    }
    return NULL;
}

/** @brief        find an object based on the object id & key information
 *  @param[in]    api_ctxt API context carrying object related information
 * TODO: ignore_dirty is on shaky ground, will try to get rid of it later
 */
api_base *
api_base::find_obj(api_ctxt_t *api_ctxt, bool ignore_dirty) {
    api_base    *obj;

    switch (api_ctxt->obj_id) {
    case OBJ_ID_SWITCHPORT:
        obj = switchport_db()->switchport_find();
        return obj;

    case OBJ_ID_VCN:
        if (api_ctxt->api_op == API_OP_DELETE) {
            obj = vcn_db()->vcn_find(&api_ctxt->api_params->vcn_key);
        } else {
            obj = vcn_db()->vcn_find(&api_ctxt->api_params->vcn_info.key);
        }
        return obj;

    case OBJ_ID_SUBNET:
        if (api_ctxt->api_op == API_OP_DELETE) {
            obj = subnet_db()->subnet_find(&api_ctxt->api_params->subnet_key);
        } else {
            obj = subnet_db()->subnet_find(&api_ctxt->api_params->subnet_info.key);
        }
        return obj;

    case OBJ_ID_TEP:
        if (api_ctxt->api_op == API_OP_DELETE) {
            obj = tep_db()->tep_find(&api_ctxt->api_params->tep_key);
        } else {
            obj = tep_db()->tep_find(&api_ctxt->api_params->tep_info.key);
        }
        break;

    case OBJ_ID_VNIC:
        if (api_ctxt->api_op == API_OP_DELETE) {
            obj = vnic_db()->vnic_find(&api_ctxt->api_params->vnic_key);
        } else {
            obj = vnic_db()->vnic_find(&api_ctxt->api_params->vnic_info.key);
        }
        return obj;

    case OBJ_ID_MAPPING:
        return NULL;
        break;

    case OBJ_ID_ROUTE_TABLE:
        if (api_ctxt->api_op == API_OP_DELETE) {
            obj = route_table_db()->route_table_find(&api_ctxt->api_params->route_table_key);
        } else {
            obj = route_table_db()->route_table_find(&api_ctxt->api_params->route_table_info.key);
        }
        break;

    case OBJ_ID_SECURITY_RULES:
        break;

    default:
        break;
    }
    return NULL;
}

}    // namespace api
