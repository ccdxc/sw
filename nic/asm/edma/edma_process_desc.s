
#include "INGRESS_p.h"
#include "ingress.h"
#include "INGRESS_tx_table_s1_t0_k.h"

#include "defines.h"

struct phv_ p;
struct tx_table_s1_t0_k_ k;
struct tx_table_s1_t0_edma_process_desc_d d;

#define   _r_ptr        r5        // Current DMA byte offset in PHV
#define   _r_index      r6        // Current DMA command index in PHV

#define   HOST          c0
#define   CMDEOP        c0

%%

.param  edma_completion

.align
edma_process_desc:

  bcf             [c2 | c3 | c7], edma_process_desc_error
  nop

  // Setup DMA CMD PTR
  phvwr           p.p4_txdma_intr_dma_cmd_ptr, EDMA_DMA_CMD_START_OFFSET
  addi            _r_index, r0, (EDMA_DMA_CMD_START_FLIT << LOG_NUM_DMA_CMDS_PER_FLIT) | EDMA_DMA_CMD_START_INDEX

  add             r7, r0, d.opcode

  // Setup DMA
.brbegin
  br              r7[1:0]
  nop

  .brcase         EDMA_OPCODE_HOST_TO_LOCAL
    b               edma_process_desc_host_to_local
    nop

  .brcase         EDMA_OPCODE_LOCAL_TO_HOST
    b               edma_process_desc_local_to_host
    nop

  .brcase         EDMA_OPCODE_LOCAL_TO_LOCAL
    b               edma_process_desc_local_to_local
    nop

  .brcase         EDMA_OPCODE_HOST_TO_HOST
    b               edma_process_desc_host_to_host
    nop
.brend

edma_process_desc_host_to_local:
    DMA_CMD_PTR(_r_ptr, _r_index, r7)
    DMA_MEM2MEM_SRC_LIF(_r_ptr, !CMDEOP, HOST, d.{src_addr}.dx, d.{len}.hx, d.{src_lif}.hx)
    DMA_CMD_NEXT(_r_index)
    DMA_CMD_PTR(_r_ptr, _r_index, r7)
    DMA_MEM2MEM_DST(_r_ptr, !CMDEOP, !HOST, d.{dst_addr}.dx, d.{len}.hx)
    DMA_CMD_NEXT(_r_index)
    b           edma_process_desc_done
    nop

edma_process_desc_local_to_host:
    DMA_CMD_PTR(_r_ptr, _r_index, r7)
    DMA_MEM2MEM_SRC(_r_ptr, !CMDEOP, !HOST, d.{src_addr}.dx, d.{len}.hx)
    DMA_CMD_NEXT(_r_index)
    DMA_CMD_PTR(_r_ptr, _r_index, r7)
    DMA_MEM2MEM_DST_LIF(_r_ptr, !CMDEOP, HOST, d.{dst_addr}.dx, d.{len}.hx, d.{dst_lif}.hx)
    DMA_CMD_NEXT(_r_index)
    b           edma_process_desc_done
    nop

edma_process_desc_local_to_local:
    DMA_CMD_PTR(_r_ptr, _r_index, r7)
    DMA_MEM2MEM_SRC(_r_ptr, !CMDEOP, !HOST, d.{src_addr}.dx, d.{len}.hx)
    DMA_CMD_NEXT(_r_index)
    DMA_CMD_PTR(_r_ptr, _r_index, r7)
    DMA_MEM2MEM_DST(_r_ptr, !CMDEOP, !HOST, d.{dst_addr}.dx, d.{len}.hx)
    DMA_CMD_NEXT(_r_index)
    b           edma_process_desc_done
    nop

edma_process_desc_host_to_host:
    DMA_CMD_PTR(_r_ptr, _r_index, r7)
    DMA_MEM2MEM_SRC_LIF(_r_ptr, !CMDEOP, HOST, d.{src_addr}.dx, d.{len}.hx, d.{src_lif}.hx)
    DMA_CMD_NEXT(_r_index)
    DMA_CMD_PTR(_r_ptr, _r_index, r7)
    DMA_MEM2MEM_DST_LIF(_r_ptr, !CMDEOP, HOST, d.{dst_addr}.dx, d.{len}.hx, d.{dst_lif}.hx)
    DMA_CMD_NEXT(_r_index)
    b           edma_process_desc_done
    nop

edma_process_desc_done:
  // Save DMA command pointer
  phvwr           p.edma_global_dma_cur_index, _r_index
  
  // Generate completion in next stage
  phvwri          p.{app_header_table0_valid...app_header_table3_valid}, (1 << 3)
  phvwri          p.common_te0_phv_table_lock_en, 0
  phvwri.e        p.common_te0_phv_table_raw_table_size, CAPRI_RAW_TABLE_SIZE_MPU_ONLY
  phvwri.f        p.common_te0_phv_table_pc, edma_completion[38:6]

edma_process_desc_error:
  phvwri.e        p.{app_header_table0_valid...app_header_table3_valid}, 0
  phvwri.f        p.p4_intr_global_drop, 1
