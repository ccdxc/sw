//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//

#ifndef __VPP_NAT_PDSA_UDS_HDLR_H__
#define __VPP_NAT_PDSA_UDS_HDLR_H__

#include <nic/vpp/infra/ipc/pdsa_vpp_hdlr.h>
#include "nat_api.h"

#ifdef __cplusplus
extern "C" {
#endif

//
// data structures
//

// Forward declaration
typedef struct pds_nat_iterate_params_s pds_nat_iterate_params_t;

typedef void (*pds_nat_iterate_t)(pds_nat_iterate_params_t *params);

typedef struct pds_nat_iterate_params_s {
    pds_nat_iterate_t itercb;
    pds_nat_port_block_export_t *pb;
    int fd;
} pds_nat_iterate_params_t;

//
// function prototypes
//

// pdsa_uds_hdlr.cc
void pds_nat_dump_init(void);

// nat44.c
void nat_pb_iterate(pds_nat_iterate_params_t *params);

#ifdef __cplusplus
}
#endif

#endif    // __VPP_NAT_PDSA_UDS_HDLR_H__
