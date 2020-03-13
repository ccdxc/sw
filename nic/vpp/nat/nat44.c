//
//  {C} Copyright 2019 Pensando Systems Inc. All rights reserved.
//

#include <vlib/vlib.h>
#include <vnet/ip/ip.h>
#include <vnet/plugin/plugin.h>
#include <vppinfra/clib.h>
#include <nic/vpp/impl/nat.h>
#include "pdsa_uds_hdlr.h"
#include "nat_api.h"

// TODO move to right place
#define PDS_MAX_VPC 64
#define PDS_MAX_DYNAMIC_NAT 32768
#define PDS_DYNAMIC_NAT_START_INDEX 1000

#define IPV4_MASK 0xffffffff
#define PORT_MASK 0xffff
#define REF_COUNT_MASK 0xffff

typedef enum {
    NAT_PB_STATE_UNKNOWN,
    NAT_PB_STATE_ADDED,
    NAT_PB_STATE_DELETED,
    NAT_PB_STATE_UPDATED,
    NAT_PB_STATE_OK,
} nat_port_block_state_t;

typedef struct nat_port_block_s {
    // configured
    u8 id[PDS_MAX_KEY_LEN];
    ip4_address_t addr;
    u16 start_port;
    u16 end_port;

    // allocated
    nat_port_block_state_t state;
    int num_flow_alloc;
    u32 *ref_count;
    nat_hw_index_t *nat_tx_hw_index;
    uword *port_bitmap;
} nat_port_block_t;

typedef struct nat_vpc_config_s {
    clib_spinlock_t lock;

    u32 random_seed;

    // configured

    // number of port blocks
    u16 num_port_blocks;
    // pool of port blocks
    nat_port_block_t *nat_pb[NAT_ADDR_TYPE_NUM][NAT_PROTO_NUM];

    // allocated

    // hash table of public ip --> pb
    uword *nat_public_ip_ht;

    // hash table of allocated ports for flows
    uword *nat_flow_ht;

    // hash table of <pvt sip, pvt port> --> <public sip, public port>
    uword *nat_src_ht;

    // hash table of pvt_ip --> public_ip
    uword *nat_pvt_ip_ht;

    // stats
    u64 alloc_fail;
    u64 dealloc_fail;
} nat_vpc_config_t;

// Flow hash table maps <nat_flow_key_t> to <nat_rx_hw_index, addr_type, proto>
typedef struct nat_flow_key_s {
    ip4_address_t dip;
    ip4_address_t sip;
    uint16_t sport;
    uint16_t dport;
    u8 protocol;
    u8 pad[3];
} nat_flow_key_t;

#define NAT_FLOW_HT_MAKE_VAL(nat_rx_hw_index, addr_type, nat_proto) \
    ((((uword)nat_rx_hw_index) << 32) | (((uword) addr_type << 16)) | ((uword) nat_proto))

#define NAT_FLOW_HT_GET_VAL(data, nat_rx_hw_index, addr_type, nat_proto) \
    nat_proto = (data) & 0xff; \
    addr_type = ((data) >> 16) & 0xff; \
    nat_rx_hw_index = ((data) >> 32) & 0xffffffff;


// Endpoint hash table maps <pvt_ip, pvt_port> to <public_ip, public_port>
#define NAT_EP_SET_PUBLIC_IP_PORT(data, public_ip, public_port, ref_count) \
    data = (((uword) ref_count) << 48) | (((uword) public_port) << 32) | \
           (uword) public_ip.as_u32;

#define NAT_EP_SET_REF_COUNT(data, ref_count) \
    data = (((uword) ref_count) << 48) | ((data) & IPV4_MASK);

#define NAT_EP_INC_REF_COUNT(data, ref_count) \
    ref_count = ((data) >> 48) & REF_COUNT_MASK; \
    ref_count++; \
    data = (((uword) ref_count) << 48) | ((data) & IPV4_MASK);

#define NAT_EP_GET_PUBLIC_IP_PORT(data, public_ip, public_port, ref_count) \
    public_ip = (data) & IPV4_MASK; \
    public_port = ((data) >> 32) & PORT_MASK; \
    ref_count = ((data) >> 48) & REF_COUNT_MASK;

#define NAT_EP_GET_REF_COUNT(data, ref_count) \
    ref_count = ((data) >> 48) & REF_COUNT_MASK;

typedef CLIB_PACKED (union nat_src_key_s {
    struct {
        ip4_address_t pvt_ip;
        uint16_t pvt_port;
    };
    u64 as_u64;
}) nat_src_key_t;

// pvt ip --> public ip hash table
#define NAT_PVT_IP_HT_DATA(public_ip, ref_count) \
    ((((uword) ref_count) << 32) | public_ip)

#define NAT_PVT_IP_HT_PUBLIC_IP(data) \
    ((data) & IPV4_MASK)

#define NAT_PVT_IP_HT_INC_REF_COUNT(data) \
    u16 _ref_count = (((data) >> 32) & REF_COUNT_MASK); \
    _ref_count++; \
    (data) = ((uword)_ref_count << 32) | ((data) & IPV4_MASK);

#define NAT_PVT_IP_HT_DEC_REF_COUNT(data, ref_count) \
    (ref_count) = (((data) >> 32) & REF_COUNT_MASK); \
    (ref_count)--; \
    (data) = (((uword)ref_count) << 32) | ((data) & IPV4_MASK);

