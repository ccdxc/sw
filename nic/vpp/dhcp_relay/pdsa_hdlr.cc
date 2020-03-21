//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//

#include <nic/vpp/infra/cfg/pdsa_db.hpp>
#include "pdsa_vpp_cfg.h"

static sdk::sdk_ret_t
pdsa_dhcp_relay_cfg_set (uint16_t subnet_id, const pds_cfg_msg_t *msg,
                         bool del)
{
    int rc = -1;

    if ((msg->dhcp_policy.spec.relay_spec.server_ip.af != IP_AF_IPV4) ||
        (msg->dhcp_policy.spec.relay_spec.agent_ip.af != IP_AF_IPV4)) {
        return sdk::SDK_RET_INVALID_ARG;
    }

    rc = pds_dhcp4_relay_config_update(subnet_id,
                                       (uint8_t *)msg->dhcp_policy.spec.key.id,
                                       del);

    if (rc == 0) {
        return sdk::SDK_RET_OK;
    }
    return sdk::SDK_RET_ERR;
}

static sdk::sdk_ret_t
pdsa_dhcp_proxy_cfg_set (uint16_t subnet_id, const pds_cfg_msg_t *msg)
{
    static bool dhcp_relay_initialized;
    int rc;

    if ((msg->dhcp_policy.spec.proxy_spec.server_ip.af != IP_AF_IPV4)) {
        return sdk::SDK_RET_INVALID_ARG;
    }

     // Not currently storing this policy. Init dhcp proxy if needed, otherwise
     // do nothing
    if (!dhcp_relay_initialized) {
         // Initialize relay to internal DHCP proxy server
        if (pds_dhcp_relay_init_cb(false) != 0) {
            return SDK_RET_ERR;
        }
        dhcp_relay_initialized = true;
    }

    rc = pds_dhcp4_proxy_config_update(
            subnet_id,
            (uint8_t *)msg->dhcp_policy.spec.key.id,
            false);

    if (rc == 0) {
        return sdk::SDK_RET_OK;
    }
    return sdk::SDK_RET_ERR;
}

static sdk::sdk_ret_t
pdsa_dhcp_proxy_cfg_del (uint16_t subnet_id, const pds_cfg_msg_t *msg)
{
    int rc;

    rc = pds_dhcp4_proxy_config_update(
                    subnet_id,
                    (uint8_t *)msg->dhcp_policy.spec.key.id,
                    true);
    if (rc == 0) {
        return sdk::SDK_RET_OK;
    }
    return sdk::SDK_RET_ERR;
}

static sdk::sdk_ret_t
pdsa_dhcp_policy_cfg_set_internal (uint16_t subnet_id, const pds_cfg_msg_t *msg)
{
    if (msg->dhcp_policy.spec.type == PDS_DHCP_POLICY_TYPE_RELAY) {
        return pdsa_dhcp_relay_cfg_set(subnet_id, msg, false);
    } else if (msg->dhcp_policy.spec.type == PDS_DHCP_POLICY_TYPE_PROXY) {
        return pdsa_dhcp_proxy_cfg_set(subnet_id, msg);
    }
    return sdk::SDK_RET_INVALID_ARG;
}

sdk::sdk_ret_t
pdsa_dhcp_policy_cfg_del_internal (uint16_t subnet_id, const pds_cfg_msg_t *msg)
{
    if (msg->dhcp_policy.spec.type == PDS_DHCP_POLICY_TYPE_RELAY) {
        return pdsa_dhcp_relay_cfg_set(subnet_id, msg, true);
    } else if (msg->dhcp_policy.spec.type == PDS_DHCP_POLICY_TYPE_PROXY) {
        return pdsa_dhcp_proxy_cfg_del(subnet_id, msg);
    }
    return sdk::SDK_RET_INVALID_ARG;
}

