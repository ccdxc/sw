//
//  {C} Copyright 2019 Pensando Systems Inc. All rights reserved.
//

#include <vlib/vlib.h>
#include <vnet/ip/ip.h>
#include <vnet/plugin/plugin.h>
#include "nat_api.h"
#include "nat_pd.h"

// TODO move to right place
#define PDS_MAX_VPC 64
#define PDS_MAX_DYNAMIC_NAT 32768

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
    u32 *nat_tx_hw_index;
    uword *port_bitmap;
} nat_port_block_t;

typedef struct nat_vpc_config_s {
    clib_spinlock_t lock;

    u32 random_seed;

    // configured

    // number of port blocks
    u16 num_port_blocks;
    // pool of port blocks
    nat_port_block_t *nat_pb[NAT_TYPE_NUM][NAT_PROTO_NUM];

    // allocated

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

typedef struct nat_flow_key_s {
    ip4_address_t dip;
    ip4_address_t sip;
    uint16_t sport;
    uint16_t dport;
    u8 protocol;
    u8 pad[3];
} nat_flow_key_t;


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

typedef struct nat_src_key_s {
    ip4_address_t pvt_ip;
    uint16_t pvt_port;
} nat_src_key_t;

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


//
// Globals
//
typedef struct pds_nat_main_s {
    clib_spinlock_t lock;
    u8 *hw_index_pool;
    nat_vpc_config_t vpc_config[PDS_MAX_VPC];
    u8 proto_map[255];
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
}

//
// Add SNAT port block
//
nat_err_t
nat_port_block_add(const u8 id[PDS_MAX_KEY_LEN], u32 vpc_id, ip4_address_t addr,
                   u8 protocol, u16 start_port, u16 end_port,
                   nat_type_t nat_type)
{
    nat_port_block_t *pb;
    nat_vpc_config_t *vpc;
    nat_proto_t nat_proto;

    ASSERT(vpc_id < PDS_MAX_VPC);
    ASSERT(nat_type < NAT_TYPE_NUM);

    vpc = &nat_main.vpc_config[vpc_id];

    nat_proto = nat_main.proto_map[protocol];
    if (nat_proto == NAT_PROTO_UNKNOWN) {
        return NAT_ERR_INVALID_PROTOCOL;
    }

    pool_foreach (pb, vpc->nat_pb[nat_type][nat_proto - 1],
    ({
        if (pds_id_equals(pb->id, id))
            // Entry already exists
            // This can happen in rollback case
            return NAT_ERR_EXISTS;
    }));

    if (vpc->num_port_blocks == 0) {
        clib_spinlock_init(&vpc->lock);
        vpc->nat_flow_ht = hash_create_mem(0, sizeof(nat_flow_key_t),
                                           sizeof(uword));
        vpc->nat_src_ht = hash_create_mem(0, sizeof(nat_src_key_t),
                                          sizeof(uword));
        vpc->nat_pvt_ip_ht = hash_create_mem(0, sizeof(ip4_address_t),
                                          sizeof(uword));
        vpc->random_seed = random_default_seed();
    }

    clib_spinlock_lock(&vpc->lock);
    pool_get_zero(vpc->nat_pb[nat_type][nat_proto - 1], pb);
    vpc->num_port_blocks++;
    pds_id_set(pb->id, id);
    pb->addr = addr;
    pb->start_port = start_port;
    pb->end_port = end_port;
    pb->state = NAT_PB_STATE_ADDED;
    clib_spinlock_unlock(&vpc->lock);

    return NAT_ERR_OK;
}

//
// Update SNAT port block
//
nat_err_t
nat_port_block_update(const u8 id[PDS_MAX_KEY_LEN], u32 vpc_id,
                      ip4_address_t addr, u8 protocol, u16 start_port,
                      u16 end_port, nat_type_t nat_type)
{
    nat_port_block_t *pb = NULL;
    nat_vpc_config_t *vpc;
    nat_proto_t nat_proto;
    nat_port_block_t *nat_pb;
    u8 found = 0;

    ASSERT(vpc_id < PDS_MAX_VPC);
    ASSERT(nat_type < NAT_TYPE_NUM);

    vpc = &nat_main.vpc_config[vpc_id];

    nat_proto = nat_main.proto_map[protocol];
    if (nat_proto == NAT_PROTO_UNKNOWN) {
        return NAT_ERR_INVALID_PROTOCOL;
    }

    nat_pb = vpc->nat_pb[nat_type][nat_proto - 1];

    pool_foreach (pb, nat_pb,
    ({
        if (pds_id_equals(pb->id, id)) {
            found = 1;
            break;
        }
    }));

    if (!found) {
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
                          nat_proto_t nat_proto, nat_type_t nat_type)
{
    clib_spinlock_lock(&vpc->lock);
    pool_put(vpc->nat_pb[nat_type][nat_proto - 1], pb);
    vpc->num_port_blocks--;
    if (vpc->num_port_blocks == 0) {
        hash_free(vpc->nat_flow_ht);
        hash_free(vpc->nat_src_ht);
        hash_free(vpc->nat_pvt_ip_ht);
    }
    clib_spinlock_unlock(&vpc->lock);
}

//
// Delete SNAT port block
//
nat_err_t
nat_port_block_del(const u8 id[PDS_MAX_KEY_LEN], u32 vpc_id,
                   ip4_address_t addr, u8 protocol, u16 start_port,
                   u16 end_port, nat_type_t nat_type)
{
    nat_port_block_t *pb = NULL;
    nat_vpc_config_t *vpc;
    nat_proto_t nat_proto;
    u8 found = 0;

    ASSERT(vpc_id < PDS_MAX_VPC);

    vpc = &nat_main.vpc_config[vpc_id];

    nat_proto = nat_main.proto_map[protocol];
    if (nat_proto == NAT_PROTO_UNKNOWN) {
        return NAT_ERR_INVALID_PROTOCOL;
    }

    pool_foreach (pb, vpc->nat_pb[nat_type][nat_proto - 1],
    ({
        if (pds_id_equals(pb->id, id)) {
            found = 1;
            break;
        }
    }));

    if (!found) {
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
                      u16 end_port, nat_type_t nat_type)
{
    nat_port_block_t *pb;
    nat_vpc_config_t *vpc;
    nat_proto_t nat_proto;

    ASSERT(vpc_id < PDS_MAX_VPC);
    ASSERT(nat_type < NAT_TYPE_NUM);

    vpc = &nat_main.vpc_config[vpc_id];

    nat_proto = nat_main.proto_map[protocol];
    if (nat_proto == NAT_PROTO_UNKNOWN) {
        return NAT_ERR_INVALID_PROTOCOL;
    }

    pool_foreach (pb, vpc->nat_pb[nat_type][nat_proto - 1],
    ({
        if (pds_id_equals(pb->id, id)) {
            if (pb->state == NAT_PB_STATE_ADDED) {
                pb->state = NAT_PB_STATE_OK;
                return NAT_ERR_OK;
            } else if (pb->state == NAT_PB_STATE_DELETED) {
                nat_port_block_del_inline(vpc, pb, nat_proto, nat_type);
                return NAT_ERR_OK;
            } else if (pb->state == NAT_PB_STATE_UPDATED) {
                pb->addr = addr;
                pb->start_port = start_port;
                pb->end_port = end_port;
                pb->state = NAT_PB_STATE_OK;
                return NAT_ERR_OK;
            }
        }
    }));

    return NAT_ERR_OK;
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

    data = hash_get_mem(vpc->nat_src_ht, &key);
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
    hash_set_mem_alloc(&vpc->nat_src_ht, &key, data);
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

    data = hash_get_mem(vpc->nat_src_ht, &key);
    if (data) {
        NAT_EP_GET_REF_COUNT(*data, ref_count);
        ref_count--;
        if (ref_count == 0) {
            hash_unset_mem_free(&vpc->nat_src_ht, &key);
        }
    }
}

always_inline void
nat_flow_add_ip_mapping(nat_vpc_config_t *vpc,
                        ip4_address_t pvt_ip,
                        ip4_address_t public_ip)
{
    uword *data;

    data = hash_get_mem(vpc->nat_pvt_ip_ht, &pvt_ip);
    if (!data) {
        hash_set_mem_alloc(&vpc->nat_pvt_ip_ht, &pvt_ip,
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

    data = hash_get_mem(vpc->nat_pvt_ip_ht, &pvt_ip);
    if (data) {
        NAT_PVT_IP_HT_DEC_REF_COUNT(*data, ref_count);
        if (ref_count == 0) {
            hash_unset_mem_free(&vpc->nat_pvt_ip_ht, &pvt_ip);
        }
    }
}

always_inline nat_err_t
nat_flow_alloc_for_pb(nat_vpc_config_t *vpc, nat_port_block_t *pb,
                      ip4_address_t sip, u16 sport,
                      ip4_address_t pvt_ip, u16 pvt_port,
                      nat_flow_key_t *flow_key)
{
    u8 *tx_hw_index_ptr = NULL, *rx_hw_index_ptr;
    u32 rx_hw_index;
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
    if (pb->ref_count[index] == 0) {
        // Allocate HW NAT index for Tx direction
        pool_get(nat_main.hw_index_pool, tx_hw_index_ptr);
    }
    // Allocate HW NAT index for Rx direction
    pool_get(nat_main.hw_index_pool, rx_hw_index_ptr);
    clib_spinlock_unlock(&nat_main.lock);

    rx_hw_index = rx_hw_index_ptr - nat_main.hw_index_pool;

    if (pb->ref_count[index] == 0) {
        clib_bitmap_set_no_check(pb->port_bitmap, index, 1);
        pb->nat_tx_hw_index[index] =
            tx_hw_index_ptr - nat_main.hw_index_pool;
        pds_snat_tbl_write_ip4(pb->nat_tx_hw_index[index], sip.as_u32, sport);
    }

    pds_snat_tbl_write_ip4(rx_hw_index, pvt_ip.as_u32, pvt_port);

    pb->ref_count[index]++;
    pb->num_flow_alloc++;

    // Add to the flow hash table
    hash_set_mem_alloc(&vpc->nat_flow_ht, flow_key, rx_hw_index);

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
                      u8 protocol)
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
                                  &key);

            return NAT_ERR_OK;
        }
    }

    return NAT_ERR_NO_RESOURCE;
}

always_inline nat_port_block_t *
nat_get_pool_for_pvt_ip(nat_vpc_config_t *vpc, nat_port_block_t *nat_pb,
                        ip4_address_t pvt_ip)
{
    uword *data;
    ip4_address_t public_ip;
    nat_port_block_t *pb = NULL;

    data = hash_get_mem(vpc->nat_pvt_ip_ht, &pvt_ip);
    if (data) {
        public_ip.as_u32 = NAT_PVT_IP_HT_PUBLIC_IP(*data);
        pool_foreach (pb, nat_pb,
        ({
            if (pb->addr.as_u32 == public_ip.as_u32) {
                return pb;
            }
        }));
    }

    return NULL;
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
               nat_type_t nat_type, ip4_address_t *sip, u16 *sport)
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

    nat_proto = nat_main.proto_map[protocol];
    if (PREDICT_FALSE(nat_proto == NAT_PROTO_UNKNOWN)) {
        return NAT_ERR_INVALID_PROTOCOL;
    }

    clib_spinlock_lock(&vpc->lock);

    nat_pb = vpc->nat_pb[nat_type][nat_proto - 1];
    if (PREDICT_FALSE(pool_elts(nat_pb) == 0)) {
        return NAT_ERR_NO_RESOURCE;
    }

    // if we have already allocated <pvt_ip, pvt_port>, try to use it
    nat_flow_get_and_add_src_endpoint_mapping(vpc, pvt_ip, pvt_port, sip,
                                              sport);
    if (*sport == 0) {
        // endpoint not found
        // Check if pvt_ip has been used
        pb = nat_get_pool_for_pvt_ip(vpc, nat_pb, pvt_ip);
        if (!pb) {
            // Try random pool
            pb = nat_get_random_pool(vpc, nat_pb);
        }
        if (pb) {
            ret = nat_flow_alloc_inline(vpc_id, dip, dport, pvt_ip, pvt_port,
                    sip, sport, pb, vpc, protocol);
        }
        if (!pb || ret != NAT_ERR_OK) {
            // random pool did not work, try all pools
            u8 allocated = 0;
            pool_foreach (pb, nat_pb,
            ({
                if (allocated || pb->state != NAT_PB_STATE_OK) continue;
                ret = nat_flow_alloc_inline(vpc_id, dip, dport, pvt_ip, pvt_port,
                                            sip, sport, pb, vpc, protocol);
                if (ret == NAT_ERR_OK) allocated = 1;
            }));
        }

    } else {
        // src endpoint found, reuse public ip, port mapping
        pool_foreach (pb, nat_pb,
        ({
            if (pb->addr.as_u32 == sip->as_u32 && pb->state == NAT_PB_STATE_OK) {
                nat_flow_key_t key = { 0 };
                key.dip = dip;
                key.sip = *sip;
                key.sport = *sport;
                key.dport = dport;
                key.protocol = protocol;
                nat_flow_alloc_for_pb(vpc, pb, *sip, *sport, pvt_ip, pvt_port,
                                      &key);
            }
        }));
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
        pool_put_index(nat_main.hw_index_pool, pb->nat_tx_hw_index[index]);
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
                 ip4_address_t sip, u16 sport, ip4_address_t pvt_ip,
                 u16 pvt_port, nat_type_t nat_type)
{
    nat_port_block_t *pb = NULL;
    nat_vpc_config_t *vpc;
    nat_flow_key_t key = { 0 };
    nat_err_t ret = NAT_ERR_NOT_FOUND;
    uword *hw_index_ptr, hw_index;
    nat_proto_t nat_proto;
    nat_port_block_t *nat_pb;

    ASSERT(vpc_id < PDS_MAX_VPC);
    vpc = &nat_main.vpc_config[vpc_id];

    key.dip = dip;
    key.sip = sip;
    key.dport = dport;
    key.sport = sport;
    key.protocol = protocol;

    nat_proto = nat_main.proto_map[protocol];
    if (PREDICT_FALSE(nat_proto == NAT_PROTO_UNKNOWN)) {
        return NAT_ERR_INVALID_PROTOCOL;
    }

    clib_spinlock_lock(&vpc->lock);

    nat_pb = vpc->nat_pb[nat_type][nat_proto - 1];

    nat_flow_get_and_del_src_endpoint_mapping(vpc, pvt_ip, pvt_port);

    hw_index_ptr = hash_get_mem(vpc->nat_flow_ht, &key);
    if (PREDICT_FALSE(!hw_index_ptr)) {
        clib_spinlock_unlock(&vpc->lock);
        return NAT_ERR_NOT_FOUND;
    }
    hw_index = *hw_index_ptr;
    hash_unset_mem_free(&vpc->nat_flow_ht, &key);
    hash_unset_mem_free(&vpc->nat_pvt_ip_ht, &pvt_ip);

    pool_foreach (pb, nat_pb,
    ({
        if (pb->addr.as_u32 == sip.as_u32)
            ret = nat_flow_dealloc_inline(vpc_id, dip, dport, sip,
                                          sport, pb);
            if (ret == NAT_ERR_OK) break;
    }));
    clib_spinlock_unlock(&vpc->lock);

    // free HW NAT indices
    clib_spinlock_lock(&nat_main.lock);
    pool_put_index(nat_main.hw_index_pool, hw_index);
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
nat_usage(u32 vpc_id, u8 protocol, nat_type_t nat_type,
          u32 *num_ports_total, u32 *num_ports_alloc,
          u32 *num_flows_alloc)
{
    nat_port_block_t *pb;
    nat_vpc_config_t *vpc;
    nat_proto_t nat_proto;
    nat_port_block_t *nat_pb;

    nat_proto = nat_main.proto_map[protocol];
    if (nat_proto == NAT_PROTO_UNKNOWN) {
        return NAT_ERR_INVALID_PROTOCOL;
    }

    *num_ports_total = 0;
    *num_ports_alloc = 0;
    *num_flows_alloc = 0;

    ASSERT(vpc_id < PDS_MAX_VPC);

    vpc = &nat_main.vpc_config[vpc_id];

    nat_pb = vpc->nat_pb[nat_type][nat_proto - 1];

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
