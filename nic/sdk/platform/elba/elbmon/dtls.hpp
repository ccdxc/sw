//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//
//===----------------------------------------------------------------------===//
///
/// \file
/// This file contains the decl. and defn. for elbmon
///
//===----------------------------------------------------------------------===//

#ifndef __DTLS_HPP__
#define __DTLS_HPP__

#include <stdint.h>

#define BIT(n) (1 << n)

//
enum pipeline {
    TXDMA = 0,
    RXDMA = 1,
    P4IG = 2,
    P4EG = 3,
    PIPE_CNT = 4,
};

enum table_type {
    TABLE_PCI = 1,
    TABLE_SRAM = 2,
    TABLE_HBM = 3,
    TABLE_TYPE_CNT = 4,
};

// AXI Bandwidth Monitors
void bwmon_read_counters(void);
void bwmon_reset_counters(void);

// PCIe
void pxb_read_counters(int verbose);
void pxb_reset_counters(int verbose);

// Stage
void sdp_read_counters(int verbose, uint8_t pipeline, uint8_t stage);
void sdp_reset_counters(int verbose, uint8_t pipeline, uint8_t stage);
void te_read_counters(int verbose, uint8_t pipeline, uint8_t stage);
void te_reset_counters(int verbose, uint8_t pipeline, uint8_t stage);
void stg_poll(int verbose, uint8_t pipeline, uint8_t stage);

void mpu_read_counters(int verbose, uint8_t pipeline, uint8_t stage,
                       uint8_t mpu);
void mpu_reset_counters(int verbose, uint8_t pipeline, uint8_t stage,
                        uint8_t mpu);
void mpu_read_table_addr(int verbose, uint8_t pipeline, uint8_t stage,
                         uint8_t mpu);

// RXDMA
void psp_read_counters(int verbose);
void psp_reset_counters(int verbose);
void prd_read_counters(int verbose);
void prd_reset_counters(int verbose);

// TXDMA
void doorbell_read_counters(int verbose);
void doorbell_reset_counters(int verbose);
void txs_read_counters(int verbose);
void txs_reset_counters(int verbose);
void npv_read_counters(int verbose);
void npv_reset_counters(int verbose);
void ptd_read_counters(int verbose);
void ptd_reset_counters(int verbose);

// P4IG/EG
void parser_read_counters(int type, int verbose, uint32_t base);
void parser_reset_counters(int verbose, uint32_t base);

// PB
void pb_read_counters(int verbose);

// CRYPTO
void crypto_read_queues(int verbose);
void crypto_read_counters(int verbose);
void crypto_reset_counters(int verbose);

#endif /* __DTLS_HPP__ */