// public ip --> port block
typedef CLIB_PACKED (union nat_pub_key_s {
    struct {
        u8 nat_addr_type;
        u8 nat_proto;
        ip4_address_t pub_ip;
    };
    u64 as_u64;
}) nat_pub_key_t;


//
// Globals
//
typedef struct pds_nat_main_s {
    clib_spinlock_t lock;
    u8 *hw_index_pool;
    nat_vpc_config_t vpc_config[PDS_MAX_VPC];
    u8 proto_map[255];
    u8 nat_proto_map[NAT_PROTO_NUM + 1];
} pds_nat_main_t;

static pds_nat_main_t nat_main;

//
// init
//
void
nat_init(void)
{
    clib_spinlock_init(&nat_main.lock);
    pool_init_fixed(nat_main.hw_index_pool, PDS_MAX_DYNAMIC_NAT);

    nat_main.proto_map[IP_PROTOCOL_UDP] = NAT_PROTO_UDP;
    nat_main.proto_map[IP_PROTOCOL_TCP] = NAT_PROTO_TCP;
    nat_main.proto_map[IP_PROTOCOL_ICMP] = NAT_PROTO_ICMP;

    nat_main.nat_proto_map[NAT_PROTO_TCP] = IP_PROTOCOL_TCP;
    nat_main.nat_proto_map[NAT_PROTO_UDP] = IP_PROTOCOL_UDP;
    nat_main.nat_proto_map[NAT_PROTO_ICMP] = IP_PROTOCOL_ICMP;
}

//
// Helper functions
//
always_inline nat_port_block_t *
nat_get_port_block_from_pub_ip(nat_vpc_config_t *vpc, nat_addr_type_t nat_addr_type,
                               nat_proto_t nat_proto, ip4_address_t addr)
{
    uword *data;

    nat_pub_key_t pub_key = { 0 };

    pub_key.nat_addr_type = (u8)nat_addr_type;
    pub_key.nat_proto = nat_proto;
    pub_key.pub_ip = addr;
    data = hash_get(vpc->nat_public_ip_ht, pub_key.as_u64);
    if (data) {
        return pool_elt_at_index(vpc->nat_pb[nat_addr_type][nat_proto - 1], *data);
    }

    return NULL;
}

always_inline nat_proto_t
get_nat_proto_from_proto(u8 protocol)
{
    return nat_main.proto_map[protocol];
}

always_inline nat_proto_t
get_proto_from_nat_proto(nat_proto_t nat_proto)
{
    return nat_main.nat_proto_map[nat_proto];
}

//
// Add SNAT port block
//
nat_err_t
nat_port_block_add(const u8 id[PDS_MAX_KEY_LEN], u32 vpc_id, ip4_address_t addr,
                   u8 protocol, u16 start_port, u16 end_port,
                   nat_addr_type_t nat_addr_type)
{
    nat_port_block_t *pb;
    nat_vpc_config_t *vpc;
    nat_proto_t nat_proto;
    nat_pub_key_t pub_key = { 0 };

    ASSERT(vpc_id < PDS_MAX_VPC);
    ASSERT(nat_addr_type < NAT_ADDR_TYPE_NUM);

    vpc = &nat_main.vpc_config[vpc_id];

    nat_proto = get_nat_proto_from_proto(protocol);
    if (nat_proto == NAT_PROTO_UNKNOWN) {
        return NAT_ERR_INVALID_PROTOCOL;
    }

    if (nat_get_port_block_from_pub_ip(vpc, nat_addr_type, nat_proto, addr)) {
        // Entry already exists
        // This can happen in rollback case

        return NAT_ERR_EXISTS;
    }

    if (vpc->num_port_blocks == 0) {
        clib_spinlock_init(&vpc->lock);
        vpc->nat_flow_ht = hash_create_mem(0, sizeof(nat_flow_key_t),
                                           sizeof(uword));
        vpc->random_seed = random_default_seed();
    }

    clib_spinlock_lock(&vpc->lock);
    pool_get_zero(vpc->nat_pb[nat_addr_type][nat_proto - 1], pb);
    vpc->num_port_blocks++;
    pds_id_set(pb->id, id);
    pb->addr = addr;
    pb->start_port = start_port;
    pb->end_port = end_port;
    pb->state = NAT_PB_STATE_ADDED;

    pub_key.nat_addr_type = (u8)nat_addr_type;
    pub_key.nat_proto = nat_proto;
    pub_key.pub_ip = addr;
    hash_set(vpc->nat_public_ip_ht, pub_key.as_u64,
             pb - vpc->nat_pb[nat_addr_type][nat_proto - 1]);
    clib_spinlock_unlock(&vpc->lock);

    return NAT_ERR_OK;
}

