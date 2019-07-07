// NVME command definition
header_type nvme_sqe_t {
  fields {
    // NVME command Dword 0
    opc		: 8;	// Opcode
    fuse	: 2;	// Fusing 2 simple commands
    rsvd0	: 4; 
    psdt	: 2;	// PRP or SGL
    cid		: 16;	// Command identifier
  
    // NVME command Dword 1
    nsid	: 32;	// Namespace identifier

    // NVME command Dword 2
    rsvd2	: 32;

    // NVME command Dword 3
    rsvd3	: 32;

    // NVME command Dwords 4 and 5 
    mptr	: 64;	// Metadata pointer

    // NVME command Dwords 6,7,8 & 9 form the data pointer (PRP or SGL)
    prp1	: 64;	// PRP1 or address of SGL
    prp2	: 64;	// PRP2 or size/type/sub_type in SGL

    // NVME command Dwords 10 and 11 
    slba	: 64;	// Starting LBA (for Read/Write) commands

    // NVME command Dword 12
    nlb		: 16;	// Number of logical blocks
    rsvd12	: 10;	
    prinfo	: 4;	// Protection information field
    fua		: 1;	// Force unit access
    lr		: 1;	// Limited retry
 
    // NVME command Dword 13
    dsm		: 8;	// Dataset management
    rsvd13	: 24;

    // NVME command Dword 14
    dw14	: 32;

    // NVME command Dword 15
    dw15	: 32;
  }
}

#define NVME_SQE_PARAMS \
opc, fuse, rsvd0, psdt, cid, nsid, rsvd2, rsvd3, mptr, prp1, prp2, slba, \
nlb, rsvd12, prinfo, fua, lr, dsm, rsvd13, dw14, dw15

#define GENERATE_NVME_SQE_D \
    modify_field(nvme_sqe_d.opc, opc);\
    modify_field(nvme_sqe_d.fuse, fuse);\
    modify_field(nvme_sqe_d.rsvd0, rsvd0);\
    modify_field(nvme_sqe_d.psdt, psdt);\
    modify_field(nvme_sqe_d.cid, cid);\
    modify_field(nvme_sqe_d.nsid, nsid);\
    modify_field(nvme_sqe_d.rsvd2, rsvd2);\
    modify_field(nvme_sqe_d.rsvd3, rsvd3);\
    modify_field(nvme_sqe_d.mptr, mptr);\
    modify_field(nvme_sqe_d.prp1, prp1);\
    modify_field(nvme_sqe_d.prp2, prp2);\
    modify_field(nvme_sqe_d.slba, slba);\
    modify_field(nvme_sqe_d.nlb, nlb);\
    modify_field(nvme_sqe_d.rsvd12, rsvd12);\
    modify_field(nvme_sqe_d.prinfo, prinfo);\
    modify_field(nvme_sqe_d.fua, fua);\
    modify_field(nvme_sqe_d.lr, lr);\
    modify_field(nvme_sqe_d.dsm, dsm);\
    modify_field(nvme_sqe_d.rsvd13, rsvd13);\
    modify_field(nvme_sqe_d.dw14, dw14);\
    modify_field(nvme_sqe_d.dw15, dw15);

// Completion Queue - cqcb 
// 64B
header_type cqcb_t {
    fields {
        pc                             : 8;
        // 7 Bytes intrinsic header
        CAPRI_QSTATE_HEADER_COMMON
        // 4 Bytes SQ ring
        CAPRI_QSTATE_HEADER_RING(0)
        /* 12 Bytes/96 bits Fixed header */

        // 2B
        proxy_pindex                    : 16;

        // 10B
        cq_base_addr                    : 64;
        log_num_wqes                    : 5;
        log_wqe_size                    : 5;
        rsvd0                           : 6;

        // 1B
        phase_tag                       : 1;
        rsvd1                           : 7; 
    
        // Bytes
        pad                             : 312;
    }
}

#define CQCB_PARAMS                                                      \
rsvd, cosA, cosB, cos_sel, eval_last, host, total, pid, pi_0, ci_0,      \
proxy_pindex, cq_base_addr, log_num_wqes, log_wqe_size, rsvd0,           \
phase_tag, rsvd1, pad
    

#define GENERATE_CQCB_D                                           \
    modify_field(cqcb_d.rsvd, rsvd);                              \
    modify_field(cqcb_d.cosA, cosA);                              \
    modify_field(cqcb_d.cosB, cosB);                              \
    modify_field(cqcb_d.cos_sel, cos_sel);                        \
    modify_field(cqcb_d.eval_last, eval_last);                    \
    modify_field(cqcb_d.host, host);                              \
    modify_field(cqcb_d.total, total);                            \
    modify_field(cqcb_d.pid, pid);                                \
    modify_field(cqcb_d.pi_0, pi_0);                              \
    modify_field(cqcb_d.ci_0, ci_0);                              \
    modify_field(cqcb_d.proxy_pindex, proxy_pindex);              \
    modify_field(cqcb_d.cq_base_addr, cq_base_addr);              \
    modify_field(cqcb_d.log_num_wqes, log_num_wqes);              \
    modify_field(cqcb_d.log_wqe_size, log_wqe_size);              \
    modify_field(cqcb_d.rsvd0, rsvd0);                            \
    modify_field(cqcb_d.phase_tag, phase_tag);                    \
    modify_field(cqcb_d.rsvd1, rsvd1);                            \
    modify_field(cqcb_d.pad, pad);                                \

#define CQCB_PARAMS_NON_STG0                                    \
    pc, CQCB_PARAMS

#define GENERATE_CQCB_D_NON_STG0                                \
    modify_field(cqcb_d.pc, pc);                                \
    GENERATE_CQCB_D

// NVME CQE definition
header_type nvme_cqe_t {
  fields {
    // NVME status Dword 0
    cspec	: 32;	// Command specific

    // NVME status Dword 1
    rsvd	: 32;

    // NVME status Dword 2
    sq_head	: 16;	// Submission queue head pointer
    sq_id	: 16;	// Submission queue identifier
	
    // NVME status Dword 3
    cid		: 16;	// Command identifier
    phase	: 1;	// Phase bit
    status	: 15;	// Status
  }
}

#define NVME_CQE_PARAMS \
    cspec, rsvd, sq_head, sq_id, cid, phase, status

#define GENERATE_NVME_CQE_D \
    modify_field(nvme_cqe_d.cspec, cspec); \
    modify_field(nvme_cqe_d.rsvd, rsvd); \
    modify_field(nvme_cqe_d.sq_head, sq_head); \
    modify_field(nvme_cqe_d.sq_id, sq_id); \
    modify_field(nvme_cqe_d.cid, cid); \
    modify_field(nvme_cqe_d.phase, phase); \
    modify_field(nvme_cqe_d.status, status);

// sqcb for stage 0
header_type sqcb_t {
    fields {
        pc                             : 8;
        // 7 Bytes intrinsic header
        CAPRI_QSTATE_HEADER_COMMON
        // 4 Bytes SQ ring
        CAPRI_QSTATE_HEADER_RING(0)
        /* 12 Bytes/96 bits Fixed header */

        sq_base_addr                    : 64;

        log_host_page_size              : 5;
        log_wqe_size                    : 5;
        log_num_wqes                    : 5;
        ring_empty_sched_eval_done      : 1;

        busy                            : 1;
        rsvd0                           : 7;
        
        cq_id                           : 16;
        lif_ns_start                    : 16;

        //37 Bytes
        pad                             : 296;
    }
}

#define SQCB_PARAMS                                                                                   \
rsvd, cosA, cosB, cos_sel, eval_last, host, total, pid, pi_0, ci_0, \
sq_base_addr, log_host_page_size, log_wqe_size, log_num_wqes, \
ring_empty_sched_eval_done, busy, rsvd0, cq_id, lif_ns_start, pad

#define GENERATE_SQCB_D                                           \
    modify_field(sqcb_d.rsvd, rsvd);                              \
    modify_field(sqcb_d.cosA, cosA);                              \
    modify_field(sqcb_d.cosB, cosB);                              \
    modify_field(sqcb_d.cos_sel, cos_sel);                        \
    modify_field(sqcb_d.eval_last, eval_last);                    \
    modify_field(sqcb_d.host, host);                              \
    modify_field(sqcb_d.total, total);                            \
    modify_field(sqcb_d.pid, pid);                                \
    modify_field(sqcb_d.pi_0, pi_0);                              \
    modify_field(sqcb_d.ci_0, ci_0);                              \
    modify_field(sqcb_d.sq_base_addr, sq_base_addr);              \
    modify_field(sqcb_d.log_host_page_size, log_host_page_size);        \
    modify_field(sqcb_d.log_wqe_size, log_wqe_size);              \
    modify_field(sqcb_d.log_num_wqes, log_num_wqes);              \
    modify_field(sqcb_d.ring_empty_sched_eval_done, ring_empty_sched_eval_done);\
    modify_field(sqcb_d.busy, busy);                              \
    modify_field(sqcb_d.rsvd0, rsvd0);                            \
    modify_field(sqcb_d.cq_id, cq_id);                            \
    modify_field(sqcb_d.lif_ns_start, lif_ns_start);              \
    modify_field(sqcb_d.pad, pad);                                \

#define SQCB_PARAMS_NON_STG0 \
    pc, SQCB_PARAMS

#define GENERATE_SQCB_D_NON_STG0                                \
    modify_field(sqcb_d.pc, pc);                                \
    GENERATE_SQCB_D

