//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//

#ifndef __VPP_NAT_PDSA_UDS_HDLR_H__
#define __VPP_NAT_PDSA_UDS_HDLR_H__

#include <nic/vpp/infra/ipc/pdsa_vpp_hdlr.h>

#ifdef __cplusplus
extern "C" {
#endif

#define NAT_ADDRESS_TYPE_INTERNET 0
#define NAT_ADDRESS_TYPE_INFRA 1

//
// data structures
//
typedef struct pds_nat_port_block_export_s {
    uint8_t id[PDS_MAX_KEY_LEN + 1];
    uint32_t addr;
    uint16_t start_port;
    uint16_t end_port;
    uint8_t protocol;
    uint8_t address_type;

    // stats
    uint32_t in_use_cnt;
    uint32_t session_cnt;
} pds_nat_port_block_export_t;

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