//
// Update SNAT port block
//
nat_err_t
nat_port_block_update(const u8 id[PDS_MAX_KEY_LEN], u32 vpc_id,
                      ip4_address_t addr, u8 protocol, u16 start_port,
                      u16 end_port, nat_addr_type_t nat_addr_type)
{
    nat_port_block_t *pb = NULL;
    nat_vpc_config_t *vpc;
    nat_proto_t nat_proto;

    ASSERT(vpc_id < PDS_MAX_VPC);
    ASSERT(nat_addr_type < NAT_ADDR_TYPE_NUM);

    vpc = &nat_main.vpc_config[vpc_id];

    nat_proto = get_nat_proto_from_proto(protocol);
    if (nat_proto == NAT_PROTO_UNKNOWN) {
        return NAT_ERR_INVALID_PROTOCOL;
    }

    pb = nat_get_port_block_from_pub_ip(vpc, nat_addr_type, nat_proto, addr);
    if (!pb) {
        // Entry does not exist
        return NAT_ERR_NOT_FOUND;
    }

    if (pb->port_bitmap && !clib_bitmap_is_zero(pb->port_bitmap)) {
        // port block is in use, cannot update
        return NAT_ERR_IN_USE;
    }

    clib_spinlock_lock(&vpc->lock);
    pb->state = NAT_PB_STATE_UPDATED;
    clib_spinlock_unlock(&vpc->lock);

    return NAT_ERR_OK;
}

always_inline void
nat_port_block_del_inline(nat_vpc_config_t *vpc, nat_port_block_t *pb,
                          nat_proto_t nat_proto, nat_addr_type_t nat_addr_type)
{
    clib_spinlock_lock(&vpc->lock);
    hash_unset(vpc->nat_public_ip_ht, pb->addr.as_u32);
    pool_put(vpc->nat_pb[nat_addr_type][nat_proto - 1], pb);
    vpc->num_port_blocks--;
    if (vpc->num_port_blocks == 0) {
        hash_free(vpc->nat_flow_ht);
    }
    clib_spinlock_unlock(&vpc->lock);
}

//
// Delete SNAT port block
//
nat_err_t
nat_port_block_del(const u8 id[PDS_MAX_KEY_LEN], u32 vpc_id,
                   ip4_address_t addr, u8 protocol, u16 start_port,
                   u16 end_port, nat_addr_type_t nat_addr_type)
{
    nat_port_block_t *pb;
    nat_vpc_config_t *vpc;
    nat_proto_t nat_proto;

    ASSERT(vpc_id < PDS_MAX_VPC);

    vpc = &nat_main.vpc_config[vpc_id];

    nat_proto = get_nat_proto_from_proto(protocol);
    if (nat_proto == NAT_PROTO_UNKNOWN) {
        return NAT_ERR_INVALID_PROTOCOL;
    }

    pb = nat_get_port_block_from_pub_ip(vpc, nat_addr_type, nat_proto, addr);
    if (!pb) {
        // Entry does not exist.
        return NAT_ERR_NOT_FOUND;
    }

    if (pb->port_bitmap && !clib_bitmap_is_zero(pb->port_bitmap)) {
        // port block is in use, cannot update
        return NAT_ERR_IN_USE;
    }

    pb->state = NAT_PB_STATE_DELETED;

    return NAT_ERR_OK;
}

//
// Commit SNAT port block
//
nat_err_t
nat_port_block_commit(const u8 id[PDS_MAX_KEY_LEN], u32 vpc_id,
                      ip4_address_t addr, u8 protocol, u16 start_port,
                      u16 end_port, nat_addr_type_t nat_addr_type)
{
    nat_port_block_t *pb;
    nat_vpc_config_t *vpc;
    nat_proto_t nat_proto;

    ASSERT(vpc_id < PDS_MAX_VPC);
    ASSERT(nat_addr_type < NAT_ADDR_TYPE_NUM);

    vpc = &nat_main.vpc_config[vpc_id];

    nat_proto = get_nat_proto_from_proto(protocol);
    if (nat_proto == NAT_PROTO_UNKNOWN) {
        return NAT_ERR_INVALID_PROTOCOL;
    }

    pb = nat_get_port_block_from_pub_ip(vpc, nat_addr_type, nat_proto, addr);
    if (!pb) {
        // This should not happen
        return NAT_ERR_NOT_FOUND;
    }
    if (pb->state == NAT_PB_STATE_ADDED) {
        pb->state = NAT_PB_STATE_OK;
        return NAT_ERR_OK;
    } else if (pb->state == NAT_PB_STATE_DELETED) {
        nat_port_block_del_inline(vpc, pb, nat_proto, nat_addr_type);
        return NAT_ERR_OK;
    } else if (pb->state == NAT_PB_STATE_UPDATED) {
        pb->addr = addr;
        pb->start_port = start_port;
        pb->end_port = end_port;
        pb->state = NAT_PB_STATE_OK;
        return NAT_ERR_OK;
    }

    return NAT_ERR_OK;
}

//
// Read SNAT port block stats
//
nat_err_t
nat_port_block_get_stats(const u8 id[PDS_MAX_KEY_LEN], u32 vpc_id,
                         u8 protocol, nat_addr_type_t nat_addr_type,
                         pds_nat_port_block_export_t *export_pb)
{
    nat_port_block_t *pb;
    nat_vpc_config_t *vpc;
    nat_port_block_t *nat_pb;
    nat_proto_t nat_proto;
    u32 in_use_cnt = 0, session_cnt = 0;

    ASSERT(vpc_id < PDS_MAX_VPC);
    ASSERT(nat_addr_type < NAT_ADDR_TYPE_NUM);

    vpc = &nat_main.vpc_config[vpc_id];

    nat_proto = get_nat_proto_from_proto(protocol);
    if (nat_proto == NAT_PROTO_UNKNOWN) {
        return NAT_ERR_INVALID_PROTOCOL;
    }

    nat_pb = vpc->nat_pb[nat_addr_type][nat_proto - 1];

    pool_foreach (pb, nat_pb,
    ({
        if (pds_id_equals(id, pb->id)) {
            in_use_cnt += clib_bitmap_count_set_bits(pb->port_bitmap);
            session_cnt += pb->num_flow_alloc;
        }
    }));

    export_pb->in_use_cnt = in_use_cnt;
    export_pb->session_cnt = session_cnt;

    return NAT_ERR_OK;
}