// nscb for stage 0
header_type nscb_t {
    fields {
        ns_size                         : 64; //in LBAs
        key_index                       : 32;
        sec_key_index                   : 32;
        ns_valid                        : 1;
        ns_active                       : 1;
        rsvd0                           : 1;
        log_lba_size                    : 5;

        //Backend Info
        backend_ns_id                   : 32;
        
        //Session Info
        num_sessions                    : 11; //1-based
        rr_session_id_to_be_served      : 10; //0-based

        // Stats/Accounting
        num_outstanding_req             : 11; //1-based

        sess_prodcb_start               : 16;

        pad                             : 40;

        valid_session_bitmap            : 256;
    }
}

#define NSCB_PARAMS                                                      \
ns_size, key_index, sec_key_index, ns_valid, ns_active, rsvd0, log_lba_size, \
backend_ns_id, num_sessions, rr_session_id_to_be_served,\
num_outstanding_req, sess_prodcb_start, pad, \
valid_session_bitmap

#define GENERATE_NSCB_D                                                  \
    modify_field(nscb_d.ns_size, ns_size);                               \
    modify_field(nscb_d.key_index, key_index);                           \
    modify_field(nscb_d.sec_key_index, sec_key_index);                   \
    modify_field(nscb_d.ns_valid, ns_valid);                             \
    modify_field(nscb_d.ns_active, ns_active);                           \
    modify_field(nscb_d.rsvd0, rsvd0);                                   \
    modify_field(nscb_d.log_lba_size, log_lba_size);                     \
    modify_field(nscb_d.backend_ns_id, backend_ns_id);                   \
    modify_field(nscb_d.num_sessions, num_sessions);                     \
    modify_field(nscb_d.rr_session_id_to_be_served, rr_session_id_to_be_served);\
    modify_field(nscb_d.num_outstanding_req, num_outstanding_req);       \
    modify_field(nscb_d.sess_prodcb_start, sess_prodcb_start); \
    modify_field(nscb_d.pad, pad);                                       \
    modify_field(nscb_d.valid_session_bitmap, valid_session_bitmap);

#define SQE_PRP_LIST_PARAMS \
ptr

#define GENERATE_SQE_PRP_LIST_D                                         \
    modify_field(sqe_prp_list_d.ptr, ptr);

// session producer tx cb
header_type sessprodtxcb_t {
    fields {
        xts_q_base_addr                 : 34;
        log_num_xts_q_entries           : 5;
        rsvd0                           : 1;
        xts_q_choke_counter             : 8;
        xts_qid                         : 16;

        dgst_q_base_addr                : 34;
        log_num_dgst_q_entries          : 5;
        rsvd2                           : 1; 
        dgst_q_choke_counter            : 8;
        dgst_qid                        : 16;

        tcp_q_base_addr                 : 34;
        log_num_tcp_q_entries           : 5;
        rsvd4                           : 1; 
        tcp_q_choke_counter             : 8;
        rsvd5                           : 16;

        xts_q_pi                        : 16;
        xts_q_ci                        : 16;

        dgst_q_pi                       : 16;
        dgst_q_ci                       : 16;


        //16 Bytes
        pad                             : 128;

        tcpcb_sesq_db_addr              : 64;
        tcpcb_sesq_db_data              : 32;

        tcp_q_pi                        : 16;
        //Keep this as the last field - as we need to pass its address elsewhere
        tcp_q_ci                        : 16;
    }
}

#define SESSPRODTXCB_PARAMS                                                    \
xts_q_base_addr, log_num_xts_q_entries, rsvd0, xts_q_choke_counter, xts_qid,   \
dgst_q_base_addr, log_num_dgst_q_entries, rsvd2, dgst_q_choke_counter, dgst_qid,  \
tcp_q_base_addr, log_num_tcp_q_entries, rsvd4, tcp_q_choke_counter, rsvd5,     \
xts_q_pi, xts_q_ci,                                                            \
dgst_q_pi, dgst_q_ci,                                                          \
pad,                                                                           \
tcpcb_sesq_db_addr, tcpcb_sesq_db_data,                                        \
tcp_q_pi, tcp_q_ci


#define GENERATE_SESSPRODTXCB_D                                                  \
    modify_field(sessprodtxcb_d.xts_q_base_addr, xts_q_base_addr);               \
    modify_field(sessprodtxcb_d.log_num_xts_q_entries, log_num_xts_q_entries);   \
    modify_field(sessprodtxcb_d.rsvd0, rsvd0);                                   \
    modify_field(sessprodtxcb_d.xts_q_choke_counter, xts_q_choke_counter);       \
    modify_field(sessprodtxcb_d.xts_qid, xts_qid);                               \
    modify_field(sessprodtxcb_d.dgst_q_base_addr, dgst_q_base_addr);             \
    modify_field(sessprodtxcb_d.log_num_dgst_q_entries, log_num_dgst_q_entries); \
    modify_field(sessprodtxcb_d.rsvd2, rsvd2);                                   \
    modify_field(sessprodtxcb_d.dgst_q_choke_counter, dgst_q_choke_counter);     \
    modify_field(sessprodtxcb_d.dgst_qid, dgst_qid);                             \
    modify_field(sessprodtxcb_d.tcp_q_base_addr, tcp_q_base_addr);               \
    modify_field(sessprodtxcb_d.log_num_tcp_q_entries, log_num_tcp_q_entries);   \
    modify_field(sessprodtxcb_d.rsvd4, rsvd4);                                   \
    modify_field(sessprodtxcb_d.tcp_q_choke_counter, tcp_q_choke_counter);       \
    modify_field(sessprodtxcb_d.rsvd5, rsvd5);                                   \
    modify_field(sessprodtxcb_d.xts_q_pi, xts_q_pi);                             \
    modify_field(sessprodtxcb_d.xts_q_ci, xts_q_ci);                             \
    modify_field(sessprodtxcb_d.dgst_q_pi, dgst_q_pi);                           \
    modify_field(sessprodtxcb_d.dgst_q_ci, dgst_q_ci);                           \
    modify_field(sessprodtxcb_d.pad, pad);                                       \
    modify_field(sessprodtxcb_d.tcpcb_sesq_db_addr, tcpcb_sesq_db_addr);         \
    modify_field(sessprodtxcb_d.tcpcb_sesq_db_data, tcpcb_sesq_db_data);         \
    modify_field(sessprodtxcb_d.tcp_q_pi, tcp_q_pi);                             \
    modify_field(sessprodtxcb_d.tcp_q_ci, tcp_q_ci);                             \

// session xts tx cb
// 64B
header_type sessxtstxcb_t {
    fields {
        //16B
        pc                             : 8;
        // 7 Bytes intrinsic header
        CAPRI_QSTATE_HEADER_COMMON
        // 4 Bytes Ring 0
        CAPRI_QSTATE_HEADER_RING(0)
        // 4 Bytes Ring 1
        CAPRI_QSTATE_HEADER_RING(1)

        //6B
        base_addr                       : 34;
        log_num_entries                 : 5;
        ring_empty_sched_eval_done      : 1;
        rsvd0                           : 8;
        
        //R0 stage0 flags
        //1B
        r0_busy                         : 1;
        rsvd1                           : 7;

        //R0 writeback stage flags
        //1B
        wb_r0_busy                      : 1;
        rsvd2                           : 7;

        //2B
        nxt_lba_offset                  : 16;

        //R1 stage0 flags
        //1B
        r1_busy                         : 1;
        rsvd3                           : 7;

        //R1 writeback stage flags
        //1B
        wb_r1_busy                      : 1;
        rsvd4                           : 7;

        //8B
        page_ptr                        : 64;

        //2B
        session_id                      : 16;

        //26B
        pad                             : 208;
    }
}

#define SESSXTSTXCB_PARAMS                                                      \
rsvd, cosA, cosB, cos_sel, eval_last, host, total, pid, pi_0, ci_0, pi_1, ci_1, \
base_addr, log_num_entries, ring_empty_sched_eval_done, \
rsvd0, r0_busy, rsvd1, \
wb_r0_busy, rsvd2, nxt_lba_offset, \
r1_busy, rsvd3, wb_r1_busy, rsvd4,  \
page_ptr, session_id, pad

#define GENERATE_SESSXTSTXCB_D  \
    modify_field(sessxtstxcb_d.rsvd, rsvd);                              \
    modify_field(sessxtstxcb_d.cosA, cosA);                              \
    modify_field(sessxtstxcb_d.cosB, cosB);                              \
    modify_field(sessxtstxcb_d.cos_sel, cos_sel);                        \
    modify_field(sessxtstxcb_d.eval_last, eval_last);                    \
    modify_field(sessxtstxcb_d.host, host);                              \
    modify_field(sessxtstxcb_d.total, total);                            \
    modify_field(sessxtstxcb_d.pid, pid);                                \
    modify_field(sessxtstxcb_d.pi_0, pi_0);                              \
    modify_field(sessxtstxcb_d.ci_0, ci_0);                              \
    modify_field(sessxtstxcb_d.pi_1, pi_1);                              \
    modify_field(sessxtstxcb_d.ci_1, ci_1);                              \
    modify_field(sessxtstxcb_d.base_addr, base_addr);  \
    modify_field(sessxtstxcb_d.log_num_entries, log_num_entries);  \
    modify_field(sessxtstxcb_d.ring_empty_sched_eval_done, ring_empty_sched_eval_done);  \
    modify_field(sessxtstxcb_d.rsvd0, rsvd0);  \
    modify_field(sessxtstxcb_d.r0_busy, r0_busy);  \
    modify_field(sessxtstxcb_d.rsvd1, rsvd1);  \
    modify_field(sessxtstxcb_d.wb_r0_busy, wb_r0_busy);  \
    modify_field(sessxtstxcb_d.rsvd2, rsvd2);  \
    modify_field(sessxtstxcb_d.nxt_lba_offset, nxt_lba_offset);  \
    modify_field(sessxtstxcb_d.r1_busy, r1_busy);  \
    modify_field(sessxtstxcb_d.rsvd3, rsvd3);  \
    modify_field(sessxtstxcb_d.wb_r1_busy, wb_r1_busy);  \
    modify_field(sessxtstxcb_d.rsvd4, rsvd4);  \
    modify_field(sessxtstxcb_d.page_ptr, page_ptr);  \
    modify_field(sessxtstxcb_d.session_id, session_id);  \
    modify_field(sessxtstxcb_d.pad, pad);  \

