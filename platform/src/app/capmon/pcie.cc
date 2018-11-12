/*
 * Copyright (c) 2018, Pensando Systems Inc.
 */
#include <stdint.h>
#include <stdio.h>

#include "dtls.hpp"

#include "cap_pp_c_hdr.h"
#include "cap_pxb_c_hdr.h"
#include "cap_top_csr_defines.h"

#include "pal.h"

void
pxb_read_target_status()
{
    uint32_t pending_ids, port0_pending_ids;

    // Pending counters (per pcie-port)
    pal_reg_rd32w(CAP_ADDR_BASE_PXB_PXB_OFFSET + CAP_PXB_CSR_STA_TGT_AXI_PENDING_BYTE_ADDRESS,
                  &pending_ids, 1);

    port0_pending_ids =
        CAP_PXB_CSR_STA_TGT_AXI_PENDING_STA_TGT_AXI_PENDING_0_2_IDS_P0_GET(pending_ids);

    // Show
    printf("  pending [0]=%d\n", port0_pending_ids);
}

void
pxb_read_target_counters()
{
    uint64_t axi_wr, axi_rd_bytes, axi_rd64;
    uint64_t axi_rd, axi_wr_bytes, axi_wr64;
    uint64_t axi_wr_db64, axi_wr_db32;

    // Write counters
    pal_reg_rd32w(CAP_ADDR_BASE_PXB_PXB_OFFSET + CAP_PXB_CSR_CNT_TGT_TOT_AXI_WR_BYTE_ADDRESS,
                  (uint32_t *)&axi_wr, 2);

    pal_reg_rd32w(CAP_ADDR_BASE_PXB_PXB_OFFSET + CAP_PXB_CSR_STA_TGT_TOT_AXI_WR_BYTE_ADDRESS,
                  (uint32_t *)&axi_wr_bytes, 2);

    pal_reg_rd32w(CAP_ADDR_BASE_PXB_PXB_OFFSET + CAP_PXB_CSR_CNT_TGT_AXI_WR64_BYTE_ADDRESS,
                  (uint32_t *)&axi_wr64, 2);

    // Read counters
    pal_reg_rd32w(CAP_ADDR_BASE_PXB_PXB_OFFSET + CAP_PXB_CSR_CNT_TGT_TOT_AXI_RD_BYTE_ADDRESS,
                  (uint32_t *)&axi_rd, 2);

    pal_reg_rd32w(CAP_ADDR_BASE_PXB_PXB_OFFSET + CAP_PXB_CSR_STA_TGT_TOT_AXI_RD_BYTE_ADDRESS,
                  (uint32_t *)&axi_rd_bytes, 2);

    pal_reg_rd32w(CAP_ADDR_BASE_PXB_PXB_OFFSET + CAP_PXB_CSR_CNT_TGT_AXI_RD64_BYTE_ADDRESS,
                  (uint32_t *)&axi_rd64, 2);

    // Doorbell transaction counters
    pal_reg_rd32w(CAP_ADDR_BASE_PXB_PXB_OFFSET + CAP_PXB_CSR_CNT_TGT_DB64_AXI_WR_BYTE_ADDRESS,
                  (uint32_t *)&axi_wr_db64, 2);

    pal_reg_rd32w(CAP_ADDR_BASE_PXB_PXB_OFFSET + CAP_PXB_CSR_CNT_TGT_DB32_AXI_WR_BYTE_ADDRESS,
                  (uint32_t *)&axi_wr_db32, 2);

    // Show
    printf("  wr=%lu wr_64=%lu bytes=%lu\n"
           "  rd=%lu rd_64=%lu bytes=%lu\n"
           "  db_64=%lu db_32=%lu\n",
           axi_wr, axi_wr64, axi_wr_bytes, axi_rd, axi_rd64, axi_rd_bytes, axi_wr_db64,
           axi_wr_db32);
}

