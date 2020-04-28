//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//

#ifndef __VPP_INFRA_OPERD_FLOW_EXPORT_H__
#define __VPP_INFRA_OPERD_FLOW_EXPORT_H__

#include <stdint.h>
#include "nic/operd/decoders/vpp/flow_decoder.h"

#ifdef __cplusplus
extern "C" {
#endif

void pds_operd_export_flow_ip4(void *flow);

void pds_operd_export_flow_ip6(uint8_t *sip,
                               uint8_t *dip,
                               uint8_t ip_proto,
                               uint16_t src_port,
                               uint16_t dst_port,
                               uint16_t lookup_id,
                               const operd_flow_stats_t *flow_stats,
                               uint32_t session_id,
                               uint8_t logtype,
                               uint8_t allow);

void pds_operd_export_flow_l2(uint8_t *smac,
                              uint8_t *dmac,
                              uint8_t ether_type,
                              uint16_t bd_id,
                              const operd_flow_stats_t *flow_stats,
                              uint32_t session_id,
                              uint8_t logtype,
                              uint8_t allow);

#ifdef __cplusplus
}
#endif

#endif  // __VPP_INFRA_OPERD_FLOW_EXPORT_H__