#define SESSXTSTXCB_PARAMS_NON_STG0 \
    pc, SESSXTSTXCB_PARAMS

#define GENERATE_SESSXTSTXCB_D_NON_STG0                                \
    modify_field(sessxtstxcb_d.pc, pc);                                \
    GENERATE_SESSXTSTXCB_D

// session dgst tx cb
// 64B
header_type sessdgsttxcb_t {
    fields {
        //16B
        pc                             : 8;
        // 7 Bytes intrinsic header
        CAPRI_QSTATE_HEADER_COMMON
        // 4 Bytes Ring 0
        CAPRI_QSTATE_HEADER_RING(0)
        // 4 Bytes Ring 1
        CAPRI_QSTATE_HEADER_RING(1)

        //8B
        rx_q_base_addr                  : 34;
        rx_q_log_num_entries            : 5;
        rsvd5                           : 25;
        
        //6B
        base_addr                       : 34;
        log_num_entries                 : 5;
        ring_empty_sched_eval_done      : 1;
        rsvd0                           : 8;
        
        //R0 stage0 flags
        //1B
        r0_busy                         : 1;
        rsvd1                           : 7;

        //R0 writeback stage flags
        //1B
        wb_r0_busy                      : 1;
        rsvd2                           : 7;

        //R1 stage0 flags
        //1B
        r1_busy                         : 1;
        rsvd3                           : 7;

        //R1 writeback stage flags
        //1B
        wb_r1_busy                      : 1;
        rsvd4                           : 7;

        //2B
        session_id                      : 16;

        //28B
        pad                             : 224;
    }
}

#define SESSDGSTTXCB_PARAMS                                                      \
rsvd, cosA, cosB, cos_sel, eval_last, host, total, pid, pi_0, ci_0, pi_1, ci_1, \
rx_q_base_addr, rx_q_log_num_entries, rsvd5, \
base_addr, log_num_entries, ring_empty_sched_eval_done, rsvd0, r0_busy, rsvd1, \
wb_r0_busy, rsvd2, r1_busy, rsvd3, wb_r1_busy, rsvd4, session_id, pad

#define GENERATE_SESSDGSTTXCB_D  \
    modify_field(sessdgsttxcb_d.rsvd, rsvd);                              \
    modify_field(sessdgsttxcb_d.cosA, cosA);                              \
    modify_field(sessdgsttxcb_d.cosB, cosB);                              \
    modify_field(sessdgsttxcb_d.cos_sel, cos_sel);                        \
    modify_field(sessdgsttxcb_d.eval_last, eval_last);                    \
    modify_field(sessdgsttxcb_d.host, host);                              \
    modify_field(sessdgsttxcb_d.total, total);                            \
    modify_field(sessdgsttxcb_d.pid, pid);                                \
    modify_field(sessdgsttxcb_d.pi_0, pi_0);                              \
    modify_field(sessdgsttxcb_d.ci_0, ci_0);                              \
    modify_field(sessdgsttxcb_d.pi_1, pi_1);                              \
    modify_field(sessdgsttxcb_d.ci_1, ci_1);                              \
    modify_field(sessdgsttxcb_d.rx_q_base_addr, rx_q_base_addr);  \
    modify_field(sessdgsttxcb_d.rx_q_log_num_entries, rx_q_log_num_entries);  \
    modify_field(sessdgsttxcb_d.rsvd5, rsvd5);  \
    modify_field(sessdgsttxcb_d.base_addr, base_addr);  \
    modify_field(sessdgsttxcb_d.log_num_entries, log_num_entries);  \
    modify_field(sessdgsttxcb_d.ring_empty_sched_eval_done, ring_empty_sched_eval_done);  \
    modify_field(sessdgsttxcb_d.rsvd0, rsvd0);  \
    modify_field(sessdgsttxcb_d.r0_busy, r0_busy);  \
    modify_field(sessdgsttxcb_d.rsvd1, rsvd1);  \
    modify_field(sessdgsttxcb_d.wb_r0_busy, wb_r0_busy);  \
    modify_field(sessdgsttxcb_d.rsvd2, rsvd2);  \
    modify_field(sessdgsttxcb_d.r1_busy, r1_busy);  \
    modify_field(sessdgsttxcb_d.rsvd3, rsvd3);  \
    modify_field(sessdgsttxcb_d.wb_r1_busy, wb_r1_busy);  \
    modify_field(sessdgsttxcb_d.rsvd4, rsvd4);  \
    modify_field(sessdgsttxcb_d.session_id, session_id);  \
    modify_field(sessdgsttxcb_d.pad, pad);  \

#define SESSDGSTTXCB_PARAMS_NON_STG0 \
    pc, SESSDGSTTXCB_PARAMS

#define GENERATE_SESSDGSTTXCB_D_NON_STG0                       \
    modify_field(sessdgsttxcb_d.pc, pc);                       \
    GENERATE_SESSDGSTTXCB_D

// resource cb
header_type resourcecb_t {
    fields {
        // ring of free rx pduids
        rx_pduid_ring_pi                : 16;
        rx_pduid_ring_proxy_pi          : 16;
        rx_pduid_ring_proxy_ci          : 16;
        rx_pduid_ring_ci                : 16;
        rx_pduid_ring_log_sz            :  5;
        rx_pduid_ring_rsvd              :  3;
        rx_pduid_ring_choke_counter     :  8;

        // ring of free tx pduids 
        tx_pduid_ring_pi                : 16;
        tx_pduid_ring_proxy_pi          : 16;
        tx_pduid_ring_proxy_ci          : 16;
        tx_pduid_ring_ci                : 16;
        tx_pduid_ring_log_sz            :  5;
        tx_pduid_ring_rsvd              :  3;
        tx_pduid_ring_choke_counter     :  8;

        // ring of free cmdids
        cmdid_ring_pi                   : 16;
        cmdid_ring_proxy_pi             : 16;
        cmdid_ring_proxy_ci             : 16;
        cmdid_ring_ci                   : 16;
        cmdid_ring_log_sz               :  5;
        cmdid_ring_rsvd                 :  3;
        cmdid_ring_choke_counter        :  8;

        //40 Bytes
        pad                             : 272;
    }
}

#define RESOURCECB_PARAMS                                                      \
rx_pduid_ring_pi, rx_pduid_ring_proxy_pi, rx_pduid_ring_proxy_ci, rx_pduid_ring_ci, \
rx_pduid_ring_log_sz, rx_pduid_ring_rsvd, rx_pduid_ring_choke_counter, \
tx_pduid_ring_pi, tx_pduid_ring_proxy_pi, tx_pduid_ring_proxy_ci, tx_pduid_ring_ci, \
tx_pduid_ring_log_sz, tx_pduid_ring_rsvd, tx_pduid_ring_choke_counter, \
cmdid_ring_pi, cmdid_ring_proxy_pi, cmdid_ring_proxy_ci, cmdid_ring_ci, \
cmdid_ring_log_sz, cmdid_ring_rsvd, cmdid_ring_choke_counter, pad


#define GENERATE_RESOURCECB_D                                                   \
    modify_field(resourcecb_d.rx_pduid_ring_pi, rx_pduid_ring_pi);                    \
    modify_field(resourcecb_d.rx_pduid_ring_proxy_pi, rx_pduid_ring_proxy_pi);        \
    modify_field(resourcecb_d.rx_pduid_ring_proxy_ci, rx_pduid_ring_proxy_ci);        \
    modify_field(resourcecb_d.rx_pduid_ring_ci, rx_pduid_ring_ci);                    \
    modify_field(resourcecb_d.rx_pduid_ring_log_sz, rx_pduid_ring_log_sz);            \
    modify_field(resourcecb_d.rx_pduid_ring_rsvd, rx_pduid_ring_rsvd);                \
    modify_field(resourcecb_d.rx_pduid_ring_choke_counter, rx_pduid_ring_choke_counter);\
    modify_field(resourcecb_d.tx_pduid_ring_pi, tx_pduid_ring_pi);                    \
    modify_field(resourcecb_d.tx_pduid_ring_proxy_pi, tx_pduid_ring_proxy_pi);        \
    modify_field(resourcecb_d.tx_pduid_ring_proxy_ci, tx_pduid_ring_proxy_ci);        \
    modify_field(resourcecb_d.tx_pduid_ring_ci, tx_pduid_ring_ci);                    \
    modify_field(resourcecb_d.tx_pduid_ring_log_sz, tx_pduid_ring_log_sz);            \
    modify_field(resourcecb_d.tx_pduid_ring_rsvd, tx_pduid_ring_rsvd);                \
    modify_field(resourcecb_d.tx_pduid_ring_choke_counter, tx_pduid_ring_choke_counter);\
    modify_field(resourcecb_d.cmdid_ring_pi, cmdid_ring_pi);                    \
    modify_field(resourcecb_d.cmdid_ring_proxy_pi, cmdid_ring_proxy_pi);        \
    modify_field(resourcecb_d.cmdid_ring_proxy_ci, cmdid_ring_proxy_ci);        \
    modify_field(resourcecb_d.cmdid_ring_ci, cmdid_ring_ci);                    \
    modify_field(resourcecb_d.cmdid_ring_log_sz, cmdid_ring_log_sz);            \
    modify_field(resourcecb_d.cmdid_ring_rsvd, cmdid_ring_rsvd);                \
    modify_field(resourcecb_d.cmdid_ring_choke_counter, cmdid_ring_choke_counter);\
    modify_field(resourcecb_d.pad, pad);                                        \