void
pxb_reset_target_counters()
{
    uint64_t zero = 0;

    // Write counters
    pal_reg_wr32w(CAP_ADDR_BASE_PXB_PXB_OFFSET + CAP_PXB_CSR_CNT_TGT_TOT_AXI_WR_BYTE_ADDRESS,
                  (uint32_t *)&zero, 2);

    pal_reg_wr32w(CAP_ADDR_BASE_PXB_PXB_OFFSET + CAP_PXB_CSR_STA_TGT_TOT_AXI_WR_BYTE_ADDRESS,
                  (uint32_t *)&zero, 2);

    pal_reg_wr32w(CAP_ADDR_BASE_PXB_PXB_OFFSET + CAP_PXB_CSR_CNT_TGT_AXI_WR64_BYTE_ADDRESS,
                  (uint32_t *)&zero, 2);

    // Read counters
    pal_reg_wr32w(CAP_ADDR_BASE_PXB_PXB_OFFSET + CAP_PXB_CSR_CNT_TGT_TOT_AXI_RD_BYTE_ADDRESS,
                  (uint32_t *)&zero, 2);

    pal_reg_wr32w(CAP_ADDR_BASE_PXB_PXB_OFFSET + CAP_PXB_CSR_STA_TGT_TOT_AXI_RD_BYTE_ADDRESS,
                  (uint32_t *)&zero, 2);

    pal_reg_wr32w(CAP_ADDR_BASE_PXB_PXB_OFFSET + CAP_PXB_CSR_CNT_TGT_AXI_RD64_BYTE_ADDRESS,
                  (uint32_t *)&zero, 2);

    // Doorbell transaction counters
    pal_reg_wr32w(CAP_ADDR_BASE_PXB_PXB_OFFSET + CAP_PXB_CSR_CNT_TGT_DB64_AXI_WR_BYTE_ADDRESS,
                  (uint32_t *)&zero, 2);

    pal_reg_wr32w(CAP_ADDR_BASE_PXB_PXB_OFFSET + CAP_PXB_CSR_CNT_TGT_DB32_AXI_WR_BYTE_ADDRESS,
                  (uint32_t *)&zero, 2);
}

void
pxb_read_target_err_counters()
{
    uint64_t tlp_drop, rsp_err, ur_cpl;

    // Unsupported request completions
    pal_reg_rd32w(CAP_ADDR_BASE_PXB_PXB_OFFSET + CAP_PXB_CSR_CNT_TGT_RSP_CA_UR_BYTE_ADDRESS,
                  (uint32_t *)&ur_cpl, 2);

    // Target errors
    pal_reg_rd32w(CAP_ADDR_BASE_PXB_PXB_OFFSET + CAP_PXB_CSR_SAT_TGT_RX_DROP_BYTE_ADDRESS,
                  (uint32_t *)&tlp_drop, 2);

    pal_reg_rd32w(CAP_ADDR_BASE_PXB_PXB_OFFSET + CAP_PXB_CSR_SAT_TGT_RSP_ERR_BYTE_ADDRESS,
                  (uint32_t *)&rsp_err, 2);

    // Show
    printf("  ur_cpl=%ld tlp_drop=%ld rsp_err=%ld\n", ur_cpl, tlp_drop, rsp_err);
}