always_inline nat_hw_index_t
nat_get_hw_index(u8 *ptr)
{
    return ptr - nat_main.hw_index_pool + PDS_DYNAMIC_NAT_START_INDEX;
}

always_inline u32
nat_get_pool_index_from_hw_index(nat_hw_index_t hw_index)
{
    return hw_index - PDS_DYNAMIC_NAT_START_INDEX;
}

always_inline void
nat_flow_get_and_add_src_endpoint_mapping(nat_vpc_config_t *vpc,
                                          ip4_address_t pvt_ip, u16 pvt_port,
                                          ip4_address_t *public_ip,
                                          u16 *public_port)
{
    nat_src_key_t key = { 0 };
    uword *data;
    u16 ref_count;

    public_ip->as_u32 = 0;
    *public_port = 0;

    key.pvt_ip = pvt_ip;
    key.pvt_port = pvt_port;

    data = hash_get(vpc->nat_src_ht, key.as_u64);
    if (data) {
        NAT_EP_INC_REF_COUNT(*data, ref_count);
    }
}

always_inline void
nat_flow_set_src_endpoint_mapping(nat_vpc_config_t *vpc,
                                  ip4_address_t pvt_ip, u16 pvt_port,
                                  ip4_address_t public_ip, u16 public_port)
{
    nat_src_key_t key = { 0 };
    uword data;

    key.pvt_ip = pvt_ip;
    key.pvt_port = pvt_port;

    NAT_EP_SET_PUBLIC_IP_PORT(data, public_ip, public_port, 1);
    hash_set(vpc->nat_src_ht, key.as_u64, data);
}

always_inline void
nat_flow_get_and_del_src_endpoint_mapping(nat_vpc_config_t *vpc,
                                          ip4_address_t pvt_ip, u16 pvt_port)
{
    nat_src_key_t key = { 0 };
    uword *data;
    u16 ref_count;

    key.pvt_ip = pvt_ip;
    key.pvt_port = pvt_port;

    data = hash_get(vpc->nat_src_ht, key.as_u64);
    if (data) {
        NAT_EP_GET_REF_COUNT(*data, ref_count);
        ref_count--;
        if (ref_count == 0) {
            hash_unset(vpc->nat_src_ht, key.as_u64);
        }
    }
}

always_inline void
nat_flow_add_ip_mapping(nat_vpc_config_t *vpc,
                        ip4_address_t pvt_ip,
                        ip4_address_t public_ip)
{
    uword *data;

    data = hash_get(vpc->nat_pvt_ip_ht, pvt_ip.as_u32);
    if (!data) {
        hash_set(vpc->nat_pvt_ip_ht, pvt_ip.as_u32,
                 NAT_PVT_IP_HT_DATA(public_ip.as_u32, 1));
    } else {
        NAT_PVT_IP_HT_INC_REF_COUNT(*data);
    }
}

always_inline void
nat_flow_del_ip_mapping(nat_vpc_config_t *vpc,
                        ip4_address_t pvt_ip)
{
    uword *data;
    u16 ref_count;

    data = hash_get(vpc->nat_pvt_ip_ht, pvt_ip.as_u32);
    if (data) {
        NAT_PVT_IP_HT_DEC_REF_COUNT(*data, ref_count);
        if (ref_count == 0) {
            hash_unset(vpc->nat_pvt_ip_ht, pvt_ip.as_u32);
        }
    }
}

always_inline nat_err_t
nat_flow_alloc_for_pb(nat_vpc_config_t *vpc, nat_port_block_t *pb,
                      ip4_address_t sip, u16 sport, ip4_address_t pvt_ip,
                      u16 pvt_port, nat_flow_key_t *flow_key,
                      nat_hw_index_t *xlate_idx,
                      nat_hw_index_t *xlate_idx_rflow,
                      nat_addr_type_t nat_addr_type, nat_proto_t nat_proto)
{
    u8 *tx_hw_index_ptr = NULL, *rx_hw_index_ptr;
    nat_hw_index_t rx_hw_index;
    u16 index = sport - pb->start_port;

    //
    // Allocate NAT hw indices
    // 
    clib_spinlock_lock(&nat_main.lock);
    if (PREDICT_FALSE(pool_elts(nat_main.hw_index_pool) >=
                PDS_MAX_DYNAMIC_NAT - 2)) {
        // no free nat hw indices;
        clib_spinlock_unlock(&nat_main.lock);
        return NAT_ERR_HW_TABLE_FULL;
    }
    if (vec_elt(pb->ref_count, index) == 0) {
        // Allocate HW NAT index for Tx direction
        pool_get(nat_main.hw_index_pool, tx_hw_index_ptr);
    }
    // Allocate HW NAT index for Rx direction
    pool_get(nat_main.hw_index_pool, rx_hw_index_ptr);
    clib_spinlock_unlock(&nat_main.lock);

    rx_hw_index = nat_get_hw_index(rx_hw_index_ptr);

    if (vec_elt(pb->ref_count, index) == 0) {
        clib_bitmap_set_no_check(pb->port_bitmap, index, 1);
        vec_elt(pb->nat_tx_hw_index, index) = nat_get_hw_index(tx_hw_index_ptr);
        pds_snat_tbl_write_ip4(vec_elt(pb->nat_tx_hw_index, index),
                                       sip.as_u32, sport);
    }

    pds_snat_tbl_write_ip4(rx_hw_index, pvt_ip.as_u32, pvt_port);
    *xlate_idx = vec_elt(pb->nat_tx_hw_index, index);
    *xlate_idx_rflow = rx_hw_index;

    vec_elt(pb->ref_count, index)++;
    pb->num_flow_alloc++;

    // Add to the flow hash table
    hash_set_mem_alloc(&vpc->nat_flow_ht, flow_key,
                       NAT_FLOW_HT_MAKE_VAL(rx_hw_index, nat_addr_type, nat_proto));

    // Add to the src endpoint hash table
    nat_flow_set_src_endpoint_mapping(vpc, pvt_ip, pvt_port, sip, sport);

    // Add the pvt_ip to the pvt_ip hash table
    nat_flow_add_ip_mapping(vpc, pvt_ip, sip);

    return NAT_ERR_OK;
}