header_type cmdid_ring_entry_t {
    fields {
        cmdid                           : 16;
    }
}

#define CMDID_RING_ENTRY_PARAMS \
    cmdid

#define GENERATE_CMDID_RING_ENTRY_D                                             \
    modify_field(cmdid_ring_entry_d.cmdid, cmdid);

header_type pduid_ring_entry_t {
    fields {
        pduid                           : 16;
    }
}

#define PDUID_RING_ENTRY_PARAMS \
    pduid

#define GENERATE_PDUID_RING_ENTRY_D                                             \
    modify_field(pduid_ring_entry_d.pduid, pduid);

header_type c2h_data_t {
    fields {
        rsvd                            : 512;
    }
}

#define C2H_DATA_PARAMS \
    rsvd

#define GENERATE_C2H_DATA_D \
    modify_field(c2h_data_d.rsvd, rsvd);


// SQ stats cb
header_type sq_statscb_t {
    fields {
        num_read_req                    : 32;
        num_write_req                   : 32;
        num_read_lbas                   : 64;
        num_write_lbas                  : 64;

        //40 Bytes
        pad                             : 320;
    }
}

#define SQ_STATSCB_PARAMS                                                      \
num_read_req, num_write_req, num_read_lbas, num_write_lbas,                    \
pad

#define GENERATE_SQ_STATSCB_D                                                  \
    modify_field(sq_statscb_d.num_read_req, num_read_req);                     \
    modify_field(sq_statscb_d.num_write_req, num_write_req);                   \
    modify_field(sq_statscb_d.num_read_lbas, num_read_lbas);                   \
    modify_field(sq_statscb_d.num_write_lbas, num_write_lbas);                 \
    modify_field(sq_statscb_d.pad, pad);                                       \

//64B
header_type cmd_context_t {
  fields {
    // NVME command Dword 0
    opc		: 8;	// Opcode
    fuse	: 2;	// Fusing 2 simple commands
    rsvd0	: 4; 
    psdt	: 2;	// PRP or SGL
    cid		: 16;	// Command identifier
  
    // NVME command Dword 1
    nsid	: 32;	// Namespace identifier

    // NVME command Dwords 10 and 11 
    slba	: 64;	// Starting LBA (for Read/Write) commands

    // NVME command Dword 12
    nlb		: 16;	// Number of logical blocks
    rsvd1   : 16;

    // backend info
    // when a response is received, lif/session_id/state etc. variables are used to 
    // cross check whether the received response is sane.
    // sq_id is used to retrieve the head_pointer and also associated cq_id of this sq 
    // so that completions can be posted.
    log_lba_size    :  5;
    lif             : 11;
    sq_id           : 24;
    session_id      : 16;    

    num_prps        :  8;
    num_pages       :  8;
    num_aols        :  8;
    state           :  8;

    log_host_page_size  :   5;
    rsvd2               :   3;

    // these pointers would eventually keep track of where we left off 
    // in the previous PDU
    prp1                :   64;
    prp2                :   64;

    prp1_offset         :   16;
    rsvd3               :   16;
    key_index           :   32;
    sec_key_index       :   32;
    pad                 :   32;
  }
}

#define CMD_CTXT_PARAMS \
opc, fuse, rsvd0, psdt, cid, nsid, slba, nlb, rsvd1, \
log_lba_size, lif, sq_id, session_id, num_prps, num_pages, num_aols, state, \
log_host_page_size, rsvd2, prp1, prp2, \
prp1_offset, rsvd3, key_index, sec_key_index, pad

#define GENERATE_CMD_CTXT_D \
    modify_field(cmd_ctxt_d.opc, opc); \
    modify_field(cmd_ctxt_d.fuse, fuse); \
    modify_field(cmd_ctxt_d.rsvd0, rsvd0); \
    modify_field(cmd_ctxt_d.psdt, psdt); \
    modify_field(cmd_ctxt_d.cid, cid); \
    modify_field(cmd_ctxt_d.nsid, nsid); \
    modify_field(cmd_ctxt_d.slba, slba); \
    modify_field(cmd_ctxt_d.nlb, nlb); \
    modify_field(cmd_ctxt_d.rsvd1, rsvd1); \
    modify_field(cmd_ctxt_d.log_lba_size, log_lba_size); \
    modify_field(cmd_ctxt_d.lif, lif); \
    modify_field(cmd_ctxt_d.sq_id, sq_id); \
    modify_field(cmd_ctxt_d.session_id, session_id); \
    modify_field(cmd_ctxt_d.num_prps, num_prps); \
    modify_field(cmd_ctxt_d.num_pages, num_pages); \
    modify_field(cmd_ctxt_d.num_aols, num_aols); \
    modify_field(cmd_ctxt_d.state, state); \
    modify_field(cmd_ctxt_d.log_host_page_size, log_host_page_size); \
    modify_field(cmd_ctxt_d.rsvd2, rsvd2); \
    modify_field(cmd_ctxt_d.prp1, prp1); \
    modify_field(cmd_ctxt_d.prp2, prp2); \
    modify_field(cmd_ctxt_d.prp1_offset, prp1_offset); \
    modify_field(cmd_ctxt_d.rsvd3, rsvd3); \
    modify_field(cmd_ctxt_d.key_index, key_index); \
    modify_field(cmd_ctxt_d.sec_key_index, sec_key_index); \
    modify_field(cmd_ctxt_d.pad, pad); \

//64B
header_type pdu_context0_t {
  fields {
    cmd_opc	            :   8;	// Cmd Opcode
    pdu_opc             :   8;    // PDU Opcode
    cid	                :   16;	// Command identifier
    nsid                :   32;	// Namespace identifier

    slba                :   64;	// Starting LBA for this PDU
    nlb                 :   16;	// Number of logical blocks for this PDU

    log_lba_size        :   5;
    log_host_page_size  :   5;
    hdgst_en            :   1;
    ddgst_en            :   1;
    rsvd0               :   4;

    num_prps            :   8;
    num_pages           :   8;

    prp1_offset         :   16;

    key_index           :   32;
    sec_key_index       :   32;

    //30B
    session_id          :   16;
    pad                 :   240;
  }
}

#define PDU_CTXT0_PARAMS \
cmd_opc, pdu_opc, cid, nsid, slba, nlb, log_lba_size, log_host_page_size, hdgst_en, \
ddgst_en, rsvd0, num_prps, num_pages, prp1_offset, key_index, sec_key_index, \
session_id, pad

#define GENERATE_PDU_CTXT0_D \
    modify_field(pdu_ctxt0_d.cmd_opc, cmd_opc);\
    modify_field(pdu_ctxt0_d.pdu_opc, pdu_opc);\
    modify_field(pdu_ctxt0_d.cid, cid);\
    modify_field(pdu_ctxt0_d.nsid, nsid);\
    modify_field(pdu_ctxt0_d.slba, slba);\
    modify_field(pdu_ctxt0_d.nlb, nlb);\
    modify_field(pdu_ctxt0_d.log_lba_size, log_lba_size);\
    modify_field(pdu_ctxt0_d.log_host_page_size, log_host_page_size);\
    modify_field(pdu_ctxt0_d.hdgst_en, hdgst_en);\
    modify_field(pdu_ctxt0_d.ddgst_en, ddgst_en);\
    modify_field(pdu_ctxt0_d.rsvd0, rsvd0);\
    modify_field(pdu_ctxt0_d.num_prps, num_prps);\
    modify_field(pdu_ctxt0_d.num_pages, num_pages);\
    modify_field(pdu_ctxt0_d.prp1_offset, prp1_offset);\
    modify_field(pdu_ctxt0_d.key_index, key_index);\
    modify_field(pdu_ctxt0_d.sec_key_index, sec_key_index);\
    modify_field(pdu_ctxt0_d.session_id, session_id);\
    modify_field(pdu_ctxt0_d.pad, pad);\

//64B
header_type pdu_context1_t {
  fields {
    //8B
    //Common Header (CH)   
    pdu_type            : 8;
    flags               : 8;
    hlen                : 8;    //Header Length in bytes (HDGST not included)
    pdo                 : 8;    //PDU Data Offset in bytes (from start of PDU)
    plen                : 32;   //Total PDU Length (CH+PSH+HDGST+PAD+DATA+DDGST)

    //16B
    //PDU Specific Header (PSH)
    psh                 : 128;

    //4B
    rx_hdgst            : 32;   //Received Header Digest
    //4B
    rx_ddgst            : 32;   //Received Data Digest

    //Computed Fields
    //8B + 8B
    hdgst_status        : 64;
    hdgst               : 64;
    
    //8B + 8B
    ddgst_status        : 64;
    ddgst               : 64;
  }
}