void
pxb_read_initiator_counters()
{
    uint64_t axi_wr, axi_wr64, axi_wr256, axi_wr_bytes;
    uint64_t axi_rd, axi_rd64, axi_rd256, axi_rd_bytes;
    uint64_t atomic_req;

    // Write counters
    pal_reg_rd32w(CAP_ADDR_BASE_PXB_PXB_OFFSET + CAP_PXB_CSR_STA_ITR_TOT_AXI_WR_BYTE_ADDRESS,
                  (uint32_t *)&axi_wr_bytes, 2);

    pal_reg_rd32w(CAP_ADDR_BASE_PXB_PXB_OFFSET + CAP_PXB_CSR_CNT_ITR_TOT_AXI_WR_BYTE_OFFSET,
                  (uint32_t *)&axi_wr, 2);

    pal_reg_rd32w(CAP_ADDR_BASE_PXB_PXB_OFFSET + CAP_PXB_CSR_CNT_ITR_AXI_WR64_BYTE_OFFSET,
                  (uint32_t *)&axi_wr64, 2);

    pal_reg_rd32w(CAP_ADDR_BASE_PXB_PXB_OFFSET + CAP_PXB_CSR_CNT_ITR_AXI_WR256_BYTE_OFFSET,
                  (uint32_t *)&axi_wr256, 2);

    // Read counters
    pal_reg_rd32w(CAP_ADDR_BASE_PXB_PXB_OFFSET + CAP_PXB_CSR_STA_ITR_TOT_AXI_RD_BYTE_ADDRESS,
                  (uint32_t *)&axi_rd_bytes, 2);

    pal_reg_rd32w(CAP_ADDR_BASE_PXB_PXB_OFFSET + CAP_PXB_CSR_CNT_ITR_TOT_AXI_RD_BYTE_OFFSET,
                  (uint32_t *)&axi_rd, 2);

    pal_reg_rd32w(CAP_ADDR_BASE_PXB_PXB_OFFSET + CAP_PXB_CSR_CNT_ITR_AXI_RD64_BYTE_OFFSET,
                  (uint32_t *)&axi_rd64, 2);

    pal_reg_rd32w(CAP_ADDR_BASE_PXB_PXB_OFFSET + CAP_PXB_CSR_CNT_ITR_AXI_RD256_BYTE_OFFSET,
                  (uint32_t *)&axi_rd256, 2);

    pal_reg_rd32w(CAP_ADDR_BASE_PXB_PXB_OFFSET + CAP_PXB_CSR_CNT_ITR_TOT_ATOMIC_REQ_BYTE_OFFSET,
                  (uint32_t *)&atomic_req, 2);

    // Show
    printf("  wr=%lu wr_64=%lu wr_256=%lu bytes=%lu\n", axi_wr, axi_wr64, axi_wr256, axi_wr_bytes);
    printf("  rd=%lu rd_64=%lu wr_256=%lu bytes=%lu\n", axi_rd, axi_rd64, axi_rd256, axi_rd_bytes);
    printf("  atomic=%ld\n", atomic_req);
}

void
pxb_reset_initiator_counters()
{
    uint64_t zero = 64;

    // Write counters
    pal_reg_wr32w(CAP_ADDR_BASE_PXB_PXB_OFFSET + CAP_PXB_CSR_STA_ITR_TOT_AXI_WR_BYTE_ADDRESS,
                  (uint32_t *)&zero, 2);

    pal_reg_wr32w(CAP_ADDR_BASE_PXB_PXB_OFFSET + CAP_PXB_CSR_CNT_ITR_TOT_AXI_WR_BYTE_OFFSET,
                  (uint32_t *)&zero, 2);

    pal_reg_wr32w(CAP_ADDR_BASE_PXB_PXB_OFFSET + CAP_PXB_CSR_CNT_ITR_AXI_WR64_BYTE_OFFSET,
                  (uint32_t *)&zero, 2);

    pal_reg_wr32w(CAP_ADDR_BASE_PXB_PXB_OFFSET + CAP_PXB_CSR_CNT_ITR_AXI_WR256_BYTE_OFFSET,
                  (uint32_t *)&zero, 2);

    // Read counters
    pal_reg_wr32w(CAP_ADDR_BASE_PXB_PXB_OFFSET + CAP_PXB_CSR_STA_ITR_TOT_AXI_RD_BYTE_ADDRESS,
                  (uint32_t *)&zero, 2);

    pal_reg_wr32w(CAP_ADDR_BASE_PXB_PXB_OFFSET + CAP_PXB_CSR_CNT_ITR_TOT_AXI_RD_BYTE_OFFSET,
                  (uint32_t *)&zero, 2);

    pal_reg_wr32w(CAP_ADDR_BASE_PXB_PXB_OFFSET + CAP_PXB_CSR_CNT_ITR_AXI_RD64_BYTE_OFFSET,
                  (uint32_t *)&zero, 2);

    pal_reg_wr32w(CAP_ADDR_BASE_PXB_PXB_OFFSET + CAP_PXB_CSR_CNT_ITR_AXI_RD256_BYTE_OFFSET,
                  (uint32_t *)&zero, 2);

    pal_reg_wr32w(CAP_ADDR_BASE_PXB_PXB_OFFSET + CAP_PXB_CSR_CNT_ITR_TOT_ATOMIC_REQ_BYTE_OFFSET,
                  (uint32_t *)&zero, 2);
}

