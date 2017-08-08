
#define PINDEX_SIZE_SHIFT                1
#define IN_DESC_PINDEX_BASE_ADDR         0xa0a0a0a0
#define RAW_TABLE_DEFAULT_SIZE           6

// 251 + 88 bits
#define RXDMA_IPSEC_HW_SW_INTRINSIC 43

// NIC Descriptor is 1024 bits - 128 bytes
#define NIC_DESC_TABLE_SIZE_SHIFT        7
#define NIC_DESC_ENTRY_SIZE_SHIFT        7
#define RNMDR_BASE_ADDR                  0xaaaaaaaa
#define RNMPR_BASE_ADDR                  0xbbbbbbbb

#define ESP_FIXED_HDR_SIZE               8

#define INDESC_SEMAPHORE_ADDR            0xabababab
#define OUTDESC_SEMAPHORE_ADDR           0xbabababa
#define IN_PAGE_SEMAPHORE_ADDR           0xcdcdcdcd
#define OUT_PAGE_SEMAPHORE_ADDR          0xdcdcdcdc

 
#define DMA_CMD_TYPE_MEM2PKT     0
#define DMA_CMD_TYPE_PHV2PKT     1
#define DMA_CMD_TYPE_PHV2MEM     2
#define DMA_CMD_TYPE_PKT2MEM     3
#define DMA_CMD_TYPE_SKIP        4
#define DMA_CMD_TYPE_MEM2MEM     5 
#define DMA_CMD_TYPE_NOP         6 

#define DMA_CMD_TYPE_MEM2MEM_TYPE_SRC        0
#define DMA_CMD_TYPE_MEM2MEM_TYPE_DST        1
#define DMA_CMD_TYPE_MEM2MEM_TYPE_PHV2MEM    2


#define PHV_RXDMA_BARCO_AOL_OFFSET_BEGIN     448
#define PHV_RXDMA_BARCO_AOL_OFFSET_BEGIN     511
#define PHV_RXDMA_IPSEC_INT_HDR_OFFSET_BEGIN 512
#define PHV_RXDMA_IPSEC_INT_HDR_OFFSET_END   575


#define IPSEC_PAD_BYTES_HBM_TABLE_BASE    0xa0000000


