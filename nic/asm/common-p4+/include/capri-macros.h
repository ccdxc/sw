#ifndef __CAPRI_MACROS_H__
#define __CAPRI_MACROS_H__

#include <capri_common.h>

#define CAPRI_READ_IDX(_idx, _table_type, _stage_entry) \
        addi		r1, r0, _idx			                ;\
        phvwri		p.table_sel, TABLE_TYPE_RAW                     ;\
        phvwri		p.table_mpu_entry_raw, _stage_entry             ;\
	phvwr		p.table_addr, r1                                ;\
        phvwri		p.table_addr[63], 1                             ;\
        phvwri	        p.table_size, 8                                 ;

#define CAPRI_GET_TABLE_ADDR(_fid, _table_base, _table_entry_size_shft, _table_state_offset) \
        addi  		r1, r0, _table_base;					\
        add 		r2, r0, _fid;						\
        sll		r2, r2, _table_entry_size_shft;				\
        add		r1, r1, r2;						\
        addi		r2, r0, _table_state_offset;				\
        add		r1, r1, r2;

#define CAPRI_GET_TABLE_ADDR_R(_fid, _table_base, _table_entry_size_shft, _table_state_offset) \
        add   		r1, r0, _table_base;					\
        add 		r2, r0, _fid;						\
        sll		r2, r2, _table_entry_size_shft;				\
        add		r1, r1, r2;						\
        addi		r2, r0, _table_state_offset;				\
        add		r1, r1, r2;

#define CAPRI_NEXT_TABLE_READ_NO_TABLE_LKUP(_num, _stage_entry) \
        phvwri  p.common_te##_num##_phv_table_lock_en, 1; \
        phvwri  p.common_te##_num##_phv_table_raw_table_size, TABLE_SIZE_0_BITS; \
        addi    r2, r0, _stage_entry; \
        srl     r1, r2, CAPRI_MPU_PC_SHIFT; \
        phvwr   p.common_te##_num##_phv_table_pc, r1; \
        phvwr   p.common_te##_num##_phv_table_addr, r0; \
        phvwri  p.app_header_table##_num##_valid, 1;

#define CAPRI_NEXT_TABLE0_READ_NO_TABLE_LKUP(_stage_entry) \
        CAPRI_NEXT_TABLE_READ_NO_TABLE_LKUP(0, _stage_entry)

#define CAPRI_NEXT_TABLE_READ(_num, _fid, _lock_en, _stage_entry, _table_base, _table_entry_size_shft, _table_state_offset, _table_read_size) \
        phvwri  p.common_te##_num##_phv_table_lock_en, 1; \
        phvwri  p.common_te##_num##_phv_table_raw_table_size, _table_read_size; \
        addi    r2, r0, _stage_entry; \
        srl     r1, r2, CAPRI_MPU_PC_SHIFT; \
        phvwr   p.common_te##_num##_phv_table_pc, r1; \
        CAPRI_GET_TABLE_ADDR_R(_fid, _table_base, _table_entry_size_shft, _table_state_offset) \
        phvwr   p.common_te##_num##_phv_table_addr, r1; \
        phvwri  p.app_header_table##_num##_valid, 1;

#define CAPRI_NEXT_TABLE0_READ(_fid, _lock_en, _stage_entry, _table_base, \
                _table_entry_size_shft, _table_state_offset, _table_read_size) \
        CAPRI_NEXT_TABLE_READ(0, _fid, _lock_en, _stage_entry, _table_base, \
                _table_entry_size_shft, _table_state_offset, _table_read_size);

#define CAPRI_NEXT_TABLE1_READ(_fid, _lock_en, _stage_entry, _table_base, \
                _table_entry_size_shft, _table_state_offset, _table_read_size) \
        CAPRI_NEXT_TABLE_READ(1, _fid, _lock_en, _stage_entry, _table_base, \
                _table_entry_size_shft, _table_state_offset, _table_read_size);

#define CAPRI_NEXT_TABLE2_READ(_fid, _lock_en, _stage_entry, _table_base, \
                _table_entry_size_shft, _table_state_offset, _table_read_size) \
        CAPRI_NEXT_TABLE_READ(2, _fid, _lock_en, _stage_entry, _table_base, \
                _table_entry_size_shft, _table_state_offset, _table_read_size);

#define CAPRI_NEXT_TABLE3_READ(_fid, _lock_en, _stage_entry, _table_base, \
                _table_entry_size_shft, _table_state_offset, _table_read_size) \
        CAPRI_NEXT_TABLE_READ(3, _fid, _lock_en, _stage_entry, _table_base, \
                _table_entry_size_shft, _table_state_offset, _table_read_size);

#define CAPRI_NEXT_IDX0_READ(_lock_en, _stage_entry, _table_base, _table_read_size) \
        CAPRI_NEXT_TABLE0_READ(0, _lock_en, _stage_entry, _table_base, 0, 0, _table_read_size) \

#define CAPRI_NEXT_IDX1_READ(_lock_en, _stage_entry, _table_base, _table_read_size) \
        CAPRI_NEXT_TABLE1_READ(0, _lock_en, _stage_entry, _table_base, 0, 0, _table_read_size) \