void
pxb_read_initiator_status()
{
    uint16_t cfg_rdlat[4];
    uint64_t rd_lat0, rd_lat1, rd_lat2, rd_lat3;
    uint32_t axi_rd_pending, axi_wr_pending;

    // Pending Transaction status
    pal_reg_rd32w(CAP_ADDR_BASE_PXB_PXB_OFFSET + CAP_PXB_CSR_STA_ITR_AXI_WR_NUM_IDS_BYTE_ADDRESS,
                  &axi_wr_pending, 1);

    pal_reg_rd32w(CAP_ADDR_BASE_PXB_PXB_OFFSET + CAP_PXB_CSR_STA_ITR_AXI_RD_NUM_IDS_BYTE_ADDRESS,
                  &axi_rd_pending, 1);

    // Latency counter config
    pal_reg_rd32w(CAP_ADDR_BASE_PXB_PXB_OFFSET + CAP_PXB_CSR_CFG_ITR_RDLAT_MEASURE_BYTE_ADDRESS,
                  (uint32_t *)&cfg_rdlat, 2);

    // Latency counters
    pal_reg_rd32w(CAP_ADDR_BASE_PXB_PXB_OFFSET + CAP_PXB_CSR_SAT_ITR_RDLAT0_BYTE_OFFSET,
                  (uint32_t *)&rd_lat0, 2);
    pal_reg_rd32w(CAP_ADDR_BASE_PXB_PXB_OFFSET + CAP_PXB_CSR_SAT_ITR_RDLAT1_BYTE_OFFSET,
                  (uint32_t *)&rd_lat1, 2);
    pal_reg_rd32w(CAP_ADDR_BASE_PXB_PXB_OFFSET + CAP_PXB_CSR_SAT_ITR_RDLAT2_BYTE_OFFSET,
                  (uint32_t *)&rd_lat2, 2);
    pal_reg_rd32w(CAP_ADDR_BASE_PXB_PXB_OFFSET + CAP_PXB_CSR_SAT_ITR_RDLAT3_BYTE_OFFSET,
                  (uint32_t *)&rd_lat3, 2);

    printf("  wr_pend=%u rd_pend=%u\n", axi_wr_pending, axi_rd_pending);

    auto rd_total = rd_lat0 + rd_lat1 + rd_lat2 + rd_lat3;
    printf("  read latency (clks) >%d=%.2f%% >%d=%.2f%% >%d=%.2f%% >%d=%.2f%%\n", cfg_rdlat[3],
           (rd_lat0 * 100.0) / rd_total, cfg_rdlat[2], (rd_lat1 * 100.0) / rd_total, cfg_rdlat[1],
           (rd_lat2 * 100.0) / rd_total, cfg_rdlat[0], (rd_lat3 * 100.0) / rd_total);
}

void
pxb_read_pport_status(uint8_t port)
{
    uint32_t stall;
    uint8_t rx_stalls = 0, tx_stalls = 0;
    int polls = 100;

    // Status counters
    for (int i = 0; i < polls; i++) {
        pal_reg_rd32w(CAP_ADDR_BASE_PP_PP_OFFSET + CAP_PP_CSR_STA_DEBUG_STALL_BYTE_ADDRESS, &stall,
                      1);
        rx_stalls += CAP_PP_CSR_STA_DEBUG_STALL_RX_GET(stall) & BIT(0);
        tx_stalls += CAP_PP_CSR_STA_DEBUG_STALL_TX_GET(stall) & BIT(0);
    }

    // Show
    printf("  rx_stl=%.2f%% tx_stl=%.2f%%\n", (rx_stalls * 100.0) / polls,
           (tx_stalls * 100.0) / polls);
}

