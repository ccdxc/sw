//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//

#include <nic/vpp/infra/cfg/pdsa_db.hpp>
#include "pdsa_impl_db_hdlr.h"

static sdk::sdk_ret_t
pds_cfg_db_subnet_set_cb (const pds_cfg_msg_t *msg)
{
    int rc;

    rc = pds_impl_db_subnet_set(msg->subnet.spec.v4_vr_ip,
                                msg->subnet.spec.v4_prefix.len,
                                (uint8_t *) msg->subnet.spec.vr_mac,
                                msg->subnet.status.hw_id);

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

    rc = pds_impl_db_vnic_set((uint8_t *)msg->vnic.spec.mac_addr,
                              msg->vnic.spec.max_sessions,
                              msg->vnic.status.hw_id,
                              subnet_hw_id);

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

void
pds_impl_db_cb_register (void)
{
#define _(_OBJ,_obj)                                        \
    pds_cfg_register_callbacks(OBJ_ID_##_OBJ,               \
                               pds_cfg_db_##_obj##_set_cb,  \
                               pds_cfg_db_##_obj##_del_cb,  \
                               NULL);

    _(VNIC, vnic)
    _(SUBNET, subnet)
#undef _

    return;
}

int
pds_cfg_db_init (void)
{
    pds_impl_db_cb_register();

    return 0;
}
