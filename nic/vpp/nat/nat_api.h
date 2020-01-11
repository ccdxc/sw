//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//

#ifndef __VPP_NAT_API_H__
#define __VPP_NAT_API_H__

#ifndef __cplusplus
#include "nic/vpp/infra/ipc/pdsa_vpp_hdlr.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

// Datastructures
typedef enum {
    NAT_ERR_OK = 0,
    NAT_ERR_EXISTS = 1,
    NAT_ERR_NOT_FOUND = 2,
    NAT_ERR_IN_USE = 3,
    NAT_ERR_HW_TABLE_FULL = 4,
    NAT_ERR_ALLOC_FAIL = 5,
    NAT_ERR_NO_RESOURCE = 6,
    NAT_ERR_INVALID_PROTOCOL = 7,
} nat_err_t;

typedef enum {
    NAT_TYPE_INTERNET = 0,
    NAT_TYPE_INFRA = 1,

    NAT_TYPE_NUM = 2
} nat_type_t;

typedef enum {
    NAT_PROTO_UNKNOWN = 0,
    NAT_PROTO_UDP = 1,
    NAT_PROTO_TCP = 2,
    NAT_PROTO_ICMP = 3,

    NAT_PROTO_NUM = 3
} nat_proto_t;

// API
#ifdef __cplusplus
nat_err_t nat_port_block_add(const uint8_t key[PDS_MAX_KEY_LEN],
                             uint32_t vpc_hw_id,
                             uint32_t addr, uint8_t protocol,
                             uint16_t start_port, uint16_t end_port,
                             nat_type_t nat_type);
nat_err_t nat_port_block_update(const uint8_t key[PDS_MAX_KEY_LEN],
                                uint32_t vpc_hw_id,
                                uint32_t addr, uint8_t protocol,
                                uint16_t start_port, uint16_t end_port,
                                nat_type_t nat_type);
nat_err_t nat_port_block_commit(const uint8_t key[PDS_MAX_KEY_LEN],
                                uint32_t vpc_hw_id,
                                uint32_t addr, uint8_t protocol,
                                uint16_t start_port, uint16_t end_port,
                                nat_type_t nat_type);
nat_err_t nat_port_block_del(const uint8_t key[PDS_MAX_KEY_LEN],
                             uint32_t vpc_hw_id,
                             uint32_t addr, uint8_t protocol,
                             uint16_t start_port, uint16_t end_port,
                             nat_type_t nat_type);
#else
void nat_init(void);
nat_err_t nat_port_block_add(const u8 id[PDS_MAX_KEY_LEN], u32 vpc_hw_id,
                             ip4_address_t addr, u8 protocol, u16 start_port,
                             u16 end_port, nat_type_t nat_type);
nat_err_t nat_port_block_update(const u8 id[PDS_MAX_KEY_LEN], u32 vpc_hw_id,
                                ip4_address_t addr, u8 protocol, u16 start_port,
                                u16 end_port, nat_type_t nat_type);
nat_err_t nat_port_block_commit(const u8 id[PDS_MAX_KEY_LEN], u32 vpc_hw_id,
                                ip4_address_t addr, u8 protocol, u16 start_port,
                                u16 end_port, nat_type_t nat_type);
nat_err_t nat_port_block_del(const u8 id[PDS_MAX_KEY_LEN], u32 vpc_hw_id,
                             ip4_address_t addr, u8 protocol, u16 start_port,
                             u16 end_port, nat_type_t nat_type);

nat_err_t nat_flow_alloc(u32 vpc_hw_id, ip4_address_t dip, u16 dport,
                         u8 protocol, ip4_address_t pvt_ip, u16 pvt_port,
                         nat_type_t nat_type,
                         ip4_address_t *sip, u16 *sport);
nat_err_t nat_flow_dealloc(u32 vpc_hw_id, ip4_address_t dip, u16 dport, u8 protocol,
                           ip4_address_t sip, u16 sport, ip4_address_t pvt_ip,
                           u16 pvt_port, nat_type_t nat_type);
nat_err_t nat_usage(u32 vpc_hw_id, u8 protocol, nat_type_t nat_type,
                    u32 *num_ports_total, u32 *num_ports_alloc,
                    u32 *num_flows_alloc);
nat_err_t nat_hw_usage(u32 *total_hw_indices, u32 *total_alloc_indices);
#endif // __cplusplus

#ifdef __cplusplus
}
#endif

#endif    // __VPP_NAT_API_H__