always_inline nat_err_t
nat_flow_alloc_inline(u32 vpc_id, ip4_address_t dip, u16 dport,
                      ip4_address_t pvt_ip, u16 pvt_port,
                      ip4_address_t *sip, u16 *sport,
                      nat_port_block_t *pb, nat_vpc_config_t *vpc,
                      u8 protocol, nat_hw_index_t *xlate_idx,
                      nat_hw_index_t *xlate_idx_rflow,
                      nat_addr_type_t nat_addr_type, nat_proto_t nat_proto)
{
    u16 num_ports;
    nat_flow_key_t key = { 0 };
    uword *v;
    u16 try_sport;

    num_ports = pb->end_port - pb->start_port + 1;

    if (pb->ref_count == NULL) {
        // No ports allocated
        pb->ref_count = vec_new(u32, num_ports);
        pb->nat_tx_hw_index = vec_new(u32, num_ports);
        clib_bitmap_alloc(pb->port_bitmap, num_ports);
    }

    key.dip = dip;
    key.sip = pb->addr;
    key.dport = dport;
    key.protocol = protocol;

    for (try_sport = pb->start_port; try_sport <= pb->end_port; try_sport++) {
        key.sport = try_sport;
        v = hash_get_mem(vpc->nat_flow_ht, &key);
        if (!v) {
            // flow does not exist, this sip/sport can be allocated
            *sip = pb->addr;
            *sport = try_sport;

            nat_flow_alloc_for_pb(vpc, pb, *sip, *sport, pvt_ip, pvt_port,
                                  &key, xlate_idx, xlate_idx_rflow,
                                  nat_addr_type, nat_proto);

            return NAT_ERR_OK;
        }
    }

    return NAT_ERR_NO_RESOURCE;
}

always_inline nat_port_block_t *
nat_get_pool_for_pvt_ip(nat_vpc_config_t *vpc, nat_port_block_t *nat_pb,
                        ip4_address_t pvt_ip, nat_addr_type_t nat_addr_type,
                        nat_proto_t nat_proto)
{
    uword *data;
    ip4_address_t public_ip;
    nat_port_block_t *pb = NULL;

    data = hash_get_mem(vpc->nat_pvt_ip_ht, &pvt_ip);
    if (data) {
        public_ip.as_u32 = NAT_PVT_IP_HT_PUBLIC_IP(*data);
        pb = nat_get_port_block_from_pub_ip(vpc, nat_addr_type, nat_proto,
                                            public_ip);
    }

    return pb;
}

always_inline nat_port_block_t *
nat_get_random_pool(nat_vpc_config_t *vpc, nat_port_block_t *nat_pb)
{
    u32 num_elts = pool_elts(nat_pb);
    u32 elt = random_u32(&vpc->random_seed) % num_elts;
    u32 i;
    nat_port_block_t *pb = NULL;

    i = 0;
    pool_foreach (pb, nat_pb,
    ({
        if (i == elt && pb->state == NAT_PB_STATE_OK) {
            return pb;
        }
        i++;
    }));

    return NULL;
}

