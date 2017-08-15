#include "../../p4/include/intrinsic.p4"

#define GLOBAL_WIDTH 128
#define STAGE_2_STAGE_WIDTH 160
#define TO_STAGE_WIDTH 128
#define RAW_TABLE_ADDR_WIDTH 64
#define RAW_TABLE_PC_WIDTH 28
#define RAW_TABLE_SIZE_WIDTH 3 

#define ADDRESS_WIDTH 64

#define RAW_TABLE_SIZE_1    0
#define RAW_TABLE_SIZE_2    1
#define RAW_TABLE_SIZE_4    2
#define RAW_TABLE_SIZE_8    3
#define RAW_TABLE_SIZE_16   4
#define RAW_TABLE_SIZE_32   5
#define RAW_TABLE_SIZE_64   6
#define RAW_TABLE_SIZE_128  7

//shall we move the below to intrinsic.p4 ??
#define P4PLUS_APPTYPE_CLASSIC_NIC  1
#define P4PLUS_APPTYPE_RDMA         2
#define P4PLUS_APPTYPE_TCPTLS       3
#define P4PLUS_APPTYPE_IPSEC        4
#define P4PLUS_APPTYPE_NDE          5
#define P4PLUS_APPTYPE_STORAGE      6
#define P4PLUS_APPTYPE_TELEMETRY    7

#define LIF_TABLE_SIZE 2048

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

#define DOORBELL_UPD_PID_CHECK              1
#
#define DOORBELL_IDX_CTL_NONE               0
#define DOORBELL_IDX_CTL_UPD_CIDX           1
#define DOORBELL_IDX_CTL_UPD_PIDX           2
#define DOORBELL_IDX_CTL_INC_PIDX           2
#
#define DOORBELL_SCHED_CTL_NONE             0
#define DOORBELL_SCHED_CTL_EVAL             1
#define DOORBELL_SCHED_CTL_CLEAR            2
#define DOORBELL_SCHED_CTL_SET              3

header_type p4_2_p4plus_app_header_t {
    fields {
        app_type : 4;
        table0_valid : 1;
        table1_valid : 1;
        table2_valid : 1;
        table3_valid : 1;
        gft_flow_id : 24;
        app_data : 360;
        app_data_pad : 120;
    }
}

header_type p4plus_2_p4_app_header_t {
    fields {
        app_type : 4;
        table0_valid : 1;
        table1_valid : 1;
        table2_valid : 1;
        table3_valid : 1;
        app_data : 360;
        app_data_pad : 144;
    }
}

header_type p4plus_common_global_t {
    fields {
        global_data : GLOBAL_WIDTH;
    }
}

header_type p4plus_common_s2s_t {
    fields {
        s2s_data : STAGE_2_STAGE_WIDTH;
    }
}

header_type p4plus_common_to_stage_t {
    fields {
        to_stage_data : TO_STAGE_WIDTH;
    }
}

header_type p4plus_common_raw_table_engine_phv_t {
    fields {
        table_lock_en : 1;
        table_raw_table_size : RAW_TABLE_SIZE_WIDTH; 
        table_pc   : RAW_TABLE_PC_WIDTH;
        table_addr : RAW_TABLE_ADDR_WIDTH;
    }
}

header_type p4plus_common_table_engine_phv_t {
    fields {
        to_stage_0 : TO_STAGE_WIDTH;
        to_stage_1 : TO_STAGE_WIDTH;
        to_stage_2 : TO_STAGE_WIDTH;
        to_stage_3 : TO_STAGE_WIDTH;

        to_stage_4 : TO_STAGE_WIDTH;
        to_stage_5 : TO_STAGE_WIDTH;
        to_stage_6 : TO_STAGE_WIDTH;
        to_stage_7 : TO_STAGE_WIDTH;

    }
}

header_type p4plus_app_specific_scratch_phv_t {
    fields {
        rsvd0 : 64;
        rsvd1 : 64;
        rsvd2 : 64;
        rsvd3 : 64;
        rsvd4 : 64;
        rsvd5 : 64;
        rsvd6 : 64;
        rsvd7 : 64;
    }
}

// this is for preserving D only
header_type scratch_metadata_t {
    fields {
        data0 : 64;
        data1 : 64;
        data2 : 64;
        data3 : 64;
        data4 : 64;
        data5 : 64;
        data6 : 64;
        data7 : 64;
    }
}

