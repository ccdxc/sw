

#include "capri-macros.h"

struct phv2pkt {
    rsvd: 41;
    end3: 10;
    start3: 10;
    end2: 10;
    start2: 10;
    end1: 10;
    start1: 10;
    end: 10;
    start: 10;
    cmd_size: 2;
    pkt_eop: 1;
    cmd_eop: 1;
    cmd_type: 3;
};

struct phv2mem {
    rsvd: 33;
    override_lif: 11;
    addr: 52;
    barrier: 1;
    round: 1;
    pcie_msg: 1;
    use_override_lif: 1;
    end: 10;
    start: 10;
    wr_fence_fence: 1;
    wr_fence: 1;
    cache: 1;
    host_addr: 1;
    cmd_eop: 1;
    cmd_type: 3;
};

struct pkt2mem {
    rsvd: 42;
    size: 14;
    rsvd1: 1;
    override_lif: 11;
    addr: 52;
    use_override_lif: 1;
    cache: 1;
    host_addr: 1;
    round: 1;
    cmd_eop: 1;
    cmd_type: 3;
};

struct mem2pkt {
    rsvd: 42;
    size: 14;
    rsvd1: 1;
    override_lif: 11;
    addr: 52;
    use_override_lif: 1;
    cache: 1;
    host_addr: 1;
    pkt_eop: 1;
    cmd_eop: 1;
    cmd_type: 3;
};

#define CAPRI_RAW_TABLE_SIZE_MPU_ONLY      (7)

#define NUM_DMA_CMDS_PER_FLIT               4
#define LOG_NUM_DMA_CMDS_PER_FLIT           2

#define BITS_PER_BYTE                       8
#define LOG_BITS_PER_BYTE                   3

#define NUM_BITS_PER_FLIT                   512
#define LOG_NUM_BITS_PER_FLIT               9

#define DMA_CMD_SIZE                        16
#define DMA_CMD_SIZE_BITS                   (DMA_CMD_SIZE * BITS_PER_BYTE) 

#define LOG_DMA_CMD_SIZE                    4
#define LOG_DMA_CMD_SIZE_BITS               (LOG_DMA_CMD_SIZE + LOG_BITS_PER_BYTE)

#define SIZEOF_FIELD_RANGE(_s, _f1, _fn) \
    (offsetof(_s, _f1) + sizeof(_s._f1) - offsetof(_s, _fn))


#define DMA_CMD_PTR(_r, _flit, _index, _r_tmp) \
    sll         _r, _flit, LOG_NUM_BITS_PER_FLIT; \
    addi        _r_tmp, _index, 1; \
    sll         _r_tmp, _r_tmp, LOG_DMA_CMD_SIZE_BITS; \
    sub         _r, _r, _r_tmp;

#define DMA_CMD_NEXT(_r_flit, _r_index, _c) \
    mincr       _r_index, LOG_NUM_DMA_CMDS_PER_FLIT, 1; \
    seq         _c, _r_index, 0; \
    add._c      _r_flit, _r_flit, 1;

/**
 * PHV2PKT
 */

#define DMA_PHV2PKT_1(_r, _s0, _e0, _r_tmp) \
    phvwrp      _r, offsetof(struct phv2pkt, cmd_type), sizeof(struct phv2pkt.cmd_type), CAPRI_DMA_COMMAND_PHV_TO_PKT; \
    add         _r_tmp, _s0, _e0, sizeof(struct phv2pkt.start); \
    phvwrp      _r, offsetof(struct phv2pkt, start), SIZEOF_FIELD_RANGE(struct phv2pkt, end, start), _r_tmp;

//  phvwrp      _r, offsetof(struct phv2pkt, cmd_type), sizeof(struct phv2pkt.cmd_type), CAPRI_DMA_COMMAND_PHV_TO_PKT; \
//  phvwrp      _r, offsetof(struct phv2pkt, cmd_size), sizeof(struct phv2pkt.cmd_size), 1; \