void
pxb_read_pport_counters(uint8_t port)
{
    uint64_t rx_req_tlp, rx_cpl_tlp, tx_req_tlp, tx_cpl_tlp;

    // TLP counters
    pal_reg_rd32w(CAP_ADDR_BASE_PP_PP_OFFSET + CAP_PP_CSR_PORT_C_CNT_C_TL_RX_REQ_BYTE_ADDRESS,
                  (uint32_t *)&rx_req_tlp, 2);

    pal_reg_rd32w(CAP_ADDR_BASE_PP_PP_OFFSET + CAP_PP_CSR_PORT_C_CNT_C_TL_RX_CPL_BYTE_ADDRESS,
                  (uint32_t *)&rx_cpl_tlp, 2);

    pal_reg_rd32w(CAP_ADDR_BASE_PP_PP_OFFSET + CAP_PP_CSR_PORT_C_CNT_C_TL_TX_REQ_BYTE_ADDRESS,
                  (uint32_t *)&tx_req_tlp, 2);

    pal_reg_rd32w(CAP_ADDR_BASE_PP_PP_OFFSET + CAP_PP_CSR_PORT_C_CNT_C_TL_TX_CPL_BYTE_ADDRESS,
                  (uint32_t *)&tx_cpl_tlp, 2);

    // Show
    printf("  rx_req_tlp=%ld rx_cpl_tlp=%ld tx_req_tlp=%ld tx_cpl_tlp=%ld\n", rx_req_tlp,
           rx_cpl_tlp, tx_req_tlp, tx_cpl_tlp);
}

