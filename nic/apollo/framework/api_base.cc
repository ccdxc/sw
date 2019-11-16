//
// {C} Copyright 2018 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This file deals with base object definition for all API objects
///
//----------------------------------------------------------------------------

#include "nic/apollo/core/trace.hpp"
#include "nic/apollo/api/pds_state.hpp"
#include "nic/apollo/framework/api_base.hpp"
#include "nic/apollo/framework/api_params.hpp"

namespace api {

api_base *
api_base::factory(api_ctxt_t *api_ctxt) {
    switch (api_ctxt->obj_id) {
    case OBJ_ID_DEVICE:
        return device_entry::factory(&api_ctxt->api_params->device_spec);

    case OBJ_ID_IF:
        return if_entry::factory(&api_ctxt->api_params->if_spec);

    case OBJ_ID_VPC:
        return vpc_entry::factory(&api_ctxt->api_params->vpc_spec);

    case OBJ_ID_SUBNET:
        return subnet_entry::factory(&api_ctxt->api_params->subnet_spec);

    case OBJ_ID_TEP:
        return tep_entry::factory(&api_ctxt->api_params->tep_spec);

    case OBJ_ID_VNIC:
        return vnic_entry::factory(&api_ctxt->api_params->vnic_spec);

    case OBJ_ID_MAPPING:
        return mapping_entry::factory(&api_ctxt->api_params->mapping_spec);

    case OBJ_ID_ROUTE_TABLE:
        return route_table::factory(&api_ctxt->api_params->route_table_spec);

    case OBJ_ID_POLICY:
        return policy::factory(&api_ctxt->api_params->policy_spec);

    case OBJ_ID_MIRROR_SESSION:
        return mirror_session::factory(&api_ctxt->api_params->mirror_session_spec);

    case OBJ_ID_METER:
        return meter_entry::factory(&api_ctxt->api_params->meter_spec);

    case OBJ_ID_TAG:
        return tag_entry::factory(&api_ctxt->api_params->tag_spec);

    case OBJ_ID_SVC_MAPPING:
        return svc_mapping::factory(&api_ctxt->api_params->svc_mapping_spec);

    case OBJ_ID_VPC_PEER:
        return vpc_peer_entry::factory(&api_ctxt->api_params->vpc_peer_spec);

    case OBJ_ID_NEXTHOP:
        return nexthop::factory(&api_ctxt->api_params->nexthop_spec);

    case OBJ_ID_NEXTHOP_GROUP:
        return nexthop_group::factory(&api_ctxt->api_params->nexthop_group_spec);

    default:
        break;
    }
    return NULL;
}

api_base *
api_base::build(api_ctxt_t *api_ctxt) {
    switch (api_ctxt->obj_id) {
    case OBJ_ID_MAPPING:
        // mapping is a stateless object, so we need to construct the object
        // from the datapath tables
        if (api_ctxt->api_op == API_OP_DELETE) {
            return mapping_entry::build(&api_ctxt->api_params->mapping_key);
        }
        return mapping_entry::build(&api_ctxt->api_params->mapping_spec.key);

    case OBJ_ID_MIRROR_SESSION:
        // mirror is a stateless object, so we need to construct the object
        // from the datapath tables
        if (api_ctxt->api_op == API_OP_DELETE) {
            return mirror_session::build(&api_ctxt->api_params->mirror_session_key);
        }
        return mirror_session::build(&api_ctxt->api_params->mirror_session_spec.key);

    case OBJ_ID_SVC_MAPPING:
        // service mapping is a stateless object, so we need to construct the
        // object from the datapath tables
        if (api_ctxt->api_op == API_OP_DELETE) {
            return svc_mapping::build(&api_ctxt->api_params->svc_mapping_key);
        }
        return svc_mapping::build(&api_ctxt->api_params->svc_mapping_spec.key);

    case OBJ_ID_VPC_PEER:
        // VPC peering is a stateless object, so we need to construct the
        // object from the datapath tables
        if (api_ctxt->api_op == API_OP_DELETE) {
            return vpc_peer_entry::build(&api_ctxt->api_params->vpc_peer_key);
        }
        return vpc_peer_entry::build(&api_ctxt->api_params->vpc_peer_spec.key);

    default:
        break;
    }
    return NULL;
}

void
api_base::soft_delete(obj_id_t obj_id, api_base *api_obj) {
    switch(obj_id) {
    case OBJ_ID_MAPPING:
        mapping_entry::soft_delete((mapping_entry *)api_obj);
        break;

    case OBJ_ID_MIRROR_SESSION:
        mirror_session::soft_delete((mirror_session *)api_obj);
        break;

    case OBJ_ID_SVC_MAPPING:
        svc_mapping::soft_delete((svc_mapping *)api_obj);
        break;

    case OBJ_ID_VPC_PEER:
        vpc_peer_entry::soft_delete((vpc_peer_entry *)api_obj);
        break;

    default:
        PDS_TRACE_ERR("Non-statless obj %u can't be soft deleted", obj_id);
        break;
    }
}

api_base *
api_base::find_obj(api_ctxt_t *api_ctxt, bool ignore_dirty) {
    switch (api_ctxt->obj_id) {
    case OBJ_ID_DEVICE:
        return device_db()->find();

    case OBJ_ID_IF:
        if (api_ctxt->api_op == API_OP_DELETE) {
            return if_db()->find(&api_ctxt->api_params->if_key);
        }
        return if_db()->find(&api_ctxt->api_params->if_spec.key);

    case OBJ_ID_VPC:
        if (api_ctxt->api_op == API_OP_DELETE) {
            return vpc_db()->find(&api_ctxt->api_params->vpc_key);
        }
        return vpc_db()->find(&api_ctxt->api_params->vpc_spec.key);

    case OBJ_ID_SUBNET:
        if (api_ctxt->api_op == API_OP_DELETE) {
            return subnet_db()->find(&api_ctxt->api_params->subnet_key);
        }
        return subnet_db()->find(&api_ctxt->api_params->subnet_spec.key);

    case OBJ_ID_TEP:
        if (api_ctxt->api_op == API_OP_DELETE) {
            return tep_db()->find(&api_ctxt->api_params->tep_key);
        }
        return tep_db()->find(&api_ctxt->api_params->tep_spec.key);

    case OBJ_ID_VNIC:
        if (api_ctxt->api_op == API_OP_DELETE) {
            return vnic_db()->find(&api_ctxt->api_params->vnic_key);
        }
        return vnic_db()->find(&api_ctxt->api_params->vnic_spec.key);

    case OBJ_ID_ROUTE_TABLE:
        if (api_ctxt->api_op == API_OP_DELETE) {
            return route_table_db()->find(&api_ctxt->api_params->route_table_key);
        }
        return route_table_db()->find(&api_ctxt->api_params->route_table_spec.key);

    case OBJ_ID_POLICY:
        if (api_ctxt->api_op == API_OP_DELETE) {
            return policy_db()->find(&api_ctxt->api_params->policy_key);
        }
        return policy_db()->find(&api_ctxt->api_params->policy_spec.key);

    case OBJ_ID_NEXTHOP:
        if (api_ctxt->api_op == API_OP_DELETE) {
            return nexthop_db()->find(&api_ctxt->api_params->nexthop_key);
        }
        return nexthop_db()->find(&api_ctxt->api_params->nexthop_spec.key);

    case OBJ_ID_TAG:
        if (api_ctxt->api_op == API_OP_DELETE) {
            return tag_db()->find(&api_ctxt->api_params->tag_key);
        }
        return tag_db()->find(&api_ctxt->api_params->tag_spec.key);

    case OBJ_ID_NEXTHOP_GROUP:
        if (api_ctxt->api_op == API_OP_DELETE) {
            return nexthop_group_db()->find(&api_ctxt->api_params->nexthop_group_key);
        }
        return nexthop_group_db()->find(&api_ctxt->api_params->nexthop_group_spec.key);

    case OBJ_ID_MAPPING:
    case OBJ_ID_MIRROR_SESSION:
    case OBJ_ID_SVC_MAPPING:
    case OBJ_ID_VPC_PEER:
        return NULL;

    default:
        break;
    }
    return NULL;
}

api_base *
api_base::find_obj(obj_id_t obj_id, void *key) {
    SDK_ASSERT(FALSE);
    return NULL;
}

bool
api_base::stateless(obj_id_t obj_id) {
    switch (obj_id) {
    case OBJ_ID_MAPPING:
    case OBJ_ID_MIRROR_SESSION:
    case OBJ_ID_SVC_MAPPING:
    case OBJ_ID_VPC_PEER:
        return true;

    default:
        break;
    }
    return false;
}

}    // namespace api