#define PDU_CTXT1_PARAMS \
pdu_type, flags, hlen, pdo, plen, psh, rx_hdgst, rx_ddgst, \
hdgst_status, hdgst, ddgst_status, ddgst

#define GENERATE_PDU_CTXT1_D \
    modify_field(pdu_ctxt1_d.pdu_type, pdu_type);\
    modify_field(pdu_ctxt1_d.flags, flags);\
    modify_field(pdu_ctxt1_d.hlen, hlen);\
    modify_field(pdu_ctxt1_d.pdo, pdo);\
    modify_field(pdu_ctxt1_d.plen, plen);\
    modify_field(pdu_ctxt1_d.psh, psh);\
    modify_field(pdu_ctxt1_d.rx_hdgst, rx_hdgst);\
    modify_field(pdu_ctxt1_d.rx_ddgst, rx_ddgst);\
    modify_field(pdu_ctxt1_d.hdgst_status, hdgst_status);\
    modify_field(pdu_ctxt1_d.hdgst, hdgst);\
    modify_field(pdu_ctxt1_d.ddgst_status, ddgst_status);\
    modify_field(pdu_ctxt1_d.ddgst, ddgst);\

// wqe that gets posted into session XTS/DGST queues
header_type sess_wqe_t {
    fields {
        cmdid : 16;
        pduid : 16;
    }
}
#define SESS_WQE_PARAMS  \
cmdid, pduid

#define GENERATE_SESS_WQE_D \
    modify_field(sess_wqe_d.cmdid, cmdid); \
    modify_field(sess_wqe_d.pduid, pduid); \


// this is the cb that is used to track the barco xts engine ring.
// before an element is produced, pi and ci is checked to make sure there is space
// in the ring.
// whenever an element is produced, pi is incremented.
// when barco xts engine consumes an element, it uses opaque tag address/data to update
// the new ci value. 
// since opaque tag is used, this cb CANNOT be in p4+ cache enabled area.
// in case of a multi lba wqe, each lba of the wqe generates a new barco xts request.
// For each such request, upon completing, barco updates the ci using opaque tag.
// The last lba of a given wqe also enables doorbell so that barco can wakeup the 
// corresponding session q to move the wqe to next phase.
// since opaque tag data is 32 bits, to simplify the implementation, allocating 32 bit
// value for ci/pi.
// The ring's opaque tag dest address is pointed to the beginning of this structure.
// Hence keep the ci as the first field in this struct

//64B
header_type xtscb_t {
  fields {
    ci                  : 32; //Keep ci the first field(opaque tag write)
    pi                  : 32;
    xts_ring_base_addr  : 34;
    log_sz              :  5;
    rsvd                :  1;
    choke_counter       :  8;
    pad                 :400;
  }
}

#define XTSCB_PARAMS \
ci, pi, xts_ring_base_addr, log_sz, rsvd, choke_counter, pad

#define GENERATE_XTSCB_D    \
    modify_field(xtscb_d.ci, ci); \
    modify_field(xtscb_d.pi, pi); \
    modify_field(xtscb_d.xts_ring_base_addr, xts_ring_base_addr); \
    modify_field(xtscb_d.log_sz, log_sz); \
    modify_field(xtscb_d.rsvd, rsvd); \
    modify_field(xtscb_d.choke_counter, choke_counter); \
    modify_field(xtscb_d.pad, pad);

// this is the cb that is used to track the dgst accelerator engine ring.
// rest of the details are same as above.
// ci doesn't have to be the first one for this cb as the opaque tag address 
// is always supplied along with the data in the descriptor
// However, to keep it similar to xtscb, defining ci as the first one

//16B
header_type dgstcb_t {
  fields {
    ci                  : 32;
    pi                  : 32;
    dgst_ring_base_addr : 34;
    log_sz              :  5;
    rsvd                :  1;
    choke_counter       :  8;
    pad                 :400;
  }
}

#define DGSTCB_PARAMS \
ci, pi, dgst_ring_base_addr, log_sz, rsvd, choke_counter, pad

#define GENERATE_DGSTCB_D \
    modify_field(dgstcb_d.ci, ci); \
    modify_field(dgstcb_d.pi, pi); \
    modify_field(dgstcb_d.dgst_ring_base_addr, dgst_ring_base_addr); \
    modify_field(dgstcb_d.log_sz, log_sz); \
    modify_field(dgstcb_d.rsvd, rsvd); \
    modify_field(dgstcb_d.choke_counter, choke_counter); \
    modify_field(dgstcb_d.pad, pad); \

/* XTS Descriptor definition */
//128B
header_type xts_desc_t {
    fields {
        input_list_address                  : 64;
        output_list_address                 : 64;
        command                             : 32;
        key_desc_index                      : 32;
        iv_address                          : 64;
        auth_tag_addr                       : 64;
        header_size                         : 32;
        status_address                      : 64;
        opaque_tag_value                    : 32;
        flags                               : 32;
        sector_size                         : 16;
        application_tag                     : 16;
        sector_num                          : 32;
        doorbell_address                    : 64;
        doorbell_data                       : 64;
        second_key_desc_index               : 32;
        //rsvd2                               :320;
    }
}

//64B
header_type xts_aol_desc_t {
    fields {
        A0      : 64;
        O0      : 32;
        L0      : 32;
        A1      : 64;
        O1      : 32;
        L1      : 32;
        A2      : 64;
        O2      : 32;
        L2      : 32;
        nxt     : 64;
        rsvd    : 64;
    }
}

header_type xts_iv_t {
    fields {
        iv_0                                : 64;
        iv_1                                : 64;
    }
}

/* dgst Descriptor definition */
//64B
header_type dgst_desc_t {
    fields {
        src             : 64;
        dst             : 64;
        cmd             : 64;
        status_addr     : 64;
        doorbell_addr   : 64;
        doorbell_data   : 64;
        opaque_tag_addr : 64;
        opaque_tag_data : 32;
        status_data     : 32;
    }
}

//64B
header_type dgst_aol_desc_t {
    fields {
        A0      : 64;
        L0      : 32;
        R0      : 32;
        A1      : 64;
        L1      : 32;
        R1      : 32;
        A2      : 64;
        L2      : 32;
        R2      : 32;
        nxt     : 64;
        rsvd    : 64;
    }
}

//16B
header_type pkt_desc_one_aol_t {
    fields {
        A0 : 64;
        O0 : 32;
        L0 : 32;
    }
}

//16B
header_type dgst_one_aol_t {
    fields {
        A0      : 64;
        L0      : 32;
        R0      : 32;
    }
}

header_type ptr64_t {
    fields {
        ptr : 64;
    }
}

header_type ptr32_t {
    fields {
        ptr : 32;
    }
}

header_type index16_t {
  fields {
    index   : 16;
  }
}

header_type index32_t {
  fields {
    index   : 32;
  }
}

header_type data32_t {
    fields {
        data: 32;
    }
}

header_type data64_t {
    fields {
        data: 64;
    }
}

header_type nmdpr_t {
    fields {
        idx :   32;
        full:   1;
    }
}

#define NMDPR_PARAMS \
idx, full

#define GENERATE_NMDPR_D    \
    modify_field(nmdpr_d.idx, idx); \
    modify_field(nmdpr_d.full, full); \

header_type nmdpr_page_t {
    fields {
        page_ptr    : 64;
        pad         : 448;
    }
}

#define NMDPR_PAGE_PARAMS \
page_ptr, pad

#define GENERATE_NMDPR_PAGE_D   \
    modify_field(nmdpr_page_d.page_ptr, page_ptr); \
    modify_field(nmdpr_page_d.pad, pad); \

header_type prp_pair_t {
    fields {
        prp1    : 64;
        prp2    : 64;
        pad     :384;
    }
}

#define PRP_PAIR_PARAMS \
prp1, prp2, pad

#define GENERATE_PRP_PAIR_D     \
    modify_field(prp_pair_d.prp1, prp1); \
    modify_field(prp_pair_d.prp2, prp2); \
    modify_field(prp_pair_d.pad, pad); \

header_type page_list_t {
    fields {
         len0    : 16;
         page0   : 48;
         len1    : 16;
         page1   : 48;
         len2    : 16;
         page2   : 48;
         len3    : 16;
         page3   : 48;
         len4    : 16;
         page4   : 48;
         len5    : 16;
         page5   : 48;
         len6    : 16;
         page6   : 48;
         len7    : 16;
         page7   : 48;
    }
}

#define PAGE_LIST_PARAMS \
len0, page0, len1, page1, len2, page2, len3, page3, len4, page4, len5, page5, len6, page6, len7, page7

#define GENERATE_PAGE_LIST_D     \
    modify_field(page_list_d.len0, len0); \
    modify_field(page_list_d.page0, page0); \
    modify_field(page_list_d.len1, len1); \
    modify_field(page_list_d.page1, page1); \
    modify_field(page_list_d.len2, len2); \
    modify_field(page_list_d.page2, page2); \
    modify_field(page_list_d.len3, len3); \
    modify_field(page_list_d.page3, page3); \
    modify_field(page_list_d.len4, len4); \
    modify_field(page_list_d.page4, page4); \
    modify_field(page_list_d.len5, len5); \
    modify_field(page_list_d.page5, page5); \
    modify_field(page_list_d.len6, len6); \
    modify_field(page_list_d.page6, page6); \
    modify_field(page_list_d.len7, len7); \
    modify_field(page_list_d.page7, page7); \

