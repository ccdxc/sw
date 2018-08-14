#define DMA_CMD_TYPE_MEM2PKT     1
#define DMA_CMD_TYPE_PHV2PKT     2
#define DMA_CMD_TYPE_PHV2MEM     3
#define DMA_CMD_TYPE_PKT2MEM     4
#define DMA_CMD_TYPE_SKIP        5
#define DMA_CMD_TYPE_MEM2MEM     6
#define DMA_CMD_TYPE_NOP         0

#define DMA_CMD_TYPE_MEM2MEM_TYPE_SRC        0
#define DMA_CMD_TYPE_MEM2MEM_TYPE_DST        1
#define DMA_CMD_TYPE_MEM2MEM_TYPE_PHV2MEM    2

header_type dma_cmd_pkt2mem_t {
    fields {
        // pkt2mem - used for copying input packet to memory.
        dma_cmd_pad              : 42;
        dma_cmd_size             : 14;
        dma_cmd_pad1             : 1;
        dma_cmd_override_lif     : 11;
        dma_cmd_addr             : 52;
        dma_cmd_use_override_lif : 1;
        dma_cmd_cache            : 1;
        dma_cmd_host_addr        : 1;
        dma_cmd_round            : 1;
        dma_cmd_eop              : 1;
        dma_cmd_type             : 3;

    }
}

#define DMA_COMMAND_PKT2MEM_FILL(_dma_cmd_pkt2mem, _addr, _size, _is_host, _cache) \
    modify_field(_dma_cmd_pkt2mem.dma_cmd_type , DMA_CMD_TYPE_PKT2MEM); \
    modify_field(_dma_cmd_pkt2mem.dma_cmd_addr , _addr); \
    modify_field(_dma_cmd_pkt2mem.dma_cmd_host_addr , _is_host); \
    modify_field(_dma_cmd_pkt2mem.dma_cmd_size , _size); \
    modify_field(_dma_cmd_pkt2mem.dma_cmd_cache , _cache); 

header_type dma_cmd_phv2mem_t {
    fields {
        dma_cmd_pad              : 33;   
        dma_cmd_override_lif     : 11;
        dma_cmd_addr             : 52;
        dma_cmd_barrier          : 1;
        dma_cmd_round            : 1;
        dma_cmd_pcie_msg         : 1;
        dma_cmd_use_override_lif : 1;
        dma_cmd_phv_end_addr     : 10;
        dma_cmd_phv_start_addr   : 10;
        dma_cmd_fence_fence      : 1;
        dma_cmd_wr_fence         : 1;
        dma_cmd_cache            : 1;
        dma_cmd_host_addr        : 1;
        dma_cmd_eop              : 1;
        dma_cmd_type             : 3;
    }
}

#define DMA_COMMAND_PHV2MEM_FILL(_dma_cmd_phv2mem, _addr, _start, _end, _is_host, _cache, _barrier, _fence) \
    modify_field(_dma_cmd_phv2mem.dma_cmd_type, DMA_CMD_TYPE_PHV2MEM); \
    modify_field(_dma_cmd_phv2mem.dma_cmd_addr, _addr); \
    modify_field(_dma_cmd_phv2mem.dma_cmd_phv_start_addr, _start); \
    modify_field(_dma_cmd_phv2mem.dma_cmd_phv_end_addr, _end); \
    modify_field(_dma_cmd_phv2mem.dma_cmd_host_addr, _is_host); \
    modify_field(_dma_cmd_phv2mem.dma_cmd_cache, _cache); \
    modify_field(_dma_cmd_phv2mem.dma_cmd_barrier, _barrier); \
    modify_field(_dma_cmd_phv2mem.dma_cmd_wr_fence, _fence);

header_type dma_cmd_mem2mem_t {
    fields {
        dma_cmd_pad              : 16;
        dma_cmd_size             : 14;
        dma_cmd_pad1             : 1;
        dma_cmd_override_lif     : 11;
        dma_cmd_addr             : 52;
        dma_cmd_barrier          : 1;
        dma_cmd_round            : 1;
        dma_cmd_pcie_msg         : 1;
        dma_cmd_use_override_lif : 1;
        dma_cmd_phv_end_addr     : 10;
        dma_cmd_phv_start_addr   : 10;
        dma_cmd_fence_fence      : 1;
        dma_cmd_wr_fence         : 1;
        dma_cmd_cache            : 1;
        dma_cmd_host_addr        : 1;
        dma_cmd_mem2mem_type     : 2;
        dma_cmd_eop              : 1;
        dma_cmd_type             : 3;
    }
}

