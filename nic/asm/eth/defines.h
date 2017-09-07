
#define LG2_RX_DESC_SIZE		4	/* 16B (log_2 64) */
#define LG2_TX_DESC_SIZE		4	/* 16B (log_2 64) */

// DMA commands are 16 byte aligned so ptr should be a multiple of that
// Take dma_cmd0_dma_cmd_type field from struct phv_ and divide by 128 to
// derive the following values.

#define TX_DMA_CMD_PTR			24
#define RX_DMA_CMD_PTR			24


#define CAPRI_DMA_COMMAND_MEM_TO_PKT      1
#define CAPRI_DMA_COMMAND_PHV_TO_PKT      2
#define CAPRI_DMA_COMMAND_PHV_TO_MEM      3
#define CAPRI_DMA_COMMAND_PKT_TO_MEM      4
#define CAPRI_DMA_COMMAND_SKIP            5
#define CAPRI_DMA_COMMAND_MEM_TO_MEM      6
#define CAPRI_DMA_COMMAND_NOP             7

#define CAPRI_MPU_PC_SHIFT                6