// rqcb for stage 0
header_type rqcb_t {
    fields {
        pc                             : 8;
        // 7 Bytes intrinsic header
        CAPRI_QSTATE_HEADER_COMMON
        // 4 Bytes RQ ring
        CAPRI_QSTATE_HEADER_RING(0)
        /* 12 Bytes/96 bits Fixed header */

        //6B
        base_addr                       : 34;
        log_wqe_size                    : 5;
        log_num_entries                 : 5;
        ring_empty_sched_eval_done      : 1;
        rsvd0                           : 3;
        
        //R0 stage0 flags
        //1B
        r0_busy                         : 1;
        rsvd1                           : 7;

        //R0 writeback stage flags
        //1B
        wb_r0_busy                      : 1;
        resource_alloc_done             : 1;
        rsvd2                           : 6;

        //2B
        session_id                      : 16;
        //2B
        pduid                           : 16;
        //2B
        segment_offset                  : 16;
        //2B
        pdu_offset                      : 16;
        //4B
        curr_plen                       : 32;

        pad                             : 256;

    }
}

#define RQCB_PARAMS                                                                                   \
rsvd, cosA, cosB, cos_sel, eval_last, host, total, pid, pi_0, ci_0, \
base_addr, log_wqe_size, log_num_entries, ring_empty_sched_eval_done, rsvd0, \
r0_busy, rsvd1, wb_r0_busy, resource_alloc_done, rsvd2, session_id, pduid, \
segment_offset, pdu_offset, curr_plen, pad

#define GENERATE_RQCB_D                                           \
    modify_field(rqcb_d.rsvd, rsvd);                              \
    modify_field(rqcb_d.cosA, cosA);                              \
    modify_field(rqcb_d.cosB, cosB);                              \
    modify_field(rqcb_d.cos_sel, cos_sel);                        \
    modify_field(rqcb_d.eval_last, eval_last);                    \
    modify_field(rqcb_d.host, host);                              \
    modify_field(rqcb_d.total, total);                            \
    modify_field(rqcb_d.pid, pid);                                \
    modify_field(rqcb_d.pi_0, pi_0);                              \
    modify_field(rqcb_d.ci_0, ci_0);                              \
    modify_field(rqcb_d.base_addr, base_addr);                    \
    modify_field(rqcb_d.log_wqe_size, log_wqe_size);              \
    modify_field(rqcb_d.log_num_entries, log_num_entries);        \
    modify_field(rqcb_d.ring_empty_sched_eval_done, ring_empty_sched_eval_done);\
    modify_field(rqcb_d.rsvd0, rsvd0);                            \
    modify_field(rqcb_d.r0_busy, r0_busy);                        \
    modify_field(rqcb_d.rsvd1, rsvd1);                            \
    modify_field(rqcb_d.wb_r0_busy, wb_r0_busy);                  \
    modify_field(rqcb_d.resource_alloc_done, resource_alloc_done);\
    modify_field(rqcb_d.rsvd2, rsvd2);                            \
    modify_field(rqcb_d.session_id, session_id);                  \
    modify_field(rqcb_d.pduid, pduid);                            \
    modify_field(rqcb_d.segment_offset, segment_offset);          \
    modify_field(rqcb_d.pdu_offset, pdu_offset);                  \
    modify_field(rqcb_d.curr_plen, curr_plen);                    \
    modify_field(rqcb_d.pad, pad);                                \

#define RQCB_PARAMS_NON_STG0 \
    pc, RQCB_PARAMS

#define GENERATE_RQCB_D_NON_STG0                                \
    modify_field(rqcb_d.pc, pc);                                \
    GENERATE_RQCB_D

// Incr refcnt cb
header_type incr_refcnt_cb_t {
    fields {
        refcnt                          : 16;
        more_pdus                       :  1;

        //62 Bytes
        pad                             : 495;
    }
}

#define INCR_REFCNT_PARAMS                                                      \
refcnt, more_pdus, pad

#define GENERATE_INCR_REFCNT_D                          \
    modify_field(incr_refcnt_cb_d.refcnt, refcnt);      \
    modify_field(incr_refcnt_cb_d.more_pdus, more_pdus);      \
    modify_field(incr_refcnt_cb_d.pad, pad);             \

// RQ stats cb
header_type rq_statscb_t {
    fields {
        num_total_pdus                  : 64;
        num_respcap_pdus                : 64;
        num_c2hdata_pdus                : 64;
        num_r2t_pdus                    : 64;

        //32 Bytes
        pad                             : 256;
    }
}

#define RQ_STATSCB_PARAMS                                                      \
num_total_pdus, num_respcap_pdus, num_c2hdata_pdus, num_r2t_pdus, pad

#define GENERATE_RQ_STATSCB_D                                                  \
    modify_field(rq_statscb_d.num_total_pdus, num_total_pdus);      \
    modify_field(rq_statscb_d.num_respcap_pdus, num_respcap_pdus);  \
    modify_field(rq_statscb_d.num_c2hdata_pdus, num_c2hdata_pdus);  \
    modify_field(rq_statscb_d.num_r2t_pdus, num_r2t_pdus);          \
    modify_field(rq_statscb_d.pad, pad);                            \

// note that it is a copy of serq_entry_new_t structure defined in TLS
// though it was defined as 64B structure there, actual descriptor is
// only 24B with 8B pad. So log_wqe_size for serq is 32B.
header_type nvme_rqe_t {
   fields {
        idesc                           : 64;
        A0                              : 64;
        O0                              : 32;
        L0                              : 32;
        pad                             : 64; 
    }
}

#define NVME_RQE_PARAMS                                             \
idesc, A0, O0, L0, pad

#define GENERATE_NVME_RQE_D                                         \
    modify_field(nvme_rqe_d.idesc, idesc);                          \
    modify_field(nvme_rqe_d.A0, A0);                                \
    modify_field(nvme_rqe_d.O0, O0);                                \
    modify_field(nvme_rqe_d.L0, L0);                                \
    modify_field(nvme_rqe_d.pad, pad);                              \

header_type pdu_hdr_t {
    fields {
        ch                              :   64;     //8B
        psh                             :   128;    //16B
        hdgst                           :   32;     //4B
        pad                             :   32;     //4B
    }
}

#define PDU_HDR_PARAMS                                              \
ch, psh, hdgst, pad

#define GENERATE_PDU_HDR_D                                          \
    modify_field(pdu_hdr_d.ch, ch);                                 \
    modify_field(pdu_hdr_d.psh, psh);                               \
    modify_field(pdu_hdr_d.hdgst, hdgst);                           \
    modify_field(pdu_hdr_d.pad, pad);                               \

header_type pdu_hdr_ch_t {
    fields {
        pdu_type                         :  8;
        flags                            :  8;
        hlen                             :  8;
        pdo                              :  8;
        plen                             :  32;
    }
}

// psh for cmd capsule pdu in little endian format
header_type pdu_hdr_cmdcap_psh_t {
    fields {
      // NVME command Dword 0
      opc         : 8;    // Opcode
      fuse        : 2;    // Fusing 2 simple commands
      rsvd0       : 4;
      psdt        : 2;    // PRP or SGL
      cid         : 16;   // Command identifier

      // NVME command Dword 1
      nsid        : 32;   // Namespace identifier

      // NVME command Dword 2
      rsvd2       : 32;

      // NVME command Dword 3
      rsvd3       : 32;

      // NVME command Dwords 4 and 5
      mptr        : 64;   // Metadata pointer

      // NVME command Dwords 6,7,8 & 9 form the data pointer (PRP or SGL)
      sgl1_addr     : 64;
      sgl1_len      : 32;
      sgl1_rsvd     : 24;
      sgl1_sub_type :  4;
      sgl1_type     :  4;

      // NVME command Dwords 10 and 11
      slba        : 64;   // Starting LBA (for Read/Write) commands

      // NVME command Dword 12
      nlb         : 16;   // Number of logical blocks
      rsvd12      : 10;
      prinfo      : 4;    // Protection information field
      fua         : 1;    // Force unit access
      lr          : 1;    // Limited retry

      // NVME command Dword 13
      dsm         : 8;    // Dataset management
      rsvd13      : 24;

      // NVME command Dword 14
      dw14        : 32;

      // NVME command Dword 15
      dw15        : 32;
    }
}

// session producer rx cb
header_type sessprodrxcb_t {
    fields {
        xts_q_base_addr                 : 34;
        log_num_xts_q_entries           : 5;
        rsvd0                           : 1;
        xts_q_choke_counter             : 8;
        xts_qid                         : 16;

        dgst_q_base_addr                : 34;
        log_num_dgst_q_entries          : 5;
        rsvd1                           : 1;
        dgst_q_choke_counter            : 8;
        dgst_qid                        : 16;

        xts_q_pi                        : 16;
        xts_q_ci                        : 16;

        dgst_q_pi                       : 16;
        dgst_q_ci                       : 16;

        //32 Bytes
        pad                             : 320;
    }
}

#define SESSPRODRXCB_PARAMS                                                    \
xts_q_base_addr, log_num_xts_q_entries, rsvd0, xts_q_choke_counter, xts_qid,   \
dgst_q_base_addr, log_num_dgst_q_entries, rsvd1, dgst_q_choke_counter, dgst_qid,  \
xts_q_pi, xts_q_ci, dgst_q_pi, dgst_q_ci,                                      \
pad                                                                            \


