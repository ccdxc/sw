/***
 *  Header Type Declarations
 ***/

/***
 *  phv
 ***/

header_type adminq_cq_desc_p {
    fields {
        cmd_status : 16;
        cpl_id : 16;
        rsvd : 7;
        color : 1;
        cmd_data0 : 32;
        cmd_data1 : 32;
        cmd_data2 : 32;
    }
}

/***
 *  D-vector Headers
 ***/

header_type adminq_qstate_d {
    fields {
        rsvd : 8;
        cosA : 4;
        cosB : 4;
        cos_sel : 8;
        eval_last : 8;
        host : 4;
        total : 4;
        pid : 16;                // 8 B

        p_index0 : 16;
        c_index0 : 16;
        p_index1 : 16;
        c_index1 : 16;

        enable : 8;
        ring_base : 64;
        ring_size : 16;
    }
}

header_type adminq_desc_d {
    fields {
        opcode: 16;
        rsvd: 16;
        cmd_data0 : 32;
        cmd_data1 : 32;
        cmd_data2 : 32;
        cmd_data3 : 32;
        cmd_data4 : 32;
        cmd_data5 : 32;
        cmd_data6 : 32;
        cmd_data7 : 32;
        cmd_data8 : 32;
        cmd_data9 : 32;
        cmd_data10 : 32;
        cmd_data11 : 32;
        cmd_data12 : 32;
        cmd_data13 : 32;
        cmd_data14 : 32;
        cmd_data15 : 32;
    }
}

header_type adminq_to_s1_k {
    fields {
        cq_ring_base : 64;
    }
}
/*****************************************************************************
 *  D-vector
 *****************************************************************************/
@pragma scratch_metadata
metadata adminq_qstate_d adminq_qstate;

@pragma scratch_metadata
metadata adminq_desc_d adminq_desc;

/*****************************************************************************
 *  K-vector
 *****************************************************************************/
// Union with Common-TXDMA PHV headers

// To Stage N PHV headers (Available in STAGE=N, MPUS=ALL)
@pragma pa_header_union ingress to_stage_1
metadata adminq_to_s1_k  adminq_to_s1;
@pragma scratch_metadata
metadata adminq_to_s1_k adminq_to_s1_scratch;  // Not required?

@pragma pa_header_union ingress common_t0_s2s
metadata adminq_cq_desc_p adminq_s2s;
@pragma scratch_metadata
metadata adminq_cq_desc_p adminq_s2s_scratch;

// Stage N to N+1 PHV headers (Available in STAGE=N,N+1 MPUS=ALL)

// Part of the PHV after Common Area
@pragma dont_trim
metadata dma_cmd_phv2pkt_t dma_cmd0;
@pragma dont_trim
metadata dma_cmd_phv2mem_t dma_cmd1;
@pragma dont_trim
metadata dma_cmd_phv2mem_t dma_cmd2;
@pragma dont_trim
metadata dma_cmd_phv2mem_t dma_cmd3;

