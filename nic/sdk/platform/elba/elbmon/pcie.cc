//
// {C} Copyright 2018 Pensando Systems Inc. All rights reserved
//
//===----------------------------------------------------------------------===//
///
/// \file
/// This file contains the implementation for displaying generic PCIE info
/// capmon -p
///
//===----------------------------------------------------------------------===//

#include <stdint.h>
#include <stdio.h>

#include "dtls.hpp"

#include "elb_pp_c_hdr.h"
#include "elb_pxb_c_hdr.h"
#include "elb_pxc_c_hdr.h"
#include "elb_top_csr_defines.h"

#include "platform/pal/include/pal.h"

#include "elbmon.hpp"

void
pxb_read_target_status()
{
    uint32_t pending_ids, port0_pending_ids;

    // Pending counters (per pcie-port)
    pal_reg_rd32w(ELB_ADDR_BASE_PXB_PXB_OFFSET +
                      ELB_PXB_CSR_STA_TGT_AXI_PENDING_BYTE_ADDRESS,
                  &pending_ids, 1);

    port0_pending_ids =
        ELB_PXB_CSR_STA_TGT_AXI_PENDING_STA_TGT_AXI_PENDING_0_2_IDS_P0_GET(
            pending_ids);

    asic->target_pending = port0_pending_ids;
    // Show
}

void
pxb_read_target_counters()
{
    uint64_t axi_wr, axi_rd_bytes, axi_rd64;
    uint64_t axi_rd, axi_wr_bytes, axi_wr64;
    uint64_t axi_wr_db64, axi_wr_db32;

    // Write counters
    pal_reg_rd32w(ELB_ADDR_BASE_PXB_PXB_OFFSET +
                      ELB_PXB_CSR_CNT_TGT_TOT_AXI_WR_BYTE_ADDRESS,
                  (uint32_t *)&axi_wr, 2);

    pal_reg_rd32w(ELB_ADDR_BASE_PXB_PXB_OFFSET +
                      ELB_PXB_CSR_CNT_TGT_TOT_AXI_PAYLOAD_WR_BYTE_ADDRESS,
                  (uint32_t *)&axi_wr_bytes, 2);

    pal_reg_rd32w(ELB_ADDR_BASE_PXB_PXB_OFFSET +
                      ELB_PXB_CSR_CNT_TGT_AXI_WR64_BYTE_ADDRESS,
                  (uint32_t *)&axi_wr64, 2);

    // Read counters
    pal_reg_rd32w(ELB_ADDR_BASE_PXB_PXB_OFFSET +
                      ELB_PXB_CSR_CNT_TGT_TOT_AXI_RD_BYTE_ADDRESS,
                  (uint32_t *)&axi_rd, 2);

    pal_reg_rd32w(ELB_ADDR_BASE_PXB_PXB_OFFSET +
                      ELB_PXB_CSR_CNT_TGT_TOT_AXI_PAYLOAD_RD_BYTE_ADDRESS,
                  (uint32_t *)&axi_rd_bytes, 2);

    pal_reg_rd32w(ELB_ADDR_BASE_PXB_PXB_OFFSET +
                      ELB_PXB_CSR_CNT_TGT_AXI_RD64_BYTE_ADDRESS,
                  (uint32_t *)&axi_rd64, 2);

    // Doorbell transaction counters
    pal_reg_rd32w(ELB_ADDR_BASE_PXB_PXB_OFFSET +
                      ELB_PXB_CSR_CNT_TGT_DB64_AXI_WR_BYTE_ADDRESS,
                  (uint32_t *)&axi_wr_db64, 2);

    pal_reg_rd32w(ELB_ADDR_BASE_PXB_PXB_OFFSET +
                      ELB_PXB_CSR_CNT_TGT_DB32_AXI_WR_BYTE_ADDRESS,
                  (uint32_t *)&axi_wr_db32, 2);

    // Fetch
    asic->axi_wr_tgt = axi_wr;
    asic->axi_wr_bytes_tgt = axi_wr_bytes;
    asic->axi_rd_tgt = axi_rd;
    asic->axi_rd_bytes_tgt = axi_rd_bytes;
    asic->axi_wr_64_tgt = axi_wr64;
    asic->axi_rd_64_tgt = axi_rd64;
    asic->axi_wr_db64 = axi_wr_db64;
    asic->axi_wr_db32 = axi_wr_db32;

    // Show
}