#define GENERATE_SESSPRODRXCB_D                                                  \
    modify_field(sessprodrxcb_d.xts_q_base_addr, xts_q_base_addr);               \
    modify_field(sessprodrxcb_d.log_num_xts_q_entries, log_num_xts_q_entries);   \
    modify_field(sessprodrxcb_d.rsvd0, rsvd0);                                   \
    modify_field(sessprodrxcb_d.xts_q_choke_counter, xts_q_choke_counter);       \
    modify_field(sessprodrxcb_d.xts_qid, xts_qid);                               \
    modify_field(sessprodrxcb_d.dgst_q_base_addr, dgst_q_base_addr);             \
    modify_field(sessprodrxcb_d.log_num_dgst_q_entries, log_num_dgst_q_entries); \
    modify_field(sessprodrxcb_d.rsvd1, rsvd1);                                   \
    modify_field(sessprodrxcb_d.dgst_q_choke_counter, dgst_q_choke_counter);     \
    modify_field(sessprodrxcb_d.dgst_qid, dgst_qid);                             \
    modify_field(sessprodrxcb_d.xts_q_pi, xts_q_pi);                             \
    modify_field(sessprodrxcb_d.xts_q_ci, xts_q_ci);                             \
    modify_field(sessprodrxcb_d.dgst_q_pi, dgst_q_pi);                           \
    modify_field(sessprodrxcb_d.dgst_q_ci, dgst_q_ci);                           \
    modify_field(sessprodrxcb_d.pad, pad);                                       \

// session dgst rx cb
// 64B
header_type sessdgstrxcb_t {
    fields {
        //16B
        pc                             : 8;
        // 7 Bytes intrinsic header
        CAPRI_QSTATE_HEADER_COMMON
        // 4 Bytes Ring 0
        CAPRI_QSTATE_HEADER_RING(0)
        // 4 Bytes Ring 1
        CAPRI_QSTATE_HEADER_RING(1)

        //8B
        rx_q_base_addr                  : 34;
        rx_q_log_num_entries            : 5;
        rsvd5                           : 25;

        //6B
        base_addr                       : 34;
        log_num_entries                 : 5;
        ring_empty_sched_eval_done      : 1;
        rsvd0                           : 8;

        //R0 stage0 flags
        //1B
        r0_busy                         : 1;
        rsvd1                           : 7;

        //R0 writeback stage flags
        //1B
        wb_r0_busy                      : 1;
        rsvd2                           : 7;

        //R1 stage0 flags
        //1B
        r1_busy                         : 1;
        rsvd3                           : 7;

        //R1 writeback stage flags
        //1B
        wb_r1_busy                      : 1;
        rsvd4                           : 7;

        //2B
        session_id                      : 16;

        //28B
        pad                             : 224;
    }
}

#define SESSDGSTRXCB_PARAMS                                                      \
rsvd, cosA, cosB, cos_sel, eval_last, host, total, pid, pi_0, ci_0, pi_1, ci_1, \
rx_q_base_addr, rx_q_log_num_entries, rsvd5, \
base_addr, log_num_entries, ring_empty_sched_eval_done, rsvd0, r0_busy, rsvd1, \
wb_r0_busy, rsvd2, r1_busy, rsvd3, wb_r1_busy, rsvd4, session_id, pad

#define GENERATE_SESSDGSTRXCB_D  \
    modify_field(sessdgstrxcb_d.rsvd, rsvd);                              \
    modify_field(sessdgstrxcb_d.cosA, cosA);                              \
    modify_field(sessdgstrxcb_d.cosB, cosB);                              \
    modify_field(sessdgstrxcb_d.cos_sel, cos_sel);                        \
    modify_field(sessdgstrxcb_d.eval_last, eval_last);                    \
    modify_field(sessdgstrxcb_d.host, host);                              \
    modify_field(sessdgstrxcb_d.total, total);                            \
    modify_field(sessdgstrxcb_d.pid, pid);                                \
    modify_field(sessdgstrxcb_d.pi_0, pi_0);                              \
    modify_field(sessdgstrxcb_d.ci_0, ci_0);                              \
    modify_field(sessdgstrxcb_d.pi_1, pi_1);                              \
    modify_field(sessdgstrxcb_d.ci_1, ci_1);                              \
    modify_field(sessdgstrxcb_d.rx_q_base_addr, rx_q_base_addr);  \
    modify_field(sessdgstrxcb_d.rx_q_log_num_entries, rx_q_log_num_entries);  \
    modify_field(sessdgstrxcb_d.rsvd5, rsvd5);  \
    modify_field(sessdgstrxcb_d.base_addr, base_addr);  \
    modify_field(sessdgstrxcb_d.log_num_entries, log_num_entries);  \
    modify_field(sessdgstrxcb_d.ring_empty_sched_eval_done, ring_empty_sched_eval_done);  \
    modify_field(sessdgstrxcb_d.rsvd0, rsvd0);  \
    modify_field(sessdgstrxcb_d.r0_busy, r0_busy);  \
    modify_field(sessdgstrxcb_d.rsvd1, rsvd1);  \
    modify_field(sessdgstrxcb_d.wb_r0_busy, wb_r0_busy);  \
    modify_field(sessdgstrxcb_d.rsvd2, rsvd2);  \
    modify_field(sessdgstrxcb_d.r1_busy, r1_busy);  \
    modify_field(sessdgstrxcb_d.rsvd3, rsvd3);  \
    modify_field(sessdgstrxcb_d.wb_r1_busy, wb_r1_busy);  \
    modify_field(sessdgstrxcb_d.rsvd4, rsvd4);  \
    modify_field(sessdgstrxcb_d.session_id, session_id);  \
    modify_field(sessdgstrxcb_d.pad, pad);  \

#define SESSDGSTRXCB_PARAMS_NON_STG0 \
    pc, SESSDGSTRXCB_PARAMS

#define GENERATE_SESSDGSTRXCB_D_NON_STG0                       \
    modify_field(sessdgstrxcb_d.pc, pc);                       \
    GENERATE_SESSDGSTRXCB_D

// session xts rx cb
// 64B
header_type sessxtsrxcb_t {
    fields {
        //16B
        pc                             : 8;
        // 7 Bytes intrinsic header
        CAPRI_QSTATE_HEADER_COMMON
        // 4 Bytes Ring 0
        CAPRI_QSTATE_HEADER_RING(0)
        // 4 Bytes Ring 1
        CAPRI_QSTATE_HEADER_RING(1)

        //6B
        base_addr                       : 34;
        log_num_entries                 : 5;
        ring_empty_sched_eval_done      : 1;
        rsvd0                           : 8;

        //R0 stage0 flags
        //1B
        r0_busy                         : 1;
        rsvd1                           : 7;

        //R0 writeback stage flags
        //1B
        wb_r0_busy                      : 1;
        rsvd2                           : 7;

        //2B
        nxt_lba_offset                  : 16;

        //R1 stage0 flags
        //1B
        r1_busy                         : 1;
        rsvd3                           : 7;

        //R1 writeback stage flags
        //1B
        wb_r1_busy                      : 1;
        rsvd4                           : 7;

        //8B
        page_ptr                        : 64;

        //2B
        session_id                      : 16;

        //26B
        pad                             : 208;
    }
}

#define SESSXTSRXCB_PARAMS                                                      \
rsvd, cosA, cosB, cos_sel, eval_last, host, total, pid, pi_0, ci_0, pi_1, ci_1, \
base_addr, log_num_entries, ring_empty_sched_eval_done, \
rsvd0, r0_busy, rsvd1, \
wb_r0_busy, rsvd2, nxt_lba_offset, \
r1_busy, rsvd3, wb_r1_busy, rsvd4,  \
page_ptr, session_id, pad

#define GENERATE_SESSXTSRXCB_D  \
    modify_field(sessxtsrxcb_d.rsvd, rsvd);                              \
    modify_field(sessxtsrxcb_d.cosA, cosA);                              \
    modify_field(sessxtsrxcb_d.cosB, cosB);                              \
    modify_field(sessxtsrxcb_d.cos_sel, cos_sel);                        \
    modify_field(sessxtsrxcb_d.eval_last, eval_last);                    \
    modify_field(sessxtsrxcb_d.host, host);                              \
    modify_field(sessxtsrxcb_d.total, total);                            \
    modify_field(sessxtsrxcb_d.pid, pid);                                \
    modify_field(sessxtsrxcb_d.pi_0, pi_0);                              \
    modify_field(sessxtsrxcb_d.ci_0, ci_0);                              \
    modify_field(sessxtsrxcb_d.pi_1, pi_1);                              \
    modify_field(sessxtsrxcb_d.ci_1, ci_1);                              \
    modify_field(sessxtsrxcb_d.base_addr, base_addr);  \
    modify_field(sessxtsrxcb_d.log_num_entries, log_num_entries);  \
    modify_field(sessxtsrxcb_d.ring_empty_sched_eval_done, ring_empty_sched_eval_done);  \
    modify_field(sessxtsrxcb_d.rsvd0, rsvd0);  \
    modify_field(sessxtsrxcb_d.r0_busy, r0_busy);  \
    modify_field(sessxtsrxcb_d.rsvd1, rsvd1);  \
    modify_field(sessxtsrxcb_d.wb_r0_busy, wb_r0_busy);  \
    modify_field(sessxtsrxcb_d.rsvd2, rsvd2);  \
    modify_field(sessxtsrxcb_d.nxt_lba_offset, nxt_lba_offset);  \
    modify_field(sessxtsrxcb_d.r1_busy, r1_busy);  \
    modify_field(sessxtsrxcb_d.rsvd3, rsvd3);  \
    modify_field(sessxtsrxcb_d.wb_r1_busy, wb_r1_busy);  \
    modify_field(sessxtsrxcb_d.rsvd4, rsvd4);  \
    modify_field(sessxtsrxcb_d.page_ptr, page_ptr);  \
    modify_field(sessxtsrxcb_d.session_id, session_id);  \
    modify_field(sessxtsrxcb_d.pad, pad);  \

