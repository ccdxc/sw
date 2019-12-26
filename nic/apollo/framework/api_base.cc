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

    case OBJ_ID_POLICER:
        return policer_entry::factory(&api_ctxt->api_params->policer_spec);

    case OBJ_ID_NAT_PORT_BLOCK:
        return nat_port_block::factory(&api_ctxt->api_params->nat_port_block_spec);

    case OBJ_ID_DHCP_RELAY:
        return dhcp_relay::factory(&api_ctxt->api_params->dhcp_relay_spec);

    case OBJ_ID_DHCP_POLICY:
        return dhcp_policy::factory(&api_ctxt->api_params->dhcp_policy_spec);

    case OBJ_ID_SECURITY_PROFILE:
        return security_profile::factory(&api_ctxt->api_params->security_profile_spec);

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

    case OBJ_ID_NAT_PORT_BLOCK:
        // NAT port block is a stateless object, so we need to build it on the
        // fly
        if (api_ctxt->api_op == API_OP_DELETE) {
            return nat_port_block::build(&api_ctxt->api_params->nat_port_block_key);
        }
        return nat_port_block::build(&api_ctxt->api_params->nat_port_block_spec.key);

    case OBJ_ID_DHCP_RELAY:
        // DHCP relay is a stateless object, so we need to build it on the fly
        if (api_ctxt->api_op == API_OP_DELETE) {
            return dhcp_relay::build(&api_ctxt->api_params->dhcp_relay_key);
        }
        return dhcp_relay::build(&api_ctxt->api_params->dhcp_relay_spec.key);

    case OBJ_ID_DHCP_POLICY:
        // DHCP policy is a stateless object, so we need to build it on the fly
        if (api_ctxt->api_op == API_OP_DELETE) {
            return dhcp_policy::build(&api_ctxt->api_params->dhcp_policy_key);
        }
        return dhcp_policy::build(&api_ctxt->api_params->dhcp_policy_spec.key);

    case OBJ_ID_SECURITY_PROFILE:
        // security profile is a stateless object, so we need to build it on
        // the fly
        if (api_ctxt->api_op == API_OP_DELETE) {
            return security_profile::build(&api_ctxt->api_params->security_profile_key);
        }
        return security_profile::build(&api_ctxt->api_params->security_profile_spec.key);

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

    case OBJ_ID_NAT_PORT_BLOCK:
        nat_port_block::soft_delete((nat_port_block *)api_obj);
        break;

    case OBJ_ID_DHCP_RELAY:
        dhcp_relay::soft_delete((dhcp_relay *)api_obj);
        break;

    case OBJ_ID_DHCP_POLICY:
        dhcp_policy::soft_delete((dhcp_policy *)api_obj);
        break;

    case OBJ_ID_SECURITY_PROFILE:
        security_profile::soft_delete((security_profile *)api_obj);
        break;

    default:
        PDS_TRACE_ERR("Non-statless obj %u can't be soft deleted", obj_id);
        break;
    }
}

