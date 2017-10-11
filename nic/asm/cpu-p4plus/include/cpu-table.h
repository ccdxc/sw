#include  "capri-macros.h"
#include "cpu-macros.h"
#include "cpu-defines.h"

#define CPU_CB_TABLE_ENTRY_SIZE         512 /* 512B */
#define CPU_CB_TABLE_ENTRY_SIZE_SHFT    9  /* 512B */
#define CPU_CB_WRITE_ARQRX_OFFSET       64
#define CPU_SCHED_RING_ASQ              0

/* debug_dol encodings */
#define CPU_DDOL_PKT_TO_ARQ             1

#define NIC_DESC_ENTRY_0_OFFSET         64         /* &((nic_desc_t *)0)->entry[0]*/
#define NIC_PAGE_HDR_SIZE               0         /* sizeof(nic_page_hdr_t) */
#define CACHE_LINE_SIZE                 0
#define NIC_PAGE_HEADROOM               (17 * CACHE_LINE_SIZE)

#define NIC_ASQ_ENTRY_SIZE              8
#define NIC_ASQ_ENTRY_SIZE_SHIFT        3          /* for 8B */

#define CPU_TO_P4PLUS_HDR_SIZE          8
#define P4PLUS_TO_P4_HDR_SIZE           10
#define VLAN_TAG_HDR_SIZE               4

#define CPU_PHV_AOL_DESC_START          CAPRI_PHV_START_OFFSET(aol_A0)
#define CPU_PHV_AOL_DESC_END            CAPRI_PHV_END_OFFSET(aol_next_pkt)

#define CPU_PHV_INTRINSIC_START         CAPRI_PHV_START_OFFSET(p4_intr_global_tm_iport)
#define CPU_PHV_INTRINSIC_END           CAPRI_PHV_END_OFFSET(p4_intr_global_tm_instance_type)

#define CPU_PHV_TXDMA_INTRINSIC_START   CAPRI_PHV_START_OFFSET(p4_txdma_intr_qid)
#define CPU_PHV_TXDMA_INTRINSIC_END     CAPRI_PHV_END_OFFSET(p4_txdma_intr_txdma_rsv)

#define CPU_PHV_VLAN_TAG_START          CAPRI_PHV_START_OFFSET(vlan_hdr_entry_etherType)
#define CPU_PHV_VLAN_TAG_END            CAPRI_PHV_END_OFFSET(vlan_hdr_entry_vid)
