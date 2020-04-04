//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//

#include <nic/vpp/infra/cfg/pdsa_db.hpp>
#include "pdsa_impl_db_hdlr.h"

#ifdef __cplusplus
extern "C" {
#endif

#include "cli.h"

static sdk::sdk_ret_t
pds_cfg_db_subnet_set_cb (const pds_cfg_msg_t *msg)
{
    int rc;

    rc = pds_impl_db_subnet_set(msg->subnet.spec.v4_prefix.len,
                                msg->subnet.spec.v4_vr_ip,
                                (uint8_t *) msg->subnet.spec.vr_mac,
                                msg->subnet.status.hw_id,
                                msg->subnet.spec.fabric_encap.val.vnid);

    if (rc == 0) {
        return sdk::SDK_RET_OK;
    } else {
        return sdk::SDK_RET_ERR;
    }
}

static sdk::sdk_ret_t
pds_cfg_db_subnet_del_cb (const pds_cfg_msg_t *msg)
{
    int rc;

    rc = pds_impl_db_subnet_del(msg->subnet.status.hw_id);

    if (rc == 0) {
        return sdk::SDK_RET_OK;
    } else {
        return sdk::SDK_RET_ERR;
    }
}

static sdk::sdk_ret_t
pds_cfg_db_vnic_set_cb (const pds_cfg_msg_t *msg)
{
    int rc;
    uint16_t subnet_hw_id = 0xffff;
    pds_cfg_msg_t subnet_msg;
    uint8_t dot1q = 0,
            dot1ad = 0;

    vpp_config_data &config = vpp_config_data::get();

    subnet_msg.obj_id = OBJ_ID_SUBNET;
    subnet_msg.subnet.key =  msg->vnic.spec.subnet;

    if (!config.exists(subnet_msg)) {
        return sdk::SDK_RET_ERR;
    }

    if (!config.get(subnet_msg)) {
        return sdk::SDK_RET_ERR;
    }
    subnet_hw_id = subnet_msg.subnet.status.hw_id;

    if (msg->vnic.spec.vnic_encap.type == PDS_ENCAP_TYPE_DOT1Q) {
        dot1q = 1;
    } else if (msg->vnic.spec.vnic_encap.type == PDS_ENCAP_TYPE_QINQ) {
        dot1ad = 1;
    }
    rc = pds_impl_db_vnic_set((uint8_t *)msg->vnic.spec.mac_addr,
                              msg->vnic.spec.max_sessions,
                              msg->vnic.status.hw_id,
                              subnet_hw_id,
                              msg->vnic.spec.flow_learn_en,
                              dot1q, dot1ad,
                              msg->vnic.spec.vnic_encap.val.vlan_tag,
                              msg->vnic.status.nh_hw_id);

    if (rc == 0) {
        return sdk::SDK_RET_OK;
    } else {
        return sdk::SDK_RET_ERR;
    }
}

static sdk::sdk_ret_t
pds_cfg_db_vnic_del_cb (const pds_cfg_msg_t *msg)
{
    int rc;

    rc = pds_impl_db_vnic_del(msg->vnic.status.hw_id);

    if (rc == 0) {
        return sdk::SDK_RET_OK;
    } else {
        return sdk::SDK_RET_ERR;
    }
}

static sdk::sdk_ret_t
pds_cfg_db_vpc_set_cb (const pds_cfg_msg_t *msg)
{
    int rc;
    uint16_t flags = 0;

    if (msg->vpc.spec.type == PDS_VPC_TYPE_CONTROL) {
        flags |= PDS_VPP_VPC_FLAGS_CONTROL_VPC;
    }
    rc = pds_impl_db_vpc_set(msg->vpc.status.hw_id, msg->vpc.status.bd_hw_id, flags);

    if (rc == 0) {
        return sdk::SDK_RET_OK;
    } else {
        return sdk::SDK_RET_ERR;
    }
}

static sdk::sdk_ret_t
pds_cfg_db_vpc_del_cb (const pds_cfg_msg_t *msg)
{
    int rc;

    rc = pds_impl_db_vpc_del(msg->vpc.status.hw_id);

    if (rc == 0) {
        return sdk::SDK_RET_OK;
    } else {
        return sdk::SDK_RET_ERR;
    }
}

static sdk::sdk_ret_t
pds_cfg_db_device_set_cb (const pds_cfg_msg_t *msg)
{
    int rc;
    const pds_device_spec_t *spec = &msg->device.spec.spec;

    rc = pds_impl_db_device_set(msg->device.spec.spec.device_mac_addr,
                                (const uint8_t *) &spec->device_ip_addr.addr,
                                (spec->device_ip_addr.af == IP_AF_IPV4) ? 1:0,
                                spec->overlay_routing_en);

    if (rc == 0) {
        return sdk::SDK_RET_OK;
    } else {
        return sdk::SDK_RET_ERR;
    }
}

static sdk::sdk_ret_t
pds_cfg_db_device_del_cb (const pds_cfg_msg_t *msg)
{
    int rc;

    rc = pds_impl_db_device_del();

    if (rc == 0) {
        return sdk::SDK_RET_OK;
    } else {
        return sdk::SDK_RET_ERR;
    }
}

#define _(obj)                          \
static sdk::sdk_ret_t                   \
pds_cfg_db_##obj##_dump_cb ()           \
{                                       \
    int rc;                             \
                                        \
    rc = obj##_impl_db_dump();          \
    if (rc == 0) {                      \
        return sdk::SDK_RET_OK;         \
    } else {                            \
        return sdk::SDK_RET_ERR;        \
    }                                   \
}

_(subnet)
_(vnic)
_(vpc)
_(device)
#undef _

void
pds_impl_db_cb_register (void)
{
#define _(_OBJ,_obj)                                        \
    pds_cfg_register_callbacks(OBJ_ID_##_OBJ,               \
                               pds_cfg_db_##_obj##_set_cb,  \
                               pds_cfg_db_##_obj##_del_cb,  \
                               NULL, NULL,                  \
                               pds_cfg_db_##_obj##_dump_cb);

    _(VNIC, vnic)
    _(SUBNET, subnet)
    _(VPC, vpc)
    _(DEVICE, device)
#undef _

    return;
}

int
pds_cfg_db_init (void)
{
    pds_impl_db_cb_register();

    return 0;
}

#ifdef __cplusplus
}

#endif
