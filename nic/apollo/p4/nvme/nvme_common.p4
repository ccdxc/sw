#ifdef NVME_APOLLO
#include "../../../p4/common-p4+/capri_dma_cmd.p4"
#include "../../../p4/common-p4+/capri_doorbell.p4"
#endif

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

// NVME status definition
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

// session producer cb
header_type sessprodcb_t {
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

        tcp_q_pi                        : 16;
        tcp_q_ci                        : 16;

        //28 Bytes
        pad                             : 224;
    }
}

#define SESSPRODCB_PARAMS                                                      \
xts_q_base_addr, log_num_xts_q_entries, rsvd0, xts_q_choke_counter, xts_qid,   \
dgst_q_base_addr, log_num_dgst_q_entries, rsvd2, dgst_q_choke_counter, dgst_qid,  \
tcp_q_base_addr, log_num_tcp_q_entries, rsvd4, tcp_q_choke_counter, rsvd5,     \
xts_q_pi, xts_q_ci,                                                            \
dgst_q_pi, dgst_q_ci,                                                          \
tcp_q_pi, tcp_q_ci,                                                            \
pad


#define GENERATE_SESSPRODCB_D                                                  \
    modify_field(sessprodcb_d.xts_q_base_addr, xts_q_base_addr);               \
    modify_field(sessprodcb_d.log_num_xts_q_entries, log_num_xts_q_entries);   \
    modify_field(sessprodcb_d.rsvd0, rsvd0);                                   \
    modify_field(sessprodcb_d.xts_q_choke_counter, xts_q_choke_counter);       \
    modify_field(sessprodcb_d.xts_qid, xts_qid);                                   \
    modify_field(sessprodcb_d.dgst_q_base_addr, dgst_q_base_addr);             \
    modify_field(sessprodcb_d.log_num_dgst_q_entries, log_num_dgst_q_entries); \
    modify_field(sessprodcb_d.rsvd2, rsvd2);                                   \
    modify_field(sessprodcb_d.dgst_q_choke_counter, dgst_q_choke_counter);     \
    modify_field(sessprodcb_d.dgst_qid, dgst_qid);                                   \
    modify_field(sessprodcb_d.tcp_q_base_addr, tcp_q_base_addr);               \
    modify_field(sessprodcb_d.log_num_tcp_q_entries, log_num_tcp_q_entries);   \
    modify_field(sessprodcb_d.rsvd4, rsvd4);                                   \
    modify_field(sessprodcb_d.tcp_q_choke_counter, tcp_q_choke_counter);       \
    modify_field(sessprodcb_d.rsvd5, rsvd5);                                   \
    modify_field(sessprodcb_d.xts_q_pi, xts_q_pi);                             \
    modify_field(sessprodcb_d.xts_q_ci, xts_q_ci);                             \
    modify_field(sessprodcb_d.dgst_q_pi, dgst_q_pi);                           \
    modify_field(sessprodcb_d.dgst_q_ci, dgst_q_ci);                           \
    modify_field(sessprodcb_d.tcp_q_pi, tcp_q_pi);                             \
    modify_field(sessprodcb_d.tcp_q_ci, tcp_q_ci);                             \
    modify_field(sessprodcb_d.pad, pad);                                       \

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
        // ring of free data pages
        page_ring_pi                    : 16;
        page_ring_proxy_ci              : 16;
        page_ring_ci                    : 16;
        page_ring_log_sz                :  5;
        page_ring_rsvd                  :  3;
        page_ring_choke_counter         :  8;

        // ring of free pduids 
        pduid_ring_pi                   : 16;
        pduid_ring_proxy_ci             : 16;
        pduid_ring_ci                   : 16;
        pduid_ring_log_sz               :  5;
        pduid_ring_rsvd                 :  3;
        pduid_ring_choke_counter        :  8;

        // ring of free cmdids
        cmdid_ring_pi                   : 16;
        cmdid_ring_proxy_ci             : 16;
        cmdid_ring_ci                   : 16;
        cmdid_ring_log_sz               :  5;
        cmdid_ring_rsvd                 :  3;
        cmdid_ring_choke_counter        :  8;

        //40 Bytes
        pad                             : 320;
    }
}

#define RESOURCECB_PARAMS                                                      \
page_ring_pi, page_ring_proxy_ci, page_ring_ci, page_ring_log_sz, \
page_ring_rsvd, page_ring_choke_counter, \
pduid_ring_pi, pduid_ring_proxy_ci, pduid_ring_ci, pduid_ring_log_sz, \
pduid_ring_rsvd, pduid_ring_choke_counter, \
cmdid_ring_pi, cmdid_ring_proxy_ci, cmdid_ring_ci, cmdid_ring_log_sz, \
cmdid_ring_rsvd, cmdid_ring_choke_counter, \
pad


