//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//

#include <nic/vpp/infra/cfg/pdsa_db.hpp>
#include "pdsa_vpp_cfg.h"

static sdk::sdk_ret_t
pdsa_dhcp_relay_cfg_set (const pds_cfg_msg_t *msg) {

    int rc;

    if ((msg->dhcp_relay.spec.server_ip.af != IP_AF_IPV4) ||
        (msg->dhcp_relay.spec.agent_ip.af != IP_AF_IPV4)) {
        return sdk::SDK_RET_INVALID_ARG;
    }

    rc = pds_dhcp4_relay_config_update(
        msg->dhcp_relay.spec.server_ip.addr.v4_addr,
        msg->dhcp_relay.spec.server_ip.addr.v4_addr,
        false);

    if (rc == 0) {
        return sdk::SDK_RET_OK;
    } else {
        return sdk::SDK_RET_ERR;
    }
}

static sdk::sdk_ret_t
pdsa_dhcp_relay_cfg_del (const pds_cfg_msg_t *msg) {
    int rc;

    if ((msg->dhcp_relay.spec.server_ip.af != IP_AF_IPV4) ||
        (msg->dhcp_relay.spec.agent_ip.af != IP_AF_IPV4)) {
        return sdk::SDK_RET_INVALID_ARG;
    }

    rc = pds_dhcp4_relay_config_update(
        msg->dhcp_relay.spec.server_ip.addr.v4_addr,
        msg->dhcp_relay.spec.server_ip.addr.v4_addr,
        true);

    if (rc == 0) {
        return sdk::SDK_RET_OK;
    } else {
        return sdk::SDK_RET_ERR;
    }
}

void
pds_dhcp_relay_cfg_init (void) {
    // initialize callbacks for cfg/oper messages received from pds-agent

    pds_cfg_register_callbacks(OBJ_ID_DHCP_RELAY,
                               pdsa_dhcp_relay_cfg_set, 
                               pdsa_dhcp_relay_cfg_del,
                               NULL);
}
