#include "../../../p4/common-p4+/capri_dma_cmd.p4"
#include "../../../p4/common-p4+/capri_doorbell.p4"

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
    dptr1	: 64;	// PRP1 or address of SGL
    dptr2	: 64;	// PRP2 or size/type/sub_type in SGL

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
opc, fuse, rsvd0, psdt, cid, nsid, rsvd2, rsvd3, mptr, dptr1, dptr2, slba, \
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
    modify_field(nvme_sqe_d.dptr1, dptr1);\
    modify_field(nvme_sqe_d.dptr2, dptr2);\
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
        /* 20 Bytes/160 bits Fixed header */

        sq_base_addr                    : 64;
        log_num_wqes                    : 5;
        rsvd0                           : 3;
        
        cq_id                           : 16;

        //41 Bytes
        pad                             : 328;
    }
}

#define SQCB_PARAMS                                                                                   \
rsvd, cosA, cosB, cos_sel, eval_last, host, total, pid, pi_0, ci_0, \
sq_base_addr, log_num_wqes, rsvd0, cq_id, pad

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
    modify_field(sqcb_d.log_num_wqes, log_num_wqes);              \
    modify_field(sqcb_d.rsvd0, rsvd0);                            \
    modify_field(sqcb_d.cq_id, cq_id);                            \
    modify_field(sqcb_d.pad, pad);                                \


// nscb for stage 0
header_type nscb_t {
    fields {
        valid_session_bitmap            : 256;

        ns_size                         : 64; //in LBAs
        ns_valid                        : 1;
        ns_active                       : 1;
        rsvd0                           : 1;
        log_lba_size                    : 5;

        //Backend Info
        backend_ns_id                   : 16;
        rsvd1                           : 8;
        
        //Session Info
        num_sessions                    : 11; //1-based
        rr_session_id_served            : 10; //0-based

        // Stats/Accounting
        num_outstanding_req             : 11; //1-based

        sess_prodcb_table_addr          : 34;

        pad                             : 94;
    }
}

#define NSCB_PARAMS                                                      \
ns_size, ns_valid, ns_active, rsvd0, log_lba_size, backend_ns_id,        \
rsvd1, num_sessions, rr_session_id_served, valid_session_bitmap,         \
num_outstanding_req, sess_prodcb_table_addr, pad

#define GENERATE_NSCB_D                                                  \
    modify_field(nscb_d.valid_session_bitmap, valid_session_bitmap);     \
    modify_field(nscb_d.ns_size, ns_size);                               \
    modify_field(nscb_d.ns_valid, ns_valid);                             \
    modify_field(nscb_d.ns_active, ns_active);                           \
    modify_field(nscb_d.rsvd0, rsvd0);                                   \
    modify_field(nscb_d.log_lba_size, log_lba_size);                     \
    modify_field(nscb_d.backend_ns_id, backend_ns_id);                   \
    modify_field(nscb_d.rsvd1, rsvd1);                                   \
    modify_field(nscb_d.num_sessions, num_sessions);                     \
    modify_field(nscb_d.rr_session_id_served, rr_session_id_served);     \
    modify_field(nscb_d.num_outstanding_req, num_outstanding_req);       \
    modify_field(nscb_d.sess_prodcb_table_addr, sess_prodcb_table_addr); \
    modify_field(nscb_d.pad, pad);                                       \

// session producer cb
header_type sessprodcb_t {
    fields {
        xts_q_base_addr                 : 34;
        log_num_xts_q_entries           : 5;
        rsvd0                           : 25;

        dgst_q_base_addr                : 34;
        log_num_dgst_q_entries          : 5;
        rsvd1                           : 25;

        xts_q_pi                        : 16;
        xts_q_ci                        : 16;

        dgst_q_pi                       : 16;
        dgst_q_ci                       : 16;

        //40 Bytes
        pad                             : 320;
    }
}

#define SESSPRODCB_PARAMS                                                      \
xts_q_base_addr, log_num_xts_q_entries, rsvd0,                                 \
dgst_q_base_addr, log_num_dgst_q_entries, rsvd1,                               \
xts_q_pi, xts_q_ci,                                                            \
dgst_q_pi, dgst_q_ci,                                                          \
pad


#define GENERATE_SESSPRODCB_D                                                  \
    modify_field(sessprodcb_d.xts_q_base_addr, xts_q_base_addr);               \
    modify_field(sessprodcb_d.log_num_xts_q_entries, log_num_xts_q_entries);   \
    modify_field(sessprodcb_d.rsvd0, rsvd0);                                   \
    modify_field(sessprodcb_d.dgst_q_base_addr, dgst_q_base_addr);             \
    modify_field(sessprodcb_d.log_num_dgst_q_entries, log_num_dgst_q_entries); \
    modify_field(sessprodcb_d.rsvd1, rsvd1);                                   \
    modify_field(sessprodcb_d.xts_q_pi, xts_q_pi);                             \
    modify_field(sessprodcb_d.xts_q_ci, xts_q_ci);                             \
    modify_field(sessprodcb_d.dgst_q_pi, dgst_q_pi);                           \
    modify_field(sessprodcb_d.dgst_q_ci, dgst_q_ci);                           \
    modify_field(sessprodcb_d.pad, pad);                                       \

// session producer cb
header_type resourcecb_t {
    fields {
        page_ring_pi                    : 16;
        page_ring_ci                    : 16;

        cmdid_ring_pi                   : 16;
        cmdid_ring_ci                   : 16;

        log_page_ring_sz                : 5;
        rsvd0                           : 3;

        log_cmdid_ring_sz               : 5;
        rsvd1                           : 3;

        //54 Bytes
        pad                             : 432;
    }
}

#define RESOURCECB_PARAMS                                                      \
page_ring_pi, page_ring_ci, cmdid_ring_pi, cmdid_ring_ci,                      \
log_page_ring_sz, rsvd0, log_cmdid_ring_sz, rsvd1,                             \
pad


#define GENERATE_RESOURCECB_D                                                  \
    modify_field(resourcecb_d.page_ring_pi, page_ring_pi);                     \
    modify_field(resourcecb_d.page_ring_ci, page_ring_ci);                     \
    modify_field(resourcecb_d.cmdid_ring_pi, cmdid_ring_pi);                   \
    modify_field(resourcecb_d.cmdid_ring_ci, cmdid_ring_ci);                   \
    modify_field(resourcecb_d.log_page_ring_sz, log_page_ring_sz);             \
    modify_field(resourcecb_d.rsvd0, rsvd0);                                   \
    modify_field(resourcecb_d.log_cmdid_ring_sz, log_cmdid_ring_sz);           \
    modify_field(resourcecb_d.rsvd1, rsvd1);                                   \
    modify_field(resourcecb_d.pad, pad);                                       \


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