#define DMA_PHV2PKT_2(_r, _s0, _e0, _s1, _e1, _r_tmp) \
    phvwrp      _r, offsetof(struct phv2pkt, cmd_type), SIZEOF_FIELD_RANGE(struct phv2pkt, cmd_size, cmd_type), (1 << offsetof(struct phv2pkt, cmd_size)) | (CAPRI_DMA_COMMAND_PHV_TO_PKT << offsetof(struct phv2pkt, cmd_type)); \
    add         _r_tmp, _s0, _e0, sizeof(struct phv2pkt.start); \
    phvwrp      _r, offsetof(struct phv2pkt, start), SIZEOF_FIELD_RANGE(struct phv2pkt, end, start), _r_tmp; \
    add         _r_tmp, _s1, _e1, sizeof(struct phv2pkt.start1); \
    phvwrp      _r, offsetof(struct phv2pkt, start1), SIZEOF_FIELD_RANGE(struct phv2pkt, end1, start1), _r_tmp;

//  phvwrp      _r, offsetof(struct phv2pkt, cmd_type), SIZEOF_FIELD_RANGE(struct phv2pkt, cmd_type, cmd_size), (2 << offsetof(struct phv2pkt, cmd_size)) | (CAPRI_DMA_COMMAND_PHV_TO_PKT << offsetof(struct phv2pkt, cmd_type)); \

#define DMA_PHV2PKT_3(_r, _s0, _e0, _s1, _e1, _s2, _e2, _r_tmp) \
    phvwrp      _r, offsetof(struct phv2pkt, cmd_type), sizeof(struct phv2pkt.cmd_type), CAPRI_DMA_COMMAND_PHV_TO_PKT; \
    phvwrp      _r, offsetof(struct phv2pkt, cmd_size), sizeof(struct phv2pkt.cmd_size), 2; \
    add         _r_tmp, _s0, _e0, sizeof(struct phv2pkt.start); \
    phvwrp      _r, offsetof(struct phv2pkt, start), SIZEOF_FIELD_RANGE(struct phv2pkt, end, start), _r_tmp; \
    add         _r_tmp, _s1, _e1, sizeof(struct phv2pkt.start1); \
    phvwrp      _r, offsetof(struct phv2pkt, start1), SIZEOF_FIELD_RANGE(struct phv2pkt, end1, start1), _r_tmp; \
    add         _r_tmp, _s2, _e2, sizeof(struct phv2pkt.start2); \
    phvwrp      _r, offsetof(struct phv2pkt, start2), SIZEOF_FIELD_RANGE(struct phv2pkt, end2, start2), _r_tmp;

//  phvwrp      _r, offsetof(struct phv2pkt, cmd_type), SIZEOF_FIELD_RANGE(struct phv2pkt, cmd_size, cmd_type), (3 << offsetof(struct phv2pkt, cmd_size)) | (CAPRI_DMA_COMMAND_PHV_TO_PKT << offsetof(struct phv2pkt, cmd_type)); \

#define DMA_PHV2PKT_4(_r, _s0, _e0, _s1, _e1, _s2, _e2, _s3, _e3, _r_tmp) \
    phvwrp      _r, offsetof(struct phv2pkt, cmd_type), sizeof(struct phv2pkt.cmd_type), CAPRI_DMA_COMMAND_PHV_TO_PKT; \
    phvwrp      _r, offsetof(struct phv2pkt, cmd_size), sizeof(struct phv2pkt.cmd_size), 3; \
    add         _r_tmp, _s0, _e0, sizeof(struct phv2pkt.start); \
    phvwrp      _r, offsetof(struct phv2pkt, start), SIZEOF_FIELD_RANGE(struct phv2pkt, end, start), _r_tmp; \
    add         _r_tmp, _s1, _e1, sizeof(struct phv2pkt.start1); \
    phvwrp      _r, offsetof(struct phv2pkt, start1), SIZEOF_FIELD_RANGE(struct phv2pkt, end1, start1), _r_tmp; \
    add         _r_tmp, _s2, _e2, sizeof(struct phv2pkt.start2); \
    phvwrp      _r, offsetof(struct phv2pkt, start2), SIZEOF_FIELD_RANGE(struct phv2pkt, end2, start2), _r_tmp; \
    add         _r_tmp, _s3, _e3, sizeof(struct phv2pkt.start3); \
    phvwrp      _r, offsetof(struct phv2pkt, start3), SIZEOF_FIELD_RANGE(struct phv2pkt, end3, start3), _r_tmp;

/**
 * PHV2MEM
 */