#define CAPRI_NEXT_IDX2_READ(_lock_en, _stage_entry, _table_base, _table_read_size) \
        CAPRI_NEXT_TABLE2_READ(0, _lock_en, _stage_entry, _table_base, 0, 0, _table_read_size) \

#define CAPRI_NEXT_IDX3_READ(_lock_en, _stage_entry, _table_base, _table_read_size) \
        CAPRI_NEXT_TABLE3_READ(0, _lock_en, _stage_entry, _table_base, 0, 0, _table_read_size) \

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
        add		r1, r0, _addr			                ;\
        phvwri		p.table_sel, _table_type                        ;\
        phvwri		p.table_mpu_entry_raw, _stage_entry             ;\
	phvwr		p.table_addr, r1                                ;\
        phvwri		p.table_addr[63], 1                             ;\
        phvwri	        p.table_size, 8                                 ;

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

// Increment 1 stat (sz = 0)
// shift by 27, to get upper 6 bits of 33 bit HBM address
// Note atomic stats region can only reside within 33 bits
// of HBM space (+ 2G)
// 0x80000000 (HBM base) is added by asic, so subtract it first
#define CAPRI_ATOMIC_STATS_INCR1(_stats_name, _addr, _offs, _val) \
        seq             c1, _val, 0; \
        bcf             [c1], _stats_name##_atomic_stats_update_done; \
        add             r1, _addr, _offs; \
        subi            r1, r1, 0x80000000; \
        srl             r2, r1, 27; \
        andi            r1, r1, ((1 << 27) - 1); \
        addi            r1, r1, CAPRI_MEM_SEM_ATOMIC_ADD_START; \
        add             r2, 0, r2, 2; \
        sll             r2, r2, 56; \
        or              r2, r2, _val; \
        memwr.dx        r1, r2
        


#define DB_ADDR_BASE                   0x68800000
#define DB_ADDR_BASE_HOST              0x68400000
#define DB_UPD_SHFT                    17
#define DB_LIF_SHFT                    6
#define DB_TYPE_SHFT                   3


#define DB_IDX_UPD_NOP                 (0x0 << 2)
#define DB_IDX_UPD_CIDX_SET            (0x1 << 2)
#define DB_IDX_UPD_PIDX_SET            (0x2 << 2)
#define DB_IDX_UPD_PIDX_INC            (0x3 << 2)

#define DB_SCHED_UPD_NOP               (0x0)
#define DB_SCHED_UPD_EVAL              (0x1)
#define DB_SCHED_UPD_CLEAR             (0x2)
#define DB_SCHED_UPD_SET               (0x3)

#define LIF_WIDTH                      11
#define QTYPE_WIDTH                    3
#define QID_WIDTH                      24        

#define LIF_TCP                        1001
#define LIF_TLS                        1002
#define LIF_GLOBALQ                    1003

#define SERQ_QID                       0
#define SESQ_QID                       1
#define ARQ_QID                        2
#define ASQ_QID                        3
#define BRQ_QID                        4
#define BSQ_QID                        5


#define CAPRI_RING_DOORBELL_ADDR(_pid_chk, _idx_upd, _sched_upd, _type, _lif) \
       addi            r5, r0, _pid_chk | _idx_upd | _sched_upd;\
       sll             r5, r5, DB_UPD_SHFT;\
       addi            r6, r0, _lif;\
       sll             r6, r6, DB_LIF_SHFT;\
       or              r5, r5, r6;\
       addi            r6, r0, _type;\
       sll             r6, r6, DB_TYPE_SHFT;\
       or              r5, r5, r6;\
       addi            r4, r5, DB_ADDR_BASE

#define CAPRI_RING_DOORBELL_ADDR_HOST(_pid_chk, _idx_upd, _sched_upd, _type, _lif) \
       addi            r5, r0, _pid_chk | _idx_upd | _sched_upd;\
       sll             r5, r5, DB_UPD_SHFT;\
       sll             r6, _lif, DB_LIF_SHFT;\
       or              r5, r5, r6;\
       addi            r6, r0, _type;\
       sll             r6, r6, DB_TYPE_SHFT;\
       or              r5, r5, r6;\
       addi            r4, r5, DB_ADDR_BASE_HOST

#define DB_PID_SHFT                    48
#define DB_QID_SHFT                    24
#define DB_RING_SHFT                   16

#define CAPRI_RING_DOORBELL_DATA(_pid, _qid, _ring, _pidx) \
        addi            r5, r0, _pid;\
        sll             r5, r5, DB_PID_SHFT;\
        add             r6, r0, _qid;\
        sll             r6, r6, DB_QID_SHFT;\
        or              r5, r5, r6;\
        addi            r6, r0, _ring;\
        sll             r6, r6, DB_RING_SHFT;\
        or              r5, r5, r6;\
        add             r6, r0, _pidx;\
        or              r5, r5, r6;\
        add             r3, r5, r0

#define CAPRI_DMA_COMMAND_SIZE            128