#define SCRATCH_METADATA_INIT(scratch) 		\
    modify_field(scratch.data0, data0);		\
    modify_field(scratch.data1, data1);		\
    modify_field(scratch.data2, data2);		\
    modify_field(scratch.data3, data3);		\
    modify_field(scratch.data4, data4);		\
    modify_field(scratch.data5, data5);		\
    modify_field(scratch.data6, data6);		\
    modify_field(scratch.data7, data7);		\

#define SCRATCH_METADATA_INIT_7(scratch) 		\
    modify_field(scratch.data0, data0);		\
    modify_field(scratch.data1, data1);		\
    modify_field(scratch.data2, data2);		\
    modify_field(scratch.data3, data3);		\
    modify_field(scratch.data4, data4);		\
    modify_field(scratch.data5, data5);		\
    modify_field(scratch.data6, data6);
#if 0
#define TABLE0_PARAMS_INIT(scratch, te)					\
   modify_field(scratch.table0_pc, te.table0_pc);			\
   modify_field(scratch.table0_raw_table_size,te.table0_raw_table_size);\
   modify_field(scratch.table0_lock_en, te.table0_lock_en);		\
   modify_field(scratch.table0_ki_global, te.table0_ki_global);		\
   modify_field(scratch.table0_ki_s2s, te.table0_ki_s2s);		\

#define TABLE1_PARAMS_INIT(scratch, te)					\
   modify_field(scratch.table0_pc, te.table1_pc);			\
   modify_field(scratch.table0_raw_table_size,te.table1_raw_table_size);\
   modify_field(scratch.table0_lock_en, te.table1_lock_en);		\
   modify_field(scratch.table0_ki_global, te.table0_ki_global);		\
   modify_field(scratch.table0_ki_s2s, te.table1_ki_s2s);		\

#define TABLE2_PARAMS_INIT(scratch, te)					\
   modify_field(scratch.table0_pc, te.table2_pc);			\
   modify_field(scratch.table0_raw_table_size,te.table2_raw_table_size);\
   modify_field(scratch.table0_lock_en, te.table2_lock_en);		\
   modify_field(scratch.table0_ki_global, te.table0_ki_global);		\
   modify_field(scratch.table0_ki_s2s, te.table2_ki_s2s);		\

#define TABLE3_PARAMS_INIT(scratch, te)					\
   modify_field(scratch.table0_pc, te.table3_pc);			\
   modify_field(scratch.table0_raw_table_size,te.table3_raw_table_size);\
   modify_field(scratch.table0_lock_en, te.table3_lock_en);		\
   modify_field(scratch.table0_ki_global, te.table0_ki_global);		\
   modify_field(scratch.table0_ki_s2s, te.table3_ki_s2s);
#endif


header_type dma_cmd_pkt2mem_t {
    fields {
        // pkt2mem - used for copying input packet to memory.
		dma_cmd_pad : 43;
        dma_cmd_round : 1;
        dma_cmd_override_lif : 11;
        dma_cmd_use_override_lif : 1;
        dma_cmd_size : 14;
        dma_cmd_addr : 52;
        dma_cmd_cache : 1;
        dma_cmd_host_addr : 1;
        dma_cmd_eop : 1;
        dma_cmd_type : 3;

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
        dma_cmd_pad : 34;
        dma_cmd_round : 1;
        dma_cmd_pcie_msg : 1;
        dma_cmd_barrier : 1;
        dma_cmd_override_lif : 11;
        dma_cmd_use_override_lif : 1;
        dma_cmd_phv_end_addr : 10;
        dma_cmd_phv_start_addr : 10;
        dma_cmd_wr_fence : 1;
        dma_cmd_addr : 52;
        dma_cmd_cache : 1;
        dma_cmd_host_addr : 1;
        dma_cmd_eop : 1;
        dma_cmd_type : 3;
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
		dma_cmd_pad : 18;
        dma_cmd_round : 1;
        dma_cmd_pcie_msg : 1;
        dma_cmd_barrier : 1;
        dma_cmd_override_lif : 11;
        dma_cmd_use_override_lif : 1;
        dma_cmd_phv_end_addr : 10;
        dma_cmd_phv_start_addr : 10;
        dma_cmd_wr_fence : 1;
        dma_cmd_size : 14;
        dma_cmd_addr : 52;
        dma_cmd_cache : 1;
        dma_cmd_host_addr : 1;
        dma_cmd_mem2mem_type : 2;
        dma_cmd_eop : 1;
        dma_cmd_type : 3;
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
        dma_cmd_phv2pkt_pad : 103;
        dma_cmd_phv_end_addr : 10;
        dma_cmd_phv_start_addr : 10;
        dma_pkt_eop : 1;
        dma_cmd_eop : 1;
        dma_cmd_type : 3;
    }
}

