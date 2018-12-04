// {C} Copyright 2018 Pensando Systems Inc. All rights reserved

#include "nic/include/hal.hpp"
#include "nic/include/hal_cfg.hpp"
#include "nic/include/pd_api.hpp"
#include "nic/hal/pd/capri/capri_loader.h"
#include "nic/hal/pd/capri/capri_hbm.hpp"
#include "nic/hal/pd/capri/capri_barco_crypto.hpp"
#include "nic/hal/pd/capri/capri_pxb_pcie.hpp"
#include "nic/include/capri_common.h"

namespace hal {
namespace pd {

#define P4PLUS_SYMBOLS_MAX  135

uint32_t
common_p4plus_symbols_init (void **p4plus_symbols, platform_type_t platform_type)
{
    uint32_t    i = 0;
    uint64_t    offset;
    uint32_t capri_coreclk_freq; //Mhz

    capri_coreclk_freq = (uint32_t)(capri_get_coreclk_freq(platform_type) / 1000000);

    HAL_TRACE_DEBUG("Capri core clock freq is {} Mhz", capri_coreclk_freq);


    *p4plus_symbols = (capri_prog_param_info_t *)
        HAL_CALLOC(hal::HAL_MEM_ALLOC_PD,
                   P4PLUS_SYMBOLS_MAX * sizeof(capri_prog_param_info_t));
    capri_prog_param_info_t *symbols =
        (capri_prog_param_info_t *)(*p4plus_symbols);

    symbols[i].name = "tcp-read-rnmdr-alloc-idx.bin";
    symbols[i].num_params = 2;
    symbols[i].params[0].name = RNMDPR_BIG_TABLE_BASE;
    symbols[i].params[0].val = get_start_offset(CAPRI_HBM_REG_NMDPR_BIG_RX);
    symbols[i].params[1].name = TCP_PROXY_STATS;
    symbols[i].params[1].val = get_start_offset(CAPRI_HBM_REG_TCP_PROXY_STATS);
    i++;

    symbols[i].name = "tcp-read-rnmpr-alloc-idx.bin";
    symbols[i].num_params = 1;
    symbols[i].params[0].name = RNMPR_TABLE_BASE;
    symbols[i].params[0].val = get_start_offset(CAPRI_HBM_REG_NMPR_BIG_RX);
    i++;

    symbols[i].name = "tcp-read-sesq-ci.bin";
    symbols[i].num_params = 1;
    symbols[i].params[0].name = TCP_PROXY_STATS;
    symbols[i].params[0].val = get_start_offset(CAPRI_HBM_REG_TCP_PROXY_STATS);
    i++;

    symbols[i].name = "tcp-read-sesq-retx-ci.bin";
    symbols[i].num_params = 1;
    symbols[i].params[0].name = TCP_PROXY_STATS;
    symbols[i].params[0].val = get_start_offset(CAPRI_HBM_REG_TCP_PROXY_STATS);
    i++;

    symbols[i].name = "tcp-clean-retx.bin";
    symbols[i].num_params = 1;
    symbols[i].params[0].name = TCP_PROXY_STATS;
    symbols[i].params[0].val = get_start_offset(CAPRI_HBM_REG_TCP_PROXY_STATS);
    i++;

    symbols[i].name = "tls-enc-read-tnmdr-alloc-idx.bin";
    symbols[i].num_params = 2;
    symbols[i].params[0].name = TNMDPR_BIG_TABLE_BASE;
    symbols[i].params[0].val = get_start_offset(CAPRI_HBM_REG_NMDPR_BIG_TX);
    symbols[i].params[1].name = TLS_PROXY_GLOBAL_STATS;
    symbols[i].params[1].val = get_start_offset(CAPRI_HBM_REG_TLS_PROXY_GLOBAL_STATS);
    i++;

    symbols[i].name = "tls-enc-read-tnmpr-alloc-idx.bin";
    symbols[i].num_params = 1;
    symbols[i].params[0].name = TNMPR_TABLE_BASE;
    symbols[i].params[0].val = get_start_offset(CAPRI_HBM_REG_NMPR_BIG_TX);
    i++;

    symbols[i].name = "tls-enc-queue-brq.bin";
    symbols[i].num_params = 1;
    symbols[i].params[0].name = BRQ_BASE;
    symbols[i].params[0].val = get_start_offset(CAPRI_HBM_REG_BRQ);
    i++;

    symbols[i].name = "tls-dec-queue-brq.bin";
    symbols[i].num_params = 1;
    symbols[i].params[0].name = BRQ_BASE;
    symbols[i].params[0].val = get_start_offset(CAPRI_HBM_REG_BARCO_RING_GCM1);
    i++;

    symbols[i].name = "tls-enc-read-rnmdr-free-idx.bin";
    symbols[i].num_params = 1;
    symbols[i].params[0].name = RNMDR_TABLE_BASE;
    symbols[i].params[0].val = get_start_offset(CAPRI_HBM_REG_NMDR_TX);
    i++;

    symbols[i].name = "tls-enc-read-rnmpr-free-idx.bin";
    symbols[i].num_params = 1;
    symbols[i].params[0].name = RNMPR_TABLE_BASE;
    symbols[i].params[0].val = get_start_offset(CAPRI_HBM_REG_NMPR_BIG_TX);
    i++;

    symbols[i].name = "esp_ipv4_tunnel_h2n_ipsec_encap_rxdma_initial_table.bin";
    symbols[i].num_params = 2;
    symbols[i].params[0].name = IPSEC_PAD_BYTES_HBM_TABLE_BASE;
    symbols[i].params[0].val = get_start_offset(CAPRI_HBM_REG_IPSEC_PAD_TABLE);
    symbols[i].params[1].name = IPSEC_GLOBAL_BAD_DMA_COUNTER_BASE_H2N;
    symbols[i].params[1].val = get_start_offset(CAPRI_HBM_REG_IPSEC_GLOBAL_DROP_STATS);
    i++;

    symbols[i].name = "esp_ipv4_tunnel_h2n_rxmda_ring_full_error.bin";
    symbols[i].num_params = 1;
    symbols[i].params[0].name = IPSEC_GLOBAL_BAD_DMA_COUNTER_BASE_H2N;
    symbols[i].params[0].val = get_start_offset(CAPRI_HBM_REG_IPSEC_GLOBAL_DROP_STATS);
    i++;

    symbols[i].name = "esp_ipv4_tunnel_h2n_ipsec_encap_rxdma_initial_table2.bin";
    symbols[i].num_params = 1;
    symbols[i].params[0].name = IPSEC_GLOBAL_BAD_DMA_COUNTER_BASE_H2N;
    symbols[i].params[0].val = get_start_offset(CAPRI_HBM_REG_IPSEC_GLOBAL_DROP_STATS);
    i++;

    symbols[i].name = "esp_ipv4_tunnel_h2n_txdma1_ipsec_encap_txdma_load_head_desc_int_header2.bin";
    symbols[i].num_params = 1;
    symbols[i].params[0].name = IPSEC_GLOBAL_BAD_DMA_COUNTER_BASE_H2N;
    symbols[i].params[0].val = get_start_offset(CAPRI_HBM_REG_IPSEC_GLOBAL_DROP_STATS);
    i++;

    symbols[i].name = "esp_ipv4_tunnel_h2n_txdma1_ipsec_write_barco_req.bin";
    symbols[i].num_params = 1;
    symbols[i].params[0].name = IPSEC_GLOBAL_BAD_DMA_COUNTER_BASE_H2N;
    symbols[i].params[0].val = get_start_offset(CAPRI_HBM_REG_IPSEC_GLOBAL_DROP_STATS);
    i++;

    symbols[i].name = "esp_ipv4_tunnel_h2n_txdma1_ring_full_error.bin";
    symbols[i].num_params = 1;
    symbols[i].params[0].name = IPSEC_GLOBAL_BAD_DMA_COUNTER_BASE_H2N;
    symbols[i].params[0].val = get_start_offset(CAPRI_HBM_REG_IPSEC_GLOBAL_DROP_STATS);
    i++;

    symbols[i].name = "esp_ipv4_tunnel_h2n_update_input_desc_aol2.bin";
    symbols[i].num_params = 1;
    symbols[i].params[0].name = IPSEC_GLOBAL_BAD_DMA_COUNTER_BASE_H2N;
    symbols[i].params[0].val = get_start_offset(CAPRI_HBM_REG_IPSEC_GLOBAL_DROP_STATS);
    i++;

    symbols[i].name = "esp_ipv4_tunnel_h2n_txdma2_ipsec_encap_txdma2_load_in_desc.bin";
    symbols[i].num_params = 1;
    symbols[i].params[0].name = IPSEC_GLOBAL_BAD_DMA_COUNTER_BASE_H2N;
    symbols[i].params[0].val = get_start_offset(CAPRI_HBM_REG_IPSEC_GLOBAL_DROP_STATS);
    i++;

    symbols[i].name = "esp_ipv4_tunnel_h2n_txdma2_ipsec_build_encap_packet.bin";
    symbols[i].num_params = 2;
    symbols[i].params[0].name = IPSEC_CB_BASE;
    symbols[i].params[0].val = get_start_offset(CAPRI_HBM_REG_IPSECCB);
    symbols[i].params[1].name = IPSEC_GLOBAL_BAD_DMA_COUNTER_BASE_H2N;
    symbols[i].params[1].val = get_start_offset(CAPRI_HBM_REG_IPSEC_GLOBAL_DROP_STATS);
    i++;

    symbols[i].name = "esp_ipv4_tunnel_h2n_update_input_desc_aol.bin";
    symbols[i].num_params = 2;
    symbols[i].params[0].name = IPSEC_CB_BASE;
    symbols[i].params[0].val = get_start_offset(CAPRI_HBM_REG_IPSECCB);
    symbols[i].params[1].name = IPSEC_GLOBAL_BAD_DMA_COUNTER_BASE_H2N;
    symbols[i].params[1].val = get_start_offset(CAPRI_HBM_REG_IPSEC_GLOBAL_DROP_STATS);
    i++;

    symbols[i].name = "esp_ipv4_tunnel_h2n_ipsec_cb_tail_enqueue_input_desc2.bin";
    symbols[i].num_params = 1;
    symbols[i].params[0].name = IPSEC_GLOBAL_BAD_DMA_COUNTER_BASE_H2N;
    symbols[i].params[0].val = get_start_offset(CAPRI_HBM_REG_IPSEC_GLOBAL_DROP_STATS);
    i++;

    symbols[i].name = "esp_ipv4_tunnel_n2h_txdma2_load_in_desc.bin";
    symbols[i].num_params = 2;
    symbols[i].params[0].name = IPSEC_CB_BASE;
    symbols[i].params[0].val = get_start_offset(CAPRI_HBM_REG_IPSECCB);
    symbols[i].params[1].name = IPSEC_GLOBAL_BAD_DMA_COUNTER_BASE_N2H;
    symbols[i].params[1].val = get_start_offset(CAPRI_HBM_REG_IPSEC_GLOBAL_DROP_STATS) + 512;
    i++;

    symbols[i].name = "esp_ipv4_tunnel_n2h_ipsec_cb_tail_enqueue_input_desc.bin";
    symbols[i].num_params = 1;
    symbols[i].params[0].name = IPSEC_GLOBAL_BAD_DMA_COUNTER_BASE_N2H;
    symbols[i].params[0].val = get_start_offset(CAPRI_HBM_REG_IPSEC_GLOBAL_DROP_STATS) + 512;
    i++;

    symbols[i].name = "esp_ipv4_tunnel_n2h_rxmda_ring_full_error.bin";
    symbols[i].num_params = 1;
    symbols[i].params[0].name = IPSEC_GLOBAL_BAD_DMA_COUNTER_BASE_N2H;
    symbols[i].params[0].val = get_start_offset(CAPRI_HBM_REG_IPSEC_GLOBAL_DROP_STATS) + 512;
    i++;

    symbols[i].name = "esp_ipv4_tunnel_n2h_write_barco_req.bin";
    symbols[i].num_params = 1;
    symbols[i].params[0].name = IPSEC_GLOBAL_BAD_DMA_COUNTER_BASE_N2H;
    symbols[i].params[0].val = get_start_offset(CAPRI_HBM_REG_IPSEC_GLOBAL_DROP_STATS) + 512;
    i++;

    symbols[i].name = "esp_ipv4_tunnel_n2h_build_decap_packet.bin";
    symbols[i].num_params = 1;
    symbols[i].params[0].name = IPSEC_GLOBAL_BAD_DMA_COUNTER_BASE_N2H;
    symbols[i].params[0].val = get_start_offset(CAPRI_HBM_REG_IPSEC_GLOBAL_DROP_STATS) + 512;
    i++;

    symbols[i].name = "esp_ipv4_tunnel_n2h_txdma2_load_ipsec_int.bin";
    symbols[i].num_params = 1;
    symbols[i].params[0].name = IPSEC_GLOBAL_BAD_DMA_COUNTER_BASE_N2H;
    symbols[i].params[0].val = get_start_offset(CAPRI_HBM_REG_IPSEC_GLOBAL_DROP_STATS) + 512;
    i++;

    symbols[i].name = "esp_ipv4_tunnel_n2h_txdma_initial_table.bin";
    symbols[i].num_params = 3;
    symbols[i].params[0].name = IPSEC_CB_BASE;
    symbols[i].params[0].val = get_start_offset(CAPRI_HBM_REG_IPSECCB);
    symbols[i].params[1].name = TLS_PROXY_BARCO_GCM1_PI_HBM_TABLE_BASE;
    symbols[i].params[1].val = get_start_offset(CAPRI_HBM_REG_TLS_PROXY_PAD_TABLE) +
                                BARCO_GCM1_PI_HBM_TABLE_OFFSET;
    symbols[i].params[2].name = IPSEC_GLOBAL_BAD_DMA_COUNTER_BASE_N2H;
    symbols[i].params[2].val = get_start_offset(CAPRI_HBM_REG_IPSEC_GLOBAL_DROP_STATS) + 512;
    i++;

    symbols[i].name = "esp_ipv4_tunnel_n2h_update_input_desc_aol.bin";
    symbols[i].num_params = 2;
    symbols[i].params[0].name = IPSEC_CB_BASE;
    symbols[i].params[0].val = get_start_offset(CAPRI_HBM_REG_IPSECCB);
    symbols[i].params[1].name = IPSEC_GLOBAL_BAD_DMA_COUNTER_BASE_N2H;
    symbols[i].params[1].val = get_start_offset(CAPRI_HBM_REG_IPSEC_GLOBAL_DROP_STATS) + 512;
    i++;

    symbols[i].name = "esp_ipv4_tunnel_h2n_allocate_input_desc_semaphore.bin";
    symbols[i].num_params = 5;
    symbols[i].params[0].name = IPSEC_RNMDR_TABLE_BASE;
    symbols[i].params[0].val = get_start_offset(CAPRI_HBM_REG_IPSEC_NMDR_RX);
    symbols[i].params[1].name = IPSEC_RNMPR_TABLE_BASE;
    symbols[i].params[1].val = get_start_offset(CAPRI_HBM_REG_IPSEC_NMPR_RX);
    symbols[i].params[2].name = IPSEC_TNMDR_TABLE_BASE;
    symbols[i].params[2].val = get_start_offset(CAPRI_HBM_REG_IPSEC_NMDR_TX);
    symbols[i].params[3].name = IPSEC_TNMPR_TABLE_BASE;
    symbols[i].params[3].val = get_start_offset(CAPRI_HBM_REG_IPSEC_NMPR_TX);
    symbols[i].params[4].name = IPSEC_GLOBAL_BAD_DMA_COUNTER_BASE_H2N;
    symbols[i].params[4].val = get_start_offset(CAPRI_HBM_REG_IPSEC_GLOBAL_DROP_STATS);
    i++;

    symbols[i].name = "esp_ipv4_tunnel_n2h_allocate_input_desc_semaphore.bin";
    symbols[i].num_params = 5;
    symbols[i].params[0].name = IPSEC_RNMDR_TABLE_BASE;
    symbols[i].params[0].val = get_start_offset(CAPRI_HBM_REG_NMDR_RX);
    symbols[i].params[1].name = IPSEC_TNMDR_TABLE_BASE;
    symbols[i].params[1].val = get_start_offset(CAPRI_HBM_REG_NMDR_TX);
    symbols[i].params[2].name = IPSEC_RNMPR_TABLE_BASE;
    symbols[i].params[2].val = get_start_offset(CAPRI_HBM_REG_NMPR_SMALL_RX);
    symbols[i].params[3].name = IPSEC_TNMPR_TABLE_BASE;
    symbols[i].params[3].val = get_start_offset(CAPRI_HBM_REG_NMPR_SMALL_TX);
    symbols[i].params[4].name = IPSEC_GLOBAL_BAD_DMA_COUNTER_BASE_N2H;
    symbols[i].params[4].val = get_start_offset(CAPRI_HBM_REG_IPSEC_GLOBAL_DROP_STATS) + 512;
    i++;

    symbols[i].name = "esp_ipv4_tunnel_n2h_txdma2_initial_table.bin";
    symbols[i].num_params = 3;
    symbols[i].params[0].name = BRQ_BASE;
    symbols[i].params[0].val = get_start_offset(CAPRI_HBM_REG_BARCO_RING_GCM1);
    symbols[i].params[1].name = TLS_PROXY_BARCO_GCM1_PI_HBM_TABLE_BASE;
    symbols[i].params[1].val = get_start_offset(CAPRI_HBM_REG_TLS_PROXY_PAD_TABLE) +
                                 BARCO_GCM1_PI_HBM_TABLE_OFFSET;
    symbols[i].params[2].name = IPSEC_GLOBAL_BAD_DMA_COUNTER_BASE_N2H;
    symbols[i].params[2].val = get_start_offset(CAPRI_HBM_REG_IPSEC_GLOBAL_DROP_STATS) + 512;
    i++;

    symbols[i].name = "esp_ipv4_tunnel_n2h_allocate_barco_req_pindex.bin";
    symbols[i].num_params = 1;
    symbols[i].params[0].name = BRQ_BASE;
    symbols[i].params[0].val = get_start_offset(CAPRI_HBM_REG_BARCO_RING_GCM1);
    symbols[i].params[1].name = IPSEC_GLOBAL_BAD_DMA_COUNTER_BASE_N2H;
    symbols[i].params[1].val = get_start_offset(CAPRI_HBM_REG_IPSEC_GLOBAL_DROP_STATS) + 512;
    i++;

    symbols[i].name = "esp_ipv4_tunnel_h2n_txdma2_ipsec_encap_txdma2_initial_table.bin";
    symbols[i].num_params = 3;
    symbols[i].params[0].name = BRQ_BASE;
    symbols[i].params[0].val = get_start_offset(CAPRI_HBM_REG_BRQ);
    symbols[i].params[1].name = TLS_PROXY_BARCO_GCM0_PI_HBM_TABLE_BASE;
    symbols[i].params[1].val = get_start_offset(CAPRI_HBM_REG_TLS_PROXY_PAD_TABLE) +
                                 BARCO_GCM0_PI_HBM_TABLE_OFFSET;
    symbols[i].params[2].name = IPSEC_GLOBAL_BAD_DMA_COUNTER_BASE_H2N;
    symbols[i].params[2].val = get_start_offset(CAPRI_HBM_REG_IPSEC_GLOBAL_DROP_STATS);

    i++;

    symbols[i].name = "esp_ipv4_tunnel_h2n_txdma1_allocate_barco_req_pindex.bin";
    symbols[i].num_params = 2;
    symbols[i].params[0].name = BRQ_BASE;
    symbols[i].params[0].val = get_start_offset(CAPRI_HBM_REG_BRQ);
    symbols[i].params[1].name = IPSEC_GLOBAL_BAD_DMA_COUNTER_BASE_H2N;
    symbols[i].params[1].val = get_start_offset(CAPRI_HBM_REG_IPSEC_GLOBAL_DROP_STATS);
    i++;


    symbols[i].name = "cpu_read_desc_pindex.bin";
    symbols[i].num_params = 1;
    symbols[i].params[0].name = RNMDPR_BIG_TABLE_BASE;
    symbols[i].params[0].val = get_start_offset(CAPRI_HBM_REG_NMDPR_BIG_RX);
    i++;

    symbols[i].name = "cpu_read_page_pindex.bin";
    symbols[i].num_params = 1;
    symbols[i].params[0].name = RNMPR_TABLE_BASE;
    symbols[i].params[0].val = get_start_offset(CAPRI_HBM_REG_NMPR_BIG_RX);
    i++;

    symbols[i].name = "cpu_tx_read_asq_descr.bin";
    symbols[i].num_params = 1;
    symbols[i].params[0].name = CPU_TX_DOT1Q_HDR_OFFSET;
    symbols[i].params[0].val = sizeof(hal::pd::cpu_to_p4plus_header_t) + \
        sizeof(hal::pd::p4plus_to_p4_header_t) + L2HDR_DOT1Q_OFFSET;
    i++;

    symbols[i].name = "tls-dec-read-tnmdr-alloc-idx.bin";
    symbols[i].num_params = 2;
    symbols[i].params[0].name = TNMDPR_BIG_TABLE_BASE;
    symbols[i].params[0].val = get_start_offset(CAPRI_HBM_REG_NMDPR_BIG_TX);
    symbols[i].params[1].name = TLS_PROXY_GLOBAL_STATS;
    symbols[i].params[1].val = get_start_offset(CAPRI_HBM_REG_TLS_PROXY_GLOBAL_STATS);
    i++;

    symbols[i].name = "tls-dec-read-tnmpr-alloc-idx.bin";
    symbols[i].num_params = 1;
    symbols[i].params[0].name = TNMPR_TABLE_BASE;
    symbols[i].params[0].val = get_start_offset(CAPRI_HBM_REG_NMPR_BIG_TX);
    i++;

    symbols[i].name = "esp_ipv4_tunnel_h2n_ipsec_cb_tail_enqueue_input_desc.bin";
    symbols[i].num_params = 2;
    symbols[i].params[0].name = IPSEC_CB_BASE;
    symbols[i].params[0].val = get_start_offset(CAPRI_HBM_REG_IPSECCB);
    symbols[i].params[1].name = IPSEC_GLOBAL_BAD_DMA_COUNTER_BASE_H2N;
    symbols[i].params[1].val = get_start_offset(CAPRI_HBM_REG_IPSEC_GLOBAL_DROP_STATS);
    i++;

    symbols[i].name = "tls-dec-read-rnmdr-free-idx.bin";
    symbols[i].num_params = 1;
    symbols[i].params[0].name = RNMDR_TABLE_BASE;
    symbols[i].params[0].val = get_start_offset(CAPRI_HBM_REG_NMDR_TX);
    i++;

    symbols[i].name = "tls-dec-read-rnmpr-free-idx.bin";
    symbols[i].num_params = 1;
    symbols[i].params[0].name = RNMPR_TABLE_BASE;
    symbols[i].params[0].val = get_start_offset(CAPRI_HBM_REG_NMPR_BIG_TX);
    i++;

    symbols[i].name = "tls-dec-bld-barco-req.bin";
    symbols[i].num_params = 1;
    symbols[i].params[0].name = ARQRX_BASE;
    symbols[i].params[0].val = get_start_offset(CAPRI_HBM_REG_ARQRX);
    i++;

    if ((offset = get_start_offset("flow_hash")) != INVALID_MEM_ADDRESS) {
        symbols[i].name = "ipfix.bin";
        symbols[i].params[0].name = P4_FLOW_HASH_BASE;
        symbols[i].params[0].val = offset;
        if ((offset = get_start_offset("flow_hash_overflow")) != INVALID_MEM_ADDRESS) {
            symbols[i].num_params = 2;
            symbols[i].params[1].name = P4_FLOW_HASH_OVERFLOW_BASE;
            symbols[i].params[1].val = offset;
        } else {
            symbols[i].num_params = 1;
        }
        i++;
    }

    if ((offset = get_start_offset("flow_info")) != INVALID_MEM_ADDRESS) {
        symbols[i].name = "ipfix_flow_hash.bin";
        symbols[i].num_params = 1;
        symbols[i].params[0].name = P4_FLOW_INFO_BASE;
        symbols[i].params[0].val = offset;
        i++;
    }

    if ((offset = get_start_offset("session_state")) != INVALID_MEM_ADDRESS) {
        symbols[i].name = "ipfix_flow_info.bin";
        symbols[i].num_params = 1;
        symbols[i].params[0].name = P4_SESSION_STATE_BASE;
        symbols[i].params[0].val = offset;
        i++;
    }

    if ((offset = get_start_offset("flow_stats")) != INVALID_MEM_ADDRESS) {
        symbols[i].name = "ipfix_session_state.bin";
        symbols[i].num_params = 2;
        symbols[i].params[0].name = P4_FLOW_STATS_BASE;
        symbols[i].params[0].val = offset;
        symbols[i].params[1].name = P4_FLOW_ATOMIC_STATS_BASE;
        symbols[i].params[1].val = get_start_offset(CAPRI_HBM_REG_P4_ATOMIC_STATS);
        i++;
    }

    symbols[i].name = "tcp-tx-read-gc-nmdr-idx.bin";
    symbols[i].num_params = 3;
    symbols[i].params[0].name = RNMDR_GC_TABLE_BASE;
    symbols[i].params[0].val = get_start_offset(CAPRI_HBM_REG_NMDR_RX_GC);
    symbols[i].params[1].name = TNMDR_GC_TABLE_BASE;
    symbols[i].params[1].val = get_start_offset(CAPRI_HBM_REG_NMDR_TX_GC);
    symbols[i].params[2].name = TCP_PROXY_STATS;
    symbols[i].params[2].val = get_start_offset(CAPRI_HBM_REG_TCP_PROXY_STATS);
    i++;

    i++;

    symbols[i].name = "gc_tx_inc_descr_free_pair_pi.bin";
    symbols[i].num_params = 2;
    symbols[i].params[0].name = RNMDPR_BIG_TABLE_BASE;
    symbols[i].params[0].val = get_start_offset(CAPRI_HBM_REG_NMDR_RX);
    symbols[i].params[1].name = TNMDPR_BIG_TABLE_BASE;
    symbols[i].params[1].val = get_start_offset(CAPRI_HBM_REG_NMDR_TX);
    i++;

    /*
     * The 'CAPRI_HBM_REG_TLS_PROXY_PAD_TABLE' region is provisioned for 1KB out of
     * which CAPRI_MAX_TLS_PAD_SIZE is used for Pad bytes. We'll use the remaining
     * HBM memory from this region to store other TCP/TLS global resources.
     */
    symbols[i].name = "gc_tx_dummy.bin";
    symbols[i].num_params = 1;
    symbols[i].params[0].name = GC_GLOBAL_TABLE_BASE;
    symbols[i].params[0].val = get_start_offset(CAPRI_HBM_REG_TLS_PROXY_PAD_TABLE) +
                                    CAPRI_GC_GLOBAL_TABLE;
    i++;

    symbols[i].name = "tcp-l7-read-rnmdr-alloc-idx.bin";
    symbols[i].num_params = 2;
    symbols[i].params[0].name = RNMDPR_BIG_TABLE_BASE;
    symbols[i].params[0].val = get_start_offset(CAPRI_HBM_REG_NMDPR_BIG_RX);
    i++;

    symbols[i].name = "tls-dec-read-l7-rnmdr-pidx.bin";
    symbols[i].num_params = 1;
    symbols[i].params[0].name = RNMDPR_BIG_TABLE_BASE;
    symbols[i].params[0].val = get_start_offset(CAPRI_HBM_REG_NMDPR_BIG_RX);
    i++;

    symbols[i].name = "tls-enc-queue-brq-mpp.bin";
    symbols[i].num_params = 1;
    symbols[i].params[0].name = BRQ_BASE;
    symbols[i].params[0].val = get_start_offset(CAPRI_HBM_REG_BARCO_RING_MPP1);
    i++;

    symbols[i].name = "tls-dec-queue-brq-mpp.bin";
    symbols[i].num_params = 1;
    symbols[i].params[0].name = BRQ_BASE;
    symbols[i].params[0].val = get_start_offset(CAPRI_HBM_REG_BARCO_RING_MPP1);
    i++;

    symbols[i].name = "resp_rx_eqcb_process.bin";
    symbols[i].num_params = 1;
    symbols[i].params[0].name = RDMA_EQ_INTR_TABLE_BASE;
    symbols[i].params[0].val = get_start_offset(CAPRI_HBM_REG_RDMA_EQ_INTR_TABLE);
    i++;

    symbols[i].name = "req_rx_eqcb_process.bin";
    symbols[i].num_params = 1;
    symbols[i].params[0].name = RDMA_EQ_INTR_TABLE_BASE;
    symbols[i].params[0].val = get_start_offset(CAPRI_HBM_REG_RDMA_EQ_INTR_TABLE);
    i++;

    symbols[i].name = "rdma_cq_rx_eqcb_process.bin";
    symbols[i].num_params = 1;
    symbols[i].params[0].name = RDMA_EQ_INTR_TABLE_BASE;
    symbols[i].params[0].val = get_start_offset(CAPRI_HBM_REG_RDMA_EQ_INTR_TABLE);
    i++;

    symbols[i].name = "resp_rx_rqcb_process.bin";
    symbols[i].num_params = 1;
    symbols[i].params[0].name = RDMA_ATOMIC_RESOURCE_ADDR;
    symbols[i].params[0].val = get_start_offset(CAPRI_HBM_REG_RDMA_ATOMIC_RESOURCE_ADDR);
    i++;

    symbols[i].name = "resp_rx_atomic_resource_process.bin";
    symbols[i].num_params = 2;
    symbols[i].params[0].name = RDMA_ATOMIC_RESOURCE_ADDR;
    symbols[i].params[0].val = get_start_offset(CAPRI_HBM_REG_RDMA_ATOMIC_RESOURCE_ADDR);
    symbols[i].params[1].name = RDMA_PCIE_ATOMIC_BASE_ADDR;
    symbols[i].params[1].val = CAPRI_PCIE_ATOMIC_BASE_ADDR;
    i++;

    symbols[i].name = "p4pt.bin";
    symbols[i].num_params = 1;
    symbols[i].params[0].name = P4PT_TCB_BASE;
    symbols[i].params[0].val = get_start_offset(CAPRI_HBM_REG_P4PT_TCB_MEM);
    i++;

    symbols[i].name = "p4pt_update_tcb.bin";
    symbols[i].num_params = 1;
    symbols[i].params[0].name = P4PT_TCB_ISCSI_REC_BASE;
    symbols[i].params[0].val = get_start_offset(CAPRI_HBM_REG_P4PT_TCB_ISCSI_REC_MEM);
    i++;

    symbols[i].name = "p4pt_update_tcb_rec.bin";
    symbols[i].num_params = 1;
    symbols[i].params[0].name = P4PT_TCB_ISCSI_STATS_BASE;
    symbols[i].params[0].val = get_start_offset(CAPRI_HBM_REG_P4PT_TCB_ISCSI_STATS_MEM);
    i++;

    symbols[i].name = "p4pt_update_tcb_stats.bin";
    symbols[i].num_params = 1;
    symbols[i].params[0].name = P4PT_TCB_ISCSI_READ_LATENCY_DISTRIBUTION_BASE;
    symbols[i].params[0].val = get_start_offset(CAPRI_HBM_REG_P4PT_TCB_ISCSI_READ_LATENCY_DISTRIBUTION_MEM);
    i++;

    symbols[i].name = "p4pt_update_read_latency_distribution.bin";
    symbols[i].num_params = 1;
    symbols[i].params[0].name = P4PT_TCB_ISCSI_WRITE_LATENCY_DISTRIBUTION_BASE;
    symbols[i].params[0].val = get_start_offset(CAPRI_HBM_REG_P4PT_TCB_ISCSI_WRITE_LATENCY_DISTRIBUTION_MEM);
    i++;

    symbols[i].name = "rawr_desc_sem_post_update.bin";
    symbols[i].num_params = 1;
    symbols[i].params[0].name = RNMDR_TABLE_BASE;
    symbols[i].params[0].val = get_start_offset(CAPRI_HBM_REG_NMDR_RX);
    i++;

    symbols[i].name = "rawr_desc_free.bin";
    symbols[i].num_params = 1;
    symbols[i].params[0].name = RNMDR_TABLE_BASE;
    symbols[i].params[0].val = get_start_offset(CAPRI_HBM_REG_NMDR_RX);
    i++;

    symbols[i].name = "rawr_ppage_sem_post_update.bin";
    symbols[i].num_params = 1;
    symbols[i].params[0].name = RNMPR_TABLE_BASE;
    symbols[i].params[0].val = get_start_offset(CAPRI_HBM_REG_NMPR_BIG_RX);
    i++;

    symbols[i].name = "rawr_mpage_sem_post_update.bin";
    symbols[i].num_params = 1;
    symbols[i].params[0].name = RNMPR_SMALL_TABLE_BASE;
    symbols[i].params[0].val = get_start_offset(CAPRI_HBM_REG_NMPR_SMALL_RX);
    i++;

    symbols[i].name = "rawr_mpage_free.bin";
    symbols[i].num_params = 2;
    symbols[i].params[0].name = RNMPR_TABLE_BASE;
    symbols[i].params[0].val = get_start_offset(CAPRI_HBM_REG_NMPR_BIG_RX);
    symbols[i].params[1].name = RNMPR_SMALL_TABLE_BASE;
    symbols[i].params[1].val = get_start_offset(CAPRI_HBM_REG_NMPR_SMALL_RX);
    i++;

    symbols[i].name = "rawc_desc_free.bin";
    symbols[i].num_params = 1;
    symbols[i].params[0].name = RNMDR_TABLE_BASE;
    symbols[i].params[0].val = get_start_offset(CAPRI_HBM_REG_NMDR_RX);
    i++;

    symbols[i].name = "rawc_page0_free.bin";
    symbols[i].num_params = 2;
    symbols[i].params[0].name = RNMPR_TABLE_BASE;
    symbols[i].params[0].val = get_start_offset(CAPRI_HBM_REG_NMPR_BIG_RX);
    symbols[i].params[1].name = RNMPR_SMALL_TABLE_BASE;
    symbols[i].params[1].val = get_start_offset(CAPRI_HBM_REG_NMPR_SMALL_RX);
    i++;

    symbols[i].name = "proxyr_mpage_sem_pindex_post_update.bin";
    symbols[i].num_params = 1;
    symbols[i].params[0].name = RNMPR_SMALL_TABLE_BASE;
    symbols[i].params[0].val = get_start_offset(CAPRI_HBM_REG_NMPR_SMALL_RX);
    i++;

    symbols[i].name = "proxyr_desc_free.bin";
    symbols[i].num_params = 1;
    symbols[i].params[0].name = RNMDR_TABLE_BASE;
    symbols[i].params[0].val = get_start_offset(CAPRI_HBM_REG_NMDR_RX);
    i++;

    symbols[i].name = "proxyr_mpage_free.bin";
    symbols[i].num_params = 2;
    symbols[i].params[0].name = RNMPR_TABLE_BASE;
    symbols[i].params[0].val = get_start_offset(CAPRI_HBM_REG_NMPR_BIG_RX);
    symbols[i].params[1].name = RNMPR_SMALL_TABLE_BASE;
    symbols[i].params[1].val = get_start_offset(CAPRI_HBM_REG_NMPR_SMALL_RX);
    i++;

    symbols[i].name = "proxyc_desc_free.bin";
    symbols[i].num_params = 1;
    symbols[i].params[0].name = RNMDR_TABLE_BASE;
    symbols[i].params[0].val = get_start_offset(CAPRI_HBM_REG_NMDR_RX);
    i++;

    symbols[i].name = "proxyc_page0_free.bin";
    symbols[i].num_params = 2;
    symbols[i].params[0].name = RNMPR_TABLE_BASE;
    symbols[i].params[0].val = get_start_offset(CAPRI_HBM_REG_NMPR_BIG_RX);
    symbols[i].params[1].name = RNMPR_SMALL_TABLE_BASE;
    symbols[i].params[1].val = get_start_offset(CAPRI_HBM_REG_NMPR_SMALL_RX);
    i++;

    symbols[i].name = "tls-dec-aesgcm-newseg-queue-barco.bin";
    symbols[i].num_params = 1;
    symbols[i].params[0].name = BRQ_BASE;
    symbols[i].params[0].val = get_start_offset(CAPRI_HBM_REG_BARCO_RING_GCM1);
    i++;

    symbols[i].name = "tls-dec-aesgcm-newseg-read-tnmdr-odesc-alloc-idx.bin";
    symbols[i].num_params = 1;
    symbols[i].params[0].name = TNMDR_TABLE_BASE;
    symbols[i].params[0].val = get_start_offset(CAPRI_HBM_REG_NMDR_TX);
    i++;

    symbols[i].name = "tls-dec-aesgcm-newseg-read-tnmpr-opage-alloc-idx.bin";
    symbols[i].num_params = 1;
    symbols[i].params[0].name = TNMPR_TABLE_BASE;
    symbols[i].params[0].val = get_start_offset(CAPRI_HBM_REG_NMPR_BIG_TX);
    i++;

    symbols[i].name = "tls-dec-aesgcm-newseg-read-tnmdr-idesc-alloc-idx.bin";
    symbols[i].num_params = 1;
    symbols[i].params[0].name = TNMDR_TABLE_BASE;
    symbols[i].params[0].val = get_start_offset(CAPRI_HBM_REG_NMDR_TX);
    i++;

    symbols[i].name = "req_tx_dcqcn_enforce_process.bin";
    symbols[i].num_params = 1;
    symbols[i].params[0].name = NUM_CLOCK_TICKS_PER_US;
    symbols[i].params[0].val = capri_coreclk_freq;
    i++;

    symbols[i].name = "resp_tx_dcqcn_enforce_process.bin";
    symbols[i].num_params = 1;
    symbols[i].params[0].name = NUM_CLOCK_TICKS_PER_US;
    symbols[i].params[0].val = capri_coreclk_freq;
    i++;


    symbols[i].name = "req_tx_timer_expiry_process.bin";
    symbols[i].num_params = 1;
    symbols[i].params[0].name = NUM_CLOCK_TICKS_PER_US;
    symbols[i].params[0].val = capri_coreclk_freq;
    i++;

    symbols[i].name = "cpu_hash_calc_id.bin";
    symbols[i].num_params = 2;
    symbols[i].params[0].name = CAPRI_CPU_HASH_MASK;
    symbols[i].params[0].val = 0x1;
    symbols[i].params[1].name = CAPRI_CPU_MAX_ARQID;
    symbols[i].params[1].val = 0x0;
    i++;

    symbols[i].name = "tls-mac-queue-brq.bin";
    symbols[i].num_params = 1;
    symbols[i].params[0].name = BRQ_BASE;
    symbols[i].params[0].val = get_start_offset(CAPRI_HBM_REG_BARCO_RING_MPP2);
    i++;

    symbols[i].name = "tls-mac-queue-enc-brq.bin";
    symbols[i].num_params = 1;
    symbols[i].params[0].name = BRQ_BASE;
    symbols[i].params[0].val = get_start_offset(CAPRI_HBM_REG_BARCO_RING_MPP3);
    i++;

    symbols[i].name = "tls-mac-read-idesc.bin";
    symbols[i].num_params = 1;
    symbols[i].params[0].name = TLS_PROXY_PAD_BYTES_HBM_TABLE_BASE;
    symbols[i].params[0].val = get_start_offset(CAPRI_HBM_REG_TLS_PROXY_PAD_TABLE);
    i++;

    symbols[i].name = "tls-mac-read-tnmdr-alloc-idx.bin";
    symbols[i].num_params = 1;
    symbols[i].params[0].name = TNMDR_TABLE_BASE;
    symbols[i].params[0].val = get_start_offset(CAPRI_HBM_REG_NMDR_TX);
    i++;

    symbols[i].name = "tls-mac-read-tnmpr-alloc-idx.bin";
    symbols[i].num_params = 1;
    symbols[i].params[0].name = TNMPR_TABLE_BASE;
    symbols[i].params[0].val = get_start_offset(CAPRI_HBM_REG_NMPR_BIG_TX);
    i++;

    symbols[i].name = "cpu_write_arq.bin";
    symbols[i].num_params = 1;
    symbols[i].params[0].name = ARQRX_BASE;
    symbols[i].params[0].val = get_start_offset(CAPRI_HBM_REG_ARQRX);
    i++;

    symbols[i].name = "tcp-write-arq.bin";
    symbols[i].num_params = 1;
    symbols[i].params[0].name = ARQRX_BASE;
    symbols[i].params[0].val = get_start_offset(CAPRI_HBM_REG_ARQRX);
    i++;

    symbols[i].name = "rawr_hash_calc_id.bin";
    symbols[i].num_params = 2;
    symbols[i].params[0].name = CAPRI_CPU_HASH_MASK;
    symbols[i].params[0].val = 0x1;
    symbols[i].params[1].name = CAPRI_CPU_MAX_ARQID;
    symbols[i].params[1].val = 0x0;
    i++;

    symbols[i].name = "tls-enc-serq-consume.bin";
    symbols[i].num_params = 2;
    symbols[i].params[0].name = TLS_PROXY_BARCO_GCM0_PI_HBM_TABLE_BASE;

    /*
     * The 'CAPRI_HBM_REG_TLS_PROXY_PAD_TABLE' region is provisioned for 1KB out of
     * which CAPRI_MAX_TLS_PAD_SIZE is used for Pad bytes. We'll use the remaining
     * HBM memory from this region to store other TLS global resources.
     */
    symbols[i].params[0].val = get_start_offset(CAPRI_HBM_REG_TLS_PROXY_PAD_TABLE) +
                                 BARCO_GCM0_PI_HBM_TABLE_OFFSET;

    symbols[i].params[1].name = TLS_PROXY_BARCO_MPP1_PI_HBM_TABLE_BASE;

    /*
     * The 'CAPRI_HBM_REG_TLS_PROXY_PAD_TABLE' region is provisioned for 1KB out of
     * which CAPRI_MAX_TLS_PAD_SIZE is used for Pad bytes. We'll use the remaining
     * HBM memory from this region to store other TLS global resources.
     */
    symbols[i].params[1].val = get_start_offset(CAPRI_HBM_REG_TLS_PROXY_PAD_TABLE) +
                                 BARCO_MPP1_PI_HBM_TABLE_OFFSET;
    i++;

    symbols[i].name = "tls-dec-read-header.bin";
    symbols[i].num_params = 2;
    symbols[i].params[0].name = TLS_PROXY_BARCO_GCM1_PI_HBM_TABLE_BASE;

    /*
     * The 'CAPRI_HBM_REG_TLS_PROXY_PAD_TABLE' region is provisioned for 1KB out of
     * which CAPRI_MAX_TLS_PAD_SIZE is used for Pad bytes. We'll use the remaining
     * HBM memory from this region to store other TLS global resources.
     */
    symbols[i].params[0].val = get_start_offset(CAPRI_HBM_REG_TLS_PROXY_PAD_TABLE) +
                                 BARCO_GCM1_PI_HBM_TABLE_OFFSET;

    symbols[i].params[1].name = TLS_PROXY_BARCO_MPP1_PI_HBM_TABLE_BASE;

    /*
     * The 'CAPRI_HBM_REG_TLS_PROXY_PAD_TABLE' region is provisioned for 1KB out of
     * which CAPRI_MAX_TLS_PAD_SIZE is used for Pad bytes. We'll use the remaining
     * HBM memory from this region to store other TLS global resources.
     */
    symbols[i].params[1].val = get_start_offset(CAPRI_HBM_REG_TLS_PROXY_PAD_TABLE) +
                                 BARCO_MPP1_PI_HBM_TABLE_OFFSET;
    i++;

    symbols[i].name = "esp_ipv4_tunnel_h2n_txdma1_ipsec_encap_txdma_initial_table.bin";
    symbols[i].num_params = 2;
    symbols[i].params[0].name = TLS_PROXY_BARCO_GCM0_PI_HBM_TABLE_BASE;
    symbols[i].params[0].val = get_start_offset(CAPRI_HBM_REG_TLS_PROXY_PAD_TABLE) +
                                 BARCO_GCM0_PI_HBM_TABLE_OFFSET;
    symbols[i].params[1].name = IPSEC_GLOBAL_BAD_DMA_COUNTER_BASE_H2N;
    symbols[i].params[1].val = get_start_offset(CAPRI_HBM_REG_IPSEC_GLOBAL_DROP_STATS);
    i++;

    symbols[i].name = "esp_ipv4_tunnel_h2n_txdma1_s1_dummy.bin";
    symbols[i].num_params = 2;
    symbols[i].params[0].name = TLS_PROXY_BARCO_GCM0_PI_HBM_TABLE_BASE;
    symbols[i].params[0].val = get_start_offset(CAPRI_HBM_REG_TLS_PROXY_PAD_TABLE) +
                                 BARCO_GCM0_PI_HBM_TABLE_OFFSET;
    symbols[i].params[1].name = IPSEC_GLOBAL_BAD_DMA_COUNTER_BASE_H2N;
    symbols[i].params[1].val = get_start_offset(CAPRI_HBM_REG_IPSEC_GLOBAL_DROP_STATS);
    i++;

    symbols[i].name = "esp_ipv4_tunnel_h2n_txdma1_allocate_barco_req_pindex2.bin";
    symbols[i].num_params = 2;
    symbols[i].params[0].name = BRQ_BASE;
    symbols[i].params[0].val = get_start_offset(CAPRI_HBM_REG_BRQ);
    symbols[i].params[1].name = IPSEC_GLOBAL_BAD_DMA_COUNTER_BASE_H2N;
    symbols[i].params[1].val = get_start_offset(CAPRI_HBM_REG_IPSEC_GLOBAL_DROP_STATS);
    i++;

    symbols[i].name = "tls-enc-gc-setup.bin";
    symbols[i].num_params = 2;
    symbols[i].params[0].name = RNMDR_TLS_GC_TABLE_BASE;
    symbols[i].params[0].val = get_start_offset(CAPRI_HBM_REG_NMDR_RX_GC) +
        CAPRI_HBM_GC_PER_PRODUCER_RING_SIZE * CAPRI_HBM_RNMDR_ENTRY_SIZE * CAPRI_RNMDR_GC_TLS_RING_PRODUCER;
    symbols[i].params[1].name = TCP_PROXY_STATS;
    symbols[i].params[1].val = get_start_offset(CAPRI_HBM_REG_TCP_PROXY_STATS);
    i++;

    symbols[i].name = "tls-dec-gc-setup.bin";
    symbols[i].num_params = 2;
    symbols[i].params[0].name = RNMDR_TLS_GC_TABLE_BASE;
    symbols[i].params[0].val = get_start_offset(CAPRI_HBM_REG_NMDR_RX_GC) +
        CAPRI_HBM_GC_PER_PRODUCER_RING_SIZE * CAPRI_HBM_RNMDR_ENTRY_SIZE * CAPRI_RNMDR_GC_TLS_RING_PRODUCER;
    symbols[i].params[1].name = TCP_PROXY_STATS;
    symbols[i].params[1].val = get_start_offset(CAPRI_HBM_REG_TCP_PROXY_STATS);
    i++;

    symbols[i].name = "tls-enc-bsq.bin";
    symbols[i].num_params = 2;
    symbols[i].params[0].name = TLS_PROXY_BARCO_GCM0_PI_HBM_TABLE_BASE;
    symbols[i].params[0].val = get_start_offset(CAPRI_HBM_REG_TLS_PROXY_PAD_TABLE) +
                                 BARCO_GCM0_PI_HBM_TABLE_OFFSET;
    symbols[i].params[1].name = TLS_PROXY_BARCO_MPP1_PI_HBM_TABLE_BASE;

    /*
     * The 'CAPRI_HBM_REG_TLS_PROXY_PAD_TABLE' region is provisioned for 1KB out of
     * which CAPRI_MAX_TLS_PAD_SIZE is used for Pad bytes. We'll use the remaining
     * HBM memory from this region to store other TLS global resources.
     */
    symbols[i].params[1].val = get_start_offset(CAPRI_HBM_REG_TLS_PROXY_PAD_TABLE) +
                                 BARCO_MPP1_PI_HBM_TABLE_OFFSET;
    i++;

    symbols[i].name = "tls-dec-bsq.bin";
    symbols[i].num_params = 2;
    symbols[i].params[0].name = TLS_PROXY_BARCO_GCM1_PI_HBM_TABLE_BASE;
    symbols[i].params[0].val = get_start_offset(CAPRI_HBM_REG_TLS_PROXY_PAD_TABLE) +
                                BARCO_GCM1_PI_HBM_TABLE_OFFSET;
    symbols[i].params[1].name = TLS_PROXY_BARCO_MPP1_PI_HBM_TABLE_BASE;

    /*
     * The 'CAPRI_HBM_REG_TLS_PROXY_PAD_TABLE' region is provisioned for 1KB out of
     * which CAPRI_MAX_TLS_PAD_SIZE is used for Pad bytes. We'll use the remaining
     * HBM memory from this region to store other TLS global resources.
     */
    symbols[i].params[1].val = get_start_offset(CAPRI_HBM_REG_TLS_PROXY_PAD_TABLE) +
                                 BARCO_MPP1_PI_HBM_TABLE_OFFSET;
    i++;

    symbols[i].name = "eth_rx_stats.bin";
    symbols[i].num_params = 1;
    symbols[i].params[0].name = LIF_STATS_BASE;
    symbols[i].params[0].val = get_start_offset(CAPRI_HBM_REG_LIF_STATS);
    i++;

    symbols[i].name = "eth_tx_stats.bin";
    symbols[i].num_params = 1;
    symbols[i].params[0].name = LIF_STATS_BASE;
    symbols[i].params[0].val = get_start_offset(CAPRI_HBM_REG_LIF_STATS);
    i++;

    // Please increment CAPRI_P4PLUS_NUM_SYMBOLS when you want to add more below
    HAL_ASSERT(i <= P4PLUS_SYMBOLS_MAX);

    return i;
}

}    // namespace pd
}    // namespace hal