#define CAPRI_DMA_COMMAND_MEM_TO_PKT      1
#define CAPRI_DMA_COMMAND_PHV_TO_PKT      2
#define CAPRI_DMA_COMMAND_PHV_TO_MEM      3
#define CAPRI_DMA_COMMAND_PKT_TO_MEM      4
#define CAPRI_DMA_COMMAND_SKIP            5
#define CAPRI_DMA_COMMAND_MEM_TO_MEM      6
#define CAPRI_DMA_COMMAND_NOP             7

#define CAPRI_MPU_PC_SHIFT                6

#define DMA_CMD_TYPE_MEM2MEM_TYPE_SRC 0
#define DMA_CMD_TYPE_MEM2MEM_TYPE_DST 1

#define TIMER_START_ARRAY_BASE         0xab000000
#define TIMER_START_ARRAY_LIF_SHIFT    12
#define TIMER_START_WIDTH_BITS         40
#define TIMER_START_TYPE_SHIFT         37
#define TIMER_START_QID_SHIFT          13
#define TIMER_START_RING_SHIFT         10

#define TM_OPORT_DMA       9 
#define TM_OPORT_P4INGRESS 10
#define TM_OPORT_P4EGRESS  11


// memwr timer_start[LIF], {type, qid, ring, delta_time}
#define CAPRI_TIMER_START(_lif, _type, _qid, _ring, _delta_time) \
        addi            r4, r0, TIMER_START_ARRAY_BASE;\
        addi            r6, r0, _lif;\
        sll             r6, r6, TIMER_START_ARRAY_LIF_SHIFT;\
        add             r4, r4, r6;\
        addi            r5, r5, 0;\
        addi            r6, r0, _type;\
        sll             r6, r6, TIMER_START_TYPE_SHIFT;\
        add             r5, r5, r6;\
        add             r6, r0, _qid;\
        sll             r6, r6, TIMER_START_QID_SHIFT;\
        add             r5, r5, r6;\
        addi            r6, r0, _ring;\
        sll             r6, r6, TIMER_START_RING_SHIFT;\
        add             r5, r5, r6;\
        add             r6, r0, _delta_time;\
        add             r5, r5, r6;\
        memwr.d         r4, r5

// Increment the consumer index(cidx) of the timer ring. Producer index(pidx)
// is incremented by the hw timer block on timer expiration
// On timer expiry, timer handler is scheduled only if pidx == cidx+1
#define CAPRI_TIMER_STOP(_cidx) \
        tbladd          _cidx, 1

// Stop the timer if started, otherwise nop
#define CAPRI_TIMER_CLEAR(_pidx, _cidx) \
        add             r6, _cidx,r0;\
        addi            r6, r6, 1;\
        seq             c7, _pidx, _cidx;\
        tbladd.c1       _cidx, 1
        
// x = offsetof(_field)
// flit = x / 512
// base = flit * 512
// start_offset = (flit + 1) * 512 - (x + sizeof(x)) + base
// start_offset = (offsetof(x) / 512 + 1) * 512 - offsetof(x) - sizeof(x) + (offsetof(x) / 512) * 512
#define CAPRI_PHV_START_OFFSET(_field) \
        (((offsetof(p, _field) / 512 + 1) * 512 - offsetof(p, _field) \
					 - sizeof(p._field) + (offsetof(p, _field) / 512) * 512) >> 3)

#define CAPRI_PHV_END_OFFSET(_field) \
        ((((offsetof(p, _field) / 512 + 1) * 512 - offsetof(p, _field)	\
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

#define CAPRI_QSTATE_HEADER_RING(_x)		\
        pi_##_x                           : 16;\
        ci_##_x                           : 16;\


/* Instruction to see the value of _x during execution */
#define CAPRI_OPERAND_DEBUG(_x)         add r1, r0, _x
        
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


/* Semaphores */
#define SERQ_PRODUCER_IDX              0xba00ba00
#define SERQ_CONSUMER_IDX             0xba00ba08

#define SESQ_PRODUCER_IDX              0xba00ba10
#define SESQ_CONSUMER_IDX              0xba00ba18

#define RNMPR_ALLOC_IDX                CAPRI_SEM_RNMPR_ALLOC_INF_ADDR
#define RNMPR_FREE_IDX                 CAPRI_SEM_RNMPR_FREE_INF_ADDR

#define RNMDR_ALLOC_IDX                CAPRI_SEM_RNMDR_ALLOC_INF_ADDR
#define RNMDR_FREE_IDX                 CAPRI_SEM_RNMDR_FREE_INF_ADDR

#define TNMPR_ALLOC_IDX                CAPRI_SEM_TNMPR_ALLOC_INF_ADDR
#define TNMPR_FREE_IDX                 CAPRI_SEM_TNMPR_FREE_INF_ADDR

#define TNMDR_ALLOC_IDX                CAPRI_SEM_TNMDR_ALLOC_INF_ADDR
#define TNMDR_FREE_IDX                 CAPRI_SEM_TNMDR_FREE_INF_ADDR

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

#endif /* #ifndef __CAPRI_MACROS_H__ */