void
pxb_reset_target_counters()
{
    uint64_t zero = 0;

    // Write counters
    pal_reg_wr32w(ELB_ADDR_BASE_PXB_PXB_OFFSET +
                      ELB_PXB_CSR_CNT_TGT_TOT_AXI_WR_BYTE_ADDRESS,
                  (uint32_t *)&zero, 2);

    pal_reg_wr32w(ELB_ADDR_BASE_PXB_PXB_OFFSET +
                      ELB_PXB_CSR_CNT_TGT_TOT_AXI_PAYLOAD_WR_BYTE_ADDRESS,
                  (uint32_t *)&zero, 2);

    pal_reg_wr32w(ELB_ADDR_BASE_PXB_PXB_OFFSET +
                      ELB_PXB_CSR_CNT_TGT_AXI_WR64_BYTE_ADDRESS,
                  (uint32_t *)&zero, 2);

    // Read counters
    pal_reg_wr32w(ELB_ADDR_BASE_PXB_PXB_OFFSET +
                      ELB_PXB_CSR_CNT_TGT_TOT_AXI_RD_BYTE_ADDRESS,
                  (uint32_t *)&zero, 2);

    pal_reg_wr32w(ELB_ADDR_BASE_PXB_PXB_OFFSET +
                      ELB_PXB_CSR_CNT_TGT_TOT_AXI_PAYLOAD_RD_BYTE_ADDRESS,
                  (uint32_t *)&zero, 2);

    pal_reg_wr32w(ELB_ADDR_BASE_PXB_PXB_OFFSET +
                      ELB_PXB_CSR_CNT_TGT_AXI_RD64_BYTE_ADDRESS,
                  (uint32_t *)&zero, 2);

    // Doorbell transaction counters
    pal_reg_wr32w(ELB_ADDR_BASE_PXB_PXB_OFFSET +
                      ELB_PXB_CSR_CNT_TGT_DB64_AXI_WR_BYTE_ADDRESS,
                  (uint32_t *)&zero, 2);

    pal_reg_wr32w(ELB_ADDR_BASE_PXB_PXB_OFFSET +
                      ELB_PXB_CSR_CNT_TGT_DB32_AXI_WR_BYTE_ADDRESS,
                  (uint32_t *)&zero, 2);
}

void
pxb_read_target_err_counters()
{
    uint64_t tlp_drop, ur_cpl;
    uint32_t pxb_sat_tgt_rsp_err;

    // Unsupported request completions
    pal_reg_rd32w(ELB_ADDR_BASE_PXB_PXB_OFFSET +
                      ELB_PXB_CSR_CNT_TGT_RSP_CA_UR_BYTE_ADDRESS,
                  (uint32_t *)&ur_cpl, 2);

    // Target errors
    pal_reg_rd32w(ELB_ADDR_BASE_PXB_PXB_OFFSET +
                      ELB_PXB_CSR_SAT_TGT_RX_DROP_BYTE_ADDRESS,
                  (uint32_t *)&tlp_drop, 2);

    pal_reg_rd32w(ELB_ADDR_BASE_PXB_PXB_OFFSET +
                      ELB_PXB_CSR_SAT_TGT_RSP_ERR_BYTE_ADDRESS,
                  (uint32_t *)&pxb_sat_tgt_rsp_err, 1);

    // Show
    asic->ur_cpl = ur_cpl;
    asic->tlp_drop = tlp_drop;
    asic->ind_cnxt_mismatch = pxb_sat_tgt_rsp_err & 0xff;
    asic->rresp_err = pxb_sat_tgt_rsp_err >> 8 & 0xff;
    asic->bresp_err = pxb_sat_tgt_rsp_err >> 16 & 0xff;
}

