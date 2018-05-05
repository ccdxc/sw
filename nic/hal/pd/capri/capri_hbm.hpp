#ifndef __CAPRI_HBM_HPP__
#define __CAPRI_HBM_HPP__

#define CAPRI_HBM_MEM_REG_NAME_MAX_LEN 80
#define CARPI_HBM_MEM_NUM_MEM_REGS 128

#define CAPRI_INVALID_OFFSET 0xFFFFFFFFFFFFFFFF

#define JP4_ATOMIC_STATS         "atomic_stats"

#define JKEY_REGIONS             "regions"
#define JKEY_REGION_NAME         "name"
#define JKEY_SIZE_KB             "size_kb"
#define JKEY_CACHE_PIPE          "cache"
#define JKEY_START_OFF           "start_offset"

#define JP4_PRGM                 "p4_program"
#define JP4PLUS_PRGM             "p4plus_program"
#define JLIF_QSTATE              "qstate"
#define JTIMERS                  "timers"

#include "nic/include/base.h"
#include "nic/include/hal_cfg.hpp"
#include "nic/hal/pd/capri/capri.hpp"

typedef enum capri_hbm_cache_pipe_s {
    // Note: Values are used in bitmap
    CAPRI_HBM_CACHE_PIPE_NONE       = 0,
    CAPRI_HBM_CACHE_PIPE_P4IG       = 1,
    CAPRI_HBM_CACHE_PIPE_P4EG       = 2,
    CAPRI_HBM_CACHE_PIPE_P4IG_P4EG  = 3,
    CAPRI_HBM_CACHE_PIPE_P4PLUS_TXDMA     = 4,
    CAPRI_HBM_CACHE_PIPE_P4PLUS_RXDMA     = 8,
    CAPRI_HBM_CACHE_PIPE_P4PLUS_PCIE_DB   = 16,
    CAPRI_HBM_CACHE_PIPE_P4PLUS_ALL       = 28,  // TxDMA + RxDMA + PCIE + DB blocks
} capri_hbm_cache_pipe_t;

typedef struct capri_hbm_region_s {
    char                    mem_reg_name[CAPRI_HBM_MEM_REG_NAME_MAX_LEN];
    uint32_t                size_kb;
    uint64_t                start_offset;
    capri_hbm_cache_pipe_t  cache_pipe;
} capri_hbm_region_t;

hal_ret_t capri_hbm_parse(capri_cfg_t *cfg);
uint64_t get_hbm_base(void);
uint64_t get_hbm_offset(const char *reg_name);
uint64_t get_start_offset(const char *reg_name);
uint32_t get_size_kb(const char *reg_name);
int32_t capri_hbm_read_mem(uint64_t addr, uint8_t *buf, uint32_t size);
int32_t capri_hbm_write_mem(uint64_t addr, uint8_t *buf, uint32_t size);

#define JKEY_REGIONS        "regions"
#define JKEY_REGION_NAME    "name"
#define JKEY_SIZE_KB        "size_kb"
#define JKEY_START_OFF      "start_offset"

#define JP4_PRGM            "p4_program"
#define JP4_SEMAPHORE       "semaphore"
#define JP4_REPL            "mcast_repl"
#define JP4_IPFIX           "ipfix"