#define SESSXTSRXCB_PARAMS_NON_STG0 \
    pc, SESSXTSRXCB_PARAMS

#define GENERATE_SESSXTSRXCB_D_NON_STG0                                \
    modify_field(sessxtsrxcb_d.pc, pc);                                \
    GENERATE_SESSXTSRXCB_D

// wqe that gets posted into sessrf queue
header_type sess_rfwqe_t {
    fields {
        cmd_id      : 16; // command identifier
        pdu_id      : 16; // PDU identifier
        num_pages   : 8;  // num-of-pages to free
    }
}

#define SESS_RFWQE_PARAMS  \
cmd_id, pdu_id, num_pages

#define GENERATE_SESS_RFWQE_D \
    modify_field(sess_rfwqe_d.cmd_id, cmd_id); \
    modify_field(sess_rfwqe_d.pdu_id, pdu_id); \
    modify_field(sess_rfwqe_d.num_pages, num_pages); 

// session resource-free cb
// 64B  
header_type rfcb_t {
    fields {
        //12B
        pc                             : 8;
        // 7 Bytes intrinsic header
        CAPRI_QSTATE_HEADER_COMMON
        // 4 Bytes Ring 0
        CAPRI_QSTATE_HEADER_RING(0)

        //6B
        base_addr                       : 34;
        log_num_entries                 : 5;
        ring_empty_sched_eval_done      : 1;
        rsvd0                           : 8;
    
        //4B
        sqid                           : 16; // Submission-Queue corresponding to this session.
        cqid                           : 16; // Completion-Queue corresponding to this session.
 
        // Status info/flags
        // 7B
        r0_busy                        : 1; // WO S0, RO S6
        rsvd1                          : 7;

        wb_r0_busy                     : 1; // WO S6, RO S0
        rsvd2                          : 7;

        in_progress                    : 1; // WO S6, RO S0
        completion_done                : 1; // WO S6, RO S0
        pduid_freed                    : 1; // WO S6, RO S0
        cmdid_freed                    : 1; // WO S6, RO S0
        rsvd3                          : 4; 

        cur_page                       : 8; 
        num_pages                      : 8; // Num pages referred by cur_pduid.
        cur_pduid                      : 16; // pduid in processing.

        pad                            : 280; 
    }
}

#define RFCB_PARAMS                                                          \
rsvd, cosA, cosB, cos_sel, eval_last, host, total, pid, pi_0, ci_0,          \
base_addr, log_num_entries, ring_empty_sched_eval_done, rsvd0,           \
sqid, cqid, r0_busy, rsvd1, wb_r0_busy, rsvd2, in_progress, completion_done, \
pduid_freed, cmdid_freed, rsvd3, cur_page, num_pages, cur_pduid, pad 


#define GENERATE_RFCB_D                                           \
    modify_field(rfcb_d.rsvd, rsvd);                              \
    modify_field(rfcb_d.cosA, cosA);                              \
    modify_field(rfcb_d.cosB, cosB);                              \
    modify_field(rfcb_d.cos_sel, cos_sel);                        \
    modify_field(rfcb_d.eval_last, eval_last);                    \
    modify_field(rfcb_d.host, host);                              \
    modify_field(rfcb_d.total, total);                            \
    modify_field(rfcb_d.pid, pid);                                \
    modify_field(rfcb_d.pi_0, pi_0);                              \
    modify_field(rfcb_d.ci_0, ci_0);                              \
    modify_field(rfcb_d.base_addr, base_addr);                    \
    modify_field(rfcb_d.log_num_entries, log_num_entries);        \
    modify_field(rfcb_d.ring_empty_sched_eval_done, ring_empty_sched_eval_done); \
    modify_field(rfcb_d.rsvd0, rsvd0);                            \
    modify_field(rfcb_d.sqid, sqid);                              \
    modify_field(rfcb_d.cqid, cqid);                              \
    modify_field(rfcb_d.r0_busy, r0_busy);                        \
    modify_field(rfcb_d.rsvd1, rsvd1);                             \
    modify_field(rfcb_d.wb_r0_busy, wb_r0_busy);                  \
    modify_field(rfcb_d.rsvd2, rsvd2);                            \
    modify_field(rfcb_d.in_progress, in_progress);                \
    modify_field(rfcb_d.completion_done, completion_done);        \
    modify_field(rfcb_d.pduid_freed, pduid_freed);                \
    modify_field(rfcb_d.cmdid_freed, cmdid_freed);                \
    modify_field(rfcb_d.rsvd3, rsvd3);                            \
    modify_field(rfcb_d.cur_page, cur_page);                      \
    modify_field(rfcb_d.num_pages, num_pages);                    \
    modify_field(rfcb_d.cur_pduid, cur_pduid);                    \
    modify_field(rfcb_d.pad, pad);                                \
        
#define RFCB_PARAMS_NON_STG0                                    \
    pc, RFCB_PARAMS
        
#define GENERATE_RFCB_D_NON_STG0                                \
    modify_field(rfcb_d.pc, pc);                                \
    GENERATE_RFCB_D

// RF stats cb
header_type rf_statscb_t {
    fields {
        //64 Bytes
        pad                             : 512;
    }
}

#define RF_STATSCB_PARAMS                                                      \
pad

#define GENERATE_RF_STATSCB_D                                                  \
    modify_field(rf_statscb_d.pad, pad);                                       \

// Page metadata cb
header_type page_metadata_cb_t {
    fields {
        refcnt                          : 16;
        more_pdus                       :  1;

        //62 Bytes
        pad                             : 495;
    }
}

#define PAGE_METADATA_PARAMS                                            \
refcnt, more_pdus, pad

#define GENERATE_PAGE_METADATA_D                          \
    modify_field(page_metadata_cb_d.refcnt, refcnt);      \
    modify_field(page_metadata_cb_d.more_pdus, more_pdus);      \
    modify_field(page_metadata_cb_d.pad, pad);             \

// PDU context page pointers
header_type pdu_ctxt_page_ptrs_cb_t {
    fields {
        page_addr1                  : 64;
        page_addr2                  : 64;
        page_addr3                  : 64;
        page_addr4                  : 64;
        page_addr5                  : 64;
        page_addr6                  : 64;
        page_addr7                  : 64;
        page_addr8                  : 64;
    }
}

#define PDU_CTXT_PAGE_PTRS_PARAMS               \
page_addr1, page_addr2, page_addr3, page_addr4, \
page_addr5, page_addr6, page_addr7, page_addr8

#define GENERATE_PDU_CTXT_PAGE_PTRS_D                                  \
    modify_field(pdu_ctxt_page_ptrs_cb_d.page_addr1, page_addr1);      \
    modify_field(pdu_ctxt_page_ptrs_cb_d.page_addr2, page_addr2);      \
    modify_field(pdu_ctxt_page_ptrs_cb_d.page_addr3, page_addr3);      \
    modify_field(pdu_ctxt_page_ptrs_cb_d.page_addr4, page_addr4);      \
    modify_field(pdu_ctxt_page_ptrs_cb_d.page_addr5, page_addr5);      \
    modify_field(pdu_ctxt_page_ptrs_cb_d.page_addr6, page_addr6);      \
    modify_field(pdu_ctxt_page_ptrs_cb_d.page_addr7, page_addr7);      \
    modify_field(pdu_ctxt_page_ptrs_cb_d.page_addr8, page_addr8);      


// nmdpr resource cb
header_type nmdpr_resourcecb_t {
    fields {
        // ring of free pages rx.
        rx_nmdpr_ring_pi                : 16;
        rx_nmdpr_ring_proxy_pi          : 16;
        rx_nmdpr_ring_proxy_ci          : 16;
        rx_nmdpr_ring_ci                : 16;
        rx_nmdpr_ring_choke_counter     : 16;

        //54 Bytes
        pad                             : 432;
    }
}

#define NMDPR_RESOURCECB_PARAMS                                   \
rx_nmdpr_ring_pi, rx_nmdpr_ring_proxy_pi, rx_nmdpr_ring_proxy_ci, \
rx_nmdpr_ring_ci, rx_nmdpr_ring_choke_counter, pad

#define GENERATE_NMDPR_RESOURCECB_D                                           \
    modify_field(nmdpr_resourcecb_d.rx_nmdpr_ring_pi, rx_nmdpr_ring_pi);      \
    modify_field(nmdpr_resourcecb_d.rx_nmdpr_ring_proxy_pi, rx_nmdpr_ring_proxy_pi);      \
    modify_field(nmdpr_resourcecb_d.rx_nmdpr_ring_proxy_ci, rx_nmdpr_ring_proxy_ci);      \
    modify_field(nmdpr_resourcecb_d.rx_nmdpr_ring_ci, rx_nmdpr_ring_ci);      \
    modify_field(nmdpr_resourcecb_d.rx_nmdpr_ring_choke_counter, rx_nmdpr_ring_choke_counter);      \
    modify_field(nmdpr_resourcecb_d.pad, pad);      

