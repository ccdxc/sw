
#include "defines.h"

/***
 *  Header Type Declarations
 ***/

/***
 *  P-vector Headers
 ***/

header_type p4plus_to_p4_classic_header_t {
    fields {
        p4plus_app_id           : 4;
        table0_valid            : 1;
        table1_valid            : 1;
        table2_valid            : 1;
        table3_valid            : 1;
        flow_index              : 24;
        lkp_inst                : 1;
        compute_inner_l4_csum   : 1;
        compute_l4_csum         : 1;
        insert_vlan_tag         : 1;
        update_udp_len          : 1;
        update_tcp_seq_no       : 1;
        update_ip_len           : 1;
        update_ip_id            : 1;
        udp_opt_bytes           : 8;
        dst_lport               : 11;
        dst_lport_valid         : 1;
        pad1                    : 1;
        tso_last_segment        : 1;
        tso_first_segment       : 1;
        tso_valid               : 1;
        ip_id_delta             : 16;
        tcp_seq_delta           : 32;
        gso_start               : 14;
        compute_inner_ip_csum   : 1;
        compute_ip_csum         : 1;
        gso_offset              : 14;
        flow_index_valid        : 1;
        gso_valid               : 1;
        vlan_tag                : 16;
    }
}

header_type eth_cq_desc_p {
    fields {
        status : 8;
        rsvd : 8;
        comp_index : 16;
        rsvd2 : 88;
        color : 1;
        rsvd3 : 7;
    }
}

header_type eth_eq_intr_desc_p {
    // Event Descriptor + Intr Assert Data
    fields {
        // Event Descriptor
        code : 16;
        lif_index : 16;
        qid : 32;
        rsvd : 56;
        gen_color : 8;

        // Intr Assert Data (packed here for convenience)
        intr_data : 32;
    }
}

/***
 *  D-vector Headers
 ***/

header_type eth_eq_qstate_d {
    fields {
        eq_ring_base : 64;
        eq_ring_size : 8;
        eq_enable : 1;
        intr_enable : 1;
        rsvd_cfg  : 6;
        eq_index : 16;
        eq_gen : 8;
        rsvd : 8;
        intr_index : 16;
    }
}

header_type eth_tx_qstate_d {
    fields {
        FIELDS_ETH_TXRX_QSTATE_COMMON

        comp_index : 16;

        // sta
        color : 1;
        armed : 1;
        rsvd_sta : 6;

        lg2_desc_sz : 4;
        lg2_cq_desc_sz : 4;
        lg2_sg_desc_sz : 4;

        __pad256 : 28;

        ring_base : 64;
        cq_ring_base : 64;
        sg_ring_base : 64;
        intr_index_or_eq_addr : 64;
    }
}

header_type eth_tx2_qstate_d {
    fields {
        tso_hdr_addr : 52;
        tso_hdr_len : 10;
        tso_hdr_rsvd : 2;
        tso_ipid_delta : 16;
        tso_seq_delta : 32;
        tso_rsvd : 16;

        //__pad512 : 384;
    }
}

#define HEADER_TX_DESC(n) \
    opcode##n : 4; \
    csum_l4_or_eot##n : 1; \
    csum_l3_or_sot##n : 1; \
    encap##n : 1; \
    vlan_insert##n : 1; \
    addr_hi##n : 4; \
    num_sg_elems##n: 4; \
    addr_lo##n : 48; \
    len##n : 16; \
    vlan_tci##n : 16; \
    csum_start_or_hdr_len##n : 16; \
    csum_offset_or_mss##n : 16;

header_type eth_tx_desc_d {
    fields {
        HEADER_TX_DESC()
    }
}

#define HEADER_SG_ELEM(n) \
    addr##n : 64; \
    len##n : 16; \
    rsvd##n : 48;

header_type eth_sg_desc_d {
    fields {
        HEADER_SG_ELEM(0)
        HEADER_SG_ELEM(1)
        HEADER_SG_ELEM(2)
        HEADER_SG_ELEM(3)
    }
}

/***
 * K+I Headers
 ***/

header_type eth_tx_global_k {
    fields {
        dma_cur_index : 6;
        sg_in_progress : 1;
        num_sg_elems : 4;
        tso_eot : 1;    // end of tso
        tso_sot : 1;    // start of tso
        host_queue : 1;
        cpu_queue : 1;
        do_cq : 1;
        do_eq : 1; // do_eq is do_arm in fetch->commit stages
        do_intr : 1;
        lif : 11;
        stats : 32;
        drop : 1;
    }
}