#define CAPRI_HBM_REG_NMDR_RX            "nmdr-rx"
#define CAPRI_HBM_REG_NMDR_TX            "nmdr-tx"
#define CAPRI_HBM_REG_NMDR_RX_GC         "nmdr-rx-gc"
#define CAPRI_HBM_REG_NMDR_TX_GC         "nmdr-tx-gc"
#define CAPRI_HBM_REG_DESCRIPTOR_RX      "descriptor-rx"
#define CAPRI_HBM_REG_DESCRIPTOR_TX      "descriptor-tx"
#define CAPRI_HBM_REG_NMPR_BIG_RX        "nmpr-big-rx"
#define CAPRI_HBM_REG_NMPR_BIG_TX        "nmpr-big-tx"
#define CAPRI_HBM_REG_PAGE_BIG_RX        "page-big-rx"
#define CAPRI_HBM_REG_PAGE_BIG_TX        "page-big-tx"
#define CAPRI_HBM_REG_NMPR_SMALL_RX      "nmpr-small-rx"
#define CAPRI_HBM_REG_NMPR_SMALL_TX      "nmpr-small-tx"
#define CAPRI_HBM_REG_PAGE_SMALL_RX      "page-small-rx"
#define CAPRI_HBM_REG_PAGE_SMALL_TX      "page-small-tx"
#define CAPRI_HBM_REG_BARCO_RING_ASYM    "brq-ring-asym"
#define CAPRI_HBM_REG_BARCO_RING_GCM0    "brq-ring-gcm0"
#define CAPRI_HBM_REG_BRQ                CAPRI_HBM_REG_BARCO_RING_GCM0
#define CAPRI_HBM_REG_BARCO_RING_GCM1    "brq-ring-gcm1"
#define CAPRI_HBM_REG_BARCO_RING_XTS0    "brq-ring-xts0"
#define CAPRI_HBM_REG_BARCO_RING_XTS1    "brq-ring-xts1"
#define CAPRI_HBM_REG_CRYPTO_ASYM_DMA_DESCR "crypto-asym-dma-descr"
#define CAPRI_HBM_REG_BARCO_RING_MPP0    "brq-ring-mpp0"
#define CAPRI_HBM_REG_BARCO_RING_MPP1    "brq-ring-mpp1"
#define CAPRI_HBM_REG_BARCO_RING_MPP2    "brq-ring-mpp2"
#define CAPRI_HBM_REG_BARCO_RING_MPP3    "brq-ring-mpp3"
#define CAPRI_HBM_REG_BARCO_RING_CP      "brq-ring-cp"
#define CAPRI_HBM_REG_BARCO_RING_CP_HOT  "brq-ring-cp-hot"
#define CAPRI_HBM_REG_BARCO_RING_DC      "brq-ring-dc"
#define CAPRI_HBM_REG_BARCO_RING_DC_HOT  "brq-ring-dc-hot"
#define CAPRI_HBM_REG_CRYPTO_SYM_MSG_DESCR "crypto-sym-msg-descr"
#define CAPRI_HBM_REG_CRYPTO_HBM_MEM     "crypto-hbm-mem"
#define CAPRI_BARCO_KEY_DESC             "key-desc-array"
#define CAPRI_BARCO_KEY_MEM              "key-mem"
#define CAPRI_HBM_REG_ASYM_KEY_DESCR     "crypto-asym-key-desc-array"
#define CAPRI_HBM_REG_IPSEC_PAD_TABLE    "ipsec_pad_table"
#define CAPRI_HBM_REG_IPSEC_IP_HDR       "ipsec_ip_header"
#define CAPRI_HBM_REG_TLS_PROXY_PAD_TABLE "tls_proxy_pad_table"
#define CAPRI_HBM_REG_TLS_PROXY_BARCO_GCM0_PI "tls_proxy_barco_gcm0_pi"
#define CAPRI_HBM_REG_BSQ                "bsq"
#define CAPRI_HBM_REG_SERQ               "serq"
#define CAPRI_HBM_REG_SESQ               "sesq"
#define CAPRI_HBM_REG_ASESQ              "asesq"
#define CAPRI_HBM_REG_IPSECCB            "ipsec-cb"
#define CAPRI_HBM_REG_IPSECCB_BARCO      "ipsec_cb_barco"
#define CAPRI_HBM_REG_ARQRX              "arq-rx"
#define CAPRI_HBM_REG_ARQTX              "arq-tx"
#define CAPRI_HBM_REG_ASQ                "asq"
#define CAPRI_HBM_REG_CPU_DESCR          "cpu-descriptor"
#define CAPRI_HBM_REG_CPU_PAGE           "cpu-page"
#define CAPRI_HBM_REG_ARQRX_QIDXR        "arqrx-qidxr"
#define CAPRI_CPU_HASH_MASK              "cpu_hash_mask"
#define CAPRI_CPU_MAX_ARQID             "cpu_max_arqid"
#define CAPRI_HBM_REG_TXS_SCHEDULER      "tx-scheduler"
#define CAPRI_HBM_REG_APP_REDIR_RAWC     "app-redir-rawc"
#define CAPRI_HBM_REG_APP_REDIR_PROXYR   "app-redir-proxyr"
#define CAPRI_HBM_REG_APP_REDIR_PROXYC   "app-redir-proxyc"
#define CAPRI_HBM_REG_RDMA_EQ_INTR_TABLE   "rdma-eq-intr-table"
#define CAPRI_HBM_REG_RDMA_ATOMIC_RESOURCE_ADDR "rdma-atomic-resource-addr"
#define CAPRI_HBM_REG_P4PT_TCB_MEM       "p4pt-tcb"
#define CAPRI_HBM_REG_P4PT_TCB_ISCSI_REC_MEM "p4pt-tcb-iscsi-rec"
#define CAPRI_HBM_REG_P4PT_TCB_ISCSI_STATS_MEM "p4pt-tcb-iscsi-stats"
#define CAPRI_HBM_REG_P4PT_TCB_ISCSI_READ_LATENCY_DISTRIBUTION_MEM "p4pt-tcb-iscsi-read-latency-distribution"
#define CAPRI_HBM_REG_P4PT_TCB_ISCSI_WRITE_LATENCY_DISTRIBUTION_MEM "p4pt-tcb-iscsi-write-latency-distribution"
#define CAPRI_HBM_REG_RSS_INDIR_TABLE    "rss_indir_table"
#define CAPRI_HBM_REG_QOS_HBM_FIFO       "qos-hbm-fifo"
#define CAPRI_NUM_SEMAPHORES            512

