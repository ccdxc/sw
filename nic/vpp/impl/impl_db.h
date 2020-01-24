//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//

#ifndef __VPP_IMPL_IMPL_DB_H__
#define __VPP_IMPL_IMPL_DB_H__

#include "nic/vpp/infra/ipc/pdsa_vpp_hdlr.h"

#ifdef __cplusplus
extern "C" {
#endif

uint16_t pds_impl_db_vpc_get(const uint8_t key[PDS_MAX_KEY_LEN]);

#ifdef __cplusplus
}
#endif

#endif    // __VPP_IMPL_IMPL_DB_H__