sdk_ret_t
api_base::free(obj_id_t obj_id, api_base *api_obj) {
    switch (obj_id) {
    case OBJ_ID_DEVICE:
        return device_entry::free((device_entry *)api_obj);

    case OBJ_ID_IF:
        return if_entry::free((if_entry *)api_obj);

    case OBJ_ID_VPC:
        return vpc_entry::free((vpc_entry *)api_obj);

    case OBJ_ID_SUBNET:
        return subnet_entry::free((subnet_entry *)api_obj);

    case OBJ_ID_TEP:
        return tep_entry::free((tep_entry *)api_obj);

    case OBJ_ID_VNIC:
        return vnic_entry::free((vnic_entry *)api_obj);

    case OBJ_ID_MAPPING:
        return mapping_entry::free((mapping_entry *)api_obj);

    case OBJ_ID_ROUTE_TABLE:
        return route_table::free((route_table *)api_obj);

    case OBJ_ID_POLICY:
        return policy::free((policy *)api_obj);

    case OBJ_ID_MIRROR_SESSION:
        return mirror_session::free((mirror_session *)api_obj);

    case OBJ_ID_METER:
        return meter_entry::free((meter_entry *)api_obj);

    case OBJ_ID_TAG:
        return tag_entry::free((tag_entry *)api_obj);

    case OBJ_ID_SVC_MAPPING:
        return svc_mapping::free((svc_mapping *)api_obj);

    case OBJ_ID_VPC_PEER:
        return vpc_peer_entry::free((vpc_peer_entry *)api_obj);

    case OBJ_ID_NEXTHOP:
        return nexthop::free((nexthop *)api_obj);

    case OBJ_ID_NEXTHOP_GROUP:
        return nexthop_group::free((nexthop_group *)api_obj);

    case OBJ_ID_POLICER:
        return policer_entry::free((policer_entry *)api_obj);

    case OBJ_ID_NAT_PORT_BLOCK:
        return nat_port_block::free((nat_port_block *)api_obj);

    case OBJ_ID_DHCP_RELAY:
        return dhcp_relay::free((dhcp_relay *)api_obj);

    case OBJ_ID_DHCP_POLICY:
        return dhcp_policy::free((dhcp_policy *)api_obj);

    case OBJ_ID_SECURITY_PROFILE:
        return security_profile::free((security_profile *)api_obj);

    default:
        break;
    }
    return SDK_RET_INVALID_OP;
}

api_base *
api_base::find_obj(api_ctxt_t *api_ctxt) {
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
        if (api_ctxt->api_op == API_OP_DELETE) {
            return mapping_db()->find(&api_ctxt->api_params->mapping_key);
        }
        return mapping_db()->find(&api_ctxt->api_params->mapping_spec.key);

    case OBJ_ID_SVC_MAPPING:
        if (api_ctxt->api_op == API_OP_DELETE) {
            return svc_mapping_db()->find(&api_ctxt->api_params->svc_mapping_key);
        }
        return svc_mapping_db()->find(&api_ctxt->api_params->svc_mapping_spec.key);

    case OBJ_ID_POLICER:
        if (api_ctxt->api_op == API_OP_DELETE) {
            return policer_db()->find(&api_ctxt->api_params->policer_key);
        }
        return policer_db()->find(&api_ctxt->api_params->policer_spec.key);

    case OBJ_ID_NAT_PORT_BLOCK:
        if (api_ctxt->api_op == API_OP_DELETE) {
            return nat_db()->find(&api_ctxt->api_params->nat_port_block_key);
        }
        return nat_db()->find(&api_ctxt->api_params->nat_port_block_spec.key);

    case OBJ_ID_DHCP_RELAY:
        if (api_ctxt->api_op == API_OP_DELETE) {
            return dhcp_db()->find(&api_ctxt->api_params->dhcp_relay_key);
        }
        return dhcp_db()->find(&api_ctxt->api_params->dhcp_relay_spec.key);

    case OBJ_ID_DHCP_POLICY:
        if (api_ctxt->api_op == API_OP_DELETE) {
            return dhcp_db()->find(&api_ctxt->api_params->dhcp_policy_key);
        }
        return dhcp_db()->find(&api_ctxt->api_params->dhcp_policy_spec.key);

    case OBJ_ID_SECURITY_PROFILE:
        if (api_ctxt->api_op == API_OP_DELETE) {
            return policy_db()->find(&api_ctxt->api_params->security_profile_key);
        }
        return policy_db()->find(&api_ctxt->api_params->security_profile_spec.key);

    case OBJ_ID_MIRROR_SESSION:
    case OBJ_ID_VPC_PEER:
        return NULL;

    default:
        break;
    }
    return NULL;
}

