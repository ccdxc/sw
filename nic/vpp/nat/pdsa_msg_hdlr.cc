//
//  {C} Copyright 2019 Pensando Systems Inc. All rights reserved.
//

#include "nic/vpp/infra/cfg/pdsa_db.hpp"
#include "nic/vpp/nat/pdsa_uds_hdlr.h"
#include "nic/vpp/infra/ipc/pdsa_hdlr.hpp"
#include "nat_api.h"

extern "C" {

static inline nat_addr_type_t
map_pds_address_type(address_type_t address_type)
{
    if (address_type == ADDR_TYPE_PUBLIC) {
        return NAT_ADDR_TYPE_INTERNET;
    } else if (address_type == ADDR_TYPE_SERVICE) {
        return NAT_ADDR_TYPE_INFRA;
    }
    SDK_ASSERT(1 == 0);
}

static sdk::sdk_ret_t
pds_vpc_cfg_hw_id_get(const pds_obj_key_t *vpc, uint16_t *vpc_hw_id) {
    pds_cfg_msg_t vpc_msg;
    vpp_config_data &config = vpp_config_data::get();

    vpc_msg.obj_id = OBJ_ID_VPC;
    vpc_msg.vpc.key = *vpc;

    if (!config.get(vpc_msg)) {
        return sdk::SDK_RET_ERR;
    }
    *vpc_hw_id = vpc_msg.vpc.status.hw_id;
    return sdk::SDK_RET_OK;
}

// callback function to process configuration message. C/U/D
static sdk::sdk_ret_t
pds_nat_cfg_set(const pds_cfg_msg_t *cfg_msg) {
    const pds_nat_port_block_cfg_msg_t *nat_msg;
    nat_addr_type_t nat_addr_type;
    nat_err_t ret = NAT_ERR_OK;
    uint16_t vpc_hw_id;
    ipv4_addr_t addr;
    sdk::sdk_ret_t vpc_ret;

    nat_msg = &cfg_msg->nat_port_block;

    SDK_ASSERT(nat_msg->spec.nat_ip_range.af == IP_AF_IPV4);

    nat_addr_type = map_pds_address_type(nat_msg->spec.address_type);

    vpc_ret = pds_vpc_cfg_hw_id_get(&nat_msg->spec.vpc, &vpc_hw_id);
    if (sdk::SDK_RET_OK != vpc_ret) {
        return vpc_ret;
    }

    if (cfg_msg->op == API_OP_CREATE) {
        for (addr = nat_msg->spec.nat_ip_range.ip_lo.v4_addr;
             addr <= nat_msg->spec.nat_ip_range.ip_hi.v4_addr; addr++) {
            ret = nat_port_block_add((const uint8_t *)nat_msg->key.id, vpc_hw_id,
                                     addr, nat_msg->spec.ip_proto,
                                     nat_msg->spec.nat_port_range.port_lo,
                                     nat_msg->spec.nat_port_range.port_hi,
                                     nat_addr_type);
            if (ret != NAT_ERR_OK) {
                break;
            }
        }
    } else {
        SDK_ASSERT(cfg_msg->op == API_OP_UPDATE);
        for (addr = nat_msg->spec.nat_ip_range.ip_lo.v4_addr;
             addr <= nat_msg->spec.nat_ip_range.ip_hi.v4_addr; addr++) {
            ret = nat_port_block_update((const uint8_t *)nat_msg->key.id, vpc_hw_id,
                                        addr, nat_msg->spec.ip_proto,
                                        nat_msg->spec.nat_port_range.port_lo,
                                        nat_msg->spec.nat_port_range.port_hi,
                                        nat_addr_type);
            if (ret != NAT_ERR_OK) {
                break;
            }
        }
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
    nat_addr_type_t nat_addr_type;
    nat_err_t ret = NAT_ERR_OK;
    uint16_t vpc_hw_id;
    ipv4_addr_t addr;
    sdk::sdk_ret_t vpc_ret;

    nat_msg = &cfg_msg->nat_port_block;

    SDK_ASSERT(nat_msg->spec.nat_ip_range.af == IP_AF_IPV4);

    vpc_ret = pds_vpc_cfg_hw_id_get(&nat_msg->spec.vpc, &vpc_hw_id);
    if (sdk::SDK_RET_OK != vpc_ret) {
        return vpc_ret;
    }
    nat_addr_type = map_pds_address_type(nat_msg->spec.address_type);

    for (addr = nat_msg->spec.nat_ip_range.ip_lo.v4_addr;
         addr <= nat_msg->spec.nat_ip_range.ip_hi.v4_addr; addr++) {
        ret = nat_port_block_del((const uint8_t *)nat_msg->key.id, vpc_hw_id,
                                 addr, nat_msg->spec.ip_proto,
                                 nat_msg->spec.nat_port_range.port_lo,
                                 nat_msg->spec.nat_port_range.port_hi,
                                 nat_addr_type);
        if (ret != NAT_ERR_OK) {
            break;
        }
    }
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
    nat_addr_type_t nat_addr_type;
    nat_err_t ret = NAT_ERR_OK;
    uint16_t vpc_hw_id;
    ipv4_addr_t addr;
    sdk::sdk_ret_t vpc_ret;

    nat_msg = &cfg_msg->nat_port_block;

    SDK_ASSERT(nat_msg->spec.nat_ip_range.af == IP_AF_IPV4);

    nat_addr_type = map_pds_address_type(nat_msg->spec.address_type);

    vpc_ret = pds_vpc_cfg_hw_id_get(&nat_msg->spec.vpc, &vpc_hw_id);
    if (sdk::SDK_RET_OK != vpc_ret) {
        return vpc_ret;
    }

    for (addr = nat_msg->spec.nat_ip_range.ip_lo.v4_addr;
         addr <= nat_msg->spec.nat_ip_range.ip_hi.v4_addr; addr++) {
        ret = nat_port_block_commit((const uint8_t *)nat_msg->key.id, vpc_hw_id,
                                    addr, nat_msg->spec.ip_proto,
                                    nat_msg->spec.nat_port_range.port_lo,
                                    nat_msg->spec.nat_port_range.port_hi,
                                    nat_addr_type);
    }

    if (ret == NAT_ERR_OK) {
        return sdk::SDK_RET_OK;
    } else {
        return sdk::SDK_RET_ERR;
    }
}

// callback function for read nat port block
static sdk::sdk_ret_t
pds_nat_cfg_get(pds_cfg_msg_t *cfg_msg) {
    pds_nat_port_block_cfg_msg_t *nat_msg;
    pds_nat_port_block_export_t plugin_nat_pb;
    nat_addr_type_t nat_addr_type;
    uint16_t vpc_hw_id;
    sdk::sdk_ret_t vpc_ret;

    nat_msg = &cfg_msg->nat_port_block;

    nat_addr_type = map_pds_address_type(nat_msg->spec.address_type);

    vpc_ret = pds_vpc_cfg_hw_id_get(&nat_msg->spec.vpc, &vpc_hw_id);
    if (sdk::SDK_RET_OK != vpc_ret) {
        return vpc_ret;
    }

    nat_port_block_get_stats((const uint8_t *)nat_msg->key.id, vpc_hw_id,
                             nat_msg->spec.ip_proto, nat_addr_type,
                             &plugin_nat_pb);

    nat_msg->stats.in_use_count = plugin_nat_pb.in_use_cnt;
    nat_msg->stats.session_count = plugin_nat_pb.session_cnt;

    return SDK_RET_OK;
}

bool
pds_nat_iterate_cb(pds_nat_iterate_params_t *params)
{
    pds_cmd_ctxt_t *ctxt = (pds_cmd_ctxt_t *)params->ctxt;
    pds_nat_port_block_export_t *pb = params->pb;
    pds_nat_port_block_cfg_msg_t *msg = &ctxt->nat_ctxt->cfg[ctxt->curr_count];
    pds_nat_port_block_spec_t *spec = &msg->spec;
    pds_nat_port_block_stats_t *stats = &msg->stats;

    if (ctxt->curr_count >= ctxt->nat_ctxt->num_entries) {
        return true;
    }

    memcpy(&spec->key.id, pb->id, PDS_MAX_KEY_LEN);
    if (pb->address_type == NAT_ADDR_TYPE_INFRA) {
        spec->address_type = ADDR_TYPE_SERVICE;
    } else if (pb->address_type == NAT_ADDR_TYPE_INTERNET) {
        spec->address_type = ADDR_TYPE_PUBLIC;
    }
    spec->ip_proto = pb->protocol;
    spec->nat_ip_range.af = IP_AF_IPV4;
    spec->nat_ip_range.ip_lo.v4_addr = pb->addr;
    spec->nat_ip_range.ip_hi.v4_addr = pb->addr;
    spec->nat_port_range.port_lo = pb->start_port;
    spec->nat_port_range.port_hi = pb->end_port;

    stats->in_use_count = pb->in_use_cnt;
    stats->session_count = pb->session_cnt;
    ctxt->curr_count ++;
    return false;
}

static sdk::sdk_ret_t
pdsa_nat_cfg_get_all(const pds_cmd_msg_t *msg, pds_cmd_ctxt_t *ctxt)
{
    pds_nat_port_block_export_t pb;
    pds_nat_iterate_params_t params;

    params.itercb = pds_nat_iterate_cb;
    params.pb = &pb;
    params.ctxt = ctxt;
    memset(pb.id, 0, sizeof(pb.id));

    nat_pb_iterate(&params);
    return sdk::SDK_RET_OK;
}

static sdk::sdk_ret_t
pdsa_nat_cfg_ctxt_init(const pds_cmd_msg_t *msg, pds_cmd_ctxt_t *ctxt)
{
    uint16_t num_port_blocks = nat_pb_count();
    ctxt->nat_ctxt = (pds_nat_port_block_cmd_ctxt_t *)
                     calloc(1, sizeof(uint16_t) +
                            (num_port_blocks *
                            sizeof(pds_nat_port_block_cfg_msg_t)));
    if (ctxt->nat_ctxt == NULL) {
        return sdk::SDK_RET_OOM;
    }
    ctxt->nat_ctxt->num_entries = num_port_blocks;
    return sdk::SDK_RET_OK;
}

static sdk::sdk_ret_t
pdsa_nat_cfg_ctxt_destroy(const pds_cmd_msg_t *msg, pds_cmd_ctxt_t *ctxt)
{
    if (ctxt->nat_ctxt) {
        free(ctxt->nat_ctxt);
        ctxt->nat_ctxt = 0;
    }
    return sdk::SDK_RET_OK;
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
                               pds_nat_cfg_act,
                               pds_nat_cfg_get);
    pds_ipc_register_cmd_callbacks(PDS_CFG_MSG_ID_NAT_PORT_BLOCK_GET_ALL,
                                   pdsa_nat_cfg_get_all,
                                   pdsa_nat_cfg_ctxt_init,
                                   pdsa_nat_cfg_ctxt_destroy);
}

} // extern "C"
