//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//

#ifndef __VPP_IMPL_APULU_PDSA_IMPL_DB_HDLR_H__
#define __VPP_IMPL_APULU_PDSA_IMPL_DB_HDLR_H__

#include <stdint.h>
#include "nic/vpp/infra/ipc/pdsa_vpp_hdlr.h"

#ifdef __cplusplus
extern "C" {
#endif

int pds_impl_db_vnic_set(uint8_t *mac,
                         uint32_t max_sessions,
                         uint16_t vnic_hw_id,
                         uint16_t subnet_hw_id,
                         uint8_t flow_log_en,
                         uint8_t dot1q,
                         uint8_t dot1ad,
                         uint16_t vlan_id,
                         uint16_t nh_hw_id);

int pds_impl_db_vnic_del(uint16_t vnic_hw_id);

int pds_impl_db_subnet_set(uint32_t subnet_ip,
                           uint8_t pfx_len,
                           uint8_t *mac,
                           uint16_t subnet_hw_id);
int pds_impl_db_subnet_del(uint16_t subnet_hw_id);

int pds_cfg_db_init(void);

int pds_impl_db_init(void);
#ifdef __cplusplus
}
#endif

#endif    // __VPP_IMPL_APULU_PDSA_IMPL_DB_HDLR_H__

