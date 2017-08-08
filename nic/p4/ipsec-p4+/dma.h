//Need to get right values from ASIC
#define DMA_CMD_TYPE_PKT_2_MEM 1

typedef struct cmd_mem2pkt_sv_s {
    u64 size : 14;
    u64 addr : 52;
    u64 cache : 1;
    u64 host_addr : 1;
    u64 pkteop : 1;
    u64 cmdeop : 1;
    u64 cmdtype : 3;
} PACKED cmd_mem2pkt_sv;

typedef struct cmd_phv2pkt_sv_s {
    u64 phv_end : 10;
    u64 phv_start : 10;
    u64 pkteop : 1;
    u64 cmdeop : 1;
    u64 cmdtype : 3;
} PACKED cmd_phv2pkt_sv;
   
typedef struct cmd_phv2mem_sv_s {
    u64 phv_end : 10;
    u64 phv_start : 10;
    u64 round : 1;
    u64 wr_fence : 1;
    u64 addr : 52;
    u64 cache : 1;
    u64 host_addr : 1;
    u64 cmdeop : 1;
    u64 cmdtype : 3;
} PACKED cmd_phv2mem_sv;

typedef struct cmd_pkt2mem_sv_s {
    u64 round : 1;
    u64 wr_fence : 1;
    u64 size : 14;
    u64 addr : 52;
    u64 cache : 1;
    u64 host_addr : 1;
    u64 cmdeop : 1;
    u64 cmdtype : 3;
} PACKED cmd_pkt2mem_sv;

typedef struct cmd_skip_sv_s {
    u64 size : 14;
    u64 cmdeop : 1;
    u64 cmdtype : 3;
} PACKED cmd_skip_sv;


typedef struct cmd_mem2mem_sv_s {
    u64 wr_round : 1;
    u64 wr_fence : 1;
    u64 wr_addr : 52;
    u64 wr_cache : 1;
    u64 wr_host_addr : 1;
    
    u64 size : 14;
    u64 rd_addr : 52;
    u64 rd_cache : 1;
    u64 rd_host_addr : 1;

    u64 cmdeop : 1;
    u64 cmdtype : 3;
} PACKED cmd_mem2mem_sv;

static inline dma_cmd_pkt_to_mem(u16 size, u64 addr)
{
    cmd_pkt2mem_sv cmd;
    memset(&cmd, 0, sizeof(cmd_pkt2mem_sv));
    cmd.cmdtype =  DMA_CMD_TYPE_PKT_2_MEM;
    cmd.size = size;
    cmd.addr = addr;
    // cache - don't know
    // in C - just do memcpy and this is not needed.
}
 