void
capmon_asic_data_store1(uint64_t axi_wr, uint64_t axi_wr_64,
                        uint64_t axi_wr_256, uint64_t axi_wr_bytes,
                        uint64_t axi_rd, uint64_t axi_rd_64,
                        uint64_t axi_rd_256, uint64_t axi_rd_bytes,
                        uint64_t atomic_req)
{
    asic->axi_wr = axi_wr;
    asic->axi_wr64 = axi_wr_64;
    asic->axi_wr256 = axi_wr_256;
    asic->axi_wr_bytes = axi_wr_bytes;
    asic->axi_rd = axi_rd;
    asic->axi_rd64 = axi_rd_64;
    asic->axi_rd256 = axi_rd_256;
    asic->axi_rd_bytes = axi_rd_bytes;
    asic->atomic_req = atomic_req;
}

void
pxb_read_initiator_counters()
{
    uint64_t axi_wr, axi_wr64, axi_wr256, axi_wr_bytes;
    uint64_t axi_rd, axi_rd64, axi_rd256, axi_rd_bytes;
    uint64_t atomic_req;

    // Write counters
    pal_reg_rd32w(ELB_ADDR_BASE_PXB_PXB_OFFSET +
                      ELB_PXB_CSR_CNT_ITR_TOT_AXI_PAYLOAD_WR_BYTE_ADDRESS,
                  (uint32_t *)&axi_wr_bytes, 2);

    pal_reg_rd32w(ELB_ADDR_BASE_PXB_PXB_OFFSET +
                      ELB_PXB_CSR_CNT_ITR_TOT_AXI_WR_BYTE_OFFSET,
                  (uint32_t *)&axi_wr, 2);

    pal_reg_rd32w(ELB_ADDR_BASE_PXB_PXB_OFFSET +
                      ELB_PXB_CSR_CNT_ITR_AXI_WR64_BYTE_OFFSET,
                  (uint32_t *)&axi_wr64, 2);

    pal_reg_rd32w(ELB_ADDR_BASE_PXB_PXB_OFFSET +
                      ELB_PXB_CSR_CNT_ITR_AXI_WR256_BYTE_OFFSET,
                  (uint32_t *)&axi_wr256, 2);

    // Read counters
    pal_reg_rd32w(ELB_ADDR_BASE_PXB_PXB_OFFSET +
                      ELB_PXB_CSR_CNT_ITR_TOT_AXI_PAYLOAD_RD_BYTE_ADDRESS,
                  (uint32_t *)&axi_rd_bytes, 2);

    pal_reg_rd32w(ELB_ADDR_BASE_PXB_PXB_OFFSET +
                      ELB_PXB_CSR_CNT_ITR_TOT_AXI_RD_BYTE_OFFSET,
                  (uint32_t *)&axi_rd, 2);

    pal_reg_rd32w(ELB_ADDR_BASE_PXB_PXB_OFFSET +
                      ELB_PXB_CSR_CNT_ITR_AXI_RD64_BYTE_OFFSET,
                  (uint32_t *)&axi_rd64, 2);

    pal_reg_rd32w(ELB_ADDR_BASE_PXB_PXB_OFFSET +
                      ELB_PXB_CSR_CNT_ITR_AXI_RD256_BYTE_OFFSET,
                  (uint32_t *)&axi_rd256, 2);

    pal_reg_rd32w(ELB_ADDR_BASE_PXB_PXB_OFFSET +
                      ELB_PXB_CSR_CNT_ITR_TOT_ATOMIC_REQ_BYTE_OFFSET,
                  (uint32_t *)&atomic_req, 2);

    // Show

    capmon_asic_data_store1(axi_wr, axi_wr64, axi_wr256, axi_wr_bytes, axi_rd,
                            axi_rd64, axi_rd256, axi_rd_bytes, atomic_req);
}

