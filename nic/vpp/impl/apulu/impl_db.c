//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//

#include <stdint.h>
#include "impl_db.h"
#include "pdsa_impl_db_hdlr.h"

pds_impl_db_ctx_t impl_db_ctx;

#define POOL_IMPL_DB_SET(_obj)                                  \
    pds_##_obj##_impl_db_entry_t *_obj##_info;                  \
    pool_get(impl_db_ctx._obj##_pool_base, _obj##_info);        \
    u16 offset = _obj##_info - impl_db_ctx._obj##_pool_base;    \
    vec_elt(impl_db_ctx._obj##_pool_idx, _obj##_hw_id) = offset;


#define IMPL_DB_ENTRY_DEL(_type, _obj)                          \
int                                                             \
pds_impl_db_##_obj##_del (_type hw_id)                          \
{                                                               \
    u16 offset;                                                 \
                                                                \
    offset = vec_elt(impl_db_ctx._obj##_pool_idx, hw_id);       \
    pool_put_index(impl_db_ctx._obj##_pool_base, offset);       \
                                                                \
    vec_elt(impl_db_ctx._obj##_pool_idx, hw_id) = 0xffff;       \
                                                                \
    return 0;                                                   \
}

int
pds_impl_db_vnic_set (uint8_t *mac,
                      uint32_t max_sessions,
                      uint16_t vnic_hw_id,
                      uint16_t subnet_hw_id)
{
    POOL_IMPL_DB_SET(vnic)

    clib_memcpy(vnic_info->mac, mac, ETH_ADDR_LEN);
    vnic_info->max_sessions = max_sessions;
    vnic_info->subnet_hw_id = subnet_hw_id;

    return 0;
}

IMPL_DB_ENTRY_DEL(uint16_t,vnic)

void pds_vnic_impl_db_init()
{
    impl_db_ctx.vnic_pool_idx = vec_new(u16, PDS_VPP_MAX_VNIC);

    // set all indices default to 0xffff
    vec_validate_init_empty(impl_db_ctx.vnic_pool_idx,
                            PDS_VPP_MAX_VNIC-1, 0xffff);

    impl_db_ctx.vnic_pool_base = NULL;

    return;
}

int
pds_impl_db_subnet_set (uint32_t subnet_ip,
                        uint8_t pfx_len,
                        uint8_t * mac,
                        uint16_t subnet_hw_id)
{
    POOL_IMPL_DB_SET(subnet)

    clib_memcpy(subnet_info->mac, mac, ETH_ADDR_LEN);
    subnet_info->prefix_len = pfx_len;
    ip46_address_set_ip4(&subnet_info->vr_ip, (ip4_address_t *) &subnet_ip);
    return 0;
}

IMPL_DB_ENTRY_DEL(uint16_t,subnet)

void
pds_subnet_impl_db_init (void)
{
    impl_db_ctx.subnet_pool_idx = vec_new(u16, PDS_VPP_MAX_SUBNET);

    // set all indices default to 0xffff
    vec_validate_init_empty(impl_db_ctx.subnet_pool_idx,
                            PDS_VPP_MAX_SUBNET-1, 0xffff);

    impl_db_ctx.subnet_pool_base = NULL;

    return;
}

int
pds_impl_db_init (void)
{
    pds_vnic_impl_db_init();
    pds_subnet_impl_db_init();
    
    return 0;
}