#define DMA_COMMAND_MEM2MEM_FILL(_dma_cmd_m2m_src, _dma_cmd_m2m_dst, _src_addr, _src_host, _dst_addr, _dst_host, _size, _cache, _fence, _barrier) \
    modify_field(_dma_cmd_m2m_src.dma_cmd_type, DMA_CMD_TYPE_MEM2MEM); \
    modify_field(_dma_cmd_m2m_src.dma_cmd_mem2mem_type, DMA_CMD_TYPE_MEM2MEM_TYPE_SRC); \ 
    modify_field(_dma_cmd_m2m_src.dma_cmd_addr, _src_addr); \
    modify_field(_dma_cmd_m2m_src.dma_cmd_host_addr, _src_host); \
    modify_field(_dma_cmd_m2m_src.dma_cmd_size, _size); \
    modify_field(_dma_cmd_m2m_src.dma_cmd_cache, _cache); \
    modify_field(_dma_cmd_m2m_src.dma_cmd_wr_fence, _fence); \
    modify_field(_dma_cmd_m2m_src.dma_cmd_barrier, _barrier); \
    modify_field(_dma_cmd_m2m_dst.dma_cmd_type, DMA_CMD_TYPE_MEM2MEM); \
    modify_field(_dma_cmd_m2m_src.dma_cmd_mem2mem_type, DMA_CMD_TYPE_MEM2MEM_TYPE_DST); \ 
    modify_field(_dma_cmd_m2m_dst.dma_cmd_addr, _dst_addr); \
    modify_field(_dma_cmd_m2m_dst.dma_cmd_host_addr, _dst_host); \
    modify_field(_dma_cmd_m2m_dst.dma_cmd_size, _size); \
    modify_field(_dma_cmd_m2m_dst.dma_cmd_cache, _cache); \
    modify_field(_dma_cmd_m2m_dst.dma_cmd_wr_fence, _fence); \
    modify_field(_dma_cmd_m2m_dst.dma_cmd_barrier, _barrier); 

header_type dma_cmd_generic_t {
    fields {
        dma_cmd_pad : 125;
        dma_cmd_type : 3;
    }
}

header_type dma_cmd_phv2pkt_t {
    fields {
        dma_cmd_phv2pkt_pad     : 41;  
        dma_cmd_phv_end_addr3   : 10;
        dma_cmd_phv_start_addr3 : 10;
        dma_cmd_phv_end_addr2   : 10;
        dma_cmd_phv_start_addr2 : 10;
        dma_cmd_phv_end_addr1   : 10;
        dma_cmd_phv_start_addr1 : 10;
        dma_cmd_phv_end_addr    : 10;
        dma_cmd_phv_start_addr  : 10;
        dma_cmd_cmdsize         : 2 ;
        dma_pkt_eop             : 1 ;
        dma_cmd_eop             : 1 ;
        dma_cmd_type            : 3 ;
    }
}

#define DMA_COMMAND_PHV2PKT_FILL(_dma_cmd_phv2pkt, _start, _end, _pkt_eop) \
    modify_field(_dma_cmd_phv2pkt.dma_cmd_type, DMA_CMD_TYPE_PHV2PKT); \
    modify_field(_dma_cmd_phv2pkt.dma_cmd_phv_start_addr, _start); \
    modify_field(_dma_cmd_phv2pkt.dma_cmd_phv_end_addr, _end); \
    modify_field(_dma_cmd_phv2pkt.dma_pkt_eop, _pkt_eop);

header_type dma_cmd_mem2pkt_t {
    fields {
        dma_cmd_mem2pkt_pad      : 42;
        dma_cmd_size             : 14;
        dma_cmd_mem2pkt_pad1     : 1;
        dma_cmd_override_lif     : 11;
        dma_cmd_addr             : 52;
        dma_cmd_use_override_lif : 1;
        dma_cmd_cache            : 1;
        dma_cmd_host_addr        : 1;
        dma_pkt_eop              : 1;
        dma_cmd_eop              : 1;
        dma_cmd_type             : 3;
    }
}

#define DMA_COMMAND_MEM2PKT_FILL(_dma_cmd_mem2pkt, _addr, _size, _pkt_eop, _cache, _host_addr) \
    modify_field(_dma_cmd_mem2pkt.dma_cmd_type, DMA_CMD_TYPE_MEM2PKT); \
    modify_field(_dma_cmd_mem2pkt.dma_cmd_addr, _addr); \
    modify_field(_dma_cmd_mem2pkt.dma_cmd_size, _size); \
    modify_field(_dma_cmd_mem2pkt.dma_pkt_eop, _pkt_eop); \
    modify_field(_dma_cmd_mem2pkt.dma_cmd_cache, _cache); \
    modify_field(_dma_cmd_mem2pkt.dma_cmd_host_addr, _host_addr); 

header_type dma_cmd_skip_t {
    fields {
        dma_cmd_skip_pad        : 109;
        dma_cmd_skip_to_eop     : 1;
        dma_cmd_size            : 14;
        dma_cmd_eop             : 1;
        dma_cmd_type            : 3;
    }
}