void
pxb_read_pport_err_counters(uint8_t port)
{
    uint64_t rx_bad_tlp, rx_bad_dllp, rx_nak_received, rx_nullified, rxbfr_overflow;
    uint64_t tx_nak_sent, txbuf_ecc_err;

    uint64_t fcpe, fc_timeout, replay_num_err, replay_timer_err;
    uint64_t core_initiated_recovery, ltssm_state_changed;
    uint64_t skp_os_err, deskew_err, phystatus_err;

    uint64_t rx_malform_tlp, rx_framing_err, rx_ecrc_err, rxbuf_ecc_err, rx_nullify,
        rx_watchdog_nullify, rx_unsupp;
    uint64_t tx_drop, txbfr_overflow;

    // Error counters
    pal_reg_rd32w(CAP_ADDR_BASE_PP_PP_OFFSET +
                      CAP_PP_CSR_PORT_P_SAT_P_PORT_CNT_RX_BAD_TLP_BYTE_ADDRESS,
                  (uint32_t *)&rx_bad_tlp, 2);

    pal_reg_rd32w(CAP_ADDR_BASE_PP_PP_OFFSET +
                      CAP_PP_CSR_PORT_P_SAT_P_PORT_CNT_RX_BAD_DLLP_BYTE_ADDRESS,
                  (uint32_t *)&rx_bad_dllp, 2);

    pal_reg_rd32w(CAP_ADDR_BASE_PP_PP_OFFSET +
                      CAP_PP_CSR_PORT_P_SAT_P_PORT_CNT_RX_NAK_RECEIVED_BYTE_ADDRESS,
                  (uint32_t *)&rx_nak_received, 2);

    pal_reg_rd32w(CAP_ADDR_BASE_PP_PP_OFFSET +
                      CAP_PP_CSR_PORT_P_SAT_P_PORT_CNT_RX_NULLIFIED_BYTE_ADDRESS,
                  (uint32_t *)&rx_nullified, 2);

    pal_reg_rd32w(CAP_ADDR_BASE_PP_PP_OFFSET +
                      CAP_PP_CSR_PORT_P_SAT_P_PORT_CNT_RXBFR_OVERFLOW_BYTE_ADDRESS,
                  (uint32_t *)&rxbfr_overflow, 2);

    //
    pal_reg_rd32w(CAP_ADDR_BASE_PP_PP_OFFSET +
                      CAP_PP_CSR_PORT_P_SAT_P_PORT_CNT_TX_NAK_SENT_BYTE_ADDRESS,
                  (uint32_t *)&tx_nak_sent, 2);

    pal_reg_rd32w(CAP_ADDR_BASE_PP_PP_OFFSET +
                      CAP_PP_CSR_PORT_P_SAT_P_PORT_CNT_TXBUF_ECC_ERR_BYTE_ADDRESS,
                  (uint32_t *)&txbuf_ecc_err, 2);

    //
    pal_reg_rd32w(CAP_ADDR_BASE_PP_PP_OFFSET + CAP_PP_CSR_PORT_P_SAT_P_PORT_CNT_FCPE_BYTE_ADDRESS,
                  (uint32_t *)&fcpe, 2);

    pal_reg_rd32w(CAP_ADDR_BASE_PP_PP_OFFSET +
                      CAP_PP_CSR_PORT_P_SAT_P_PORT_CNT_FC_TIMEOUT_BYTE_ADDRESS,
                  (uint32_t *)&fc_timeout, 2);

    pal_reg_rd32w(CAP_ADDR_BASE_PP_PP_OFFSET +
                      CAP_PP_CSR_PORT_P_SAT_P_PORT_CNT_REPLAY_NUM_ERR_BYTE_ADDRESS,
                  (uint32_t *)&replay_num_err, 2);

    pal_reg_rd32w(CAP_ADDR_BASE_PP_PP_OFFSET +
                      CAP_PP_CSR_PORT_P_SAT_P_PORT_CNT_REPLAY_TIMER_ERR_BYTE_ADDRESS,
                  (uint32_t *)&replay_timer_err, 2);

    pal_reg_rd32w(CAP_ADDR_BASE_PP_PP_OFFSET +
                      CAP_PP_CSR_PORT_P_SAT_P_PORT_CNT_CORE_INITIATED_RECOVERY_BYTE_ADDRESS,
                  (uint32_t *)&core_initiated_recovery, 2);

    pal_reg_rd32w(CAP_ADDR_BASE_PP_PP_OFFSET +
                      CAP_PP_CSR_PORT_P_SAT_P_PORT_CNT_LTSSM_STATE_CHANGED_BYTE_ADDRESS,
                  (uint32_t *)&ltssm_state_changed, 2);

    pal_reg_rd32w(CAP_ADDR_BASE_PP_PP_OFFSET +
                      CAP_PP_CSR_PORT_P_SAT_P_PORT_CNT_8B10B_128B130B_SKP_OS_ERR_BYTE_ADDRESS,
                  (uint32_t *)&skp_os_err, 2);

    pal_reg_rd32w(CAP_ADDR_BASE_PP_PP_OFFSET +
                      CAP_PP_CSR_PORT_P_SAT_P_PORT_CNT_DESKEW_ERR_BYTE_ADDRESS,
                  (uint32_t *)&deskew_err, 2);

    pal_reg_rd32w(CAP_ADDR_BASE_PP_PP_OFFSET +
                      CAP_PP_CSR_PORT_P_SAT_P_PORT_CNT_PHYSTATUS_ERR_BYTE_ADDRESS,
                  (uint32_t *)&phystatus_err, 2);

    pal_reg_rd32w(CAP_ADDR_BASE_PP_PP_OFFSET +
                      CAP_PP_CSR_PORT_C_SAT_C_PORT_CNT_RX_MALFORM_TLP_BYTE_ADDRESS,
                  (uint32_t *)&rx_malform_tlp, 2);

    pal_reg_rd32w(CAP_ADDR_BASE_PP_PP_OFFSET +
                      CAP_PP_CSR_PORT_C_SAT_C_PORT_CNT_RX_FRAMING_ERR_BYTE_ADDRESS,
                  (uint32_t *)&rx_framing_err, 2);

    pal_reg_rd32w(CAP_ADDR_BASE_PP_PP_OFFSET +
                      CAP_PP_CSR_PORT_C_SAT_C_PORT_CNT_RX_ECRC_ERR_BYTE_ADDRESS,
                  (uint32_t *)&rx_ecrc_err, 2);

    pal_reg_rd32w(CAP_ADDR_BASE_PP_PP_OFFSET +
                      CAP_PP_CSR_PORT_C_SAT_C_PORT_CNT_RX_NULLIFY_BYTE_ADDRESS,
                  (uint32_t *)&rx_nullify, 2);

    pal_reg_rd32w(CAP_ADDR_BASE_PP_PP_OFFSET +
                      CAP_PP_CSR_PORT_C_SAT_C_PORT_CNT_RX_WATCHDOG_NULLIFY_BYTE_ADDRESS,
                  (uint32_t *)&rx_watchdog_nullify, 2);

    pal_reg_rd32w(CAP_ADDR_BASE_PP_PP_OFFSET +
                      CAP_PP_CSR_PORT_C_SAT_C_PORT_CNT_RX_UNSUPP_BYTE_ADDRESS,
                  (uint32_t *)&rx_unsupp, 2);

    pal_reg_rd32w(CAP_ADDR_BASE_PP_PP_OFFSET +
                      CAP_PP_CSR_PORT_C_SAT_C_PORT_CNT_RXBUF_ECC_ERR_BYTE_ADDRESS,
                  (uint32_t *)&rxbuf_ecc_err, 2);

    pal_reg_rd32w(CAP_ADDR_BASE_PP_PP_OFFSET +
                      CAP_PP_CSR_PORT_C_SAT_C_PORT_CNT_TX_DROP_BYTE_ADDRESS,
                  (uint32_t *)&tx_drop, 2);

    pal_reg_rd32w(CAP_ADDR_BASE_PP_PP_OFFSET +
                      CAP_PP_CSR_PORT_C_SAT_C_PORT_CNT_TXBFR_OVERFLOW_BYTE_ADDRESS,
                  (uint32_t *)&txbfr_overflow, 2);

    // Show
    printf("  rx_bad_tlp=%ld rx_bad_dllp=%ld rx_nak_rcvd=%ld\n"
           "  rx_nullified=%ld rxbfr_overflow=%ld\n"
           "  tx_nak_sent=%ld txbuf_ecc_err=%ld\n"
           "  fcpe=%ld fc_timeout=%ld replay_num_err=%ld replay_timer_err=%ld\n"
           "  core_initiated_recovery=%ld ltssm_state_changed=%ld\n"
           "  skp_os_err=%ld deskew_err=%ld phystatus_err=%ld\n"
           "  rx_malform_tlp=%ld rx_framing_err=%ld rx_ecrc_err=%ld\n"
           "  rx_nullify=%ld rx_watchdog_nullify=%ld rx_unsupp=%ld rxbuf_ecc_err=%ld\n"
           "  tx_drop=%ld txbfr_overflow=%ld\n",
           rx_bad_tlp, rx_bad_dllp, rx_nak_received, rx_nullified, rxbfr_overflow, tx_nak_sent,
           txbuf_ecc_err, fcpe, fc_timeout, replay_num_err, replay_timer_err,
           core_initiated_recovery, ltssm_state_changed, skp_os_err, deskew_err, phystatus_err,
           rx_malform_tlp, rx_framing_err, rx_ecrc_err, rx_nullify, rx_watchdog_nullify, rx_unsupp,
           rxbuf_ecc_err, tx_drop, txbfr_overflow);
}

void
pxb_read_counters(int verbose)
{
    printf(" Target\n");
    pxb_read_target_status();
    if (verbose) {
        pxb_read_target_counters();
        pxb_read_target_err_counters();
    }

    printf(" Initiator\n");
    pxb_read_initiator_status();
    if (verbose) {
        pxb_read_initiator_counters();
    }

    printf(" Port 0\n");
    pxb_read_pport_status(0);
    if (verbose) {
        pxb_read_pport_counters(0);
        pxb_read_pport_err_counters(0);
    }
}

void
pxb_reset_counters(int verbose)
{
    if (verbose) {
        pxb_reset_target_counters();
    }

    if (verbose) {
        pxb_reset_initiator_counters();
    }
}
