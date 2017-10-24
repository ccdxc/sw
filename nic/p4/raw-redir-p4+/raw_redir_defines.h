
#define BITS_PER_BYTE                   8
#define SIZE_IN_BITS(bytes)             ((bytes) * BITS_PER_BYTE)
#define SIZE_IN_BYTES(bits)             ((bits) / BITS_PER_BYTE)

/*
 * From capri_hbm.hpp
 */
#define JUMBO_FRAME_SIZE                9216
#define ETH_FRAME_SIZE                  1536

#define RAWR_PPAGE_SIZE                 JUMBO_FRAME_SIZE
#define RAWR_MPAGE_SIZE                 ETH_FRAME_SIZE

                                         
/*
 * Command structure for DMA_CMD_TYPE_SKIP
 * (structure currently not available in p4/common-p4+/common_headers.p4)
 */
header_type dma_cmd_skip_t {
    fields {
        rsvd                : 109;
        dma_cmd_skip_to_eop : 1;
        dma_cmd_size        : 14;
        dma_cmd_eop         : 1;
        dma_cmd_type        : 3;
    }
}
