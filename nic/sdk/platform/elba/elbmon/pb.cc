//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//
//===----------------------------------------------------------------------===//
///
/// \file
/// This file contains the implementation for displaying pb counters
///
//===----------------------------------------------------------------------===//

#include <stdint.h>
#include <stdio.h>

#include "dtls.hpp"

#include "elb_pbc_c_hdr.h"
#include "elb_top_csr_defines.h"

#include "nic/sdk/platform/pal/include/pal.h"

const char *portname[12] = {"Eth0", "Eth1", "Eth2", "Eth3", "Eth4", "Eth5",
                            "Eth6", "Eth7", "BMC ", "DMA ", "P4EG", "P4IG"};

void
pb_read_iq_counters (int verbose)
{
    uint32_t depth, index, shift;
    bool portempty;
    int port;
    uint32_t iq_cnt[28] = {0};
    uint32_t iq_port_addr[12] = {ELB_PBC_CSR_PORT_0_STA_ACCOUNT_BYTE_ADDRESS,
                                 ELB_PBC_CSR_PORT_1_STA_ACCOUNT_BYTE_ADDRESS,
                                 ELB_PBC_CSR_PORT_2_STA_ACCOUNT_BYTE_ADDRESS,
                                 ELB_PBC_CSR_PORT_3_STA_ACCOUNT_BYTE_ADDRESS,
                                 ELB_PBC_CSR_PORT_4_STA_ACCOUNT_BYTE_ADDRESS,
                                 ELB_PBC_CSR_PORT_5_STA_ACCOUNT_BYTE_ADDRESS,
                                 ELB_PBC_CSR_PORT_6_STA_ACCOUNT_BYTE_ADDRESS,
                                 ELB_PBC_CSR_PORT_7_STA_ACCOUNT_BYTE_ADDRESS,
                                 ELB_PBC_CSR_PORT_8_STA_ACCOUNT_BYTE_ADDRESS,
                                 ELB_PBC_CSR_PORT_9_STA_ACCOUNT_BYTE_ADDRESS,
                                 ELB_PBC_CSR_PORT_10_STA_ACCOUNT_BYTE_ADDRESS,
                                 ELB_PBC_CSR_PORT_11_STA_ACCOUNT_BYTE_ADDRESS};
    int iq_port_queues[12] = {8, 8, 8, 8, 8, 8, 8, 8, 8, 16, 32, 32};

    for (port = 0; port < 12; port++) {
        portempty = true;
        pal_reg_rd32w(ELB_ADDR_BASE_PB_PBC_OFFSET + iq_port_addr[port], iq_cnt,
                      (28 * iq_port_queues[port]) / 32);
        for (int q = 0; q < iq_port_queues[port]; q++) {
            index = (q * 28) / 32;
            shift = (q * 28) % 32;
            depth = iq_cnt[index] >> shift;
            if (shift > 4)
                depth |= iq_cnt[index + 1] << (32 - shift);
            depth &= 0x3fff; // {watermark[14], occupancy[14]}
            if (depth != 0) {
                if (portempty == true) {
                    printf("PB IQ %s", portname[port]);
                    portempty = false;
                }
                printf(" Q[%2d]=%6u entries", q, depth);
            }
        }
        if (portempty == false)
            printf("\n");
    }
}

void
pb_read_counters (int verbose)
{
    uint32_t oq_cnt[14];
    uint32_t depth, index, shift;
    bool portempty;
    int port;

    for (port = 0; port < 12; port++) {
        portempty = true;
        pal_reg_rd32w(ELB_ADDR_BASE_PB_PBC_OFFSET +
                          ELB_PBC_CSR_STA_OQ_BYTE_ADDRESS +
                          (port * 4 * ELB_PBC_CSR_STA_OQ_ARRAY_ELEMENT_SIZE),
                      oq_cnt, 14);

        for (int q = 0; q < 32; q++) {
            index = (q * 14) / 32;
            shift = (q * 14) % 32;
            depth = oq_cnt[index] >> shift;
            if (shift > 18)
                depth |= oq_cnt[index + 1] << (32 - shift);
            depth &= 0x3fff; // {watermark[14], occupancy[14]}
            if (depth != 0) {
                if (portempty == true) {
                    printf("PB OQ %s", portname[port]);
                    portempty = false;
                }
                printf(" Q[%2d]=%6u entries", q, depth);
            }
        }
        if (portempty == false)
            printf("\n");
    }
}
