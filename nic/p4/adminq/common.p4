

/*
 * ADMINQ
 */

header_type adminq_qstate {
    fields {
        pc : 8;
        rsvd : 8;
        cosA : 4;
        cosB : 4;
        cos_sel : 8;
        eval_last : 8;
        host : 4;
        total : 4;
        pid : 16;

        p_index0 : 16;
        c_index0 : 16;
        comp_index : 16;
        ci_fetch   : 16;

        enable : 1;
        color : 1;
        host_queue : 1;
        rsvd1 : 5;

        ring_base : 64;
        ring_size : 16;
        cq_ring_base : 64;
        intr_assert_addr : 32;

        nicmgr_qstate_addr : 64;
    }
}

// 64 B
header_type adminq_cmd_desc {
    fields {
        opcode : 16;
        data : 496;
    }
}

// 16 B
header_type adminq_comp_desc {
    fields {
        status : 8;
        rsvd : 8;
        comp_index : 16;
        data : 88;
        color : 1;
        rsvd2 : 7;
    }
}

/*
 * NICMGR
 */

header_type nicmgr_qstate {
    fields {
        pc : 8;
        rsvd : 8;
        cosA : 4;
        cosB : 4;
        cos_sel : 8;
        eval_last : 8;
        host : 4;
        total : 4;
        pid : 16;

        p_index0 : 16;
        c_index0 : 16;
        comp_index : 16;
        ci_fetch   : 16;

        enable : 1;
        color : 1;
        rsvd1 : 6;

        ring_base : 64;
        ring_size : 16;
        cq_ring_base : 64;
        intr_assert_addr : 32;
    }
}

// 128 B
header_type nicmgr_req_desc {
    fields {
        //lif : 11;
        lif : 16;
        //qtype : 3;
        qtype : 8;
        //qid : 24;
        qid : 32;
        comp_index : 16;
        //adminq_qstate_addr : 34;
        adminq_qstate_addr : 64;
        //pad : 440;
        pad : 376;
        adminq_cmd_desc : 512;
    }
}

// 128 B
header_type nicmgr_resp_desc {
    fields {
        //lif : 11;
        lif : 16;
        //qtype : 3;
        qtype : 8;
        //qid : 24;
        qid : 32;
        comp_index : 16;
        //adminq_qstate_addr : 34;
        adminq_qstate_addr : 64;
        //pad : 440;
        pad : 376;
        adminq_comp_desc : 512;
    }
}

/*
 * D-vector
 */

@pragma scratch_metadata
metadata adminq_qstate adminq_qstate;

@pragma scratch_metadata
metadata nicmgr_qstate nicmgr_qstate;
