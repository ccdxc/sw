//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//

#ifndef __VPP_IMPL_APULU_MAPPING_H__
#define __VPP_IMPL_APULU_MAPPING_H__

#include <nic/vpp/infra/utils.h>
#include <nic/apollo/packet/apulu/p4_cpu_hdr.h>

#ifdef __cplusplus
extern "C" {
#endif

// Function prototypes
int pds_dst_mac_get(void *p4_rx_meta, mac_addr_t mac_addr, uint32_t dst_addr);
void pds_mapping_table_init(void);
void pds_impl_db_vr_ip_mac_get(uint16_t subnet, uint32_t *vr_ip,
                               uint8_t **vr_mac);

#ifdef __cplusplus
}
#endif
#endif    // __VPP_IMPL_APULU_MAPPING_H__