#define DMA_COMMAND_PHV2PKT_FILL(_dma_cmd_phv2pkt, _start, _end, _pkt_eop) \
    modify_field(_dma_cmd_phv2pkt.dma_cmd_type, DMA_CMD_TYPE_PHV2PKT); \
    modify_field(_dma_cmd_phv2pkt.dma_cmd_phv_start_addr, _start); \
    modify_field(_dma_cmd_phv2pkt.dma_cmd_phv_end_addr, _end); \
    modify_field(_dma_cmd_phv2pkt.dma_pkt_eop, _pkt_eop);

header_type dma_cmd_mem2pkt_t {
    fields {
        dma_cmd_mem2pkt_pad : 43;
        dma_cmd_override_lif : 11;
        dma_cmd_use_override_lif : 1;
        dma_cmd_size : 14;
        dma_cmd_addr : 52;
        dma_cmd_cache : 1;
        dma_cmd_host_addr : 1;
        dma_pkt_eop : 1;
        dma_cmd_eop : 1;
        dma_cmd_type : 3;
    }
}

#define DMA_COMMAND_MEM2PKT_FILL(_dma_cmd_mem2pkt, _addr, _size, _pkt_eop, _cache, _host_addr) \
    modify_field(_dma_cmd_mem2pkt.dma_cmd_type, DMA_CMD_TYPE_MEM2PKT); \
    modify_field(_dma_cmd_mem2pkt.dma_cmd_addr, _addr); \
    modify_field(_dma_cmd_mem2pkt.dma_cmd_size, _size); \
    modify_field(_dma_cmd_mem2pkt.dma_pkt_eop, _pkt_eop); \
    modify_field(_dma_cmd_mem2pkt.dma_cmd_cache, _cache); \
    modify_field(_dma_cmd_mem2pkt.dma_cmd_host_addr, _host_addr); 

header_type doorbell_addr_t {
    fields {
        offset : 3;
        qtype : 3;
        lif : 11;
        upd_pid_chk : 1;
        upd_index_ctl : 2;
        upd_sched_ctl : 2;
    }
}

#define DOORBELL_ADDR_FILL(_doorbell, _offset, _qtype, _lif, _upd_pid_chk, \
                           _upd_index_ctl, _upd_sched_ctl) \
    modify_field(_doorbell.offset, _offset); \
    modify_field(_doorbell.qtype, _qtype); \
    modify_field(_doorbell.lif, _lif); \
    modify_field(_doorbell.upd_pid_chk, _upd_pid_chk); \
    modify_field(_doorbell.upd_index_ctl, _upd_index_ctl); \
    modify_field(_doorbell.upd_sched_ctl, _upd_sched_ctl);

header_type doorbell_data_t {
    fields {
        pid : 16;
        qid : 24;
        pad : 5;
        ring : 3;
        index : 16;
    }
}

#define DOORBELL_DATA_FILL(_doorbell, _index, _ring, _qid, _pid) \
    modify_field(_doorbell.index, _index); \
    modify_field(_doorbell.ring, _ring); \
    modify_field(_doorbell.pad, 0); \
    modify_field(_doorbell.qid, _qid); \
    modify_field(_doorbell.pid, _pid);

header_type pkt_descr_t {
    fields {
        reserved : 64;
        next_addr : 64;
        L2 : 32;
        O2 : 32;
        A2 : 64;
        L1 : 32;
        O1 : 32;
        A1 : 64;
        L0 : 32;
        O0 : 32;
        A0 : 64;
        scratch : 512;
    }
}

header_type pkt_descr_scratch_t {
    fields {
        scratch : 512;
    }
}

header_type pkt_descr_aol_t {
    fields {
        reserved : 64;
        next_addr : 64;
        L2 : 32;
        O2 : 32;
        A2 : 64;
        L1 : 32;
        O1 : 32;
        A1 : 64;
        L0 : 32;
        O0 : 32;
        A0 : 64;

    }
}

header_type ring_entry_t {
    fields {
        descr_addr : 64;
    }
}