void
pxb_reset_initiator_counters()
{
    uint64_t zero = 64;

    // Write counters
    pal_reg_wr32w(ELB_ADDR_BASE_PXB_PXB_OFFSET +
                      ELB_PXB_CSR_CNT_ITR_TOT_AXI_PAYLOAD_WR_BYTE_ADDRESS,
                  (uint32_t *)&zero, 2);

    pal_reg_wr32w(ELB_ADDR_BASE_PXB_PXB_OFFSET +
                      ELB_PXB_CSR_CNT_ITR_TOT_AXI_WR_BYTE_OFFSET,
                  (uint32_t *)&zero, 2);

    pal_reg_wr32w(ELB_ADDR_BASE_PXB_PXB_OFFSET +
                      ELB_PXB_CSR_CNT_ITR_AXI_WR64_BYTE_OFFSET,
                  (uint32_t *)&zero, 2);

    pal_reg_wr32w(ELB_ADDR_BASE_PXB_PXB_OFFSET +
                      ELB_PXB_CSR_CNT_ITR_AXI_WR256_BYTE_OFFSET,
                  (uint32_t *)&zero, 2);

    // Read counters
    pal_reg_wr32w(ELB_ADDR_BASE_PXB_PXB_OFFSET +
                      ELB_PXB_CSR_CNT_ITR_TOT_AXI_PAYLOAD_RD_BYTE_ADDRESS,
                  (uint32_t *)&zero, 2);

    pal_reg_wr32w(ELB_ADDR_BASE_PXB_PXB_OFFSET +
                      ELB_PXB_CSR_CNT_ITR_TOT_AXI_RD_BYTE_OFFSET,
                  (uint32_t *)&zero, 2);

    pal_reg_wr32w(ELB_ADDR_BASE_PXB_PXB_OFFSET +
                      ELB_PXB_CSR_CNT_ITR_AXI_RD64_BYTE_OFFSET,
                  (uint32_t *)&zero, 2);

    pal_reg_wr32w(ELB_ADDR_BASE_PXB_PXB_OFFSET +
                      ELB_PXB_CSR_CNT_ITR_AXI_RD256_BYTE_OFFSET,
                  (uint32_t *)&zero, 2);

    pal_reg_wr32w(ELB_ADDR_BASE_PXB_PXB_OFFSET +
                      ELB_PXB_CSR_CNT_ITR_TOT_ATOMIC_REQ_BYTE_OFFSET,
                  (uint32_t *)&zero, 2);
}

void
capmon_asic_data_store2(uint32_t axi_wr_pending, uint32_t axi_rd_pending,
                        uint64_t rd_lat0, uint64_t rd_lat1, uint64_t rd_lat2,
                        uint64_t rd_lat3, uint64_t rd_total,
                        uint16_t *cfg_rdlat)
{
    asic->axi_wr_pend = axi_wr_pending;
    asic->axi_rd_pend = axi_rd_pending;
    asic->rd_lat0 = rd_lat0;
    asic->rd_lat1 = rd_lat1;
    asic->rd_lat2 = rd_lat2;
    asic->rd_lat3 = rd_lat3;

    asic->cfg_rdlat[0] = cfg_rdlat[0];
    asic->cfg_rdlat[1] = cfg_rdlat[1];
    asic->cfg_rdlat[2] = cfg_rdlat[2];
    asic->cfg_rdlat[3] = cfg_rdlat[3];
}