//
// Allocate SNAT port during flow creation (VPP datapath)
//
nat_err_t
nat_flow_alloc(u32 vpc_id, ip4_address_t dip, u16 dport,
               u8 protocol, ip4_address_t pvt_ip, u16 pvt_port,
               nat_addr_type_t nat_addr_type, ip4_address_t *sip, u16 *sport,
               nat_hw_index_t *xlate_idx, nat_hw_index_t *xlate_idx_rflow)
{
    nat_port_block_t *pb = NULL;
    nat_vpc_config_t *vpc;
    nat_err_t ret = NAT_ERR_OK;
    nat_proto_t nat_proto;
    nat_port_block_t *nat_pb;

    ASSERT(vpc_id < PDS_MAX_VPC);

    vpc = &nat_main.vpc_config[vpc_id];
    if (PREDICT_FALSE(!vpc || vpc->num_port_blocks == 0)) {
        return NAT_ERR_NO_RESOURCE;
    }

    nat_proto = get_nat_proto_from_proto(protocol);
    if (PREDICT_FALSE(nat_proto == NAT_PROTO_UNKNOWN)) {
        return NAT_ERR_INVALID_PROTOCOL;
    }

    clib_spinlock_lock(&vpc->lock);

    nat_pb = vpc->nat_pb[nat_addr_type][nat_proto - 1];
    if (PREDICT_FALSE(pool_elts(nat_pb) == 0)) {
        return NAT_ERR_NO_RESOURCE;
    }

    // if we have already allocated <pvt_ip, pvt_port>, try to use it
    nat_flow_get_and_add_src_endpoint_mapping(vpc, pvt_ip, pvt_port, sip,
                                              sport);
    if (*sport == 0) {
        // endpoint not found
        // Check if pvt_ip has been used
        pb = nat_get_pool_for_pvt_ip(vpc, nat_pb, pvt_ip, nat_addr_type, nat_proto);
        if (!pb) {
            // Try random pool
            pb = nat_get_random_pool(vpc, nat_pb);
        }
        if (pb) {
            ret = nat_flow_alloc_inline(vpc_id, dip, dport, pvt_ip, pvt_port,
                    sip, sport, pb, vpc, protocol, xlate_idx, xlate_idx_rflow,
                    nat_addr_type, nat_proto);
        }
        if (!pb || ret != NAT_ERR_OK) {
            // random pool did not work, try all pools
            u8 allocated = 0;
            pool_foreach (pb, nat_pb,
            ({
                if (allocated || pb->state != NAT_PB_STATE_OK) continue;
                ret = nat_flow_alloc_inline(vpc_id, dip, dport, pvt_ip,
                                            pvt_port, sip, sport, pb, vpc,
                                            protocol, xlate_idx,
                                            xlate_idx_rflow, nat_addr_type,
                                            nat_proto);
                if (ret == NAT_ERR_OK) allocated = 1;
            }));
        }

    } else {
        // src endpoint found, reuse public ip, port mapping
        pb = nat_get_port_block_from_pub_ip(vpc, nat_addr_type, nat_proto, *sip);
        if (pb) {
            nat_flow_key_t key = { 0 };
            key.dip = dip;
            key.sip = *sip;
            key.sport = *sport;
            key.dport = dport;
            key.protocol = protocol;
            nat_flow_alloc_for_pb(vpc, pb, *sip, *sport, pvt_ip, pvt_port,
                                  &key, xlate_idx, xlate_idx_rflow,
                                  nat_addr_type, nat_proto);
        }
    }
    clib_spinlock_unlock(&vpc->lock);

    if (ret != NAT_ERR_OK) {
        vpc->alloc_fail++;
    }

    return ret;
}

always_inline nat_err_t
nat_flow_dealloc_inline(u32 vpc_id, ip4_address_t dip, u16 dport,
                        ip4_address_t sip, u16 sport,
                        nat_port_block_t *pb)
{
    u16 index;

    if (sport < pb->start_port || sport > pb->end_port) {
        // not in this port block
        return NAT_ERR_NOT_FOUND;
    }

    index = sport - pb->start_port;
    pb->ref_count[index]--;
    if (pb->ref_count[index] == 0) {
        clib_bitmap_set_no_check(pb->port_bitmap, index, 0);

        clib_spinlock_lock(&nat_main.lock);
        pool_put_index(nat_main.hw_index_pool,
                       nat_get_pool_index_from_hw_index(
                       vec_elt(pb->nat_tx_hw_index, index)));
        clib_spinlock_unlock(&nat_main.lock);
    }
    pb->num_flow_alloc--;
    if (pb->num_flow_alloc == 0) {
        vec_free(pb->ref_count);
        vec_free(pb->nat_tx_hw_index);
        clib_bitmap_free(pb->port_bitmap);
    }

    return NAT_ERR_OK;
}

//
// Deallocate SNAT port for flow
//
nat_err_t
nat_flow_dealloc(u32 vpc_id, ip4_address_t dip, u16 dport, u8 protocol,
                 ip4_address_t sip, u16 sport)
{
    nat_port_block_t *pb = NULL;
    nat_vpc_config_t *vpc;
    nat_flow_key_t key = { 0 };
    nat_err_t ret = NAT_ERR_NOT_FOUND;
    uword *data, hw_index;
    nat_proto_t nat_proto;
    nat_addr_type_t nat_addr_type;
    ip4_address_t pvt_ip;
    u16 pvt_port;

    ASSERT(vpc_id < PDS_MAX_VPC);
    vpc = &nat_main.vpc_config[vpc_id];

    key.dip = dip;
    key.sip = sip;
    key.dport = dport;
    key.sport = sport;
    key.protocol = protocol;

    clib_spinlock_lock(&vpc->lock);

    data = hash_get_mem(vpc->nat_flow_ht, &key);
    if (PREDICT_FALSE(!data)) {
        clib_spinlock_unlock(&vpc->lock);
        return NAT_ERR_NOT_FOUND;
    }
    NAT_FLOW_HT_GET_VAL(*data, hw_index, nat_addr_type, nat_proto)

    pds_snat_tbl_read_ip4(hw_index, &pvt_ip.as_u32, &pvt_port);

    nat_flow_get_and_del_src_endpoint_mapping(vpc, pvt_ip, pvt_port);

    hash_unset_mem_free(&vpc->nat_flow_ht, &key);
    hash_unset_mem_free(&vpc->nat_pvt_ip_ht, &pvt_ip);

    pb = nat_get_port_block_from_pub_ip(vpc, nat_addr_type, nat_proto, sip);
    if (pb) {
        ret = nat_flow_dealloc_inline(vpc_id, dip, dport, sip,
                                      sport, pb);
    }
    clib_spinlock_unlock(&vpc->lock);

    // free HW NAT indices
    clib_spinlock_lock(&nat_main.lock);
    pool_put_index(nat_main.hw_index_pool,
                   nat_get_pool_index_from_hw_index(hw_index));
    clib_spinlock_unlock(&nat_main.lock);

    if (ret != NAT_ERR_OK) {
        vpc->dealloc_fail++;
    }

    return NAT_ERR_OK;
}

