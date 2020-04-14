//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//

#ifndef __VPP_INFRA_API_INTF_H__
#define __VPP_INFRA_API_INTF_H__

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// vpp header files cannot be compiled with cplusplus compiler
// and hence we have this glue header file which contains only
// forward declarations.

uint32_t pds_infra_get_sw_ifindex_by_name(uint8_t *intf);
void pds_infra_set_all_intfs_status(uint8_t admin_up);

#ifdef __cplusplus
}
#endif

#endif  /* __VPP_INFRA_API_INTF_H__ */

