/**
 * Copyright (c) 2018 Pensando Systems, Inc.
 *
 * @file    api_obj.cc
 *
 * @brief   base object definition for all API objects
 */

#include "nic/hal/apollo/api/oci_state.hpp"
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
        break;
    case OBJ_ID_VCN:
        obj = vcn_entry::factory(&api_ctxt->vcn_info);
        return obj;
    case OBJ_ID_SUBNET:
        obj = subnet_entry::factory(&api_ctxt->subnet_info);
        return obj;
    case OBJ_ID_TEP:
        break;
    case OBJ_ID_VNIC:
        obj = vnic_entry::factory(&api_ctxt->vnic_info);
        return obj;
    case OBJ_ID_ROUTE:
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
        break;
    case OBJ_ID_VCN:
        obj = vcn_db()->vcn_find(&api_ctxt->vcn_key);
        return obj;
    case OBJ_ID_SUBNET:
        obj = subnet_db()->subnet_find(&api_ctxt->subnet_key);
        return obj;
    case OBJ_ID_TEP:
        break;
    case OBJ_ID_VNIC:
        obj = vnic_db()->vnic_find(&api_ctxt->vnic_key);
        return obj;
    case OBJ_ID_ROUTE:
        break;
    case OBJ_ID_SECURITY_RULES:
        break;
    default:
        break;
    }
    return NULL;
}

}    // namespace api