void
pxb_read_initiator_status()
{
    uint16_t cfg_rdlat[4];
    uint64_t rd_lat0, rd_lat1, rd_lat2, rd_lat3;
    uint32_t axi_rd_pending, axi_wr_pending;

    // Pending Transaction status
    pal_reg_rd32w(ELB_ADDR_BASE_PXB_PXB_OFFSET +
                      ELB_PXB_CSR_STA_ITR_AXI_WR_NUM_IDS_BYTE_ADDRESS,
                  &axi_wr_pending, 1);

    pal_reg_rd32w(ELB_ADDR_BASE_PXB_PXB_OFFSET +
                      ELB_PXB_CSR_STA_ITR_AXI_RD_NUM_IDS_BYTE_ADDRESS,
                  &axi_rd_pending, 1);

    // Latency counter config
    pal_reg_rd32w(ELB_ADDR_BASE_PXB_PXB_OFFSET +
                      ELB_PXB_CSR_CFG_ITR_RDLAT_MEASURE_BYTE_ADDRESS,
                  (uint32_t *)&cfg_rdlat, 2);

    // Latency counters
    pal_reg_rd32w(ELB_ADDR_BASE_PXB_PXB_OFFSET +
                      ELB_PXB_CSR_SAT_ITR_RDLAT0_BYTE_OFFSET,
                  (uint32_t *)&rd_lat0, 2);
    pal_reg_rd32w(ELB_ADDR_BASE_PXB_PXB_OFFSET +
                      ELB_PXB_CSR_SAT_ITR_RDLAT1_BYTE_OFFSET,
                  (uint32_t *)&rd_lat1, 2);
    pal_reg_rd32w(ELB_ADDR_BASE_PXB_PXB_OFFSET +
                      ELB_PXB_CSR_SAT_ITR_RDLAT2_BYTE_OFFSET,
                  (uint32_t *)&rd_lat2, 2);
    pal_reg_rd32w(ELB_ADDR_BASE_PXB_PXB_OFFSET +
                      ELB_PXB_CSR_SAT_ITR_RDLAT3_BYTE_OFFSET,
                  (uint32_t *)&rd_lat3, 2);

    auto rd_total = rd_lat0 + rd_lat1 + rd_lat2 + rd_lat3;
    capmon_asic_data_store2(axi_wr_pending, axi_rd_pending, rd_lat0, rd_lat1,
                            rd_lat2, rd_lat3, rd_total, cfg_rdlat);
}

void
pxb_read_pport_status(uint8_t port)
{
    uint32_t stall;
    uint8_t rx_stalls = 0, tx_stalls = 0;
    int polls = 100;

    // Status counters
    for (int i = 0; i < polls; i++) {
        pal_reg_rd32w(ELB_ADDR_BASE_PP_PP_0_OFFSET +
                          ELB_PP_CSR_STA_DEBUG_STALL_BYTE_ADDRESS,
                      &stall, 1);
        rx_stalls += ELB_PP_CSR_STA_DEBUG_STALL_RX_GET(stall) & BIT(0);
        tx_stalls += ELB_PP_CSR_STA_DEBUG_STALL_TX_GET(stall) & BIT(0);
    }

    // Show
    asic->rx_stl = (rx_stalls * 100.0) / polls;
    asic->tx_stl = (tx_stalls * 100.0) / polls;
}

void
pxb_read_pport_counters(uint8_t port)
{
    uint64_t rx_req_tlp, rx_cpl_tlp, tx_req_tlp, tx_cpl_tlp;

    // TLP counters
    pal_reg_rd32w(ELB_ADDR_BASE_PP_PXC_0_OFFSET +
                      ELB_PXC_CSR_CNT_C_TL_RX_REQ_POSTED_BYTE_ADDRESS,              // HEMANT TODO.. add non-posted req
                  (uint32_t *)&rx_req_tlp, 2);

    pal_reg_rd32w(ELB_ADDR_BASE_PP_PXC_0_OFFSET +
                      ELB_PXC_CSR_CNT_C_TL_RX_CPL_BYTE_ADDRESS,
                  (uint32_t *)&rx_cpl_tlp, 2);

    pal_reg_rd32w(ELB_ADDR_BASE_PP_PXC_0_OFFSET +
                      ELB_PXC_CSR_CNT_C_TL_TX_REQ_POSTED_BYTE_ADDRESS,       // HEMANT TODO .. add non-posted req
                  (uint32_t *)&tx_req_tlp, 2);

    pal_reg_rd32w(ELB_ADDR_BASE_PP_PXC_0_OFFSET +
                      ELB_PXC_CSR_CNT_C_TL_TX_CPL_BYTE_ADDRESS,
                  (uint32_t *)&tx_cpl_tlp, 2);

    // Show
    asic->rx_req_tlp = rx_req_tlp;
    asic->rx_cpl_tlp = rx_cpl_tlp;
    asic->tx_req_tlp = tx_req_tlp;
    asic->tx_cpl_tlp = tx_cpl_tlp;
}