#define GENERATE_RESOURCECB_D                                                   \
    modify_field(resourcecb_d.page_ring_pi, page_ring_pi);                      \
    modify_field(resourcecb_d.page_ring_proxy_ci, page_ring_proxy_ci);          \
    modify_field(resourcecb_d.page_ring_ci, page_ring_ci);                      \
    modify_field(resourcecb_d.page_ring_log_sz, page_ring_log_sz);              \
    modify_field(resourcecb_d.page_ring_rsvd, page_ring_rsvd);                  \
    modify_field(resourcecb_d.page_ring_choke_counter, page_ring_choke_counter);\
    modify_field(resourcecb_d.pduid_ring_pi, pduid_ring_pi);                    \
    modify_field(resourcecb_d.pduid_ring_proxy_ci, pduid_ring_proxy_ci);        \
    modify_field(resourcecb_d.pduid_ring_ci, pduid_ring_ci);                    \
    modify_field(resourcecb_d.pduid_ring_log_sz, pduid_ring_log_sz);            \
    modify_field(resourcecb_d.pduid_ring_rsvd, pduid_ring_rsvd);                \
    modify_field(resourcecb_d.pduid_ring_choke_counter, pduid_ring_choke_counter);\
    modify_field(resourcecb_d.cmdid_ring_pi, cmdid_ring_pi);                    \
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
    pdu_opc             :   8;    // PUD Opcode
    cid	                :   16;	// Command identifier
    nsid                :   32;	// Namespace identifier

    slba                :   64;	// Starting LBA for this PDU
    nlb                 :   16;	// Number of logical blocks for this PDU

    log_lba_size        :   5;
    log_host_page_size  :   5;
    rsvd0               :   6;

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
cmd_opc, pdu_opc, cid, nsid, slba, nlb, log_lba_size, log_host_page_size, rsvd0, \
num_prps, num_pages, prp1_offset, key_index, sec_key_index, session_id, pad

#define GENERATE_PDU_CTXT0_D \
    modify_field(pdu_ctxt0_d.cmd_opc, cmd_opc);\
    modify_field(pdu_ctxt0_d.pdu_opc, pdu_opc);\
    modify_field(pdu_ctxt0_d.cid, cid);\
    modify_field(pdu_ctxt0_d.nsid, nsid);\
    modify_field(pdu_ctxt0_d.slba, slba);\
    modify_field(pdu_ctxt0_d.nlb, nlb);\
    modify_field(pdu_ctxt0_d.log_lba_size, log_lba_size);\
    modify_field(pdu_ctxt0_d.log_host_page_size, log_host_page_size);\
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

//64B
header_type xtscb_t {
  fields {
    pi                  : 32;
    ci                  : 32;
    xts_ring_base_addr  : 34;
    log_sz              :  5;
    rsvd                :  1;
    choke_counter       :  8;
    pad                 :400;
  }
}

#define XTSCB_PARAMS \
pi, ci, xts_ring_base_addr, log_sz, rsvd, choke_counter, pad

#define GENERATE_XTSCB_D    \
    modify_field(xtscb_d.pi, pi); \
    modify_field(xtscb_d.ci, ci); \
    modify_field(xtscb_d.xts_ring_base_addr, xts_ring_base_addr); \
    modify_field(xtscb_d.log_sz, log_sz); \
    modify_field(xtscb_d.rsvd, rsvd); \
    modify_field(xtscb_d.choke_counter, choke_counter); \
    modify_field(xtscb_d.pad, pad);

// this is the cb that is used to track the dgst accelerator engine ring.
// rest of the details are same as above.

//16B
header_type dgstcb_t {
  fields {
    pi                  : 32;
    ci                  : 32;
    dgst_ring_base_addr : 34;
    log_sz              :  5;
    rsvd                :  1;
    choke_counter       :  8;
    pad                 :400;
  }
}

#define DGSTCB_PARAMS \
pi, ci, dgst_ring_base_addr, log_sz, rsvd, choke_counter, pad

#define GENERATE_DGSTCB_D \
    modify_field(dgstcb_d.pi, pi); \
    modify_field(dgstcb_d.ci, ci); \
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
        opaque_tag_write_en                 : 1;
        rsvd1                               : 31;
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
        cmd             : 16;
        datain_len      : 16;
        extended_len    : 16;
        threshold_len   : 16;
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