//
// Get SNAT usage
//
nat_err_t
nat_usage(u32 vpc_id, u8 protocol, nat_addr_type_t nat_addr_type,
          u32 *num_ports_total, u32 *num_ports_alloc,
          u32 *num_flows_alloc)
{
    nat_port_block_t *pb;
    nat_vpc_config_t *vpc;
    nat_proto_t nat_proto;
    nat_port_block_t *nat_pb;

    nat_proto = get_nat_proto_from_proto(protocol);
    if (nat_proto == NAT_PROTO_UNKNOWN) {
        return NAT_ERR_INVALID_PROTOCOL;
    }

    *num_ports_total = 0;
    *num_ports_alloc = 0;
    *num_flows_alloc = 0;

    ASSERT(vpc_id < PDS_MAX_VPC);

    vpc = &nat_main.vpc_config[vpc_id];

    nat_pb = vpc->nat_pb[nat_addr_type][nat_proto - 1];

    pool_foreach (pb, nat_pb,
    ({
        *num_ports_total += pb->end_port - pb->start_port + 1;
        *num_ports_alloc += clib_bitmap_count_set_bits(pb->port_bitmap);
        *num_flows_alloc += pb->num_flow_alloc;
    }));

    return NAT_ERR_OK;
}

//
// Get NAT hw usage
//
nat_err_t
nat_hw_usage(u32 *total_hw_indices, u32 *total_alloc_indices)
{
    *total_hw_indices = PDS_MAX_DYNAMIC_NAT;
    *total_alloc_indices = pool_elts(nat_main.hw_index_pool);

    return NAT_ERR_OK;
}

//
// Get SNAT usage
//
bool
nat_pb_iterate_for_proto_and_type(u32 vpc_id, nat_proto_t nat_proto,
                                  nat_addr_type_t nat_addr_type,
                                  pds_nat_iterate_params_t *params)
{
    nat_port_block_t *pb;
    nat_vpc_config_t *vpc;
    nat_port_block_t *nat_pb;
    pds_nat_port_block_export_t *pb_out = params->pb;

    ASSERT(vpc_id < PDS_MAX_VPC);

    vpc = &nat_main.vpc_config[vpc_id];

    nat_pb = vpc->nat_pb[nat_addr_type][nat_proto - 1];

    pool_foreach (pb, nat_pb,
    ({
        pds_id_set(pb_out->id, pb->id);
        pb_out->addr = pb->addr.as_u32;
        pb_out->start_port = pb->start_port;
        pb_out->end_port = pb->end_port;
        if (nat_addr_type == NAT_ADDR_TYPE_INTERNET) {
            pb_out->address_type = NAT_ADDR_TYPE_INTERNET;
        } else {
            pb_out->address_type = NAT_ADDR_TYPE_INFRA;
        }
        pb_out->protocol = get_proto_from_nat_proto(nat_proto);
        pb_out->in_use_cnt = clib_bitmap_count_set_bits(pb->port_bitmap);
        pb_out->session_cnt = pb->num_flow_alloc;

        if (params->itercb(params)) {
            goto end;
        }
    }));

    return false;

end:
    return true;
}

uint16_t
nat_pb_count() {
    uint16_t count = 0;
    for (int i = 0; i < PDS_MAX_VPC; i++) {
        count += nat_main.vpc_config[i].num_port_blocks;
    }
    return count;
}

//
// Iterate over NAT port blocks
//
void
nat_pb_iterate(pds_nat_iterate_params_t *params)
{
    for (int i = 0; i < PDS_MAX_VPC; i++) {
        if (nat_main.vpc_config[i].num_port_blocks == 0) {
            continue;
        }
        if (nat_pb_iterate_for_proto_and_type(i, NAT_PROTO_UDP, NAT_ADDR_TYPE_INTERNET,
                                              params)) {
            break;
        }
        if (nat_pb_iterate_for_proto_and_type(i, NAT_PROTO_TCP, NAT_ADDR_TYPE_INTERNET,
                                              params)) {
            break;
        }
        if (nat_pb_iterate_for_proto_and_type(i, NAT_PROTO_ICMP, NAT_ADDR_TYPE_INTERNET,
                                              params)) {
            break;
        }
        if (nat_pb_iterate_for_proto_and_type(i, NAT_PROTO_UDP, NAT_ADDR_TYPE_INFRA,
                                              params)) {
            break;
        }
        if (nat_pb_iterate_for_proto_and_type(i, NAT_PROTO_TCP, NAT_ADDR_TYPE_INFRA,
                                              params)) {
            break;
        }
        if (nat_pb_iterate_for_proto_and_type(i, NAT_PROTO_ICMP, NAT_ADDR_TYPE_INFRA,
                                              params)) {
            break;
        }
    }
    return;
}