header_type eth_tx_t0_s2s_k {
    fields {
        cq_desc_addr : 64;
        eq_desc_addr : 64;
        intr_index : 16; // intr_index is arm_index in fetch->commit stages
        num_todo : 4;
        num_desc : 4;
        do_sg : 1;
        do_tso : 1;
    }
}

header_type eth_tx_t1_s2s_k {
    fields {
        tso_hdr_addr : 52;
        tso_hdr_len : 10;
        tso_hdr_rsvd : 2;
        tso_ipid_delta : 16;
        tso_seq_delta : 32;
    }
}

header_type eth_tx_to_s1_k {
    fields {
        qstate_addr : 64;
    }
}

header_type eth_tx_to_s2_k {
    fields {
        qid : 24;
        qtype : 3;
        pad : 5;
        my_ci : 16;
        tso_hdr_addr : 52;
        tso_hdr_len : 10;
        tso_hdr_rsvd : 2;
    }
}

header_type eth_tx_to_s3_k {
    fields {
        HEADER_TX_DESC()
    }
}

/*****************************************************************************
 *  D-vector
 *****************************************************************************/
@pragma scratch_metadata
metadata eth_eq_qstate_d eth_eq_qstate;

@pragma scratch_metadata
metadata eth_tx_qstate_d eth_tx_qstate;

@pragma scratch_metadata
metadata eth_tx2_qstate_d eth_tx2_qstate;

@pragma scratch_metadata
metadata eth_tx_desc_d eth_tx_desc;

@pragma scratch_metadata
metadata eth_sg_desc_d eth_sg_desc;

/*****************************************************************************
 *  K-vector
 *****************************************************************************/

// Union with Common-TXDMA PHV headers

// Global PHV headers (Available in STAGES=ALL, MPUS=ALL)
@pragma pa_header_union ingress common_global
metadata eth_tx_global_k eth_tx_global;
@pragma scratch_metadata
metadata eth_tx_global_k eth_tx_global_scratch;

// To Stage N PHV headers (Available in STAGE=N, MPUS=ALL)

// to_stage_0 used as P-vector for eq_desc

@pragma pa_header_union ingress to_stage_1
metadata eth_tx_to_s1_k eth_tx_to_s1;
@pragma scratch_metadata
metadata eth_tx_to_s1_k eth_tx_to_s1_scratch;

@pragma pa_header_union ingress to_stage_2
metadata eth_tx_to_s2_k eth_tx_to_s2;
@pragma scratch_metadata
metadata eth_tx_to_s2_k eth_tx_to_s2_scratch;

@pragma pa_header_union ingress to_stage_3
metadata eth_tx_to_s3_k eth_tx_to_s3;
@pragma scratch_metadata
metadata eth_tx_to_s3_k eth_tx_to_s3_scratch;

// Stage to Stage headers (Available in STAGES=ALL, MPUS=N)
@pragma pa_header_union ingress common_t0_s2s
metadata eth_tx_t0_s2s_k eth_tx_t0_s2s;
@pragma scratch_metadata
metadata eth_tx_t0_s2s_k eth_tx_t0_s2s_scratch;

@pragma pa_header_union ingress common_t1_s2s
metadata eth_tx_t1_s2s_k eth_tx_t1_s2s;
@pragma scratch_metadata
metadata eth_tx_t1_s2s_k eth_tx_t1_s2s_scratch;

// common_t2_s2s used as P-vector for eq_desc

/*****************************************************************************
 * P-vector
 *****************************************************************************/

// Use to_stage_0 for cq_desc
@pragma pa_header_union ingress to_stage_0
@pragma dont_trim
metadata eth_cq_desc_p cq_desc;

// Use common_t2_s2s for eq_desc
@pragma pa_header_union ingress common_t2_s2s
@pragma dont_trim
metadata eth_eq_intr_desc_p eq_desc;

// Use the App Header from Flit0
@pragma dont_trim
@pragma pa_header_union ingress app_header
metadata p4plus_to_p4_classic_header_t eth_tx_app_hdr;

// DMA headers
@pragma pa_align 512
@pragma dont_trim
metadata dma_cmd_mem2pkt_t mem2pkt;
@pragma dont_trim
metadata dma_cmd_phv2pkt_t phv2pkt;
@pragma dont_trim
metadata dma_cmd_phv2mem_t phv2mem;
@pragma dont_trim
metadata dma_cmd_pkt2mem_t pkt2mem;
@pragma dont_trim
metadata dma_cmd_mem2mem_t mem2mem;
@pragma dont_trim
@pragma pa_header_union ingress mem2pkt phv2pkt phv2mem pkt2mem mem2mem
metadata dma_cmd_generic_t dma;
