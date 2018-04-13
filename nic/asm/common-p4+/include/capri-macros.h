#ifndef __CAPRI_MACROS_H__
#define __CAPRI_MACROS_H__

#include <capri_common.h>
#include <capri_barco.h>

#define CAPRI_READ_IDX(_idx, _table_type, _stage_entry) \
        addi        r1, r0, _idx                                    ;\
        phvwri      p.table_sel, TABLE_TYPE_RAW                     ;\
        phvwri      p.table_mpu_entry_raw, _stage_entry             ;\
        phvwr       p.table_addr, r1                                ;\
        phvwri      p.table_addr[63], 1                             ;\
        phvwr       p.table_size, 8                                 ;

/*
    TODO: Once we get phvwrpair instruction, optimize this
   MR:
        phvwri      p.common_te##_num##_phv_table_pc, _stage_entry[33:6]; \
        add         r1, _table_base, _table_state_offset; \
        phvwrpair   p.common_te##_num##_phv_table_addr, r1 , p.common_te##_num##_phv_table_lock_en, _lock_en | _table_read_size | _stage_entry
        phvwri      p.app_header_table##_num##_valid, 1;
*/
#define CAPRI_NEXT_TABLE_READ_NO_TABLE_LKUP(_num, _stage_entry) \
        phvwri      p.{common_te##_num##_phv_table_lock_en...common_te##_num##_phv_table_raw_table_size}, \
                    (0 << 3 | TABLE_SIZE_0_BITS); \
        phvwri      p.common_te##_num##_phv_table_pc, _stage_entry[33:6]; \
        phvwr   p.common_te##_num##_phv_table_addr, r0; \
        phvwri  p.app_header_table##_num##_valid, 1;

#define CAPRI_NEXT_TABLE0_READ_NO_TABLE_LKUP(_stage_entry) \
        CAPRI_NEXT_TABLE_READ_NO_TABLE_LKUP(0, _stage_entry)

#define CAPRI_NEXT_TABLE_READ(_num, _lock_en, _stage_entry, _table_base, _table_read_size) \
        phvwri      p.{common_te##_num##_phv_table_lock_en...common_te##_num##_phv_table_raw_table_size}, \
                    (_lock_en << 3 | _table_read_size); \
        phvwri      p.common_te##_num##_phv_table_pc, _stage_entry[33:6]; \
        phvwr       p.common_te##_num##_phv_table_addr, _table_base; \
        phvwri      p.app_header_table##_num##_valid, 1;

#define CAPRI_NEXT_TABLE_READ_e(_num, _lock_en, _stage_entry, _table_base, _table_read_size) \
        phvwri      p.{common_te##_num##_phv_table_lock_en...common_te##_num##_phv_table_raw_table_size}, \
                    (_lock_en << 3 | _table_read_size); \
        phvwri      p.common_te##_num##_phv_table_pc, _stage_entry[33:6]; \
        phvwr       p.common_te##_num##_phv_table_addr, _table_base; \
        phvwri.e     p.app_header_table##_num##_valid, 1;

#define CAPRI_NEXT_TABLE_READ_i(_num, _lock_en, _stage_entry, _table_base, _table_read_size) \
        phvwri      p.{common_te##_num##_phv_table_lock_en...common_te##_num##_phv_table_raw_table_size}, \
                    (_lock_en << 3 | _table_read_size); \
        phvwri      p.common_te##_num##_phv_table_pc, _stage_entry[33:6]; \
        phvwri      p.common_te##_num##_phv_table_addr, _table_base; \
        phvwri      p.app_header_table##_num##_valid, 1;

#define CAPRI_NEXT_TABLE_READ_OFFSET(_num, _lock_en, _stage_entry, _table_base, _table_state_offset, _table_read_size) \
        phvwri      p.{common_te##_num##_phv_table_lock_en...common_te##_num##_phv_table_raw_table_size}, \
                    (_lock_en << 3 | _table_read_size); \
        phvwri      p.common_te##_num##_phv_table_pc, _stage_entry[33:6]; \
        add         r1, _table_base, _table_state_offset; \
        phvwr       p.common_te##_num##_phv_table_addr, r1; \
        phvwri      p.app_header_table##_num##_valid, 1;

#define CAPRI_NEXT_TABLE_READ_INDEX(_num, _index, _lock_en, _stage_entry, _table_base, _table_entry_size_shft, _table_read_size) \
        phvwri      p.{common_te##_num##_phv_table_lock_en...common_te##_num##_phv_table_raw_table_size}, \
                    (_lock_en << 3 | _table_read_size); \
        phvwri      p.common_te##_num##_phv_table_pc, _stage_entry[33:6]; \
        sll         r2, _index, _table_entry_size_shft; \
        add         r1, _table_base, r2; \
        phvwr       p.common_te##_num##_phv_table_addr, r1; \
        phvwri      p.app_header_table##_num##_valid, 1;

#define CAPRI_CLEAR_TABLE_VALID(_num) \
        phvwri  p.app_header_table##_num##_valid, 0;

#define CAPRI_CLEAR_TABLE0_VALID \
        CAPRI_CLEAR_TABLE_VALID(0)

#define CAPRI_CLEAR_TABLE1_VALID \
        CAPRI_CLEAR_TABLE_VALID(1)

#define CAPRI_CLEAR_TABLE2_VALID \
        CAPRI_CLEAR_TABLE_VALID(2)

#define CAPRI_CLEAR_TABLE3_VALID \
        CAPRI_CLEAR_TABLE_VALID(3)

#define CAPRI_READ_ADDR(_addr, _table_type, _stage_entry) \
        add             r1, r0, _addr                                   ;\
        phvwri          p.table_sel, _table_type                        ;\
        phvwri          p.table_mpu_entry_raw, _stage_entry             ;\
        phvwr           p.table_addr, r1                                ;\
        phvwri          p.table_addr[63], 1                             ;\
        phvwri          p.table_size, 8                                 ;

#define CAPRI_STATS_INC(_stats_name, _stats_overflow_width, _inc_val, _d_field) \
        add             r2, _d_field, _inc_val; \
        bgei            r2, ((1 << _stats_overflow_width) - 1), _stats_name##_stats_update; \
        nop; \
        tbladd          _d_field, _inc_val; \
        b               _stats_name##_stats_update_end; \
        nop

#define CAPRI_STATS_INC_UPDATE(_inc_val, _d_field, _p_field) \
        phvwr           _p_field, _d_field; \
        tblwr           _d_field, _inc_val; \

// shift by 27, to get upper 6 bits of 33 bit HBM address
// Note atomic stats region can only reside within 33 bits
// of HBM space (+ 2G)
// 0x80000000 (HBM base) is added by asic, so subtract it first
// r1 = addr, r2 = data
#define CAPRI_ATOMIC_STATS_ADDR_HI(_sz, _addr, _offs) \
        add             r1, _addr, _offs; \
        subi            r1, r1, 0x80000000; \
        srl             r2, r1, 27; \
        andi            r1, r1, ((1 << 27) - 1); \
        addi            r1, r1, CAPRI_MEM_SEM_ATOMIC_ADD_START; \
        add             r2, _sz, r2, 2; \
        sll             r2, r2, 56; \

// Increment 1 stat (sz = 0)
#define CAPRI_ATOMIC_STATS_INCR1(_stats_name, _addr, _offs, _val) \
        seq             c1, _val, 0; \
        bcf             [c1], _stats_name##_atomic_stats_update_done; \
        CAPRI_ATOMIC_STATS_ADDR_HI(0, _addr, _offs); \
        or              r2, r2, _val; \
        memwr.dx        r1, r2

#define CAPRI_ATOMIC_STATS_INCR1_COND(_cx, _addr, _offs, _val)   \
        add.##_cx       r1, _addr, _offs; \
        subi.##_cx      r1, r1, 0x80000000; \
        srl.##_cx       r2, r1, 27; \
        andi.##_cx      r1, r1, ((1 << 27) - 1); \
        addi.##_cx      r1, r1, CAPRI_MEM_SEM_ATOMIC_ADD_START; \
        add.##_cx       r2, 0, r2, 2; \
        sll.##_cx       r2, r2, 56; \
        or.##_cx        r2, r2, _val; \
        memwr.dx.##_cx  r1, r2

#define CAPRI_ATOMIC_STATS_INCR1_NO_CHECK(_addr, _offs, _val) \
        CAPRI_ATOMIC_STATS_ADDR_HI(0, _addr, _offs); \
        or              r2, r2, _val; \
        memwr.dx        r1, r2

// Increment 2 stats (sz = 1)
#define CAPRI_ATOMIC_STATS_INCR2_NO_CHECK(_addr, _offs, _val0, \
                _val1) \
        CAPRI_ATOMIC_STATS_ADDR_HI(1, _addr, _offs); \
        or              r2, r2, _val0; \
        or              r2, r2, _val1, 32; \
        memwr.dx        r1, r2

// Increment 4 stats (sz = 2)
#define CAPRI_ATOMIC_STATS_INCR4_NO_CHECK(_addr, _offs, _val0, \
                _val1, _val2, _val3) \
        CAPRI_ATOMIC_STATS_ADDR_HI(2, _addr, _offs); \
        or              r2, r2, _val0; \
        or              r2, r2, _val1, 16; \
        or              r2, r2, _val2, 32; \
        or              r2, r2, _val3, 48; \
        memwr.dx        r1, r2

// Increment 7 stats (sz = 3)
#define CAPRI_ATOMIC_STATS_INCR7_NO_CHECK(_addr, _offs, _val0, \
                _val1, _val2, _val3, _val4, _val5, _val6) \
        CAPRI_ATOMIC_STATS_ADDR_HI(3, _addr, _offs); \
        or              r2, r2, _val0; \
        or              r2, r2, _val1, 8; \
        or              r2, r2, _val2, 16; \
        or              r2, r2, _val3, 24; \
        or              r2, r2, _val4, 32; \
        or              r2, r2, _val5, 40; \
        or              r2, r2, _val6, 48; \
        memwr.dx        r1, r2
        
#define CAPRI_COUNTER16_INC(_counter16, _atomic_counter_offset, _val)                                             \
        slt             c1, d.##_counter16, 0xFFFF;                                                               \
        CAPRI_ATOMIC_STATS_INCR1_COND(!c1, k.tls_global_phv_qstate_addr, _atomic_counter_offset, d.##_counter16); \
        tblwr.!c1       d.##_counter16, 0;                                                                        \
        nop.!c1;                                                                                                  \
        tbladd           d.##_counter16, _val;



#define DB_ADDR_BASE                   0x8800000
#define DB_ADDR_BASE_HOST              0x8400000
#define DB_UPD_SHFT                    17
#define DB_LIF_SHFT                    6
#define DB_TYPE_SHFT                   3


#define LIF_WIDTH                      11
#define QTYPE_WIDTH                    3
#define QID_WIDTH                      24        

#define LIF_TCP                        1001
#define LIF_TLS                        1002
#define LIF_CPU                        1003
#define LIF_APP_REDIR                  1006
#define LIF_GC                         1007

#define SERQ_QID                       0
#define SESQ_QID                       1
#define ARQ_QID                        2
#define ASQ_QID                        3
#define BRQ_QID                        4
#define BSQ_QID                        5

#define CAPRI_DOORBELL_ADDR(_pid_chk, _idx_upd, _sched_upd, _type, _lif) \
    ((((_pid_chk) | (_idx_upd) | (_sched_upd)) << DB_UPD_SHFT) + \
     ((_lif) << DB_LIF_SHFT) + \
     ((_type) << DB_TYPE_SHFT) + \
     DB_ADDR_BASE)

#define CAPRI_DOORBELL_ADDR_HOST(_pid_chk, _idx_upd, _sched_upd, _type) \
    ((((_pid_chk) | (_idx_upd) | (_sched_upd)) << DB_UPD_SHFT) + \
     DB_ADDR_BASE_HOST)

#define CAPRI_DOORBELL_ADDR_LOCAL(_pid_chk, _idx_upd, _sched_upd, _type) \
    ((((_pid_chk) | (_idx_upd) | (_sched_upd)) << DB_UPD_SHFT) + \
     DB_ADDR_BASE)

#define CAPRI_RING_DOORBELL_ADDR2(_pid_chk, _idx_upd, _sched_upd, _type, _lif) \
       addi            r5, r0, _pid_chk | _idx_upd | _sched_upd;\
       sll             r5, r5, DB_UPD_SHFT;\
       sll             r6, _lif, DB_LIF_SHFT;\
       or              r5, r5, r6;\
       add             r6, r0, _type;\
       sll             r6, r6, DB_TYPE_SHFT;\
       or              r5, r5, r6;\
       addi            r4, r5, DB_ADDR_BASE

#define CAPRI_RING_DOORBELL_ADDR_HOST(_pid_chk, _idx_upd, _sched_upd, _type, _lif) \
        addi            r4, r0, CAPRI_DOORBELL_ADDR_HOST(_pid_chk, _idx_upd, _sched_upd, _type); \
        add             r4, r4, _lif, DB_LIF_SHFT; \
        add             r4, r4, _type, DB_TYPE_SHFT;

#define CAPRI_RING_DOORBELL_ADDR(_pid_chk, _idx_upd, _sched_upd, _type, _lif) \
        addi            r4, r0, CAPRI_DOORBELL_ADDR_LOCAL(_pid_chk, _idx_upd, _sched_upd, _type); \
        add             r4, r4, _lif, DB_LIF_SHFT; \
        add             r4, r4, _type, DB_TYPE_SHFT;


#define DB_PID_SHFT                    48
#define DB_QID_SHFT                    24
#define DB_RING_SHFT                   16

#define CAPRI_RING_DOORBELL_DATA(_pid, _qid, _ring, _idx) \
        add             r3, _idx, _pid, DB_PID_SHFT;\
        or              r3, r3, _qid, DB_QID_SHFT;\
        or              r3, r3, _ring, DB_RING_SHFT;

#define CAPRI_DMA_COMMAND_SIZE            128

#define CAPRI_DMA_COMMAND_MEM_TO_PKT      1
#define CAPRI_DMA_COMMAND_PHV_TO_PKT      2
#define CAPRI_DMA_COMMAND_PHV_TO_MEM      3
#define CAPRI_DMA_COMMAND_PKT_TO_MEM      4
#define CAPRI_DMA_COMMAND_SKIP            5
#define CAPRI_DMA_COMMAND_MEM_TO_MEM      6
#define CAPRI_DMA_COMMAND_NOP             0

#define CAPRI_MPU_PC_SHIFT                6

#define DMA_CMD_TYPE_MEM2MEM_TYPE_SRC 0
#define DMA_CMD_TYPE_MEM2MEM_TYPE_DST 1

#define TM_OPORT_DMA       9 
#define TM_OPORT_P4EGRESS  10
#define TM_OPORT_P4INGRESS 11


// x = offsetof(_field)
// flit = x / 512
// base = flit * 512
// start_offset = (flit + 1) * 512 - (x + sizeof(x)) + base
// start_offset = (offsetof(x) / 512 + 1) * 512 - offsetof(x) - sizeof(x) + (offsetof(x) / 512) * 512
#define CAPRI_PHV_START_OFFSET(_field) \
        (((offsetof(p, _field) / 512 + 1) * 512 - offsetof(p, _field) \
        - sizeof(p._field) + (offsetof(p, _field) / 512) * 512) >> 3)

#define CAPRI_PHV_END_OFFSET(_field) \
        ((((offsetof(p, _field) / 512 + 1) * 512 - offsetof(p, _field) \
        + (offsetof(p, _field) / 512) * 512) >> 3) - 1)

#define CAPRI_QSTATE_HEADER_COMMON \
        pc                              : 8;\
        cos_a                           : 4;\
        cos_b                           : 4;\
        cos_sel                         : 8;\
        eval_last                       : 8;\
        total_rings                     : 4;\
        host_rings                      : 4;\
        pid                             : 16;\

#define CAPRI_QSTATE_HEADER_RING(_x)           \
        pi_##_x                           : 16;\
        ci_##_x                           : 16;\


/* Instruction to see the value of _x during execution */
#ifndef RTL
#define CAPRI_OPERAND_DEBUG(_x)         add r1, r0, _x
#else
#define CAPRI_OPERAND_DEBUG(_x)
#endif

#undef CAPRI_OPERAND_DEBUG
#define CAPRI_OPERAND_DEBUG(_x)
        
#define RNMDR_TABLE_BASE                hbm_rnmdr_table_base
#define RNMDR_TABLE_ENTRY_SIZE          8 /* 8B */
#define RNMDR_TABLE_ENTRY_SIZE_SHFT     3 /* 8B */
#define RNMDR_TABLE_SIZE                255
#define RNMDR_TABLE_SIZE_SHFT           8

#define RNMPR_TABLE_BASE                hbm_rnmpr_table_base
#define RNMPR_TABLE_ENTRY_SIZE          8 /* 8B */
#define RNMPR_TABLE_ENTRY_SIZE_SHFT     3 /* 8B */
#define RNMPR_TABLE_SIZE                255
#define RNMPR_TABLE_SIZE_SHFT           8

#define RNMPR_SMALL_TABLE_BASE          hbm_rnmpr_small_table_base
#define RNMPR_SMALL_TABLE_ENTRY_SIZE    8 /* 8B */
#define RNMPR_SMALL_TABLE_ENTRY_SIZE_SHFT 3 /* 8B */
#define RNMPR_SMALL_TABLE_SIZE          255
#define RNMPR_SMALL_TABLE_SIZE_SHFT     8

#define TNMDR_TABLE_BASE                hbm_tnmdr_table_base
#define TNMDR_TABLE_ENTRY_SIZE          8 /* 8B */
#define TNMDR_TABLE_ENTRY_SIZE_SHFT     3 /* 8B */
#define TNMDR_TABLE_SIZE                255
#define TNMDR_TABLE_SIZE_SHFT           8

#define TNMPR_TABLE_BASE                hbm_tnmpr_table_base
#define TNMPR_TABLE_ENTRY_SIZE          8 /* 8B */
#define TNMPR_TABLE_ENTRY_SIZE_SHFT     3 /* 8B */
#define TNMPR_TABLE_SIZE                255
#define TNMPR_TABLE_SIZE_SHFT           8

#define RDMA_EQ_INTR_TABLE_BASE              hbm_rdma_eq_intr_table_base
#define RDMA_EQ_INTR_TABLE_ENTRY_SIZE        8 /* 8B */
#define RDMA_EQ_INTR_TABLE_ENTRY_SIZE_SHFT   3 /* 8B */
#define RDMA_EQ_INTR_TABLE_SIZE              255
#define RDMA_EQ_INTR_TABLE_SHFT              8

#define RNMDR_GC_TABLE_BASE             hbm_rnmdr_gc_table_base
#define TNMDR_GC_TABLE_BASE             hbm_tnmdr_gc_table_base

#define RNMDR_GC_PRODUCER_TCP           CAPRI_RNMDR_GC_TCP_RING_PRODUCER
#define RNMDR_GC_PRODUCER_ARM           CAPRI_RNMDR_GC_CPU_ARM_RING_PRODUCER
#define RNMDR_GC_PER_PRODUCER_SHIFT     CAPRI_HBM_GC_PER_PRODUCER_RING_SHIFT

#define TNMDR_GC_PRODUCER_TCP           CAPRI_TNMDR_GC_TCP_RING_PRODUCER
#define TNMDR_GC_PER_PRODUCER_SHIFT     CAPRI_HBM_GC_PER_PRODUCER_RING_SHIFT

/* Semaphores */
#define SERQ_PRODUCER_IDX              0xba00ba00
#define SERQ_CONSUMER_IDX             0xba00ba08

#define SESQ_PRODUCER_IDX              0xba00ba10
#define SESQ_CONSUMER_IDX              0xba00ba18

#define RNMPR_ALLOC_IDX                CAPRI_SEM_RNMPR_ALLOC_INF_ADDR
#define RNMPR_FREE_IDX                 CAPRI_SEM_RNMPR_FREE_INC_ADDR

#define RNMDR_ALLOC_IDX                CAPRI_SEM_RNMDR_ALLOC_INF_ADDR
#define RNMDR_FREE_IDX                 CAPRI_SEM_RNMDR_FREE_INC_ADDR

#define TNMPR_ALLOC_IDX                CAPRI_SEM_TNMPR_ALLOC_INF_ADDR
#define TNMPR_FREE_IDX                 CAPRI_SEM_TNMPR_FREE_INC_ADDR

#define TNMDR_ALLOC_IDX                CAPRI_SEM_TNMDR_ALLOC_INF_ADDR
#define TNMDR_FREE_IDX                 CAPRI_SEM_TNMDR_FREE_INC_ADDR

#define TCP_NMDR_GC_IDX                CAPRI_SEM_TCP_NMDR_GC_IDX_INC_ADDR

#define ARQ_PRODUCER_IDX               0xba00ba60
#define ARQ_CONSUMER_IDX               0xba00ba68

#define ASQ_PRODUCER_IDX               0xba00ba70
#define ASQ_CONSUMER_IDX               0xba00ba78

#define BRQ_PRODUCER_IDX               0xba00ba80
#define BRQ_CONSUMER_IDX               0xba00ba88

#define BRQ_BASE                       hbm_brq_base
#define BRQ_TABLE_SIZE                 255
#define BRQ_TABLE_SIZE_SHFT            8

#define BRQ_QPCB_BASE                  0xba00ba90

#define ARQRX_BASE                     hbm_arqrx_base
#define ARQTX_BASE                     hbm_arqtx_base
#define ARQRX_QIDXR_BASE               hbm_arqrx_qidxr_base 
#define CAPRI_CPU_HASH_MASK            cpu_hash_mask
#define CAPRI_CPU_MAX_ARQID            cpu_max_arqid

#define TABLE_TYPE_RAW                 0
#define TABLE_LOCK_DIS                 0
#define TABLE_LOCK_EN                  1

#define TABLE_SIZE_8_BITS              0
#define TABLE_SIZE_16_BITS             1
#define TABLE_SIZE_32_BITS             2
#define TABLE_SIZE_64_BITS             3
#define TABLE_SIZE_128_BITS            4
#define TABLE_SIZE_256_BITS            5
#define TABLE_SIZE_512_BITS            6
#define TABLE_SIZE_0_BITS              7

#define CAPRI_MPU_STAGE_0              0
#define CAPRI_MPU_STAGE_1              1
#define CAPRI_MPU_STAGE_2              2
#define CAPRI_MPU_STAGE_3              3
#define CAPRI_MPU_STAGE_4              4
#define CAPRI_MPU_STAGE_5              5
#define CAPRI_MPU_STAGE_6              6
#define CAPRI_MPU_STAGE_7              7

#define CAPRI_MPU_TABLE_0              0
#define CAPRI_MPU_TABLE_1              1
#define CAPRI_MPU_TABLE_2              2
#define CAPRI_MPU_TABLE_3              3

#define CAPRI_SET_DEBUG_STAGE0_3(_p, _stage, _table) \
        phvwrmi         _p, ((1 << _table) << (4 * _stage)), \
                            ((1 << _table) << (4 * _stage))

#define CAPRI_SET_DEBUG_STAGE4_7(_p, _stage, _table) \
        phvwrmi         _p, ((1 << _table) << (4 * (_stage - 4))), \
                            ((1 << _table) << (4 * (_stage - 4)))

#define CAPRI_DMA_CMD_PHV2MEM_SETUP(_dma_cmd_prefix, __addr, _sfield, _efield)                   \
        phvwri      p.{##_dma_cmd_prefix##_phv_end_addr...##_dma_cmd_prefix##_type},            \
                    ((CAPRI_PHV_END_OFFSET(_efield) << 18) |                                    \
                     (CAPRI_PHV_START_OFFSET(_sfield) << 8) |                                   \
                     CAPRI_DMA_COMMAND_PHV_TO_MEM);                                             \
        phvwr       p.##_dma_cmd_prefix##_addr, __addr

#define CAPRI_DMA_CMD_PHV2MEM_SETUP_STOP(_dma_cmd_prefix, __addr, _sfield, _efield)             \
        phvwri      p.{##_dma_cmd_prefix##_phv_end_addr...##_dma_cmd_prefix##_type},            \
                    ((CAPRI_PHV_END_OFFSET(_efield) << 18) |                                    \
                     (CAPRI_PHV_START_OFFSET(_sfield) << 8) |                                   \
                     1 << 3 | CAPRI_DMA_COMMAND_PHV_TO_MEM);                                    \
        phvwr       p.##_dma_cmd_prefix##_addr, __addr

#define CAPRI_DMA_CMD_PKT2MEM_SETUP(_dma_cmd_prefix, __addr, _len)                               \
        phvwr       p.##_dma_cmd_prefix##_size, _len;                                           \
        phvwrpair   p.##_dma_cmd_prefix##_addr, __addr,                                          \
                        p.##_dma_cmd_prefix##_type, CAPRI_DMA_COMMAND_PKT_TO_MEM

#define CAPRI_DMA_CMD_SKIP_SETUP(_dma_cmd_prefix)                                               \
        phvwri      p.##_dma_cmd_prefix##_skip_to_eop, 1;                                       \
        phvwri      p.##_dma_cmd_prefix##_type, CAPRI_DMA_COMMAND_SKIP

#define CAPRI_DMA_CMD_PHV2MEM_SETUP_I(_dma_cmd_prefix, __addr, _sfield, _efield)                 \
        phvwri      p.{##_dma_cmd_prefix##_phv_end_addr...##_dma_cmd_prefix##_type},            \
                    ((CAPRI_PHV_END_OFFSET(_efield) << 18) |                                    \
                     (CAPRI_PHV_START_OFFSET(_sfield) << 8) |                                   \
                     CAPRI_DMA_COMMAND_PHV_TO_MEM);                                             \
        phvwri      p.##_dma_cmd_prefix##_addr, __addr;

#define CAPRI_DMA_CMD_RING_DOORBELL2_INC_PI(_dma_cmd_prefix, _lif, __type, _qid, _ring, _sfield, _efield) \
        phvwri      p.{##_dma_cmd_prefix##_phv_end_addr...##_dma_cmd_prefix##_type},            \
                    ((CAPRI_PHV_END_OFFSET(_efield) << 18) |                                    \
                     (CAPRI_PHV_START_OFFSET(_sfield) << 8) |                                   \
                     CAPRI_DMA_COMMAND_PHV_TO_MEM);                                             \
        phvwri      p.##_dma_cmd_prefix##_addr,                                                 \
                    CAPRI_DOORBELL_ADDR(0, DB_IDX_UPD_PIDX_INC, DB_SCHED_UPD_SET,__type, _lif); \
        CAPRI_RING_DOORBELL_DATA(0, _qid, _ring, 0);                                            \
        phvwr       p.{_sfield..._efield}, r3.dx;                                               \
        phvwri      p.##_dma_cmd_prefix##_type, CAPRI_DMA_COMMAND_PHV_TO_MEM;

#define CAPRI_DMA_CMD_RING_DOORBELL2_SET_PI(_dma_cmd_prefix, _lif, __type, _qid, _ring, _pidx, _sfield, _efield) \
        phvwri      p.{##_dma_cmd_prefix##_phv_end_addr...##_dma_cmd_prefix##_type},            \
                    ((CAPRI_PHV_END_OFFSET(_efield) << 18) |                                    \
                     (CAPRI_PHV_START_OFFSET(_sfield) << 8) |                                   \
                     CAPRI_DMA_COMMAND_PHV_TO_MEM);                                             \
        phvwri      p.##_dma_cmd_prefix##_addr,                                                 \
                    CAPRI_DOORBELL_ADDR(0, DB_IDX_UPD_PIDX_SET, DB_SCHED_UPD_SET,__type, _lif); \
        CAPRI_RING_DOORBELL_DATA(0, _qid, _ring, _pidx);                                        \
        phvwr       p.{_sfield..._efield}, r3.dx;

#define CAPRI_DMA_CMD_RING_DOORBELL2_SET_PI_STOP_FENCE(_dma_cmd_prefix, _lif, __type, _qid, _ring, _pidx, _sfield, _efield) \
        phvwri      p.{##_dma_cmd_prefix##_phv_end_addr...##_dma_cmd_prefix##_type},   \
                    ((CAPRI_PHV_END_OFFSET(_efield) << 18) |                                    \
                     (CAPRI_PHV_START_OFFSET(_sfield) << 8) |                                   \
                     (1 << 3 | 1 << 6) |                                                        \
                     CAPRI_DMA_COMMAND_PHV_TO_MEM);                                             \
        phvwri      p.##_dma_cmd_prefix##_addr,                                                 \
                    CAPRI_DOORBELL_ADDR(0, DB_IDX_UPD_PIDX_SET, DB_SCHED_UPD_SET,__type, _lif); \
        CAPRI_RING_DOORBELL_DATA(0, _qid, _ring, _pidx);                                        \
        phvwr       p.{_sfield..._efield}, r3.dx;

#define CAPRI_DMA_CMD_RING_DOORBELL2(_dma_cmd_prefix, _lif, __type, _qid, _ring, _pidx, _sfield, _efield) \
        CAPRI_DMA_CMD_RING_DOORBELL2_INC_PI(_dma_cmd_prefix, _lif, __type, _qid, _ring, _sfield, _efield)


#define CAPRI_DMA_CMD_RING_DOORBELL_INC_PI(_dma_cmd_prefix, _lif, __type, _qid, _ring, _field)  \
        phvwri      p.{##_dma_cmd_prefix##_phv_end_addr...##_dma_cmd_prefix##_type},  \
                    ((CAPRI_PHV_END_OFFSET(_field) << 18) |                                     \
                     (CAPRI_PHV_START_OFFSET(_field) << 8) |                                    \
                     CAPRI_DMA_COMMAND_PHV_TO_MEM);                                             \
        phvwri      p.##_dma_cmd_prefix##_addr,                                                 \
                    CAPRI_DOORBELL_ADDR(0, DB_IDX_UPD_PIDX_INC, DB_SCHED_UPD_SET,__type, _lif); \
        CAPRI_RING_DOORBELL_DATA(0, _qid, _ring, 0);                                            \
        phvwr       p.{_field}, r3.dx;

#define CAPRI_DMA_CMD_RING_DOORBELL_SET_PI(_dma_cmd_prefix, _lif, __type, _qid, _ring, _pidx, _field)  \
        phvwri      p.{##_dma_cmd_prefix##_phv_end_addr...##_dma_cmd_prefix##_type},  \
                    ((CAPRI_PHV_END_OFFSET(_field) << 18) |                                     \
                     (CAPRI_PHV_START_OFFSET(_field) << 8) |                                    \
                     CAPRI_DMA_COMMAND_PHV_TO_MEM);                                             \
        phvwri      p.##_dma_cmd_prefix##_addr,                                                 \
                    CAPRI_DOORBELL_ADDR(0, DB_IDX_UPD_PIDX_SET, DB_SCHED_UPD_SET,__type, _lif); \
        CAPRI_RING_DOORBELL_DATA(0, _qid, _ring, _pidx);                                        \
        phvwr       p.{_field}, r3.dx;

#define CAPRI_DMA_CMD_RING_DOORBELL(_dma_cmd_prefix, _lif, __type, _qid, _ring, _pidx, _field)  \
        CAPRI_DMA_CMD_RING_DOORBELL_INC_PI(_dma_cmd_prefix, _lif, __type, _qid, _ring, _field)

#define CAPRI_DMA_CMD_PHV2PKT_SETUP(_dma_cmd_prefix, _sfield, _efield)                          \
        phvwri      p.{##_dma_cmd_prefix##_phv_end_addr...##_dma_cmd_prefix##_type},            \
                    ((CAPRI_PHV_END_OFFSET(_efield) << 17) |                                    \
                     (CAPRI_PHV_START_OFFSET(_sfield) << 7) |                                   \
                     CAPRI_DMA_COMMAND_PHV_TO_PKT);

#define CAPRI_DMA_CMD_PHV2PKT_SETUP2(_dma_cmd_prefix, _sfield, _efield, _sfield1, _efield1)     \
        phvwri      p.{##_dma_cmd_prefix##_phv_end_addr1...##_dma_cmd_prefix##_phv_start_addr1}, \
                    ((CAPRI_PHV_END_OFFSET(_efield1) << 10) | CAPRI_PHV_START_OFFSET(_sfield1)); \
        phvwri      p.{##_dma_cmd_prefix##_phv_end_addr...##_dma_cmd_prefix##_type},            \
                    ((CAPRI_PHV_END_OFFSET(_efield) << 17) |                                    \
                     (CAPRI_PHV_START_OFFSET(_sfield) << 7) |                                   \
                     (1 << 5) |                                                                 \
                     CAPRI_DMA_COMMAND_PHV_TO_PKT);

#define CAPRI_DMA_CMD_MEM2PKT_SETUP(_dma_cmd_prefix, __addr, _len)                              \
        phvwrpair   p.##_dma_cmd_prefix##_size, _len,                                           \
                        p.##_dma_cmd_prefix##_addr, __addr;                                     \
        phvwri      p.##_dma_cmd_prefix##_type, CAPRI_DMA_COMMAND_MEM_TO_PKT

#define CAPRI_DMA_CMD_MEM2PKT_SETUP_PKT_STOP(_dma_cmd_prefix, __addr, _len)                     \
        phvwrpair   p.##_dma_cmd_prefix##_addr, __addr,                                         \
                        p.##_dma_cmd_prefix##_size, _len;                                       \
        phvwri      p.{##_dma_cmd_prefix##_pkt_eop...##_dma_cmd_prefix##_type},                 \
                        (1 << 4 | CAPRI_DMA_COMMAND_MEM_TO_PKT)

#define CAPRI_DMA_CMD_STOP_FENCE(_dma_cmd_prefix)                                               \
        phvwrpair   p.##_dma_cmd_prefix##_wr_fence, 1,                                               \
                        p.##_dma_cmd_prefix##_eop, 1

#define CAPRI_DMA_CMD_PKT_STOP(_dma_cmd_prefix)                                                 \
        phvwri      p.##_dma_cmd_prefix##_pkt_eop, 1;                                           \

#define CAPRI_DMA_CMD_STOP(_dma_cmd_prefix)                                                     \
        phvwri      p.##_dma_cmd_prefix##_eop, 1;                                               \


// Timers
#define CAPRI_FAST_TIMER_ADDR(_lif) \
        (CAPRI_MEM_FAST_TIMER_START + (_lif << 3))

#define CAPRI_SLOW_TIMER_ADDR(_lif) \
        (CAPRI_MEM_SLOW_TIMER_START + (_lif << 3))

#define TIMER_QID_SHFT              3
#define TIMER_RING_SHFT             27
#define TIMER_DELTA_TIME_SHFT       30

// Result in r3
#define CAPRI_TIMER_DATA(_type, _qid, _ring, _delta_time) \
        add             r3, _type, _qid, TIMER_QID_SHFT;\
        or              r3, r3, _ring, TIMER_RING_SHFT;\
        or              r3, r3, _delta_time, TIMER_DELTA_TIME_SHFT;

#endif /* #ifndef __CAPRI_MACROS_H__ */
