//
//  {C} Copyright 2019 Pensando Systems Inc. All rights reserved.
//

#include "nic/vpp/infra/cfg/pdsa_db.hpp"
#include "nat_api.h"

extern "C" {

static inline nat_type_t
map_pds_address_type(address_type_t address_type)
{
    if (address_type == ADDR_TYPE_PUBLIC) {
        return NAT_TYPE_INTERNET;
    } else if (address_type == ADDR_TYPE_SERVICE) {
        return NAT_TYPE_INFRA;
    }
    SDK_ASSERT(1 == 0);
}

// callback function to process configuration message. C/U/D
static sdk::sdk_ret_t
pds_nat_cfg_set(const pds_cfg_msg_t *cfg_msg) {
    const pds_nat_port_block_cfg_msg_t *nat_msg;
    nat_type_t nat_type;
    nat_err_t ret;
    uint32_t vpc_hw_id;

    nat_msg = &cfg_msg->nat_port_block;

    SDK_ASSERT(nat_msg->spec.nat_ip_range.af == IP_AF_IPV4);
    SDK_ASSERT(nat_msg->spec.nat_ip_range.ip_lo.v4_addr ==
               nat_msg->spec.nat_ip_range.ip_hi.v4_addr);

    nat_type = map_pds_address_type(nat_msg->spec.address_type);

    // TODO : map nat_msg->spec.vpc to vpc_hw_id
    vpc_hw_id = 0;

    if (cfg_msg->op == API_OP_CREATE) {
        ret = nat_port_block_add((const uint8_t *)nat_msg->key.id, vpc_hw_id,
                                 nat_msg->spec.nat_ip_range.ip_lo.v4_addr,
                                 nat_msg->spec.ip_proto,
                                 nat_msg->spec.nat_port_range.port_lo,
                                 nat_msg->spec.nat_port_range.port_hi,
                                 nat_type);
    } else {
        SDK_ASSERT(cfg_msg->op == API_OP_UPDATE);
        ret = nat_port_block_update((const uint8_t *)nat_msg->key.id, vpc_hw_id,
                                    nat_msg->spec.nat_ip_range.ip_lo.v4_addr,
                                    nat_msg->spec.ip_proto,
                                    nat_msg->spec.nat_port_range.port_lo,
                                    nat_msg->spec.nat_port_range.port_hi,
                                    nat_type);
    }
    if (ret == NAT_ERR_OK) {
        return sdk::SDK_RET_OK;
    } else {
        return sdk::SDK_RET_ERR;
    }
}

// callback function for deleting a nat port block entry
static sdk::sdk_ret_t
pds_nat_cfg_del(const pds_cfg_msg_t *cfg_msg) {
    const pds_nat_port_block_cfg_msg_t *nat_msg;
    nat_type_t nat_type;
    nat_err_t ret;
    uint32_t vpc_hw_id;

    nat_msg = &cfg_msg->nat_port_block;

    SDK_ASSERT(nat_msg->spec.nat_ip_range.af == IP_AF_IPV4);
    SDK_ASSERT(nat_msg->spec.nat_ip_range.ip_lo.v4_addr ==
               nat_msg->spec.nat_ip_range.ip_hi.v4_addr);

    // TODO : map nat_msg->spec.vpc to vpc_hw_id
    vpc_hw_id = 0;

    nat_type = map_pds_address_type(nat_msg->spec.address_type);

    ret = nat_port_block_del((const uint8_t *)nat_msg->key.id, vpc_hw_id,
                             nat_msg->spec.nat_ip_range.ip_lo.v4_addr,
                             nat_msg->spec.ip_proto,
                             nat_msg->spec.nat_port_range.port_lo,
                             nat_msg->spec.nat_port_range.port_hi,
                             nat_type);
    if (ret == NAT_ERR_OK) {
        return sdk::SDK_RET_OK;
    } else {
        return sdk::SDK_RET_ERR;
    }
}

// callback function for commiting config
static sdk::sdk_ret_t
pds_nat_cfg_act(const pds_cfg_msg_t *cfg_msg) {
    const pds_nat_port_block_cfg_msg_t *nat_msg;
    nat_type_t nat_type;
    nat_err_t ret;
    uint32_t vpc_hw_id;

    nat_msg = &cfg_msg->nat_port_block;

    SDK_ASSERT(nat_msg->spec.nat_ip_range.af == IP_AF_IPV4);
    SDK_ASSERT(nat_msg->spec.nat_ip_range.ip_lo.v4_addr ==
               nat_msg->spec.nat_ip_range.ip_hi.v4_addr);

    nat_type = map_pds_address_type(nat_msg->spec.address_type);

    // TODO : map nat_msg->spec.vpc to vpc_hw_id
    vpc_hw_id = 0;

    ret = nat_port_block_commit((const uint8_t *)nat_msg->key.id, vpc_hw_id,
                                nat_msg->spec.nat_ip_range.ip_lo.v4_addr,
                                nat_msg->spec.ip_proto,
                                nat_msg->spec.nat_port_range.port_lo,
                                nat_msg->spec.nat_port_range.port_hi,
                                nat_type);

    if (ret == NAT_ERR_OK) {
        return sdk::SDK_RET_OK;
    } else {
        return sdk::SDK_RET_ERR;
    }
}

// initialize callbacks for NAT configuration
//
// Note: This is called from C code, and must have C linkage
//
void
pds_nat_cfg_init(void) {
    pds_cfg_register_callbacks(OBJ_ID_NAT_PORT_BLOCK,
                               pds_nat_cfg_set,
                               pds_nat_cfg_del,
                               pds_nat_cfg_act);
}

} // extern "C"
