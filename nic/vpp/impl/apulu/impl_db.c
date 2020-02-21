//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//

#include "impl_db.h"
#include "pdsa_impl_db_hdlr.h"

pds_impl_db_ctx_t impl_db_ctx;

#define POOL_IMPL_DB_ADD(obj)                                   \
    pds_impl_db_##obj##_entry_t *obj##_info;                    \
    pool_get(impl_db_ctx.obj##_pool_base, obj##_info);          \
    u16 offset = obj##_info - impl_db_ctx.obj##_pool_base;      \
    vec_elt(impl_db_ctx.obj##_pool_idx, obj##_hw_id) = offset;

#define POOL_IMPL_DB_GET(obj, hw_id)                            \
    pds_impl_db_##obj##_entry_t *obj##_info;                    \
    u16 _offset = vec_elt(impl_db_ctx.obj##_pool_idx, hw_id);   \
    if (_offset == 0xffff) return NULL;                         \
    obj##_info = pool_elt_at_index(impl_db_ctx.obj##_pool_base, \
                                   _offset);                    \

#define IMPL_DB_ENTRY_DEL(type, obj)                            \
int                                                             \
pds_impl_db_##obj##_del (type hw_id)                            \
{                                                               \
    u16 offset;                                                 \
                                                                \
    offset = vec_elt(impl_db_ctx.obj##_pool_idx, hw_id);        \
    pool_put_index(impl_db_ctx.obj##_pool_base, offset);        \
                                                                \
    vec_elt(impl_db_ctx.obj##_pool_idx, hw_id) = 0xffff;        \
                                                                \
    return 0;                                                   \
}

#define IMPL_DB_ENTRY_GET(type, obj)                            \
pds_impl_db_##obj##_entry_t *                                   \
pds_impl_db_##obj##_get (type hw_id)                            \
{                                                               \
    POOL_IMPL_DB_GET(obj, hw_id);                               \
    return obj##_info;                                          \
}

int
pds_impl_db_vnic_set (uint8_t *mac,
                      uint32_t max_sessions,
                      uint16_t vnic_hw_id,
                      uint16_t subnet_hw_id,
                      uint8_t flow_log_en,
                      uint8_t dot1q,
                      uint8_t dot1ad,
                      uint16_t vlan_id,
                      uint16_t nh_hw_id)
{
    POOL_IMPL_DB_ADD(vnic);

    clib_memcpy(vnic_info->mac, mac, ETH_ADDR_LEN);
    vnic_info->max_sessions = max_sessions;
    vnic_info->subnet_hw_id = subnet_hw_id;
    if (flow_log_en) {
        vnic_info->flow_log_en = 1;
    } else {
        vnic_info->flow_log_en = 0;
    }
    vnic_info->l2_encap_len = sizeof(ethernet_header_t);
    vnic_info->vlan_id = vlan_id;
    if (dot1q) {
        vnic_info->enacp_type = PDS_ETH_ENCAP_DOT1Q;
        vnic_info->l2_encap_len += sizeof(ethernet_vlan_header_t);
    } else if (dot1ad) {
        vnic_info->enacp_type = PDS_ETH_ENCAP_DOT1AD;
        vnic_info->l2_encap_len += (2 * sizeof(ethernet_vlan_header_t));
    } else {
        vnic_info->enacp_type = PDS_ETH_ENCAP_NO_VLAN;
    }
    vnic_info->nh_hw_id = nh_hw_id;

    return 0;
}

IMPL_DB_ENTRY_DEL(uint16_t, vnic);
IMPL_DB_ENTRY_GET(uint16_t, vnic);

void pds_impl_db_vnic_init()
{
    // set all indices default to 0xffff
    vec_validate_init_empty(impl_db_ctx.vnic_pool_idx,
                            (PDS_VPP_MAX_VNIC - 1), 0xffff);

    impl_db_ctx.vnic_pool_base = NULL;

    return;
}

int
pds_impl_db_subnet_set (uint8_t pfx_len,
                        uint32_t vr_ip,
                        uint8_t *mac,
                        uint16_t subnet_hw_id,
                        uint32_t vnid)
{
    POOL_IMPL_DB_ADD(subnet);

    subnet_info->prefix_len = pfx_len;
    ip46_address_set_ip4(&subnet_info->vr_ip, (ip4_address_t *) &vr_ip);
    clib_memcpy(subnet_info->mac, mac, ETH_ADDR_LEN);
    // store vnid in format required for vxlan packet so that no
    // conversion required in packet path
    vnid = vnid << 8;
    subnet_info->vnid = clib_host_to_net_u32(vnid);
    return 0;
}

IMPL_DB_ENTRY_DEL(uint16_t, subnet);
IMPL_DB_ENTRY_GET(uint16_t, subnet);

int
pds_impl_db_device_set (const u8 *mac, const u8 *ip, u8 ip4, u8 bridging_en)
{
    pds_impl_db_device_entry_t *dev = &impl_db_ctx.device;

    dev->bridging_en = bridging_en;
    clib_memcpy(dev->device_mac, mac, ETH_ADDR_LEN);
    if (ip4) {
        ip46_address_set_ip4(&dev->device_ip, (ip4_address_t *) ip);
    } else {
        ip46_address_set_ip6(&dev->device_ip, (ip6_address_t *) ip);
    }
    return 0;
}

pds_impl_db_device_entry_t *
pds_impl_db_device_get (void)
{
    return &impl_db_ctx.device;
}

int
pds_impl_db_device_del (void)
{
    clib_memset(&impl_db_ctx.device, 0, sizeof(pds_impl_db_device_entry_t));
    return 0;
}

u8
pds_impl_db_bridging_en_get (void)
{
    return impl_db_ctx.device.bridging_en;
}

void
pds_impl_db_subnet_init (void)
{
    // set all indices default to 0xffff
    vec_validate_init_empty(impl_db_ctx.subnet_pool_idx,
                            (PDS_VPP_MAX_SUBNET - 1), 0xffff);

    impl_db_ctx.subnet_pool_base = NULL;

    return;
}

int
pds_impl_db_init (void)
{
    clib_memset(&impl_db_ctx, 0, sizeof(impl_db_ctx));
    pds_impl_db_vnic_init();
    pds_impl_db_subnet_init();

    return 0;
}