void
capmon_asic_data_store3(uint64_t rx_bad_tlp, uint64_t rx_bad_dllp,
                        uint64_t rx_nak_received, uint64_t rx_nullified,
                        uint64_t rxbfr_overflow, uint64_t tx_nak_sent,
                        uint64_t txbuf_ecc_err)
{
    asic->rx_bad_tlp = rx_bad_tlp;
    asic->rx_bad_dllp = rx_bad_dllp;
    asic->rx_nak_received = rx_nak_received;
    asic->rx_nullified = rx_nullified;
    asic->rxbfr_overflow = rxbfr_overflow;
    asic->tx_nak_sent = tx_nak_sent;
    asic->txbuf_ecc_err = txbuf_ecc_err;
}

void
capmon_asic_data_store4(uint64_t fcpe, uint64_t fc_timeout,
                        uint64_t replay_num_err, uint64_t replay_timer_err)
{
    asic->fcpe = fcpe;
    asic->fc_timeout = fc_timeout;
    asic->replay_num_err = replay_num_err;
    asic->replay_timer_err = replay_timer_err;
}

void
capmon_asic_data_store5(uint8_t core_initiated_recovery,
                        uint8_t ltssm_state_changed, uint64_t skp_os_err,
                        uint64_t deskew_err, uint64_t phystatus_err)
{
    asic->core_initiated_recovery = core_initiated_recovery;
    asic->ltssm_state_changed = ltssm_state_changed;
    asic->skp_os_err = skp_os_err;
    asic->deskew_err = deskew_err;
    asic->phystatus_err = phystatus_err;
}

void
capmon_asic_data_store6(uint64_t rx_malform_tlp, uint64_t rx_framing_err,
                        uint64_t rx_ecrc_err, uint64_t rx_nullified,
                        uint64_t rx_watchdog_nullify, uint64_t rx_unsupp,
                        uint64_t rxbuf_ecc_err, uint64_t tx_drop,
                        uint64_t txbfr_overflow)
{
    asic->rx_malform_tlp = rx_malform_tlp;
    asic->rx_framing_err = rx_framing_err;
    asic->rx_ecrc_err = rx_ecrc_err;
    asic->rxbuf_ecc_err = rxbuf_ecc_err;
    asic->rx_nullified = rx_nullified;
    asic->rx_watchdog_nullify = rx_watchdog_nullify;
    asic->rx_unsupp = rx_unsupp;
    asic->tx_drop = tx_drop;
    asic->txbfr_overflow = txbfr_overflow;
}

