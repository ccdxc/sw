#ifndef __CAPRI_MACROS_H__
#define __CAPRI_MACROS_H__

#define CAPRI_NEXT_TABLE_READ(_fid, _table_sel, _stage_entry, _table_base, _table_entry_size_shft, _table_state_offset, _table_read_size)                                                                   \
  addi		r1, r0, _table_base;					\
  add 		r2, r0, _fid;						\
  sll		r2, r2, _table_entry_size_shft;				\
  add		r1, r1,r2;						\
  addi		r2, r0, _table_state_offset;				\
  add		r1, r1, r2;						\
  phvwri	p.table_sel, _table_sel;			        \
  phvwri	p.table_mpu_entry_raw, _stage_entry;		        \
  phvwr		p.table_addr, r1;					\
  phvwri	p.table_addr[63], 1;				        \
  phvwri	p.table_size, _table_read_size;

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

#define CAPRI_NEXT_TABLE0_READ(_fid, _lock_en, _stage_entry, _table_base, _table_entry_size_shft, _table_state_offset, _table_read_size) \
        phvwri  p.common_te0_phv_table_lock_en, 1; \
        phvwri  p.common_te0_phv_table_raw_table_size, _table_read_size; \
        addi    r2, r0, _stage_entry; \
        srl     r1, r2, CAPRI_MPU_PC_SHIFT; \
        phvwr   p.common_te0_phv_table_pc, r1; \
        CAPRI_GET_TABLE_ADDR_R(_fid, _table_base, _table_entry_size_shft, _table_state_offset) \
        phvwr   p.common_te0_phv_table_addr, r1; \
        phvwri  p.tcp_app_header_table0_valid, 1;


#define CAPRI_NEXT_TABLE1_READ(_fid, _lock_en, _stage_entry, _table_base, _table_entry_size_shft, _table_state_offset, _table_read_size) \
        phvwri  p.common_te1_phv_table_lock_en, 1; \
        phvwri  p.common_te1_phv_table_raw_table_size, _table_read_size; \
        addi    r2, r0, _stage_entry; \
        srl     r1, r2, CAPRI_MPU_PC_SHIFT; \
        phvwr   p.common_te1_phv_table_pc, r1; \
        CAPRI_GET_TABLE_ADDR_R(_fid, _table_base, _table_entry_size_shft, _table_state_offset) \
        phvwr   p.common_te1_phv_table_addr, r1; \
        phvwri  p.tcp_app_header_table1_valid, 1;

#define CAPRI_NEXT_TABLE2_READ(_fid, _lock_en, _stage_entry, _table_base, _table_entry_size_shft, _table_state_offset, _table_read_size) \
        phvwri  p.common_te2_phv_table_lock_en, 1; \
        phvwri  p.common_te2_phv_table_raw_table_size, _table_read_size; \
        addi    r2, r0, _stage_entry; \
        srl     r1, r2, CAPRI_MPU_PC_SHIFT; \
        phvwr   p.common_te2_phv_table_pc, r1; \
        CAPRI_GET_TABLE_ADDR_R(_fid, _table_base, _table_entry_size_shft, _table_state_offset) \
        phvwr   p.common_te2_phv_table_addr, r1; \
        phvwri  p.tcp_app_header_table2_valid, 1;

#define CAPRI_NEXT_TABLE3_READ(_fid, _lock_en, _stage_entry, _table_base, _table_entry_size_shft, _table_state_offset, _table_read_size) \
        phvwri  p.common_te3_phv_table_lock_en, 1; \
        phvwri  p.common_te3_phv_table_raw_table_size, _table_read_size; \
        addi    r2, r0, _stage_entry; \
        srl     r1, r2, CAPRI_MPU_PC_SHIFT; \
        phvwr   p.common_te3_phv_table_pc, r1; \
        CAPRI_GET_TABLE_ADDR_R(_fid, _table_base, _table_entry_size_shft, _table_state_offset) \
        phvwr   p.common_te3_phv_table_addr, r1; \
        phvwri  p.tcp_app_header_table3_valid, 1;

#define CAPRI_NEXT_IDX0_READ(_lock_en, _stage_entry, _table_base, _table_read_size) \
        CAPRI_NEXT_TABLE0_READ(0, _lock_en, _stage_entry, _table_base, 0, 0, _table_read_size) \

#define CAPRI_NEXT_IDX1_READ(_lock_en, _stage_entry, _table_base, _table_read_size) \
        CAPRI_NEXT_TABLE1_READ(0, _lock_en, _stage_entry, _table_base, 0, 0, _table_read_size) \

#define CAPRI_NEXT_IDX2_READ(_lock_en, _stage_entry, _table_base, _table_read_size) \
        CAPRI_NEXT_TABLE2_READ(0, _lock_en, _stage_entry, _table_base, 0, 0, _table_read_size) \

#define CAPRI_NEXT_IDX3_READ(_lock_en, _stage_entry, _table_base, _table_read_size) \
        CAPRI_NEXT_TABLE3_READ(0, _lock_en, _stage_entry, _table_base, 0, 0, _table_read_size) \

#define CAPRI_CLEAR_TABLE0_VALID \
        phvwri  p.tcp_app_header_table0_valid, 0;

#define CAPRI_CLEAR_TABLE1_VALID \
        phvwri  p.tcp_app_header_table1_valid, 0;

#define CAPRI_CLEAR_TABLE2_VALID \
        phvwri  p.tcp_app_header_table2_valid, 0;

#define CAPRI_CLEAR_TABLE3_VALID \
        phvwri  p.tcp_app_header_table3_valid, 0;

#define CAPRI_READ_ADDR(_addr, _table_type, _stage_entry) \
        add		r1, r0, _addr			                ;\
        phvwri		p.table_sel, _table_type                        ;\
        phvwri		p.table_mpu_entry_raw, _stage_entry             ;\
	phvwr		p.table_addr, r1                                ;\
        phvwri		p.table_addr[63], 1                             ;\
        phvwri	        p.table_size, 8                                 ;


#define DB_ADDR_BASE                   0xc0000000
#define DB_UPD_SHFT                    16
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
       add             r4, r5, r0

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

#define TIMER_START_ARRAY_BASE         0xab000000
#define TIMER_START_ARRAY_LIF_SHIFT    12
#define TIMER_START_WIDTH_BITS         40
#define TIMER_START_TYPE_SHIFT         37
#define TIMER_START_QID_SHIFT          13
#define TIMER_START_RING_SHIFT         10

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
        
#define CAPRI_PHV_START_OFFSET(_start) \
        (sizeof (p) >> 3) - ((offsetof(p, _start) + sizeof(p._start)) >> 3);

#define CAPRI_PHV_END_OFFSET(_end) \
        (sizeof(p) >> 3) - (offsetof(p, _end) >> 3); 

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
        
#endif /* #ifndef __CAPRI_MACROS_H__ */
