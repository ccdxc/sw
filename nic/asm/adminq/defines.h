
#define ADMINQ_DESC_SIZE                  6 /* 64B */

// DMA commands are 16 byte aligned so ptr should be a multiple of that
// Take dma_cmd0_dma_cmd_type field from struct phv_ and divide by 128 to
// derive the following values.

#define ADMINQ_DMA_CMD_PTR                22

#define LOAD_NEXT_PC(_phv, _prog_name, _n) \
  addi            r2, r0, loword(_prog_name); \
  addui           r2, r2, hiword(_prog_name); \
  srl             r1, r2, CAPRI_MPU_PC_SHIFT; \
  phvwr           _phv.common_te##_n##_phv_table_pc, r1;
