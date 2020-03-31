#ifndef __ASIC_HBM_HPP__
#define __ASIC_HBM_HPP__

#include "asic/asic.hpp"
#include "platform/utils/mpartition.hpp"
#include "include/sdk/types.hpp"

#include "platform/capri/capri_hbm_rw.hpp"  /* WIP-WIP: remove capri */
#include "asic/rw/asicrw.hpp"

namespace sdk  {
namespace asic {

#define JP4_PRGM                 "p4_program"
#define JP4PLUS_PRGM             "p4plus_program"
#define JLIF_QSTATE              "qstate"
#define JTIMERS                  "timers"

#define JP4_SEMAPHORE       "semaphore"
#define JP4_REPL            "mcast_repl"
#define JP4_IPFIX           "ipfix"

#define ASIC_HBM_REG_P4_ATOMIC_STATS    "p4_atomic_stats"
#define ASIC_HBM_REG_LIF_STATS          "lif_stats"
#define ASIC_HBM_REG_PORT_STATS         "port_stats"
#define ASIC_HBM_REG_FLOW_HASH          "flow_hash"             /* not in-use */
#define ASIC_HBM_REG_FLOW_HASH_OVFLOW   "flow_hash_overflow"    /* not in-use */
#define ASIC_HBM_REG_IPSEC_NMDR_RX      "ipsec-nmdr-rx"
#define ASIC_HBM_REG_IPSEC_NMDR_TX      "ipsec-nmdr-tx"
#define ASIC_HBM_REG_IPSEC_BIG_NMDR_RX  "ipsec-big-nmdr-rx"
#define ASIC_HBM_REG_IPSEC_BIG_NMDR_TX  "ipsec-big-nmdr-tx"
#define ASIC_HBM_REG_NMDR_RX_GC         "nmdr-rx-gc"
#define ASIC_HBM_REG_NMDR_TX_GC         "nmdr-tx-gc"
#define ASIC_HBM_REG_IPSEC_DESC_RX      "ipsec-descriptor-rx"
#define ASIC_HBM_REG_IPSEC_DESC_TX      "ipsec-descriptor-tx"
#define ASIC_HBM_REG_IPSEC_BIG_DESC_RX  "ipsec-big-descriptor-rx"
#define ASIC_HBM_REG_IPSEC_BIG_DESC_TX  "ipsec-big-descriptor-tx"
#define ASIC_HBM_REG_IPSEC_NMPR_RX      "ipsec-nmpr-rx"
#define ASIC_HBM_REG_IPSEC_NMPR_TX      "ipsec-nmpr-tx"
#define ASIC_HBM_REG_IPSEC_BIG_NMPR_RX  "ipsec-nmpr-big-rx"
#define ASIC_HBM_REG_IPSEC_BIG_NMPR_TX  "ipsec-nmpr-big-tx"
#define ASIC_HBM_REG_ENC_PAGE_BIG_RX    "enc-page-big-rx"
#define ASIC_HBM_REG_ENC_PAGE_BIG_TX    "enc-page-big-tx"
#define ASIC_HBM_REG_DEC_PAGE_BIG_RX    "dec-page-big-rx"
#define ASIC_HBM_REG_DEC_PAGE_BIG_TX    "dec-page-big-tx"
#define ASIC_HBM_REG_PAGE_SMALL_RX      "page-small-rx"
#define ASIC_HBM_REG_PAGE_SMALL_TX      "page-small-tx"
#define ASIC_HBM_REG_BARCO_RING_ASYM    "brq-ring-asym"
#define ASIC_HBM_REG_BARCO_RING_GCM0    "brq-ring-gcm0"
#define ASIC_HBM_REG_BRQ                ASIC_HBM_REG_BARCO_RING_GCM0
#define ASIC_HBM_REG_BARCO_RING_GCM1    "brq-ring-gcm1"
#define ASIC_HBM_REG_BARCO_RING_XTS0    "brq-ring-xts0"
#define ASIC_HBM_REG_BARCO_RING_XTS1    "brq-ring-xts1"
#define ASIC_HBM_REG_CRYPTO_ASYM_DMA_DESCR "crypto-asym-dma-descr"
#define ASIC_HBM_REG_BARCO_RING_MPP0    "brq-ring-mpp0"
#define ASIC_HBM_REG_BARCO_RING_MPP1    "brq-ring-mpp1"
#define ASIC_HBM_REG_BARCO_RING_MPP2    "brq-ring-mpp2"
#define ASIC_HBM_REG_BARCO_RING_MPP3    "brq-ring-mpp3"
#define ASIC_HBM_REG_BARCO_RING_CP      "brq-ring-cp"
#define ASIC_HBM_REG_BARCO_RING_CP_HOT  "brq-ring-cp-hot"
#define ASIC_HBM_REG_BARCO_RING_DC      "brq-ring-dc"
#define ASIC_HBM_REG_BARCO_RING_DC_HOT  "brq-ring-dc-hot"
#define ASIC_HBM_REG_CRYPTO_SYM_MSG_DESCR "crypto-sym-msg-descr"
#define ASIC_HBM_REG_CRYPTO_HBM_MEM     "crypto-hbm-mem"
#define ASIC_HBM_REG_CRYPTO_HBM_MEM_BIG  "crypto-hbm-mem-big"
#define ASIC_BARCO_KEY_DESC             "key-desc-array"
#define ASIC_BARCO_KEY_MEM              "key-mem"
#define ASIC_HBM_REG_ASYM_KEY_DESCR     "crypto-asym-key-desc-array"
#define ASIC_HBM_REG_IPSEC_PAD_TABLE    "ipsec_pad_table"
#define ASIC_HBM_REG_TLS_PROXY_PAD_TABLE "tls_proxy_pad_table"
#define ASIC_HBM_REG_TLS_PROXY_BARCO_GCM0_PI "tls_proxy_barco_gcm0_pi"
#define ASIC_HBM_REG_BSQ                "bsq"
#define ASIC_HBM_REG_SERQ               "serq"
#define ASIC_HBM_REG_SESQ               "sesq"
#define ASIC_HBM_REG_ASESQ              "asesq"
#define ASIC_HBM_REG_IPSECCB            "ipsec-cb"
#define ASIC_HBM_REG_IPSECCB_BARCO      "ipsec_cb_barco"
#define ASIC_HBM_REG_ARQRX              "arq-rx"
#define ASIC_HBM_REG_ASQ                "asq"
#define ASIC_HBM_REG_ASCQ               "ascq"
#define ASIC_HBM_REG_TCP_ACTL_Q         "tcp-actl-q"
#define ASIC_HBM_REG_CPU_TX_DR          "cpu-tx-dr"
#define ASIC_HBM_REG_CPU_TX_PR          "cpu-tx-pr"
#define ASIC_HBM_REG_CPU_RX_DPR         "cpu-rx-dpr"
#define ASIC_HBM_REG_CPU_TX_DESCR       "cpu-tx-descriptor"
#define ASIC_HBM_REG_CPU_TX_PAGE        "cpu-tx-page"
#define ASIC_HBM_REG_CPU_RX_DESC_PAGE   "cpu-rx-desc-page"
#define ASIC_CPU_HASH_MASK              "cpu_hash_mask"
#define ASIC_CPU_MAX_ARQID              "cpu_max_arqid"
#define CPU_TX_DOT1Q_HDR_OFFSET          "cpu_tx_dot1q_hdr_offset"
#define ASIC_HBM_REG_TXS_SCHEDULER      "tx-scheduler"
#define ASIC_HBM_REG_APP_REDIR_RAWC     "app-redir-rawc"
#define ASIC_HBM_REG_APP_REDIR_PROXYR   "app-redir-proxyr"
#define ASIC_HBM_REG_APP_REDIR_PROXYC   "app-redir-proxyc"
#define ASIC_HBM_REG_RDMA_EQ_INTR_TABLE   "rdma-eq-intr-table"
#define ASIC_HBM_REG_RDMA_ATOMIC_RESOURCE_ADDR "rdma-atomic-resource-addr"
#define ASIC_HBM_REG_RDMA_STATS_HDRS    "rdma-stats-hdrs"
#define ASIC_HBM_REG_P4PT_TCB_MEM       "p4pt-tcb"
#define ASIC_HBM_REG_P4PT_TCB_ISCSI_REC_MEM "p4pt-tcb-iscsi-rec"
#define ASIC_HBM_REG_P4PT_TCB_ISCSI_STATS_MEM "p4pt-tcb-iscsi-stats"
#define ASIC_HBM_REG_P4PT_TCB_ISCSI_READ_LATENCY_DISTRIBUTION_MEM "p4pt-tcb-iscsi-read-latency-distribution"
#define ASIC_HBM_REG_P4PT_TCB_ISCSI_WRITE_LATENCY_DISTRIBUTION_MEM "p4pt-tcb-iscsi-write-latency-distribution"
#define ASIC_HBM_REG_RSS_INDIR_TABLE    "rss_indir_table"
#define ASIC_HBM_REG_QOS_HBM_FIFO       "qos-hbm-fifo"
#define ASIC_HBM_REG_NMDPR_SMALL_TX    "nmdpr-small-tx"
#define ASIC_HBM_REG_NMDPR_SMALL_RX    "nmdpr-small-rx"
#define ASIC_HBM_REG_NMDPR_BIG_TX      "nmdpr-big-tx"
#define ASIC_HBM_REG_NMDPR_BIG_RX      "nmdpr-big-rx"
#define ASIC_HBM_REG_NMDPR_OBJS_SMALL_TX   "small-tx-objs"
#define ASIC_HBM_REG_NMDPR_OBJS_SMALL_RX   "small-rx-objs"
#define ASIC_HBM_REG_NMDPR_OBJS_BIG_TX     "big-tx-objs"
#define ASIC_HBM_REG_NMDPR_OBJS_BIG_RX     "big-rx-objs"
#define ASIC_HBM_REG_IPSEC_GLOBAL_DROP_STATS "ipsec-global-drop-stats"
#define ASIC_HBM_REG_TCP_PROXY_STATS       "tcp_proxy_stats"
#define ASIC_HBM_REG_TLS_PROXY_GLOBAL_STATS    "tls_proxy_global_stats"
#define ASIC_HBM_REG_TCP_OOO_QBASE_RING    "tcp_ooo_qbase_ring"
#define ASIC_HBM_REG_TCP_OOO_QUEUE         "tcp_ooo_queue"
#define ASIC_HBM_REG_TCP_OOO_RX2TX_QUEUE   "tcp_ooo_rx2tx_queue"
#define ASIC_HBM_REG_PER_FTE_STATS         "fte-stats"
#define ASIC_HBM_REG_SESSION_SUMMARY_STATS "session-summary-stats"
#define ASIC_HBM_REG_P4_HWERR_STATS        "p4_hwerr_stats"
#define ASIC_HBM_REG_QOS_DSCP_COS_MAP     "qos-dscp-cos-map"
#define ASIC_HBM_REG_TCP_PROXY_PER_FLOW_STATS "tcp_proxy_per_flow_stats"
#define ASIC_HBM_REG_NWSEC_RULE_STATS            "nwsec_rule_stats"
#define ASIC_HBM_REG_FLOW_TELEMETRY_STATS        "flow_telemetry_stats"
#define ASIC_HBM_REG_NVME                        "nvme"
#define ASIC_HBM_REG_OPAQUE_TAG                  "opaque_tag"
#define ASIC_HBM_NVME_TX_NMDPR_RING_BASE         ASIC_HBM_REG_NMDPR_BIG_TX
#define ASIC_HBM_NVME_RX_NMDPR_RING_BASE         ASIC_HBM_REG_NMDPR_BIG_RX
#define ASIC_HBM_REG_CLOCK_GETTIMEOFDAY     "clock"
#define ASIC_HBM_REG_IPFIX_STATS            "ipfix_stats"

#define ASIC_NUM_SEMAPHORES            512

#define RNMDR_TABLE_BASE        "hbm_rnmdr_table_base"
#define RNMPR_TABLE_BASE        "hbm_rnmpr_table_base"
#define IPSEC_RNMDR_TABLE_BASE  "hbm_ipsec_rnmdr_table_base"
#define IPSEC_BIG_RNMPR_TABLE_BASE  "hbm_ipsec_big_rnmpr_table_base"
#define IPSEC_RNMPR_TABLE_BASE  "hbm_ipsec_rnmpr_table_base"
#define RNMPR_SMALL_TABLE_BASE  "hbm_rnmpr_small_table_base"
#define TNMDR_TABLE_BASE        "hbm_tnmdr_table_base"
#define TNMPR_TABLE_BASE        "hbm_tnmpr_table_base"
#define IPSEC_PAGE_ADDR_RX      "hbm_ipsec_page_rx"
#define IPSEC_PAGE_ADDR_TX      "hbm_ipsec_page_tx"
#define IPSEC_TNMDR_TABLE_BASE  "hbm_ipsec_tnmdr_table_base"
#define IPSEC_TNMPR_TABLE_BASE  "hbm_ipsec_tnmpr_table_base"
#define IPSEC_BIG_TNMPR_TABLE_BASE  "hbm_ipsec_big_tnmpr_table_base"
#define TNMPR_SMALL_TABLE_BASE  "hbm_tnmpr_small_table_base"
#define BRQ_BASE                "hbm_brq_base"
#define BRQ_GCM1_BASE           "hbm_brq_gcm1_base"
#define BRQ_MPP1_BASE           "hbm_brq_mpp1_base"
#define BRQ_MPP2_BASE           "hbm_brq_mpp2_base"
#define BRQ_MPP3_BASE           "hbm_brq_mpp3_base"
#define TCP_RX_STATS_TABLE_BASE "hbm_tcp_rx_stats_table_base"
#define RNMDR_GC_TABLE_BASE     "hbm_rnmdr_gc_table_base"
#define TNMDR_GC_TABLE_BASE     "hbm_tnmdr_gc_table_base"
#define RNMDPR_SMALL_TABLE_BASE "hbm_rnmdpr_small_table_base"
#define RNMDPR_BIG_TABLE_BASE   "hbm_rnmdpr_big_table_base"
#define TNMDPR_SMALL_TABLE_BASE "hbm_tnmdpr_small_table_base"
#define TNMDPR_BIG_TABLE_BASE   "hbm_tnmdpr_big_table_base"
#define CPU_RX_DPR_TABLE_BASE   "hbm_cpu_rx_dpr_table_base"

#define TCP_OOQ_TABLE_BASE      "hbm_ooq_table_base"

#define RNMDR_TLS_GC_TABLE_BASE "hbm_rnmdr_tls_gc_table_base"
#define GC_GLOBAL_TABLE_BASE    "hbm_gc_table_base"
#define TCP_PROXY_STATS         "tcp_proxy_stats"
#define IPSEC_ENC_NMDR_PI       "ipsec_enc_nmdr_pi"
#define IPSEC_ENC_NMDR_CI       "ipsec_enc_nmdr_ci"
#define IPSEC_DEC_NMDR_PI       "ipsec_dec_nmdr_pi"
#define IPSEC_DEC_NMDR_CI       "ipsec_dec_nmdr_ci"

#define IPSEC_CB_BASE "ipsec_cb_base"
#define IPSEC_PAD_BYTES_HBM_TABLE_BASE "ipsec_pad_table_base"
#define IPSEC_IP_HDR_BASE "ipsec_ip_hdr_base"
#define IPSEC_GLOBAL_BAD_DMA_COUNTER_BASE_H2N "ipsec_global_drop_h2n_counters"
#define IPSEC_GLOBAL_BAD_DMA_COUNTER_BASE_N2H "ipsec_global_drop_n2h_counters"


#define TLS_PROXY_PAD_BYTES_HBM_TABLE_BASE "tls_pad_table_base"
#define TLS_PROXY_BARCO_GCM0_PI_HBM_TABLE_BASE "tls_barco_gcm0_pi_table_base"
#define TLS_PROXY_BARCO_GCM1_PI_HBM_TABLE_BASE "tls_barco_gcm1_pi_table_base"
#define TLS_PROXY_BARCO_MPP0_PI_HBM_TABLE_BASE "tls_barco_mpp0_pi_table_base"
#define TLS_PROXY_BARCO_MPP1_PI_HBM_TABLE_BASE "tls_barco_mpp1_pi_table_base"
#define TLS_PROXY_BARCO_MPP2_PI_HBM_TABLE_BASE "tls_barco_mpp2_pi_table_base"
#define TLS_PROXY_BARCO_MPP3_PI_HBM_TABLE_BASE "tls_barco_mpp3_pi_table_base"

#define ARQRX_BASE                      "hbm_arqrx_base"

#define TCP_ACTL_Q_BASE                 "tcp_actl_q_base"

#define P4_FLOW_HASH_BASE               "p4_flow_hash_base"
#define P4_FLOW_HASH_OVERFLOW_BASE      "p4_flow_hash_overflow_base"
#define P4_FLOW_INFO_BASE               "p4_flow_info_base"
#define P4_SESSION_STATE_BASE           "p4_session_state_base"
#define P4_FLOW_STATS_BASE              "p4_flow_stats_base"
#define IPFIX_STATS_BASE                "ipfix_stats_base"
#define IPFIX_EXPORTED_FLOW_STATS       "ipfix_exported_flow_stats"
#define IPFIX_EXPORTED_FLOW_STATS_BASE  "ipfix_exported_flow_stats_base"

#define NUM_CLOCK_TICKS_PER_CNP         "rdma_num_clock_ticks_per_cnp"
#define NUM_CLOCK_TICKS_PER_US          "rdma_num_clock_ticks_per_us"
#define RDMA_EQ_INTR_TABLE_BASE         "hbm_rdma_eq_intr_table_base"
#define RDMA_ATOMIC_RESOURCE_ADDR       "rdma_atomic_resource_addr"
#define RDMA_PCIE_ATOMIC_BASE_ADDR      "rdma_pcie_atomic_base_addr"
#define RDMA_STATS_HDRS_ADDR            "rdma_stats_hdrs_addr"

#define P4PT_TCB_BASE                   "p4pt_tcb_base"
#define P4PT_TCB_ISCSI_REC_BASE         "p4pt_tcb_iscsi_rec_base"
#define P4PT_TCB_ISCSI_STATS_BASE       "p4pt_tcb_iscsi_stats_base"
#define P4PT_TCB_ISCSI_READ_LATENCY_DISTRIBUTION_BASE   "p4pt_tcb_iscsi_read_latency_distribution_base"
#define P4PT_TCB_ISCSI_WRITE_LATENCY_DISTRIBUTION_BASE  "p4pt_tcb_iscsi_write_latency_distribution_base"

#define TLS_PROXY_GLOBAL_STATS          "tls_proxy_global_stats"

#define LIF_STATS_BASE                  "lif_stats_base"

#define QOS_DSCP_COS_MAP_BASE           "qos_dscp_cos_map_addr"

#define NVME_NSCB_BASE                  "nvme_nscb_base"
#define NVME_CMD_CONTEXT_BASE           "nvme_cmd_context_base"
#define NVME_CMD_CONTEXT_RING_BASE      "nvme_cmd_context_ring_base"
#define NVME_RESOURCECB                 "nvme_resourcecb_addr"
#define NVME_NMDPR_RESOURCECB           "nvme_nmdpr_resourcecb_addr"
#define NVME_TXSESSPRODCB_BASE          "nvme_txsessprodcb_base"
#define NVME_RXSESSPRODCB_BASE          "nvme_rxsessprodcb_base"
#define NVME_TX_PDU_CONTEXT_BASE        "nvme_tx_pdu_context_base"
#define NVME_TX_PDU_CONTEXT_RING_BASE   "nvme_tx_pdu_context_ring_base"
#define NVME_RX_PDU_CONTEXT_BASE        "nvme_rx_pdu_context_base"
#define NVME_RX_PDU_CONTEXT_RING_BASE   "nvme_rx_pdu_context_ring_base"
#define NVME_TX_SESS_XTSQ_BASE          "nvme_tx_sess_xtsq_base"
#define NVME_TX_SESS_DGSTQ_BASE         "nvme_tx_sess_dgstq_base"
#define NVME_RX_SESS_XTSQ_BASE          "nvme_rx_sess_xtsq_base"
#define NVME_RX_SESS_DGSTQ_BASE         "nvme_rx_sess_dgstq_base"
#define NVME_TX_XTS_AOL_ARRAY_BASE      "nvme_tx_xts_aol_array_base"
#define NVME_TX_XTS_IV_ARRAY_BASE       "nvme_tx_xts_iv_array_base"
#define NVME_RX_XTS_AOL_ARRAY_BASE      "nvme_rx_xts_aol_array_base"
#define NVME_RX_XTS_IV_ARRAY_BASE       "nvme_rx_xts_iv_array_base"
#define NVME_TX_HWXTSCB                 "nvme_tx_hwxtscb_addr"
#define NVME_RX_HWXTSCB                 "nvme_rx_hwxtscb_addr"
#define NVME_TX_HWDGSTCB                "nvme_tx_hwdgstcb_addr"
#define NVME_RX_HWDGSTCB                "nvme_rx_hwdgstcb_addr"
#define NVME_TX_NMDPR_RING_BASE         "nvme_tx_nmdpr_ring_base"
#define NVME_RX_NMDPR_RING_BASE         "nvme_rx_nmdpr_ring_base"

//void               set_mem_partition(mpartition *mempartition);
mem_addr_t asic_get_mem_base(void);
mem_addr_t asic_get_mem_offset(const char *reg_name);
mem_addr_t asic_get_mem_addr(const char *reg_name);
uint32_t asic_get_mem_size_kb(const char *reg_name);
mpartition_region_t *asic_get_mem_region(char *reg_name);
mpartition_region_t *asic_get_hbm_region_by_address(uint64_t addr);
void asic_reset_hbm_regions(asic_cfg_t *asic_cfg);

}    // namespace asic
}    // namespace sdk

using sdk::asic::asic_get_mem_base;
using sdk::asic::asic_get_mem_offset;
using sdk::asic::asic_get_mem_addr;
using sdk::asic::asic_get_mem_size_kb;
using sdk::asic::asic_get_mem_region;
using sdk::asic::asic_get_hbm_region_by_address;

#endif    // __ASIC_HBM_HPP__