sdk::sdk_ret_t
pdsa_dhcp_policy_cfg_set (const pds_cfg_msg_t *msg)
{
    int rc;
    uint16_t vpc = 0;
    pds_cfg_msg_t vpc_msg;
    vpp_config_data &config = vpp_config_data::get();

    if (msg->dhcp_policy.spec.type == PDS_DHCP_POLICY_TYPE_RELAY) {
        if ((msg->dhcp_policy.spec.relay_spec.server_ip.af != IP_AF_IPV4) ||
                (msg->dhcp_policy.spec.relay_spec.agent_ip.af != IP_AF_IPV4)) {
            return sdk::SDK_RET_INVALID_ARG;
        }
        vpc_msg.obj_id = OBJ_ID_VPC;
        vpc_msg.vpc.key = msg->dhcp_policy.spec.relay_spec.vpc;

        if (config.get(vpc_msg)) {
            // if vpc and dhcp conifgs are in same batch then we
            // dont have VPC cfg yet in cfg db. right now there
            // is no support to route dhcp packets in a different
            // vpc, its always underlay vpc. so not treating this
            // as error for now. dela with this when we start supporting
            // dhcp servers in a non-underlay vpc
            vpc = vpc_msg.vpc.status.hw_id;
        }

        rc = pds_dhcp4_server_add((uint8_t *)msg->dhcp_policy.key.id,
                msg->dhcp_policy.spec.relay_spec.server_ip.addr.v4_addr,
                msg->dhcp_policy.spec.relay_spec.agent_ip.addr.v4_addr, vpc);

    } else if (msg->dhcp_policy.spec.type == PDS_DHCP_POLICY_TYPE_PROXY) {
        if (msg->dhcp_policy.spec.proxy_spec.server_ip.af != IP_AF_IPV4) {
            return sdk::SDK_RET_INVALID_ARG;
        }
        rc = pds_dhcp4_server_add((uint8_t *)msg->dhcp_policy.key.id,
                        msg->dhcp_policy.spec.proxy_spec.server_ip.addr.v4_addr,
                        0, 0);
    } else {
        SDK_ASSERT(0);
    }

    if (rc == 0) {
        return sdk::SDK_RET_OK;
    }
    return sdk::SDK_RET_ERR;
}

sdk::sdk_ret_t
pdsa_dhcp_policy_cfg_del (const pds_cfg_msg_t *msg)
{
    int rc;

    rc = pds_dhcp4_server_del((uint8_t *)msg->dhcp_policy.key.id);
    if (rc == 0) {
        return sdk::SDK_RET_OK;
    }
    return sdk::SDK_RET_ERR;
}

static sdk::sdk_ret_t
pdsa_dhcp_policy_cfg_del_all (uint16_t subnet_id)
{
    int rc;

    rc = pds_dhcp4_cfg_add_del_all(subnet_id);
    if (rc == 0) {
        return sdk::SDK_RET_OK;
    }
    return sdk::SDK_RET_ERR;
}

static void
pdsa_dhcp_subnet_cfg (const pds_cfg_msg_t *msg, bool del)
{
    pds_cfg_msg_t dhcp_msg;
    vpp_config_data &config = vpp_config_data::get();
    int i;

    // delete existing servers and add new ones
    (void)pdsa_dhcp_policy_cfg_del_all(msg->subnet.status.hw_id);
    if (del) {
        return;
    }

    for (i = 0; i < msg->subnet.spec.num_dhcp_policy; i++) {
        dhcp_msg.obj_id = OBJ_ID_DHCP_POLICY;
        dhcp_msg.dhcp_policy.key =  msg->subnet.spec.dhcp_policy[i];

        if (!config.get(dhcp_msg)) {
            continue;
        }
        (void)pdsa_dhcp_policy_cfg_set_internal(msg->subnet.status.hw_id, &dhcp_msg);
    }
    return;
}

void
pds_dhcp_relay_cfg_init (void) {
    // initialize callbacks for cfg/oper messages received from pds-agent
    pds_cfg_register_callbacks(OBJ_ID_DHCP_POLICY,
                               pdsa_dhcp_policy_cfg_set,
                               pdsa_dhcp_policy_cfg_del,
                               NULL);

    pds_cfg_register_notify_callbacks(OBJ_ID_SUBNET,
                                      pdsa_dhcp_subnet_cfg);
}
