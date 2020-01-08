//
//  {C} Copyright 2019 Pensando Systems Inc. All rights reserved.
//

#include "nic/vpp/infra/ipc/pdsa_hdlr.hpp"
#include "nic/vpp/infra/ipc/pdsa_vpp_hdlr.h"
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

    nat_msg = &cfg_msg->nat_port_block;

    SDK_ASSERT(nat_msg->spec.nat_ip_range.af == IP_AF_IPV4);
    SDK_ASSERT(nat_msg->spec.nat_ip_range.ip_lo.v4_addr ==
               nat_msg->spec.nat_ip_range.ip_hi.v4_addr);

    nat_type = map_pds_address_type(nat_msg->spec.address_type);

    if (cfg_msg->op == API_OP_CREATE) {
        ret = nat_port_block_add(nat_msg->key.id, nat_msg->spec.vpc.id,
                                 nat_msg->spec.nat_ip_range.ip_lo.v4_addr,
                                 nat_msg->spec.ip_proto,
                                 nat_msg->spec.nat_port_range.port_lo,
                                 nat_msg->spec.nat_port_range.port_hi,
                                 nat_type);
    } else {
        SDK_ASSERT(cfg_msg->op == API_OP_UPDATE);
        ret = nat_port_block_update(nat_msg->key.id, nat_msg->spec.vpc.id,
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

    nat_msg = &cfg_msg->nat_port_block;

    SDK_ASSERT(nat_msg->spec.nat_ip_range.af == IP_AF_IPV4);
    SDK_ASSERT(nat_msg->spec.nat_ip_range.ip_lo.v4_addr ==
               nat_msg->spec.nat_ip_range.ip_hi.v4_addr);

    nat_type = map_pds_address_type(nat_msg->spec.address_type);

    ret = nat_port_block_del(nat_msg->key.id, nat_msg->spec.vpc.id,
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
pds_nat_cfg_process(const pds_cfg_msg_t *cfg_msg) {
    const pds_nat_port_block_cfg_msg_t *nat_msg;
    nat_type_t nat_type;
    nat_err_t ret;

    nat_msg = &cfg_msg->nat_port_block;

    SDK_ASSERT(nat_msg->spec.nat_ip_range.af == IP_AF_IPV4);
    SDK_ASSERT(nat_msg->spec.nat_ip_range.ip_lo.v4_addr ==
               nat_msg->spec.nat_ip_range.ip_hi.v4_addr);

    nat_type = map_pds_address_type(nat_msg->spec.address_type);

    ret = nat_port_block_commit(nat_msg->key.id, nat_msg->spec.vpc.id,
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

// initialize callbacks for flow configuration
//
// Note: This is called from C code, and must have C linkage
//
void
pds_nat_ipc_init(void) {
    pds_ipc_register_callbacks(OBJ_ID_NAT_PORT_BLOCK,
                               pds_nat_cfg_set,
                               pds_nat_cfg_del,
                               pds_nat_cfg_process);
}

} // extern "C"