//
// Iterate over NAT flows
//
typedef void *(*nat_flow_iter_cb_t)(void *ctxt, u32 vpc_id,
                                    nat_flow_key_t *key, u64 val);

void *
nat_flow_iterate(void *ctxt, nat_flow_iter_cb_t iter_cb)
{
    nat_vpc_config_t *vpc;
    nat_flow_key_t *key;
    u64 value;

    for (int vpc_id = 0; vpc_id < PDS_MAX_VPC; vpc_id++) {
        vpc = &nat_main.vpc_config[vpc_id];
        if (vpc->num_port_blocks == 0) {
            continue;
        }
        hash_foreach_mem(key, value, vpc->nat_flow_ht,
        ({
            ctxt = iter_cb(ctxt, vpc_id, key, value);
        }));
    }

    return ctxt;
}

static void *
nat_cli_show_flow_cb(void *ctxt, u32 vpc_id, nat_flow_key_t *key, u64 val)
{
    u32 hw_index;
    nat_proto_t nat_proto;
    nat_addr_type_t nat_addr_type;
    ip4_address_t sip, dip, pvt_ip;
    u16 sport, dport, pvt_port;
    vlib_main_t *vm = (vlib_main_t *)ctxt;

    NAT_FLOW_HT_GET_VAL(val, hw_index, nat_addr_type, nat_proto)
    (void)nat_proto;

    pds_snat_tbl_read_ip4(hw_index, &pvt_ip.as_u32, &pvt_port);

    sip.as_u32 = clib_host_to_net_u32(key->sip.as_u32);
    dip.as_u32 = clib_host_to_net_u32(key->dip.as_u32);
    sport = clib_host_to_net_u16(key->sport);
    dport = clib_host_to_net_u16(key->dport);
    pvt_ip.as_u32 = clib_host_to_net_u32(pvt_ip.as_u32);
    pvt_port = clib_host_to_net_u16(pvt_port);
    vlib_cli_output(vm, "%-10U%-10U%-10U%-10U%-10U%-10d%-10d%-10U%-10U\n",
                    format_ip4_address, &sip,
                    format_ip4_address, &dip,
                    format_tcp_udp_port, sport,
                    format_tcp_udp_port, dport,
                    format_ip_protocol, key->protocol,
                    hw_index, nat_addr_type,
                    format_ip4_address, &pvt_ip,
                    format_tcp_udp_port, pvt_port);

    return vm;
}

typedef struct nat_clear_cb_s {
    nat_flow_key_t key;
    u64 val;
    u32 vpc_id;
} nat_clear_cb_t;

static void *
nat_cli_clear_flow_cb(void *ctxt, u32 vpc_id, nat_flow_key_t *key, u64 val)
{
    nat_clear_cb_t *clear_flow_pool = (nat_clear_cb_t *)ctxt;
    nat_clear_cb_t *elem;

    pool_get(clear_flow_pool, elem);
    elem->key = *key;
    elem->val = val;
    elem->vpc_id = vpc_id;

    return clear_flow_pool;
}

static clib_error_t *
nat_cli_show_flow(vlib_main_t *vm,
                  unformat_input_t *input,
                  vlib_cli_command_t *cmd)
{
    vlib_cli_output(vm, "%-10s%-10s%-10s%-10s%-10s%-10s%-10s%-10s%-10s\n",
                    "SIP", "DIP", "SPORT", "DPORT", "PROTO", "HW INDEX", "ADDR TYPE", "PVT IP", "PVT PORT");
    (void)nat_flow_iterate((void *)vm, &nat_cli_show_flow_cb);

    return 0;
}

static clib_error_t *
nat_clear_flow_helper(bool vpp_cli, vlib_main_t *vm)
{
    nat_clear_cb_t *clear_flow_pool = NULL;
    nat_clear_cb_t *elem;
    nat_err_t nat_ret;

    clear_flow_pool = nat_flow_iterate((void *)clear_flow_pool, &nat_cli_clear_flow_cb);

    pool_foreach (elem, clear_flow_pool,
    ({
        nat_ret = nat_flow_dealloc(elem->vpc_id, elem->key.dip,
                                   elem->key.dport, elem->key.protocol,
                                   elem->key.sip, elem->key.sport);
        if (nat_ret != NAT_ERR_OK) {
            if (vpp_cli) {
                vlib_cli_output(vm, "NAT flow delete failed %d", nat_ret);
            }
        }
    }));

    pool_free(clear_flow_pool);
    return 0;
}

static clib_error_t *
nat_cli_clear_all_flows(vlib_main_t *vm,
                        unformat_input_t *input,
                        vlib_cli_command_t *cmd)
{
    return nat_clear_flow_helper(true, vm);
}

int
nat_clear_all_flows(void)
{
    nat_clear_flow_helper(false, NULL);
    return 0;
}

VLIB_CLI_COMMAND(nat_cli_show_flow_command, static) =
{
    .path = "show nat flow",
    .short_help = "show nat flow",
    .function = nat_cli_show_flow,
};

VLIB_CLI_COMMAND(nat_cli_clear_flow_command, static) =
{
    .path = "clear nat flow",
    .short_help = "clear nat flow",
    .function = nat_cli_clear_all_flows,
};