void
pxb_read_pport_err_counters(uint8_t port)
{
    uint64_t rx_bad_tlp, rx_bad_dllp, rx_nak_received, rx_nullified,
        rxbfr_overflow;
    uint64_t tx_nak_sent, txbuf_ecc_err;

    uint64_t fcpe, fc_timeout, replay_num_err, replay_timer_err;
    uint8_t core_initiated_recovery, ltssm_state_changed;
    uint64_t skp_os_err, deskew_err, phystatus_err;

    uint64_t rx_malform_tlp, rx_framing_err, rx_ecrc_err, rxbuf_ecc_err,
        rx_nullify, rx_watchdog_nullify, rx_unsupp;
    uint64_t tx_drop, txbfr_overflow;

    // Error counters
    pal_reg_rd32w(ELB_ADDR_BASE_PP_PXC_0_OFFSET +
                      ELB_PXC_CSR_PORT_P_SAT_P_PORT_CNT_RX_BAD_TLP_BYTE_ADDRESS,
                  (uint32_t *)&rx_bad_tlp, 2);

    pal_reg_rd32w(ELB_ADDR_BASE_PP_PXC_0_OFFSET +
                      ELB_PXC_CSR_PORT_P_SAT_P_PORT_CNT_RX_BAD_DLLP_BYTE_ADDRESS,
                  (uint32_t *)&rx_bad_dllp, 2);

    pal_reg_rd32w(
        ELB_ADDR_BASE_PP_PXC_0_OFFSET +
            ELB_PXC_CSR_PORT_P_SAT_P_PORT_CNT_RX_NAK_RECEIVED_BYTE_ADDRESS,
        (uint32_t *)&rx_nak_received, 2);

    pal_reg_rd32w(
        ELB_ADDR_BASE_PP_PXC_0_OFFSET +
            ELB_PXC_CSR_PORT_P_SAT_P_PORT_CNT_RX_NULLIFIED_BYTE_ADDRESS,
        (uint32_t *)&rx_nullified, 2);

    pal_reg_rd32w(
        ELB_ADDR_BASE_PP_PXC_0_OFFSET +
            ELB_PXC_CSR_PORT_P_SAT_P_PORT_CNT_RXBFR_OVERFLOW_BYTE_ADDRESS,
        (uint32_t *)&rxbfr_overflow, 2);

    //
    pal_reg_rd32w(ELB_ADDR_BASE_PP_PXC_0_OFFSET +
                      ELB_PXC_CSR_PORT_P_SAT_P_PORT_CNT_TX_NAK_SENT_BYTE_ADDRESS,
                  (uint32_t *)&tx_nak_sent, 2);

    pal_reg_rd32w(
        ELB_ADDR_BASE_PP_PXC_0_OFFSET +
            ELB_PXC_CSR_PORT_P_SAT_P_PORT_CNT_TXBUF_ECC_ERR_BYTE_ADDRESS,
        (uint32_t *)&txbuf_ecc_err, 2);

    //
    pal_reg_rd32w(ELB_ADDR_BASE_PP_PXC_0_OFFSET +
                      ELB_PXC_CSR_PORT_P_SAT_P_PORT_CNT_FCPE_BYTE_ADDRESS,
                  (uint32_t *)&fcpe, 2);

    pal_reg_rd32w(ELB_ADDR_BASE_PP_PXC_0_OFFSET +
                      ELB_PXC_CSR_PORT_P_SAT_P_PORT_CNT_FC_TIMEOUT_BYTE_ADDRESS,
                  (uint32_t *)&fc_timeout, 2);

    pal_reg_rd32w(
        ELB_ADDR_BASE_PP_PXC_0_OFFSET +
            ELB_PXC_CSR_PORT_P_SAT_P_PORT_CNT_REPLAY_NUM_ERR_BYTE_ADDRESS,
        (uint32_t *)&replay_num_err, 2);

    pal_reg_rd32w(
        ELB_ADDR_BASE_PP_PXC_0_OFFSET +
            ELB_PXC_CSR_PORT_P_SAT_P_PORT_CNT_REPLAY_TIMER_ERR_BYTE_ADDRESS,
        (uint32_t *)&replay_timer_err, 2);

    core_initiated_recovery = pal_reg_rd8(
        ELB_ADDR_BASE_PP_PXC_0_OFFSET +
            ELB_PXC_CSR_PORT_P_SAT_P_PORT_CNT_CORE_INITIATED_RECOVERY_BYTE_ADDRESS);

    ltssm_state_changed = pal_reg_rd8(
        ELB_ADDR_BASE_PP_PXC_0_OFFSET +
            ELB_PXC_CSR_PORT_P_SAT_P_PORT_CNT_LTSSM_STATE_CHANGED_BYTE_ADDRESS);

    pal_reg_rd32w(
        ELB_ADDR_BASE_PP_PXC_0_OFFSET +
            ELB_PXC_CSR_PORT_P_SAT_P_PORT_CNT_8B10B_128B130B_SKP_OS_ERR_BYTE_ADDRESS,
        (uint32_t *)&skp_os_err, 2);

    pal_reg_rd32w(ELB_ADDR_BASE_PP_PXC_0_OFFSET +
                      ELB_PXC_CSR_PORT_P_SAT_P_PORT_CNT_DESKEW_ERR_BYTE_ADDRESS,
                  (uint32_t *)&deskew_err, 2);

    pal_reg_rd32w(
        ELB_ADDR_BASE_PP_PXC_0_OFFSET +
            ELB_PXC_CSR_PORT_P_SAT_P_PORT_CNT_PHYSTATUS_ERR_BYTE_ADDRESS,
        (uint32_t *)&phystatus_err, 2);

    pal_reg_rd32w(
        ELB_ADDR_BASE_PP_PXC_0_OFFSET +
            ELB_PXC_CSR_SAT_C_PORT_CNT_RX_TLP_ERRS_BYTE_ADDRESS,          // TODO HEMANT .. expand sub fields 
        (uint32_t *)&rx_malform_tlp, 2);

    pal_reg_rd32w(
        ELB_ADDR_BASE_PP_PXC_0_OFFSET +
            ELB_PXC_CSR_SAT_C_PORT_CNT_RX_ERRS_BYTE_ADDRESS,             // TODO HEMANT .. expand sub fields 
        (uint32_t *)&rx_framing_err, 2);

    pal_reg_rd32w(ELB_ADDR_BASE_PP_PXC_0_OFFSET +
                      ELB_PXC_CSR_SAT_C_PORT_CNT_RX_ERRS_BYTE_ADDRESS,         // TODO HEMANT .. expand sub fields 
                  (uint32_t *)&rx_ecrc_err, 2);

    pal_reg_rd32w(ELB_ADDR_BASE_PP_PXC_0_OFFSET +
                      ELB_PXC_CSR_SAT_C_PORT_CNT_RX_EVENTS_BYTE_ADDRESS,       // TODO HEMANT .. expand sub fields 
                  (uint32_t *)&rx_nullify, 2);

    pal_reg_rd32w(
        ELB_ADDR_BASE_PP_PXC_0_OFFSET +
            ELB_PXC_CSR_SAT_C_PORT_CNT_RX_WATCHDOG_NULLIFY_BYTE_ADDRESS,
        (uint32_t *)&rx_watchdog_nullify, 2);

    pal_reg_rd32w(ELB_ADDR_BASE_PP_PXC_0_OFFSET +
                      ELB_PXC_CSR_SAT_C_PORT_CNT_RX_EVENTS_BYTE_ADDRESS,  // TODO HEMANT .. expand sub fields 
                  (uint32_t *)&rx_unsupp, 2);

    pal_reg_rd32w(
        ELB_ADDR_BASE_PP_PXC_0_OFFSET +
            ELB_PXC_CSR_SAT_C_PORT_CNT_RX_ERRS_BYTE_ADDRESS,               // TODO HEMANT ... expand sub fields 
        (uint32_t *)&rxbuf_ecc_err, 2);

    pal_reg_rd32w(ELB_ADDR_BASE_PP_PXC_0_OFFSET +
                      ELB_PXC_CSR_SAT_C_PORT_CNT_TX_DROP_BYTE_ADDRESS,
                  (uint32_t *)&tx_drop, 2);

    pal_reg_rd32w(
        ELB_ADDR_BASE_PP_PXC_0_OFFSET +
            ELB_PXC_CSR_SAT_C_PORT_CNT_TXBFR_OVERFLOW_BYTE_ADDRESS,
        (uint32_t *)&txbfr_overflow, 2);

    // Show
    capmon_asic_data_store3(rx_bad_tlp, rx_bad_dllp, rx_nak_received,
                            rx_nullified, rxbfr_overflow, tx_nak_sent,
                            txbuf_ecc_err);
    capmon_asic_data_store4(fcpe, fc_timeout, replay_num_err, replay_timer_err);
    capmon_asic_data_store5(core_initiated_recovery, ltssm_state_changed,
                            skp_os_err, deskew_err, phystatus_err);
    capmon_asic_data_store6(rx_malform_tlp, rx_framing_err, rx_ecrc_err,
                            rx_nullify, rx_watchdog_nullify, rx_unsupp,
                            rxbuf_ecc_err, tx_drop, txbfr_overflow);
}

void
pxb_read_counters(int verbose)
{
    pxb_read_target_status();
    if (verbose) {
        pxb_read_target_counters();
        pxb_read_target_err_counters();
    }

    pxb_read_initiator_status();
    if (verbose) {
        pxb_read_initiator_counters();
    }

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