api_base *
api_base::find_obj(obj_id_t obj_id, void *key) {
    api_base *api_obj = NULL;

    switch (obj_id) {
    case OBJ_ID_DEVICE:
        api_obj = device_db()->find();
        break;

    case OBJ_ID_IF:
        api_obj = if_db()->find((pds_if_key_t *)key);
        break;

    case OBJ_ID_VPC:
        api_obj = vpc_db()->find((pds_vpc_key_t *)key);
        break;

    case OBJ_ID_SUBNET:
        api_obj = subnet_db()->find((pds_subnet_key_t *)key);
        break;

    case OBJ_ID_TEP:
        api_obj = tep_db()->find((pds_tep_key_t *)key);
        break;

    case OBJ_ID_VNIC:
        api_obj = vnic_db()->find((pds_vnic_key_t *)key);
        break;

    case OBJ_ID_MAPPING:
        api_obj = mapping_db()->find((pds_mapping_key_t *)key);
        break;

    case OBJ_ID_ROUTE_TABLE:
        api_obj = route_table_db()->find((pds_route_table_key_t *)key);
        break;

    case OBJ_ID_POLICY:
        api_obj = policy_db()->find((pds_policy_key_t *)key);
        break;

    case OBJ_ID_MIRROR_SESSION:
    case OBJ_ID_METER:
        PDS_TRACE_ERR("find_obj() no implemented for obj id %u\n", obj_id);
        break;

    case OBJ_ID_TAG:
        api_obj = tag_db()->find((pds_tag_key_t *)key);
        break;

    case OBJ_ID_SVC_MAPPING:
        api_obj = svc_mapping_db()->find((pds_svc_mapping_key_t *)key);
        break;

    case OBJ_ID_VPC_PEER:
        PDS_TRACE_ERR("find_obj() no implemented for obj id %u\n", obj_id);
        break;

    case OBJ_ID_NEXTHOP:
        api_obj = nexthop_db()->find((pds_nexthop_key_t *)key);
        break;

    case OBJ_ID_NEXTHOP_GROUP:
        api_obj = nexthop_group_db()->find((pds_nexthop_group_key_t *)key);
        break;

    case OBJ_ID_POLICER:
        api_obj = policer_db()->find((pds_policer_key_t *)key);
        break;

    case OBJ_ID_NAT_PORT_BLOCK:
        api_obj = nat_db()->find((pds_nat_port_block_key_t *)key);
        break;

    case OBJ_ID_DHCP_RELAY:
        api_obj = dhcp_db()->find((pds_dhcp_relay_key_t *)key);
        break;

    case OBJ_ID_DHCP_POLICY:
        api_obj = dhcp_db()->find((pds_dhcp_policy_key_t *)key);
        break;

    case OBJ_ID_SECURITY_PROFILE:
        api_obj = policy_db()->find((pds_security_profile_key_t *)key);
        break;

    default:
        PDS_TRACE_ERR("find_obj() no implemented for obj id %u\n", obj_id);
        break;
    }

    if (api_obj) {
        return api_framework_obj(api_obj);
    }
    return NULL;
}

bool
api_base::stateless(obj_id_t obj_id) {
    switch (obj_id) {
    case OBJ_ID_MAPPING:
    case OBJ_ID_MIRROR_SESSION:
    case OBJ_ID_SVC_MAPPING:
    case OBJ_ID_VPC_PEER:
    case OBJ_ID_NAT_PORT_BLOCK:
    case OBJ_ID_DHCP_RELAY:
    case OBJ_ID_DHCP_POLICY:
    case OBJ_ID_SECURITY_PROFILE:
        return true;

    default:
        break;
    }
    return false;
}

bool
api_base::circulate(obj_id_t obj_id) {
    switch (obj_id) {
    case OBJ_ID_SUBNET:
    case OBJ_ID_VNIC:
    case OBJ_ID_NAT_PORT_BLOCK:
    case OBJ_ID_DHCP_RELAY:
    case OBJ_ID_DHCP_POLICY:
    case OBJ_ID_SECURITY_PROFILE:
        return true;
    default:
        break;
    }
    return false;
}

}    // namespace api
