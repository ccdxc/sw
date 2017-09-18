#include "INGRESS_p.h"
#include "ingress.h"
#include "cpu-table.h"

struct phv_ p;
struct common_p4plus_stage0_app_header_table_k k;
struct common_p4plus_stage0_app_header_table_d d;

%%
    .param          cpu_rx_read_desc_pindex_start
    .param          cpu_rx_read_page_pindex_start
    .param          cpu_rx_read_arqrx_pindex_start
    .param          ARQRX_BASE
    .param          ARQRX_QIDXR_BASE
    .align
cpu_rx_read_shared_stage0_start:
    CAPRI_CLEAR_TABLE0_VALID
    addi    r4, r0, 0x1
	
    // set the qid in the phv so that the rest of the stages can use it
    //phvwr		p.common_phv_fid, k.p4_rxdma_intr_qid
    add     r4, r0, k.{p4_rxdma_intr_qstate_addr_sbit0_ebit1...p4_rxdma_intr_qstate_addr_sbit2_ebit33}
    phvwr   p.common_phv_qstate_addr, k.{p4_rxdma_intr_qstate_addr_sbit0_ebit1...p4_rxdma_intr_qstate_addr_sbit2_ebit33}
    add     r4, r0, k.cpu_app_header_packet_len
    phvwr   p.to_s3_payload_len, k.cpu_app_header_packet_len

table_read_DESC_SEMAPHORE:
    addi    r3, r0, RNMDR_ALLOC_IDX 
    CAPRI_NEXT_TABLE_READ(1, TABLE_LOCK_DIS, 
                         cpu_rx_read_desc_pindex_start,
                         r3,
                         TABLE_SIZE_16_BITS)
table_read_PAGE_SEMAPHORE:
    addi    r3, r0, RNMPR_ALLOC_IDX
    CAPRI_NEXT_TABLE_READ(2, TABLE_LOCK_DIS, 
                         cpu_rx_read_page_pindex_start,
                         r3,
                         TABLE_SIZE_16_BITS)
table_read_ARQRX_PINDEX:
    phvwri  p.to_s3_arqrx_base, ARQRX_BASE
    CPU_ARQRX_QIDX_ADDR(0, r3, ARQRX_QIDXR_BASE)
    CAPRI_NEXT_TABLE_READ(3, TABLE_LOCK_EN,
                          cpu_rx_read_arqrx_pindex_start,
                          r3,
                          TABLE_SIZE_64_BITS)
    nop.e
    nop