#define PAGE_SCRATCH_SIZE               512
#define PAGE_SCRATCH_SIZE_BYTES         8

#define    JUMBO_FRAME_SIZE             9216
#define    ETH_FRAME_SIZE               1536

#define RNMDR_TABLE_BASE        "hbm_rnmdr_table_base"
#define RNMPR_TABLE_BASE        "hbm_rnmpr_table_base"
#define RNMPR_SMALL_TABLE_BASE  "hbm_rnmpr_small_table_base"
#define TNMDR_TABLE_BASE        "hbm_tnmdr_table_base"
#define TNMPR_TABLE_BASE        "hbm_tnmpr_table_base"
#define TNMPR_SMALL_TABLE_BASE  "hbm_tnmpr_small_table_base"
#define BRQ_BASE                "hbm_brq_base"
#define TCP_RX_STATS_TABLE_BASE "hbm_tcp_rx_stats_table_base"
#define RNMDR_GC_TABLE_BASE     "hbm_rnmdr_gc_table_base"
#define TNMDR_GC_TABLE_BASE     "hbm_tnmdr_gc_table_base"

#define RNMDR_TLS_GC_TABLE_BASE "hbm_rnmdr_tls_gc_table_base"

#define IPSEC_CB_BASE "ipsec_cb_base"
#define IPSEC_PAD_BYTES_HBM_TABLE_BASE "ipsec_pad_table_base"
#define IPSEC_IP_HDR_BASE "ipsec_ip_hdr_base"

#define TLS_PROXY_PAD_BYTES_HBM_TABLE_BASE "tls_pad_table_base"
#define TLS_PROXY_BARCO_GCM0_PI_HBM_TABLE_BASE "tls_barco_gcm0_pi_table_base"
#define TLS_PROXY_BARCO_GCM1_PI_HBM_TABLE_BASE "tls_barco_gcm1_pi_table_base"

#define ARQRX_BASE              "hbm_arqrx_base"
#define ARQTX_BASE              "hbm_arqtx_base"
#define ARQRX_QIDXR_BASE        "hbm_arqrx_qidxr_base"

#define P4_FLOW_HASH_BASE               "p4_flow_hash_base"
#define P4_FLOW_HASH_OVERFLOW_BASE      "p4_flow_hash_overflow_base"
#define P4_FLOW_INFO_BASE               "p4_flow_info_base"
#define P4_SESSION_STATE_BASE           "p4_session_state_base"
#define P4_FLOW_STATS_BASE              "p4_flow_stats_base"
#define P4_FLOW_ATOMIC_STATS_BASE       "p4_flow_atomic_stats_base"
#define NUM_CLOCK_TICKS_PER_CNP         "rdma_num_clock_ticks_per_cnp"
#define NUM_CLOCK_TICKS_PER_US          "rdma_num_clock_ticks_per_us"
#define RDMA_EQ_INTR_TABLE_BASE         "hbm_rdma_eq_intr_table_base"

#define RDMA_ATOMIC_RESOURCE_ADDR   "rdma_atomic_resource_addr"
#define RDMA_PCIE_ATOMIC_BASE_ADDR  "rdma_pcie_atomic_base_addr"

#define P4PT_TCB_BASE 					"p4pt_tcb_base"
#define P4PT_TCB_ISCSI_REC_BASE 			"p4pt_tcb_iscsi_rec_base"
#define P4PT_TCB_ISCSI_STATS_BASE 			"p4pt_tcb_iscsi_stats_base"
#define P4PT_TCB_ISCSI_READ_LATENCY_DISTRIBUTION_BASE 	"p4pt_tcb_iscsi_read_latency_distribution_base"
#define P4PT_TCB_ISCSI_WRITE_LATENCY_DISTRIBUTION_BASE 	"p4pt_tcb_iscsi_write_latency_distribution_base"

typedef struct capri_descr_s {
        uint64_t        scratch[8];
        uint64_t        A0;
        uint32_t        O0;
        uint32_t        L0;
        uint64_t        A1;
        uint32_t        O1;
        uint32_t        L1;
        uint64_t        A2;
        uint32_t        O2;
        uint32_t        L2;
        uint64_t        next_addr;
        uint64_t        reserved;
} capri_descr_t;

typedef struct capri_big_page_s {
        uint64_t        scratch[PAGE_SCRATCH_SIZE_BYTES];
        char            data[JUMBO_FRAME_SIZE];
} capri_big_page_t;

typedef struct capri_small_page_s {
        uint64_t        scratch[PAGE_SCRATCH_SIZE_BYTES];
        char            data[ETH_FRAME_SIZE];
} capri_small_page_t;

extern hal_ret_t
capri_hbm_cache_init(hal::hal_cfg_t *hal_cfg);

extern hal_ret_t
capri_hbm_cache_regions_init();
#endif    // __CAPRI_HPP__