#define DMA_HBM_PHV2MEM(_r, _c, _addr, _s, _e, _r_tmp) \
    phvwrp      _r, offsetof(struct phv2mem, cmd_type), sizeof(struct phv2mem.cmd_type), CAPRI_DMA_COMMAND_PHV_TO_MEM; \
    phvwrp      _r, offsetof(struct phv2mem, addr), sizeof(struct phv2mem.addr), _addr; \
    add         _r_tmp, _s, _e, sizeof(struct phv2mem.start); \
    phvwrp      _r, offsetof(struct phv2mem, start), SIZEOF_FIELD_RANGE(struct phv2mem, end, start), _r_tmp; \
    phvwrp._c   _r, offsetof(struct phv2mem, cmd_eop), sizeof(struct phv2mem.cmd_eop), 1;

#define DMA_HOST_PHV2MEM(_r, _c, _addr, _s, _e, _r_tmp) \
    phvwrp      _r, offsetof(struct phv2mem, cmd_type), SIZEOF_FIELD_RANGE(struct phv2mem, host_addr, cmd_type), (1 << offsetof(struct phv2mem, host_addr)) | (CAPRI_DMA_COMMAND_PHV_TO_MEM << offsetof(struct phv2mem, cmd_type)); \
    phvwrp      _r, offsetof(struct phv2mem, addr), sizeof(struct phv2mem.addr), _addr; \
    add         _r_tmp, _s, _e, sizeof(struct phv2mem.start); \
    phvwrp      _r, offsetof(struct phv2mem, start), SIZEOF_FIELD_RANGE(struct phv2mem, end, start), _r_tmp; \
    phvwrp._c   _r, offsetof(struct phv2mem, cmd_eop), sizeof(struct phv2mem.cmd_eop), 1;

/**
 * PKT2MEM
 */

#define DMA_HBM_PKT2MEM(_r, _addr, _size) \
    phvwrp      _r, offsetof(struct pkt2mem, cmd_type), sizeof(struct pkt2mem.cmd_type), CAPRI_DMA_COMMAND_PKT_TO_MEM; \
    phvwrp      _r, offsetof(struct pkt2mem, addr), sizeof(struct pkt2mem.addr), _addr; \
    phvwrp      _r, offsetof(struct pkt2mem, size), sizeof(struct pkt2mem.size), _size;

#define DMA_HOST_PKT2MEM(_r, _addr, _size) \
    phvwrp      _r, offsetof(struct pkt2mem, cmd_type), SIZEOF_FIELD_RANGE(struct pkt2mem, host_addr, cmd_type), (1 << offsetof(struct pkt2mem, host_addr)) | (CAPRI_DMA_COMMAND_PKT_TO_MEM << offsetof(struct pkt2mem, cmd_type)); \
    phvwrp      _r, offsetof(struct pkt2mem, addr), sizeof(struct pkt2mem.addr), _addr; \
    phvwrp      _r, offsetof(struct pkt2mem, size), sizeof(struct pkt2mem.size), _size;

/**
 * MEM2PKT
 */

#define DMA_HBM_MEM2PKT(_r, _c, _addr, _size) \
    phvwrp      _r, offsetof(struct mem2pkt, cmd_type), sizeof(struct mem2pkt.cmd_type), CAPRI_DMA_COMMAND_MEM_TO_PKT; \
    phvwrp      _r, offsetof(struct mem2pkt, addr), sizeof(struct mem2pkt.addr), _addr; \
    phvwrp      _r, offsetof(struct mem2pkt, size), sizeof(struct mem2pkt.size), _size; \
    phvwrp._c   _r, offsetof(struct mem2pkt, pkt_eop), sizeof(struct mem2pkt.pkt_eop), 1;

#define DMA_HOST_MEM2PKT(_r, _c, _addr, _size) \
    phvwrp      _r, offsetof(struct mem2pkt, cmd_type), SIZEOF_FIELD_RANGE(struct mem2pkt, host_addr, cmd_type), (1 << offsetof(struct mem2pkt, host_addr)) | (CAPRI_DMA_COMMAND_MEM_TO_PKT << offsetof(struct mem2pkt, cmd_type)); \
    phvwrp      _r, offsetof(struct mem2pkt, addr), sizeof(struct mem2pkt.addr), _addr; \
    phvwrp      _r, offsetof(struct mem2pkt, size), sizeof(struct mem2pkt.size), _size; \
    phvwrp._c   _r, offsetof(struct mem2pkt, pkt_eop), sizeof(struct mem2pkt.pkt_eop), 1;
