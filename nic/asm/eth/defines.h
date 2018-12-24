
#include "nic/include/capri_common.h"
#include "nic/p4/common/defines.h"
#include "nic/asm/common-p4+/include/capri-macros.h"

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

struct mem2mem {
    rsvd: 16;
    size: 14;
    rsvd1: 1;
    override_lif: 11;
    addr: 52;
    barrier: 1;
    round: 1;
    pcie_msg: 1;
    use_override_lif: 1;
    phv_end: 10;
    phv_start: 10;
    wr_fence_fence: 1;
    wr_fence: 1;
    cache: 1;
    host_addr: 1;
    mem2mem_type: 2;
    cmd_eop: 1;
    cmd_type: 3;
};

struct skip {
    rsvd: 109;
    skip_to_eop: 1;
    size: 14;
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

// Constants
#define _C_TRUE          c0
#define _C_FALSE        !c0

/*
 * Stats
 */
#define LIF_STATS_SIZE          (4096)      // Size of per lif stats
#define LIF_STATS_RX_OFFSET     (0)         // Relative (to lif stats) offset of RX stats
#define LIF_STATS_TX_OFFSET     (2048)      // Relative (to lif stats) offset of TX stats

/*
 * DMA command macros
 */
#define DMA_CMD_PTR(_r_ptr, _r_index, _r_tmp) \
    sll         _r_ptr, _r_index[6:2], LOG_NUM_BITS_PER_FLIT; \
    add         _r_tmp, _r_index[1:0], 1; \
    sll         _r_tmp, _r_tmp, LOG_DMA_CMD_SIZE_BITS; \
    sub         _r_ptr, _r_ptr, _r_tmp;

#define DMA_CMD_NEXT(_r_index) \
    addi        _r_index, _r_index, 1

#define DMA_CMD_PREV(_r_index) \
    addi        _r_index, _r_index, -1

/**
 * SKIP
 */

#define DMA_SKIP(_r, _c, _size) \
    phvwrp      _r, offsetof(struct skip, cmd_type), sizeof(struct skip.cmd_type), CAPRI_DMA_COMMAND_SKIP; \
    phvwrp      _r, offsetof(struct skip, size), sizeof(struct skip.size), _size; \
    phvwrp._c   _r, offsetof(struct skip, cmd_eop), sizeof(struct skip.cmd_eop), 1;

#define DMA_SKIP_TO_EOP(_r, _c) \
    phvwrp      _r, offsetof(struct skip, cmd_type), sizeof(struct skip.cmd_type), CAPRI_DMA_COMMAND_SKIP; \
    phvwrp      _r, offsetof(struct skip, skip_to_eop), sizeof(struct skip.skip_to_eop), 1; \
    phvwrp._c   _r, offsetof(struct skip, cmd_eop), sizeof(struct skip.cmd_eop), 1;

/**
 * PHV2PKT
 */

#define DMA_PHV2PKT_1(_r, _s0, _e0, _r_tmp) \
    phvwrp      _r, offsetof(struct phv2pkt, cmd_type), sizeof(struct phv2pkt.cmd_type), CAPRI_DMA_COMMAND_PHV_TO_PKT; \
    add         _r_tmp, _s0, _e0, sizeof(struct phv2pkt.start); \
    phvwrp      _r, offsetof(struct phv2pkt, start), SIZEOF_FIELD_RANGE(struct phv2pkt, end, start), _r_tmp;

#define DMA_PHV2PKT_2(_r, _s0, _e0, _s1, _e1, _r_tmp) \
    phvwrp      _r, offsetof(struct phv2pkt, cmd_type), SIZEOF_FIELD_RANGE(struct phv2pkt, cmd_size, cmd_type), (1 << offsetof(struct phv2pkt, cmd_size)) | (CAPRI_DMA_COMMAND_PHV_TO_PKT << offsetof(struct phv2pkt, cmd_type)); \
    add         _r_tmp, _s0, _e0, sizeof(struct phv2pkt.start); \
    phvwrp      _r, offsetof(struct phv2pkt, start), SIZEOF_FIELD_RANGE(struct phv2pkt, end, start), _r_tmp; \
    add         _r_tmp, _s1, _e1, sizeof(struct phv2pkt.start1); \
    phvwrp      _r, offsetof(struct phv2pkt, start1), SIZEOF_FIELD_RANGE(struct phv2pkt, end1, start1), _r_tmp;

#define DMA_PHV2PKT_3(_r, _s0, _e0, _s1, _e1, _s2, _e2, _r_tmp) \
    phvwrp      _r, offsetof(struct phv2pkt, cmd_type), sizeof(struct phv2pkt.cmd_type), CAPRI_DMA_COMMAND_PHV_TO_PKT; \
    phvwrp      _r, offsetof(struct phv2pkt, cmd_size), sizeof(struct phv2pkt.cmd_size), 2; \
    add         _r_tmp, _s0, _e0, sizeof(struct phv2pkt.start); \
    phvwrp      _r, offsetof(struct phv2pkt, start), SIZEOF_FIELD_RANGE(struct phv2pkt, end, start), _r_tmp; \
    add         _r_tmp, _s1, _e1, sizeof(struct phv2pkt.start1); \
    phvwrp      _r, offsetof(struct phv2pkt, start1), SIZEOF_FIELD_RANGE(struct phv2pkt, end1, start1), _r_tmp; \
    add         _r_tmp, _s2, _e2, sizeof(struct phv2pkt.start2); \
    phvwrp      _r, offsetof(struct phv2pkt, start2), SIZEOF_FIELD_RANGE(struct phv2pkt, end2, start2), _r_tmp;

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

#define DMA_PHV2MEM(_r, _c, _host, _addr, _s, _e, _r_tmp) \
    phvwrp      _r, offsetof(struct phv2mem, cmd_type), sizeof(struct phv2mem.cmd_type), CAPRI_DMA_COMMAND_PHV_TO_MEM; \
    phvwrp      _r, offsetof(struct phv2mem, host_addr), sizeof(struct phv2mem.host_addr), _host; \
    phvwrp      _r, offsetof(struct phv2mem, addr), sizeof(struct phv2mem.addr), _addr; \
    add         _r_tmp, _s, _e, sizeof(struct phv2mem.start); \
    phvwrp      _r, offsetof(struct phv2mem, start), SIZEOF_FIELD_RANGE(struct phv2mem, end, start), _r_tmp; \
    phvwrp._c   _r, offsetof(struct phv2mem, cmd_eop), sizeof(struct phv2mem.cmd_eop), 1;

#define DMA_PHV2MEM_WF(_r, _c, _host, _addr, _s, _e, _r_tmp) \
    phvwrp      _r, offsetof(struct phv2mem, cmd_type), sizeof(struct phv2mem.cmd_type), CAPRI_DMA_COMMAND_PHV_TO_MEM; \
    phvwrp      _r, offsetof(struct phv2mem, host_addr), sizeof(struct phv2mem.host_addr), _host; \
    phvwrp      _r, offsetof(struct phv2mem, addr), sizeof(struct phv2mem.addr), _addr; \
    add         _r_tmp, _s, _e, sizeof(struct phv2mem.start); \
    phvwrp      _r, offsetof(struct phv2mem, start), SIZEOF_FIELD_RANGE(struct phv2mem, end, start), _r_tmp; \
    phvwrp      _r, offsetof(struct phv2mem, wr_fence), sizeof(struct phv2mem.wr_fence), 1; \
    phvwrp._c   _r, offsetof(struct phv2mem, cmd_eop), sizeof(struct phv2mem.cmd_eop), 1;

#define DMA_PHV2MEM_LIF(_r, _c, _host, _addr, _s, _e, _lif, _r_tmp) \
    phvwrp      _r, offsetof(struct phv2mem, cmd_type), sizeof(struct phv2mem.cmd_type), CAPRI_DMA_COMMAND_PHV_TO_MEM; \
    phvwrp      _r, offsetof(struct phv2mem, host_addr), sizeof(struct phv2mem.host_addr), _host; \
    phvwrp      _r, offsetof(struct phv2mem, addr), sizeof(struct phv2mem.addr), _addr; \
    phvwrp      _r, offsetof(struct phv2mem, use_override_lif), sizeof(struct phv2mem.use_override_lif), 1; \
    phvwrp      _r, offsetof(struct phv2mem, override_lif), sizeof(struct phv2mem.override_lif), _lif; \
    add         _r_tmp, _s, _e, sizeof(struct phv2mem.start); \
    phvwrp      _r, offsetof(struct phv2mem, start), SIZEOF_FIELD_RANGE(struct phv2mem, end, start), _r_tmp; \
    phvwrp._c   _r, offsetof(struct phv2mem, cmd_eop), sizeof(struct phv2mem.cmd_eop), 1;

//
#define DMA_HBM_PHV2MEM(_r, _c, _addr, _s, _e, _r_tmp) \
    phvwrp      _r, offsetof(struct phv2mem, cmd_type), sizeof(struct phv2mem.cmd_type), CAPRI_DMA_COMMAND_PHV_TO_MEM; \
    phvwrp      _r, offsetof(struct phv2mem, addr), sizeof(struct phv2mem.addr), _addr; \
    add         _r_tmp, _s, _e, sizeof(struct phv2mem.start); \
    phvwrp      _r, offsetof(struct phv2mem, start), SIZEOF_FIELD_RANGE(struct phv2mem, end, start), _r_tmp; \
    phvwrp._c   _r, offsetof(struct phv2mem, cmd_eop), sizeof(struct phv2mem.cmd_eop), 1;

#define DMA_HBM_PHV2MEM_WF(_r, _c, _addr, _s, _e, _r_tmp) \
    phvwrp      _r, offsetof(struct phv2mem, cmd_type), sizeof(struct phv2mem.cmd_type), CAPRI_DMA_COMMAND_PHV_TO_MEM; \
    phvwrp      _r, offsetof(struct phv2mem, addr), sizeof(struct phv2mem.addr), _addr; \
    add         _r_tmp, _s, _e, sizeof(struct phv2mem.start); \
    phvwrp      _r, offsetof(struct phv2mem, start), SIZEOF_FIELD_RANGE(struct phv2mem, end, start), _r_tmp; \
    phvwrp      _r, offsetof(struct phv2mem, wr_fence), sizeof(struct phv2mem.wr_fence), 1; \
    phvwrp._c   _r, offsetof(struct phv2mem, cmd_eop), sizeof(struct phv2mem.cmd_eop), 1;

#define DMA_HBM_PHV2MEM_LIF(_r, _c, _addr, _s, _e, _lif, _r_tmp) \
    phvwrp      _r, offsetof(struct phv2mem, cmd_type), sizeof(struct phv2mem.cmd_type), CAPRI_DMA_COMMAND_PHV_TO_MEM; \
    phvwrp      _r, offsetof(struct phv2mem, addr), sizeof(struct phv2mem.addr), _addr; \
    phvwrp      _r, offsetof(struct phv2mem, use_override_lif), sizeof(struct phv2mem.use_override_lif), 1; \
    phvwrp      _r, offsetof(struct phv2mem, override_lif), sizeof(struct phv2mem.override_lif), _lif; \
    add         _r_tmp, _s, _e, sizeof(struct phv2mem.start); \
    phvwrp      _r, offsetof(struct phv2mem, start), SIZEOF_FIELD_RANGE(struct phv2mem, end, start), _r_tmp; \
    phvwrp._c   _r, offsetof(struct phv2mem, cmd_eop), sizeof(struct phv2mem.cmd_eop), 1;

//
#define DMA_HOST_PHV2MEM(_r, _c, _addr, _s, _e, _r_tmp) \
    phvwrp      _r, offsetof(struct phv2mem, cmd_type), SIZEOF_FIELD_RANGE(struct phv2mem, host_addr, cmd_type), (1 << offsetof(struct phv2mem, host_addr)) | (CAPRI_DMA_COMMAND_PHV_TO_MEM << offsetof(struct phv2mem, cmd_type)); \
    phvwrp      _r, offsetof(struct phv2mem, addr), sizeof(struct phv2mem.addr), _addr; \
    add         _r_tmp, _s, _e, sizeof(struct phv2mem.start); \
    phvwrp      _r, offsetof(struct phv2mem, start), SIZEOF_FIELD_RANGE(struct phv2mem, end, start), _r_tmp; \
    phvwrp._c   _r, offsetof(struct phv2mem, cmd_eop), sizeof(struct phv2mem.cmd_eop), 1;

#define DMA_HOST_PHV2MEM_WF(_r, _c, _addr, _s, _e, _r_tmp) \
    phvwrp      _r, offsetof(struct phv2mem, cmd_type), SIZEOF_FIELD_RANGE(struct phv2mem, host_addr, cmd_type), (1 << offsetof(struct phv2mem, host_addr)) | (CAPRI_DMA_COMMAND_PHV_TO_MEM << offsetof(struct phv2mem, cmd_type)); \
    phvwrp      _r, offsetof(struct phv2mem, addr), sizeof(struct phv2mem.addr), _addr; \
    add         _r_tmp, _s, _e, sizeof(struct phv2mem.start); \
    phvwrp      _r, offsetof(struct phv2mem, start), SIZEOF_FIELD_RANGE(struct phv2mem, end, start), _r_tmp; \
    phvwrp      _r, offsetof(struct phv2mem, wr_fence), sizeof(struct phv2mem.wr_fence), 1; \
    phvwrp._c   _r, offsetof(struct phv2mem, cmd_eop), sizeof(struct phv2mem.cmd_eop), 1;

#define DMA_HOST_PHV2MEM_LIF(_r, _c, _addr, _s, _e, _lif, _r_tmp) \
    phvwrp      _r, offsetof(struct phv2mem, cmd_type), SIZEOF_FIELD_RANGE(struct phv2mem, host_addr, cmd_type), (1 << offsetof(struct phv2mem, host_addr)) | (CAPRI_DMA_COMMAND_PHV_TO_MEM << offsetof(struct phv2mem, cmd_type)); \
    phvwrp      _r, offsetof(struct phv2mem, addr), sizeof(struct phv2mem.addr), _addr; \
    phvwrp      _r, offsetof(struct phv2mem, use_override_lif), sizeof(struct phv2mem.use_override_lif), 1; \
    phvwrp      _r, offsetof(struct phv2mem, override_lif), sizeof(struct phv2mem.override_lif), _lif; \
    add         _r_tmp, _s, _e, sizeof(struct phv2mem.start); \
    phvwrp      _r, offsetof(struct phv2mem, start), SIZEOF_FIELD_RANGE(struct phv2mem, end, start), _r_tmp; \
    phvwrp._c   _r, offsetof(struct phv2mem, cmd_eop), sizeof(struct phv2mem.cmd_eop), 1;

/**
 * PKT2MEM
 */

#define DMA_PKT2MEM(_r, _c, _host, _addr, _size) \
    phvwrp      _r, offsetof(struct pkt2mem, cmd_type), sizeof(struct pkt2mem.cmd_type), CAPRI_DMA_COMMAND_PKT_TO_MEM; \
    phvwrp      _r, offsetof(struct pkt2mem, host_addr), sizeof(struct pkt2mem.host_addr), _host; \
    phvwrp      _r, offsetof(struct pkt2mem, addr), sizeof(struct pkt2mem.addr), _addr; \
    phvwrp      _r, offsetof(struct pkt2mem, size), sizeof(struct pkt2mem.size), _size; \
    phvwrp._c   _r, offsetof(struct pkt2mem, cmd_eop), sizeof(struct pkt2mem.cmd_eop), 1;

#define DMA_HBM_PKT2MEM(_r, _c, _addr, _size) \
    phvwrp      _r, offsetof(struct pkt2mem, cmd_type), sizeof(struct pkt2mem.cmd_type), CAPRI_DMA_COMMAND_PKT_TO_MEM; \
    phvwrp      _r, offsetof(struct pkt2mem, addr), sizeof(struct pkt2mem.addr), _addr; \
    phvwrp      _r, offsetof(struct pkt2mem, size), sizeof(struct pkt2mem.size), _size; \
    phvwrp._c   _r, offsetof(struct pkt2mem, cmd_eop), sizeof(struct pkt2mem.cmd_eop), 1;

#define DMA_HOST_PKT2MEM(_r, _c, _addr, _size) \
    phvwrp      _r, offsetof(struct pkt2mem, cmd_type), SIZEOF_FIELD_RANGE(struct pkt2mem, host_addr, cmd_type), (1 << offsetof(struct pkt2mem, host_addr)) | (CAPRI_DMA_COMMAND_PKT_TO_MEM << offsetof(struct pkt2mem, cmd_type)); \
    phvwrp      _r, offsetof(struct pkt2mem, addr), sizeof(struct pkt2mem.addr), _addr; \
    phvwrp      _r, offsetof(struct pkt2mem, size), sizeof(struct pkt2mem.size), _size; \
    phvwrp._c   _r, offsetof(struct pkt2mem, cmd_eop), sizeof(struct pkt2mem.cmd_eop), 1;

/**
 * MEM2PKT
 */

#define DMA_MEM2PKT(_r, _c, _host, _addr, _size) \
    phvwrp      _r, offsetof(struct mem2pkt, cmd_type), sizeof(struct mem2pkt.cmd_type), CAPRI_DMA_COMMAND_MEM_TO_PKT; \
    phvwrp      _r, offsetof(struct mem2pkt, host_addr), sizeof(struct mem2pkt.host_addr), _host; \
    phvwrp      _r, offsetof(struct mem2pkt, addr), sizeof(struct mem2pkt.addr), _addr; \
    phvwrp      _r, offsetof(struct mem2pkt, size), sizeof(struct mem2pkt.size), _size; \
    phvwrp._c   _r, offsetof(struct mem2pkt, pkt_eop), sizeof(struct mem2pkt.pkt_eop), 1;

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

/**
 * MEM2MEM
 */
#define CAPRI_DMA_COMMAND_MEM_TO_MEM_SRC        0
#define CAPRI_DMA_COMMAND_MEM_TO_MEM_DST        1
#define CAPRI_DMA_COMMAND_MEM_TO_MEM_PHV2MEM    2     // RXDMA only


#define DMA_MEM2MEM_SRC(_r, _c, _h, _addr, _size) \
    phvwrp      _r, offsetof(struct mem2mem, cmd_type), sizeof(struct mem2mem.cmd_type), CAPRI_DMA_COMMAND_MEM_TO_MEM; \
    phvwrp      _r, offsetof(struct mem2mem, mem2mem_type), sizeof(struct mem2mem.mem2mem_type), CAPRI_DMA_COMMAND_MEM_TO_MEM_SRC; \
    phvwrp._h   _r, offsetof(struct mem2mem, host_addr), sizeof(struct mem2mem.host_addr), 1; \
    phvwrp      _r, offsetof(struct mem2mem, addr), sizeof(struct mem2mem.addr), _addr; \
    phvwrp      _r, offsetof(struct mem2mem, size), sizeof(struct mem2mem.size), _size;

#define DMA_MEM2MEM_SRC_LIF(_r, _c, _h, _addr, _size, _lif) \
    phvwrp      _r, offsetof(struct mem2mem, cmd_type), sizeof(struct mem2mem.cmd_type), CAPRI_DMA_COMMAND_MEM_TO_MEM; \
    phvwrp      _r, offsetof(struct mem2mem, mem2mem_type), sizeof(struct mem2mem.mem2mem_type), CAPRI_DMA_COMMAND_MEM_TO_MEM_SRC; \
    phvwrp      _r, offsetof(struct mem2mem, use_override_lif), sizeof(struct mem2mem.use_override_lif), 1; \
    phvwrp      _r, offsetof(struct mem2mem, override_lif), sizeof(struct mem2mem.override_lif), _lif; \
    phvwrp._h   _r, offsetof(struct mem2mem, host_addr), sizeof(struct mem2mem.host_addr), 1; \
    phvwrp      _r, offsetof(struct mem2mem, addr), sizeof(struct mem2mem.addr), _addr; \
    phvwrp      _r, offsetof(struct mem2mem, size), sizeof(struct mem2mem.size), _size;

#define DMA_MEM2MEM_DST(_r, _c, _h, _addr, _size) \
    phvwrp      _r, offsetof(struct mem2mem, cmd_type), sizeof(struct mem2mem.cmd_type), CAPRI_DMA_COMMAND_MEM_TO_MEM; \
    phvwrp      _r, offsetof(struct mem2mem, mem2mem_type), sizeof(struct mem2mem.mem2mem_type), CAPRI_DMA_COMMAND_MEM_TO_MEM_DST; \
    phvwrp._h   _r, offsetof(struct mem2mem, host_addr), sizeof(struct mem2mem.host_addr), 1; \
    phvwrp      _r, offsetof(struct mem2mem, addr), sizeof(struct mem2mem.addr), _addr; \
    phvwrp      _r, offsetof(struct mem2mem, size), sizeof(struct mem2mem.size), _size; \
    phvwrp._c   _r, offsetof(struct mem2mem, cmd_eop), sizeof(struct mem2mem.cmd_eop), 1;

#define DMA_MEM2MEM_DST_LIF(_r, _c, _h, _addr, _size, _lif) \
    phvwrp      _r, offsetof(struct mem2mem, cmd_type), sizeof(struct mem2mem.cmd_type), CAPRI_DMA_COMMAND_MEM_TO_MEM; \
    phvwrp      _r, offsetof(struct mem2mem, mem2mem_type), sizeof(struct mem2mem.mem2mem_type), CAPRI_DMA_COMMAND_MEM_TO_MEM_DST; \
    phvwrp      _r, offsetof(struct mem2mem, use_override_lif), sizeof(struct mem2mem.use_override_lif), 1; \
    phvwrp      _r, offsetof(struct mem2mem, override_lif), sizeof(struct mem2mem.override_lif), _lif; \
    phvwrp._h   _r, offsetof(struct mem2mem, host_addr), sizeof(struct mem2mem.host_addr), 1; \
    phvwrp      _r, offsetof(struct mem2mem, addr), sizeof(struct mem2mem.addr), _addr; \
    phvwrp      _r, offsetof(struct mem2mem, size), sizeof(struct mem2mem.size), _size; \
    phvwrp._c   _r, offsetof(struct mem2mem, cmd_eop), sizeof(struct mem2mem.cmd_eop), 1;

/**
 * Atomic Counters
 */

#define ATOMIC_INC_VAL_1(_r_b, _r_o, _r_a, _r_v, _v0)   \
    add         _r_a, _r_b, _r_o[26:0]; \
    or          _r_v, r0, _v0; \
    or          _r_v, _r_v, _r_o[31:27], 58; \
    memwr.dx    _r_a, _r_v

#define ATOMIC_INC_VAL_2(_r_b, _r_o, _r_a, _r_v, _v0, _v1)   \
    add         _r_a, _r_b, _r_o[26:0]; \
    or          _r_v, r0, _v0; \
    or          _r_v, _r_v, _v1, 32; \
    or          _r_v, _r_v, 1, 56; \
    or          _r_v, _r_v, _r_o[31:27], 58; \
    memwr.dx    _r_a, _r_v

#define ATOMIC_INC_VAL_3(_r_b, _r_o, _r_a, _r_v, _v0, _v1, _v2)   \
    add         _r_a, _r_b, _r_o[26:0]; \
    or          _r_v, r0, _v0; \
    or          _r_v, _r_v, _v1, 16; \
    or          _r_v, _r_v, _v2, 32; \
    or          _r_v, _r_v, 2, 56; \
    or          _r_v, _r_v, _r_o[31:27], 58; \
    memwr.dx    _r_a, _r_v

#define ATOMIC_INC_VAL_4(_r_b, _r_o, _r_a, _r_v, _v0, _v1, _v2, _v3)   \
    add         _r_a, _r_b, _r_o[26:0]; \
    or          _r_v, r0, _v0; \
    or          _r_v, _r_v, _v1, 16; \
    or          _r_v, _r_v, _v2, 32; \
    or          _r_v, _r_v, _v3, 48; \
    or          _r_v, _r_v, 2, 56; \
    or          _r_v, _r_v, _r_o[31:27], 58; \
    memwr.dx    _r_a, _r_v

#define ATOMIC_INC_VAL_5(_r_b, _r_o, _r_a, _r_v, _v0, _v1, _v2, _v3, _v4)   \
    add         _r_a, _r_b, _r_o[26:0]; \
    or          _r_v, r0, _v0; \
    or          _r_v, _r_v, _v1, 8; \
    or          _r_v, _r_v, _v2, 16; \
    or          _r_v, _r_v, _v3, 24; \
    or          _r_v, _r_v, _v4, 32; \
    or          _r_v, _r_v, 3, 56; \
    or          _r_v, _r_v, _r_o[31:27], 58; \
    memwr.dx    _r_a, _r_v

#define ATOMIC_INC_VAL_6(_r_b, _r_o, _r_a, _r_v, _v0, _v1, _v2, _v3, _v4, _v5)   \
    add         _r_a, _r_b, _r_o[26:0]; \
    or          _r_v, r0, _v0; \
    or          _r_v, _r_v, _v1, 8; \
    or          _r_v, _r_v, _v2, 16; \
    or          _r_v, _r_v, _v3, 24; \
    or          _r_v, _r_v, _v4, 32; \
    or          _r_v, _r_v, _v5, 40; \
    or          _r_v, _r_v, 3, 56; \
    or          _r_v, _r_v, _r_o[31:27], 58; \
    memwr.dx    _r_a, _r_v

#define ATOMIC_INC_VAL_7(_r_b, _r_o, _r_a, _r_v, _v0, _v1, _v2, _v3, _v4, _v5, _v6)   \
    add         _r_a, _r_b, _r_o[26:0]; \
    or          _r_v, r0, _v0; \
    or          _r_v, _r_v, _v1, 8; \
    or          _r_v, _r_v, _v2, 16; \
    or          _r_v, _r_v, _v3, 24; \
    or          _r_v, _r_v, _v4, 32; \
    or          _r_v, _r_v, _v5, 40; \
    or          _r_v, _r_v, _v6, 48; \
    or          _r_v, _r_v, 3, 56; \
    or          _r_v, _r_v, _r_o[31:27], 58; \
    memwr.dx    _r_a, _r_v
