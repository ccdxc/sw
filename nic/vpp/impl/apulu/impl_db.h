//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//

#ifndef __VPP_IMPL_APULU_DB_H__
#define __VPP_IMPL_APULU_DB_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <vnet/ip/ip.h>
#include <nic/vpp/infra/utils.h>
#include <api.h>

#define PDS_VPP_MAX_SUBNET  64
#define PDS_VPP_MAX_VNIC    128

#define PDS_VPP_INVALID_VPC_HW_ID 0

typedef struct {
    mac_addr_t mac;             // vnic mac
    uint32_t max_sessions;      // max number of sessions from this vnic
    uint16_t subnet_hw_id;      // subnet hwid to index subnet store in infra
} pds_vnic_impl_db_entry_t;


typedef struct {
    mac_addr_t mac;               // subnet mac
    u8  prefix_len;               // subnet prefix
    ip46_address_t  vr_ip;        // subnet ip
} pds_subnet_impl_db_entry_t;

typedef struct {
#define _(_obj)                                        \
    u16 * _obj##_pool_idx;                             \
    pds_##_obj##_impl_db_entry_t * _obj##_pool_base;

    _(subnet)
    _(vnic)
#undef _
    uword *vpc_ht;
} pds_impl_db_ctx_t;

extern pds_impl_db_ctx_t impl_db_ctx;

#ifdef __cplusplus
}
#endif

#endif    // __VPP_IMPL_APULU_DB_H__

