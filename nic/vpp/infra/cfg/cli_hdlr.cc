//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//
#include <nic/sdk/include/sdk/eth.hpp>
#include <nic/apollo/api/include/pds_vpc.hpp>
#include "pdsa_db.hpp"
#include "cli_hdlr.h"

#ifdef __cplusplus
extern "C" {
#endif

int
impl_db_dump (char *obj)
{
    vpp_config_batch config_batch = vpp_config_batch::get();

    if (strcmp(obj, "vpc") == 0) {
        config_batch.dump(OBJ_ID_VPC);
    } else if (strcmp(obj, "subnet") == 0) {
        config_batch.dump(OBJ_ID_SUBNET);
    } else if (strcmp(obj, "device") == 0) {
        config_batch.dump(OBJ_ID_DEVICE);
    } else if (strcmp(obj, "vnic") == 0) {
        config_batch.dump(OBJ_ID_VNIC);
    }

    return 0;
}

static void
vpc_cfg_entry_dump (pds_cfg_msg_t *msg, void *buf)
{
    char type[10];

    if (msg->vpc.spec.type == PDS_VPC_TYPE_UNDERLAY) {
        strcpy(type,"UNDERLAY");
    } else if (msg->vpc.spec.type == PDS_VPC_TYPE_TENANT) {
        strcpy(type,"TENANT");
    } else if (msg->vpc.spec.type == PDS_VPC_TYPE_CONTROL) {
        strcpy(type,"CONTROL");
    } else {
        strcpy(type,"NONE");
    }

    snprintf((char *)buf, 124, "%-40s%-10s%-40s%-20s%-14s\n",
             msg->vpc.spec.key.str(), type, msg->vpc.spec.v4_route_table.str(),
             macaddr2str(msg->vpc.spec.vr_mac),
             pds_encap2str(&(msg->vpc.spec.fabric_encap)));

    buf_entry_dump(buf);

    return;
}

static void
subnet_cfg_entry_dump (pds_cfg_msg_t *msg, void *buf)
{
    int len = 40 * PDS_MAX_SUBNET_DHCP_POLICY;
    char dhcp_buf[len];

    memset(dhcp_buf, 0, len);
    for (int i = 0; i < msg->subnet.spec.num_dhcp_policy; i++) {
        sprintf(dhcp_buf + strlen(dhcp_buf), "%-40s",
                msg->subnet.spec.dhcp_policy[i].str());
    }

    for (int i = msg->subnet.spec.num_dhcp_policy;
         i < PDS_MAX_SUBNET_DHCP_POLICY; i++) {
        sprintf(dhcp_buf + strlen(dhcp_buf), "%-40s", "-");
    }

    snprintf((char *)buf, 420, "%-40s%-40s%-40s%-20s%-16s%-20s%-40s%-s%-3d\n\n",
             msg->subnet.spec.key.str(),
             msg->subnet.spec.vpc.str(), msg->subnet.spec.host_if.str(),
             ipv4pfx2str(&(msg->subnet.spec.v4_prefix)),
             ipv4addr2str(msg->subnet.spec.v4_vr_ip),
             macaddr2str(msg->subnet.spec.vr_mac),
             msg->subnet.spec.v4_route_table.str(),
             dhcp_buf, msg->subnet.spec.tos);

    buf_entry_dump(buf);

    return;
}

static void
vnic_cfg_entry_dump (pds_cfg_msg_t *msg, void *buf)
{
    char switch_vnic[6];

    if (msg->vnic.spec.switch_vnic) {
        strcpy(switch_vnic, "TRUE");
    } else {
        strcpy(switch_vnic, "FALSE");
    }
    snprintf((char *)buf, 219, "%-40s%-40s%-14s%-20s%-14s%-20x%-20x%-11s"
             "%-40s\n\n", msg->vnic.spec.key.str(), msg->vnic.spec.subnet.str(),
             pds_encap2str(&(msg->vnic.spec.vnic_encap)),
             macaddr2str(msg->vnic.spec.mac_addr),
             pds_encap2str(&(msg->vnic.spec.fabric_encap)),
             msg->vnic.spec.rx_mirror_session_bmap,
             msg->vnic.spec.tx_mirror_session_bmap, switch_vnic,
             msg->vnic.spec.host_if.str());

    buf_entry_dump(buf);

    return;
}

static void
device_cfg_entry_dump (pds_cfg_msg_t *msg, void *buf)
{
    char dev_profile[30];
    char bridge_enable[7];
    char learn_enable[6];
    char oren[17];
    char oper_mode[5];
    char flags[33];

    if (msg->device.spec.spec.bridging_en) {
        strcpy(bridge_enable, "BRIDGE");
    } else {
        strcpy(bridge_enable, "-");
    }

    if (msg->device.spec.spec.learning_en) {
        strcpy(learn_enable, "LEARN");
    } else {
        strcpy(learn_enable, "-");
    }

    if (msg->device.spec.spec.overlay_routing_en) {
        strcpy(oren, "OVERLAY ROUTING");
    } else {
        strcpy(oren, "-");
    }

    if (msg->device.spec.spec.device_profile == 0) {
        sprintf(dev_profile, "PDS_DEVICE_PROFILE_DEFAULT");
    } else {
        sprintf(dev_profile, "PDS_DEVICE_PROFILE_%dPF",
                msg->device.spec.spec.memory_profile +1);
    }

    if (msg->device.spec.spec.dev_oper_mode == 0) {
        strcpy(oper_mode, "NONE");
    } else if (msg->device.spec.spec.dev_oper_mode == 1) {
        strcpy(oper_mode, "BITW");
    } else {
        strcpy(oper_mode, "HOST");
    }

    sprintf(flags, "%s/%s/%s", bridge_enable, learn_enable, oren);
    snprintf((char *)buf, 163, "%-16s%-20s%-16s%-12s%-30s%-16d%-10s%-33s\n",
             ipaddr2str(&(msg->device.spec.spec.device_ip_addr)),
             macaddr2str(msg->device.spec.spec.device_mac_addr),
             ipaddr2str(&(msg->device.spec.spec.gateway_ip_addr)), "DEFAULT",
             dev_profile,  msg->device.spec.spec.learn_age_timeout, oper_mode,
             flags);

    buf_entry_dump(buf);

    return;
}

static void
nat_cfg_entry_dump (pds_cfg_msg_t *msg, void *buf)
{
    snprintf((char *)buf, 184, "%-40s%-40s%-20s%-20s%-10d%-10d%-10d%-10d"
             "%-10d\n", msg->nat_port_block.spec.key.str(),
             msg->nat_port_block.spec.vpc.str(),
             ipv4addr2str(msg->nat_port_block.spec.nat_ip_range.ip_lo.v4_addr),
             ipv4addr2str(msg->nat_port_block.spec.nat_ip_range.ip_hi.v4_addr),
             msg->nat_port_block.spec.ip_proto,
             msg->nat_port_block.spec.nat_port_range.port_lo,
             msg->nat_port_block.spec.nat_port_range.port_hi,
             msg->nat_port_block.stats.in_use_count,
             msg->nat_port_block.stats.session_count);

    buf_entry_dump(buf);

    return;
}

static void
dhcp_proxy_cfg_entry_dump (pds_cfg_msg_t *msg, void *buf)
{
    if (msg->dhcp_policy.spec.type != PDS_DHCP_POLICY_TYPE_PROXY) {
        return;
    }
    snprintf((char *)buf, 274, "%-40s%-20s%-10d%-20s%-20s%-20s%-14d%-130s\n",
             msg->dhcp_policy.spec.key.str(),
             ipaddr2str(&(msg->dhcp_policy.spec.proxy_spec.server_ip)),
             msg->dhcp_policy.spec.proxy_spec.mtu,
             ipaddr2str(&(msg->dhcp_policy.spec.proxy_spec.gateway_ip)),
             ipaddr2str(&(msg->dhcp_policy.spec.proxy_spec.dns_server_ip)),
             ipaddr2str(&(msg->dhcp_policy.spec.proxy_spec.ntp_server_ip)),
             msg->dhcp_policy.spec.proxy_spec.lease_timeout,
             msg->dhcp_policy.spec.proxy_spec.domain_name);

    buf_entry_dump(buf);

    return;
}

static void
dhcp_relay_cfg_entry_dump (pds_cfg_msg_t *msg, void *buf)
{
    if (msg->dhcp_policy.spec.type != PDS_DHCP_POLICY_TYPE_RELAY) {
        return;
    }
    snprintf((char *)buf, 120, "%-40s%-40s%-20s%-20s\n",
             msg->dhcp_policy.key.str(),
             msg->dhcp_policy.spec.relay_spec.vpc.str(),
             ipaddr2str(&(msg->dhcp_policy.spec.relay_spec.server_ip)),
             ipaddr2str(&(msg->dhcp_policy.spec.relay_spec.agent_ip)));

    buf_entry_dump(buf);

    return;
}

int
dhcp_cfg_db_dump_hdlr (void *buf, uint8_t proxy)
{
    pds_cfg_msg_t cfg_msg;
    vpp_config_data &config = vpp_config_data::get();;

    cfg_msg.obj_id = OBJ_ID_DHCP_POLICY;
    if (proxy) {
        config.walk(cfg_msg, &dhcp_proxy_cfg_entry_dump, buf);
    } else {
        config.walk(cfg_msg, &dhcp_relay_cfg_entry_dump, buf);
    }
    return 0;
}

static void
security_profile_cfg_entry_dump (pds_cfg_msg_t *msg, void *buf)
{
    char con_en[20];
    char action[5];
    if (msg->security_profile.spec.conn_track_en) {
        strcpy(con_en, "CONNECTION TRACKING");
    } else {
        strcpy(con_en, "-");
    }

    if (msg->security_profile.spec.default_action.fw_action.action ==
        SECURITY_RULE_ACTION_DENY ) {
        strcpy(action, "DENY");
    } else {
        strcpy(action, "ALLOW");
    }
    snprintf((char *)buf, 300, "%-40s%-20s%-20s%-20d%-20d%-20d%-20d%-20d%-20d"
             "%-20d%-20d%-20d%-20d%-20d\n",
             msg->security_profile.spec.key.str(), con_en, action,
             msg->security_profile.spec.tcp_idle_timeout,
             msg->security_profile.spec.udp_idle_timeout,
             msg->security_profile.spec.icmp_idle_timeout,
             msg->security_profile.spec.other_idle_timeout,
             msg->security_profile.spec.tcp_syn_timeout,
             msg->security_profile.spec.tcp_halfclose_timeout,
             msg->security_profile.spec.tcp_close_timeout,
             msg->security_profile.spec.tcp_drop_timeout,
             msg->security_profile.spec.udp_drop_timeout,
             msg->security_profile.spec.icmp_drop_timeout,
             msg->security_profile.spec.other_drop_timeout);

    buf_entry_dump(buf);

    return;
}

#define _(obj, OBJ)                                            \
int                                                            \
obj##_cfg_db_dump_hdlr (void *buf)                             \
{                                                              \
    pds_cfg_msg_t cfg_msg;                                     \
    vpp_config_data &config = vpp_config_data::get();;         \
    cfg_msg.obj_id = OBJ_ID_##OBJ;                             \
    config.walk(cfg_msg, &obj##_cfg_entry_dump, (void *)buf);  \
                                                               \
    return 0;                                                  \
}                                                              \

    _(vnic, VNIC)
    _(device, DEVICE)
    _(vpc, VPC)
    _(subnet, SUBNET)
    _(nat, NAT_PORT_BLOCK)
    _(security_profile, SECURITY_PROFILE)

#undef _

#ifdef __cplusplus
}
#endif
