

/*
 * egress.h
 * Mahesh Shirshyad (Pensando Systems)
 */

/* This file contains assembly level data structures for all Egress processing
 * needed for MPU to read and act on action data and action input. 
 *
 * Every Egress P4table after match hit can optionally provide
 *   1. Action Data (Parameters provided in P4 action functions)
 *   2. Action Input (Table Action routine using data extracted
 *                    into PHV either from header or result of
 *                    previous table action stored in PHV)
 */

/*
 * This file is generated from P4 program. Any changes made to this file will
 * be lost.
 */

/* TBD: In HBM case actiondata need to be packed before and after Key Fields
 * For now all actiondata follows Key 
 */


/* ASM Key Structure for p4-table 'twice_nat' */
/* P4-table 'twice_nat' is index table */

/* K + I fields */
struct twice_nat_k {
    union { /* Sourced from field/hdr union */
        /* FieldType = P */
        __pad_0 : 16; /* phvbit[0], Flit[0], FlitOffset[0] */
        /* FieldType = K */
        nat_metadata_twice_nat_idx : 16; /* phvbit[1168], Flit[2], FlitOffset[144] */
    };
    __pad_to_512b : 496;
};

/* K + D fields */
struct twice_nat_twice_nat_rewrite_info_d {

    ip : 128;
    l4_port : 16;
};

struct twice_nat_d {
    action_id : 8; /* TBD: Does this need to go after listing all actions? */
                   /* Answer depends on understanding bit layout/endianess.
                    * For now assuming action-pc in bitlayout comes before 
                    * action-data 
                    */
    union {
        struct twice_nat_twice_nat_rewrite_info_d  twice_nat_rewrite_info_d;
    } u;
    __pad_to_512b : 360;
};



/* ASM Key Structure for p4-table 'copp_action' */
/* P4-table 'copp_action' is index table */

/* K + I fields */
struct copp_action_k {
    /* FieldType = P */
    __pad_0 : 8; /* phvbit[0], Flit[0], FlitOffset[0] */
    union { /* Sourced from field/hdr union */
        /* FieldType = P */
        __pad_8 : 8; /* phvbit[0], Flit[0], FlitOffset[0] */
        /* FieldType = K */
        copp_metadata_policer_index : 8; /* phvbit[1184], Flit[2], FlitOffset[160] */
    };
    union { /* Sourced from field/hdr union */
        /* FieldType = P */
        __pad_16 : 16; /* phvbit[0], Flit[0], FlitOffset[0] */
        /* FieldType = I */
        control_metadata_packet_len : 16; /* phvbit[1392], Flit[2], FlitOffset[368] */
    };
    /* FieldType = P */
    __pad_32 : 1; /* phvbit[0], Flit[0], FlitOffset[0] */
    /* FieldType = I */
    copp_metadata_policer_color : 2; /* phvbit[3217], Flit[6], FlitOffset[145] */
    /* FieldType = P */
    __pad_35 : 5; /* phvbit[0], Flit[0], FlitOffset[0] */
    __pad_to_512b : 472;
};

/* K + D fields */
struct copp_action_copp_action_d {

    permitted_packets : 4;
    permitted_bytes : 18;
    denied_packets : 4;
    denied_bytes : 18;
};

struct copp_action_d {
    struct copp_action_copp_action_d  copp_action_d;
    __pad_to_512b : 468;
};



/* ASM Key Structure for p4-table 'rewrite' */
/* P4-table 'rewrite' is index table */

/* K + I fields */
struct rewrite_k {
    union { /* Sourced from field/hdr union */
        /* FieldType = P */
        __pad_0 : 128; /* phvbit[0], Flit[0], FlitOffset[0] */
        /* FieldType = I */
        nat_metadata_nat_ip : 128; /* phvbit[1024], Flit[2], FlitOffset[0] */
    };
    union { /* Sourced from field/hdr union */
        /* FieldType = P */
        __pad_128 : 16; /* phvbit[0], Flit[0], FlitOffset[0] */
        /* FieldType = I */
        nat_metadata_nat_l4_port : 16; /* phvbit[1152], Flit[2], FlitOffset[128] */
    };
    union { /* Sourced from field/hdr union */
        /* FieldType = P */
        __pad_144 : 16; /* phvbit[0], Flit[0], FlitOffset[0] */
        /* FieldType = K */
        rewrite_metadata_rewrite_index : 16; /* phvbit[1272], Flit[2], FlitOffset[248] */
    };
    /* FieldType = P */
    __pad_160 : 1; /* phvbit[0], Flit[0], FlitOffset[0] */
    union { /* Sourced from field/hdr union */
        struct {
            /* K/I = I */
            ipv4_valid : 1; /* phvbit[473], Flit[0], FlitOffset[473] */
            /* K/I = I */
            ipv6_valid : 1; /* phvbit[474], Flit[0], FlitOffset[474] */
            /* K/I = I */
            vlan_tag_valid : 1; /* phvbit[475], Flit[0], FlitOffset[475] */
        };
    };
    /* FieldType = P */
    __pad_164 : 4; /* phvbit[0], Flit[0], FlitOffset[0] */
    union { /* Sourced from field/hdr union */
        /* FieldType = P */
        __pad_168 : 8; /* phvbit[0], Flit[0], FlitOffset[0] */
        /* FieldType = I */
        rewrite_metadata_mac_sa_rewrite : 8; /* phvbit[1328], Flit[2], FlitOffset[304] */
    };
    union { /* Sourced from field/hdr union */
        /* FieldType = P */
        __pad_176 : 8; /* phvbit[0], Flit[0], FlitOffset[0] */
        /* FieldType = I */
        rewrite_metadata_mac_da_rewrite : 8; /* phvbit[1336], Flit[2], FlitOffset[312] */
    };
    union { /* Sourced from field/hdr union */
        /* FieldType = P */
        __pad_184 : 8; /* phvbit[0], Flit[0], FlitOffset[0] */
        /* FieldType = I */
        rewrite_metadata_ttl_dec : 8; /* phvbit[1344], Flit[2], FlitOffset[320] */
    };
    union { /* Sourced from field/hdr union */
        /* FieldType = P */
        __pad_256 : 8; /* phvbit[0], Flit[0], FlitOffset[0] */
        /* FieldType = I */
        qos_metadata_dscp_en : 8; /* phvbit[1360], Flit[2], FlitOffset[336] */
    };
    union { /* Sourced from field/hdr union */
        /* FieldType = P */
        __pad_264 : 8; /* phvbit[0], Flit[0], FlitOffset[0] */
        /* FieldType = I */
        qos_metadata_dscp : 8; /* phvbit[1368], Flit[2], FlitOffset[344] */
    };
    /* FieldType = I */
    vlan_tag_etherType : 16; /* phvbit[1568], Flit[3], FlitOffset[32] */
    /* FieldType = I */
    ipv6_hopLimit : 8; /* phvbit[1640], Flit[3], FlitOffset[104] */
    union { /* Sourced from field/hdr union */
        /* FieldType = P */
        __pad_296 : 8; /* phvbit[0], Flit[0], FlitOffset[0] */
        /* FieldType = I */
        ipv4_ttl : 8; /* phvbit[1648], Flit[3], FlitOffset[112] */
    };
    /* FieldType = I */
    nat_metadata_twice_nat_ip : 128; /* phvbit[3072], Flit[6], FlitOffset[0] */
    /* FieldType = I */
    nat_metadata_twice_nat_l4_port : 16; /* phvbit[3200], Flit[6], FlitOffset[128] */
    __pad_to_512b : 128;
};

/* K + D fields */
struct rewrite_ipv4_nat_dst_tcp_rewrite_d {

    mac_sa : 48;
    mac_da : 48;
};
struct rewrite_ipv6_nat_src_rewrite_d {

    mac_sa : 48;
    mac_da : 48;
};
struct rewrite_ipv4_twice_nat_tcp_rewrite_d {

    mac_sa : 48;
    mac_da : 48;
};
struct rewrite_ipv6_nat_src_tcp_rewrite_d {

    mac_sa : 48;
    mac_da : 48;
};
struct rewrite_ipv6_twice_nat_tcp_rewrite_d {

    mac_sa : 48;
    mac_da : 48;
};
struct rewrite_ipv4_twice_nat_rewrite_d {

    mac_sa : 48;
    mac_da : 48;
};
struct rewrite_l3_rewrite_d {

    mac_sa : 48;
    mac_da : 48;
};
struct rewrite_ipv6_twice_nat_rewrite_d {

    mac_sa : 48;
    mac_da : 48;
};
struct rewrite_ipv4_nat_dst_udp_rewrite_d {

    mac_sa : 48;
    mac_da : 48;
};
struct rewrite_ipv6_twice_nat_udp_rewrite_d {

    mac_sa : 48;
    mac_da : 48;
};
struct rewrite_ipv6_nat_dst_rewrite_d {

    mac_sa : 48;
    mac_da : 48;
};
struct rewrite_ipv4_nat_src_rewrite_d {

    mac_sa : 48;
    mac_da : 48;
};
struct rewrite_ipv6_nat_dst_tcp_rewrite_d {

    mac_sa : 48;
    mac_da : 48;
};
struct rewrite_ipv4_nat_src_udp_rewrite_d {

    mac_sa : 48;
    mac_da : 48;
};
struct rewrite_ipv4_nat_dst_rewrite_d {

    mac_sa : 48;
    mac_da : 48;
};
struct rewrite_ipv6_nat_src_udp_rewrite_d {

    mac_sa : 48;
    mac_da : 48;
};
struct rewrite_ipv6_nat_dst_udp_rewrite_d {

    mac_sa : 48;
    mac_da : 48;
};
struct rewrite_ipv4_nat_src_tcp_rewrite_d {

    mac_sa : 48;
    mac_da : 48;
};
struct rewrite_ipv4_twice_nat_udp_rewrite_d {

    mac_sa : 48;
    mac_da : 48;
};

struct rewrite_d {
    action_id : 8; /* TBD: Does this need to go after listing all actions? */
                   /* Answer depends on understanding bit layout/endianess.
                    * For now assuming action-pc in bitlayout comes before 
                    * action-data 
                    */
    union {
        struct rewrite_ipv4_nat_dst_tcp_rewrite_d  ipv4_nat_dst_tcp_rewrite_d;
        struct rewrite_ipv6_nat_src_rewrite_d  ipv6_nat_src_rewrite_d;
        struct rewrite_ipv4_twice_nat_tcp_rewrite_d  ipv4_twice_nat_tcp_rewrite_d;
        struct rewrite_ipv6_nat_src_tcp_rewrite_d  ipv6_nat_src_tcp_rewrite_d;
        struct rewrite_ipv6_twice_nat_tcp_rewrite_d  ipv6_twice_nat_tcp_rewrite_d;
        struct rewrite_ipv4_twice_nat_rewrite_d  ipv4_twice_nat_rewrite_d;
        struct rewrite_l3_rewrite_d  l3_rewrite_d;
        struct rewrite_ipv6_twice_nat_rewrite_d  ipv6_twice_nat_rewrite_d;
        struct rewrite_ipv4_nat_dst_udp_rewrite_d  ipv4_nat_dst_udp_rewrite_d;
        struct rewrite_ipv6_twice_nat_udp_rewrite_d  ipv6_twice_nat_udp_rewrite_d;
        struct rewrite_ipv6_nat_dst_rewrite_d  ipv6_nat_dst_rewrite_d;
        struct rewrite_ipv4_nat_src_rewrite_d  ipv4_nat_src_rewrite_d;
        struct rewrite_ipv6_nat_dst_tcp_rewrite_d  ipv6_nat_dst_tcp_rewrite_d;
        struct rewrite_ipv4_nat_src_udp_rewrite_d  ipv4_nat_src_udp_rewrite_d;
        struct rewrite_ipv4_nat_dst_rewrite_d  ipv4_nat_dst_rewrite_d;
        struct rewrite_ipv6_nat_src_udp_rewrite_d  ipv6_nat_src_udp_rewrite_d;
        struct rewrite_ipv6_nat_dst_udp_rewrite_d  ipv6_nat_dst_udp_rewrite_d;
        struct rewrite_ipv4_nat_src_tcp_rewrite_d  ipv4_nat_src_tcp_rewrite_d;
        struct rewrite_ipv4_twice_nat_udp_rewrite_d  ipv4_twice_nat_udp_rewrite_d;
    } u;
    __pad_to_512b : 408;
};



/* ASM Key Structure for p4-table 'egress_policer_action' */
/* P4-table 'egress_policer_action' is index table */

/* K + I fields */
struct egress_policer_action_k {
    /* FieldType = P */
    __pad_0 : 16; /* phvbit[0], Flit[0], FlitOffset[0] */
    union { /* Sourced from field/hdr union */
        /* FieldType = P */
        __pad_16 : 16; /* phvbit[0], Flit[0], FlitOffset[0] */
        /* FieldType = K */
        policer_metadata_egress_policer_index : 16; /* phvbit[1256], Flit[2], FlitOffset[232] */
    };
    union { /* Sourced from field/hdr union */
        /* FieldType = P */
        __pad_32 : 16; /* phvbit[0], Flit[0], FlitOffset[0] */
        /* FieldType = I */
        control_metadata_packet_len : 16; /* phvbit[1392], Flit[2], FlitOffset[368] */
    };
    /* FieldType = P */
    __pad_48 : 24; /* phvbit[0], Flit[0], FlitOffset[0] */
    /* FieldType = I */
    policer_metadata_egress_policer_color : 1; /* phvbit[3248], Flit[6], FlitOffset[176] */
    /* FieldType = P */
    __pad_73 : 7; /* phvbit[0], Flit[0], FlitOffset[0] */
    __pad_to_512b : 432;
};

/* K + D fields */
struct egress_policer_action_egress_policer_action_d {

    permitted_packets : 4;
    permitted_bytes : 18;
    denied_packets : 4;
    denied_bytes : 18;
};

struct egress_policer_action_d {
    struct egress_policer_action_egress_policer_action_d  egress_policer_action_d;
    __pad_to_512b : 468;
};



/* ASM Key Structure for p4-table 'replica' */
/* P4-table 'replica' Mpu/Keyless table.*/

/* K + I fields */
struct replica_k {
    union { /* Sourced from field/hdr union */
        struct {
            /* K/I = I */
            tm_replication_data_qtype : 3; /* phvbit[856], Flit[1], FlitOffset[344] */
            /* K/I = I */
            tm_replication_data_lif_sbit0_ebit4 : 5; /* phvbit[859], Flit[1], FlitOffset[347] */
        };
    };
    union { /* Sourced from field/hdr union */
        struct {
            /* K/I = I */
            tm_replication_data_lif_sbit5_ebit10 : 6; /* phvbit[859], Flit[1], FlitOffset[347] */
            /* K/I = I */
            tm_replication_data_tunnel_rewrite_index_sbit0_ebit1 : 2; /* phvbit[870], Flit[1], FlitOffset[358] */
        };
    };
    /* FieldType = I */
    tm_replication_data_tunnel_rewrite_index_sbit2_ebit9 : 8; /* phvbit[870], Flit[1], FlitOffset[358] */
    /* FieldType = I */
    tm_replication_data_qid_or_vnid : 24; /* phvbit[880], Flit[1], FlitOffset[368] */
    /* FieldType = P */
    __pad_48 : 4; /* phvbit[0], Flit[0], FlitOffset[0] */
    /* FieldType = I */
    tm_replication_data_rewrite_index : 12; /* phvbit[908], Flit[1], FlitOffset[396] */
    __pad_to_512b : 448;
};

/* K + D fields */




/* ASM Key Structure for p4-table 'mirror' */
/* P4-table 'mirror' is index table */

/* K + I fields */
struct mirror_k {
    /* FieldType = P */
    __pad_0 : 8; /* phvbit[0], Flit[0], FlitOffset[0] */
    /* FieldType = K */
    capri_intrinsic_tm_span_session : 8; /* phvbit[72], Flit[0], FlitOffset[72] */
    __pad_to_512b : 496;
};

/* K + D fields */
struct mirror_erspan_mirror_d {

    dst_lif : 11;
    truncate_len : 14;
    tunnel_rewrite_index : 10;
};
struct mirror_remote_span_d {

    dst_lif : 11;
    truncate_len : 14;
    tunnel_rewrite_index : 10;
    vlan : 24;
};
struct mirror_local_span_d {

    dst_lif : 11;
    truncate_len : 14;
};

struct mirror_d {
    action_id : 8; /* TBD: Does this need to go after listing all actions? */
                   /* Answer depends on understanding bit layout/endianess.
                    * For now assuming action-pc in bitlayout comes before 
                    * action-data 
                    */
    union {
        struct mirror_erspan_mirror_d  erspan_mirror_d;
        struct mirror_remote_span_d  remote_span_d;
        struct mirror_local_span_d  local_span_d;
    } u;
    __pad_to_512b : 445;
};



/* ASM Key Structure for p4-table 'tx_stats' */
/* P4-table 'tx_stats' is index table */

/* K + I fields */
struct tx_stats_k {
    union { /* Sourced from field/hdr union */
        /* FieldType = P */
        __pad_0 : 8; /* phvbit[0], Flit[0], FlitOffset[0] */
        /* FieldType = I */
        flow_lkp_metadata_pkt_type : 8; /* phvbit[1232], Flit[2], FlitOffset[208] */
    };
    /* FieldType = P */
    __pad_8 : 24; /* phvbit[0], Flit[0], FlitOffset[0] */
    union { /* Sourced from field/hdr union */
        /* FieldType = P */
        __pad_32 : 16; /* phvbit[0], Flit[0], FlitOffset[0] */
        /* FieldType = I */
        control_metadata_packet_len : 16; /* phvbit[1392], Flit[2], FlitOffset[368] */
    };
    union { /* Sourced from field/hdr union */
        /* FieldType = P */
        __pad_48 : 16; /* phvbit[0], Flit[0], FlitOffset[0] */
        /* FieldType = K */
        control_metadata_src_lif : 16; /* phvbit[1424], Flit[2], FlitOffset[400] */
    };
    /* FieldType = I */
    capri_intrinsic_drop : 1; /* phvbit[120], Flit[0], FlitOffset[120] */
    /* FieldType = P */
    __pad_65 : 15; /* phvbit[0], Flit[0], FlitOffset[0] */
    __pad_to_512b : 432;
};

/* K + D fields */
struct tx_stats_tx_stats_d {

    tx_ucast_pkts : 16;
    tx_mcast_pkts : 16;
    tx_bcast_pkts : 16;
    tx_ucast_bytes : 20;
    tx_mcast_bytes : 20;
    tx_bcast_bytes : 20;
    tx_egress_drops : 16;
};

struct tx_stats_d {
    struct tx_stats_tx_stats_d  tx_stats_d;
    __pad_to_512b : 388;
};



/* ASM Key Structure for p4-table 'ddos_src_dst_policer_action' */
/* P4-table 'ddos_src_dst_policer_action' is index table */

/* K + I fields */
struct ddos_src_dst_policer_action_k {
    union { /* Sourced from field/hdr union */
        /* FieldType = P */
        __pad_0 : 16; /* phvbit[0], Flit[0], FlitOffset[0] */
        /* FieldType = K */
        ddos_metadata_ddos_src_dst_policer_idx : 16; /* phvbit[1208], Flit[2], FlitOffset[184] */
    };
    /* FieldType = P */
    __pad_16 : 4; /* phvbit[0], Flit[0], FlitOffset[0] */
    /* FieldType = I */
    ddos_metadata_ddos_src_dst_policer_color : 2; /* phvbit[3236], Flit[6], FlitOffset[164] */
    /* FieldType = P */
    __pad_22 : 2; /* phvbit[0], Flit[0], FlitOffset[0] */
    __pad_to_512b : 488;
};

/* K + D fields */
struct ddos_src_dst_policer_action_ddos_src_dst_policer_action_d {

    ddos_src_dst_policer_saved_color : 2;
    ddos_src_dst_policer_dropped_packets : 22;
};

struct ddos_src_dst_policer_action_d {
    action_id : 8; /* TBD: Does this need to go after listing all actions? */
                   /* Answer depends on understanding bit layout/endianess.
                    * For now assuming action-pc in bitlayout comes before 
                    * action-data 
                    */
    union {
        struct ddos_src_dst_policer_action_ddos_src_dst_policer_action_d  ddos_src_dst_policer_action_d;
    } u;
    __pad_to_512b : 480;
};



/* ASM Key Structure for p4-table 'tunnel_decap' */
/* P4-table 'tunnel_decap' Mpu/Keyless table.*/

/* K + I fields */

/* K + D fields */




/* ASM Key Structure for p4-table 'ddos_src_dst_policer' */
/* P4-table 'ddos_src_dst_policer' is index table */

/* K + I fields */
struct ddos_src_dst_policer_k {
    union { /* Sourced from field/hdr union */
        /* FieldType = P */
        __pad_0 : 16; /* phvbit[0], Flit[0], FlitOffset[0] */
        /* FieldType = K */
        ddos_metadata_ddos_src_dst_policer_idx : 16; /* phvbit[1208], Flit[2], FlitOffset[184] */
    };
    __pad_to_512b : 496;
};

/* K + D fields */




/* ASM Key Structure for p4-table 'decode_roce_opcode' */
/* P4-table 'decode_roce_opcode' is index table */

/* K + I fields */
struct decode_roce_opcode_k {
    /* FieldType = P */
    __pad_0 : 8; /* phvbit[0], Flit[0], FlitOffset[0] */
    /* FieldType = K */
    roce_bth_opCode : 8; /* phvbit[2176], Flit[4], FlitOffset[128] */
    /* FieldType = P */
    __pad_16 : 4; /* phvbit[0], Flit[0], FlitOffset[0] */
    /* FieldType = I */
    capri_intrinsic_tm_oport : 4; /* phvbit[4], Flit[0], FlitOffset[4] */
    /* FieldType = I */
    roce_bth_destQP : 24; /* phvbit[2184], Flit[4], FlitOffset[136] */
    __pad_to_512b : 464;
};

/* K + D fields */
struct decode_roce_opcode_decode_roce_opcode_d {

    raw_flags : 16;
    len : 8;
    qtype : 3;
};

struct decode_roce_opcode_d {
    action_id : 8; /* TBD: Does this need to go after listing all actions? */
                   /* Answer depends on understanding bit layout/endianess.
                    * For now assuming action-pc in bitlayout comes before 
                    * action-data 
                    */
    union {
        struct decode_roce_opcode_decode_roce_opcode_d  decode_roce_opcode_d;
    } u;
    __pad_to_512b : 477;
};



/* ASM Key Structure for p4-table 'tunnel_encap_update_inner' */
/* P4-table 'tunnel_encap_update_inner' is index table */

/* K + I fields */
struct tunnel_encap_update_inner_k {
    /* FieldType = P */
    __pad_8 : 3; /* phvbit[0], Flit[0], FlitOffset[0] */
    /* FieldType = I */
    tcp_valid : 1; /* phvbit[426], Flit[0], FlitOffset[426] */
    /* FieldType = I */
    icmp_valid : 1; /* phvbit[427], Flit[0], FlitOffset[427] */
    /* FieldType = I */
    udp_valid : 1; /* phvbit[443], Flit[0], FlitOffset[443] */
    /* FieldType = I */
    ipv4_valid : 1; /* phvbit[473], Flit[0], FlitOffset[473] */
    /* FieldType = I */
    ipv6_valid : 1; /* phvbit[474], Flit[0], FlitOffset[474] */
    union { /* Sourced from field/hdr union */
        struct {
            /* K/I = I */
            ipv6_version : 4; /* phvbit[1584], Flit[3], FlitOffset[48] */
            /* K/I = I */
            ipv6_trafficClass_sbit0_ebit3 : 4; /* phvbit[1588], Flit[3], FlitOffset[52] */
        };
        struct {
            /* K/I = I */
            ipv4_version : 4; /* phvbit[1584], Flit[3], FlitOffset[48] */
            /* K/I = I */
            ipv4_ihl : 4; /* phvbit[1588], Flit[3], FlitOffset[52] */
        };
    };
    union { /* Sourced from field/hdr union */
        struct {
            /* K/I = I */
            ipv6_trafficClass_sbit4_ebit7 : 4; /* phvbit[1588], Flit[3], FlitOffset[52] */
            /* K/I = I */
            ipv6_flowLabel_sbit0_ebit3 : 4; /* phvbit[1596], Flit[3], FlitOffset[60] */
        };
        /* FieldType = I */
        ipv4_diffserv : 8; /* phvbit[1592], Flit[3], FlitOffset[56] */
    };
    union { /* Sourced from field/hdr union */
        /* FieldType = I */
        ipv6_flowLabel_sbit4_ebit19 : 16; /* phvbit[1596], Flit[3], FlitOffset[60] */
        /* FieldType = I */
        ipv4_totalLen : 16; /* phvbit[1600], Flit[3], FlitOffset[64] */
    };
    union { /* Sourced from field/hdr union */
        /* FieldType = I */
        ipv6_payloadLen : 16; /* phvbit[1616], Flit[3], FlitOffset[80] */
        /* FieldType = I */
        ipv4_identification : 16; /* phvbit[1616], Flit[3], FlitOffset[80] */
    };
    union { /* Sourced from field/hdr union */
        /* FieldType = I */
        ipv6_nextHdr : 8; /* phvbit[1632], Flit[3], FlitOffset[96] */
        struct {
            /* K/I = I */
            ipv4_flags : 3; /* phvbit[1632], Flit[3], FlitOffset[96] */
            /* K/I = I */
            ipv4_fragOffset_sbit0_ebit4 : 5; /* phvbit[1635], Flit[3], FlitOffset[99] */
        };
    };
    union { /* Sourced from field/hdr union */
        /* FieldType = I */
        ipv6_hopLimit : 8; /* phvbit[1640], Flit[3], FlitOffset[104] */
        /* FieldType = I */
        ipv4_fragOffset_sbit5_ebit12 : 8; /* phvbit[1635], Flit[3], FlitOffset[99] */
    };
    union { /* Sourced from field/hdr union */
        /* FieldType = I */
        ipv6_srcAddr_sbit0_ebit7 : 8; /* phvbit[1648], Flit[3], FlitOffset[112] */
        /* FieldType = I */
        ipv4_ttl : 8; /* phvbit[1648], Flit[3], FlitOffset[112] */
    };
    union { /* Sourced from field/hdr union */
        /* FieldType = I */
        ipv6_srcAddr_sbit8_ebit15 : 8; /* phvbit[1648], Flit[3], FlitOffset[112] */
        /* FieldType = I */
        ipv4_protocol : 8; /* phvbit[1656], Flit[3], FlitOffset[120] */
    };
    union { /* Sourced from field/hdr union */
        /* FieldType = I */
        ipv6_srcAddr_sbit16_ebit31 : 16; /* phvbit[1648], Flit[3], FlitOffset[112] */
        /* FieldType = I */
        ipv4_hdrChecksum : 16; /* phvbit[1664], Flit[3], FlitOffset[128] */
    };
    union { /* Sourced from field/hdr union */
        /* FieldType = I */
        ipv6_srcAddr_sbit32_ebit63 : 32; /* phvbit[1648], Flit[3], FlitOffset[112] */
        /* FieldType = I */
        ipv4_srcAddr : 32; /* phvbit[1680], Flit[3], FlitOffset[144] */
    };
    union { /* Sourced from field/hdr union */
        /* FieldType = I */
        ipv6_srcAddr_sbit64_ebit95 : 32; /* phvbit[1648], Flit[3], FlitOffset[112] */
        /* FieldType = I */
        ipv4_dstAddr : 32; /* phvbit[1712], Flit[3], FlitOffset[176] */
    };
    /* FieldType = I */
    ipv6_srcAddr_sbit96_ebit127 : 32; /* phvbit[1648], Flit[3], FlitOffset[112] */
    /* FieldType = I */
    ipv6_dstAddr : 128; /* phvbit[1776], Flit[3], FlitOffset[240] */
    /* FieldType = I */
    udp_srcPort : 16; /* phvbit[2112], Flit[4], FlitOffset[64] */
    /* FieldType = I */
    udp_dstPort : 16; /* phvbit[2128], Flit[4], FlitOffset[80] */
    /* FieldType = I */
    udp_len : 16; /* phvbit[2144], Flit[4], FlitOffset[96] */
    /* FieldType = I */
    udp_checksum : 16; /* phvbit[2160], Flit[4], FlitOffset[112] */
    /* FieldType = I */
    nat_metadata_update_checksum : 1; /* phvbit[3216], Flit[6], FlitOffset[144] */
    /* FieldType = P */
    __pad_449 : 7; /* phvbit[0], Flit[0], FlitOffset[0] */
    __pad_to_512b : 112;
};

/* K + D fields */

struct tunnel_encap_update_inner_d {
    action_id : 8; /* TBD: Does this need to go after listing all actions? */
                   /* Answer depends on understanding bit layout/endianess.
                    * For now assuming action-pc in bitlayout comes before 
                    * action-data 
                    */
    __pad_to_512b : 504;
};



/* ASM Key Structure for p4-table 'egress_policer' */
/* P4-table 'egress_policer' is index table */

/* K + I fields */
struct egress_policer_k {
    union { /* Sourced from field/hdr union */
        /* FieldType = P */
        __pad_0 : 16; /* phvbit[0], Flit[0], FlitOffset[0] */
        /* FieldType = K */
        policer_metadata_egress_policer_index : 16; /* phvbit[1256], Flit[2], FlitOffset[232] */
    };
    /* FieldType = I */
    policer_metadata_egress_policer_color : 1; /* phvbit[3248], Flit[6], FlitOffset[176] */
    /* FieldType = P */
    __pad_17 : 7; /* phvbit[0], Flit[0], FlitOffset[0] */
    __pad_to_512b : 488;
};

/* K + D fields */




/* ASM Key Structure for p4-table 'ddos_service_policer' */
/* P4-table 'ddos_service_policer' is index table */

/* K + I fields */
struct ddos_service_policer_k {
    union { /* Sourced from field/hdr union */
        /* FieldType = P */
        __pad_0 : 16; /* phvbit[0], Flit[0], FlitOffset[0] */
        /* FieldType = K */
        ddos_metadata_ddos_service_policer_idx : 16; /* phvbit[1192], Flit[2], FlitOffset[168] */
    };
    __pad_to_512b : 496;
};

/* K + D fields */




/* ASM Key Structure for p4-table 'ddos_service_policer_action' */
/* P4-table 'ddos_service_policer_action' is index table */

/* K + I fields */
struct ddos_service_policer_action_k {
    union { /* Sourced from field/hdr union */
        /* FieldType = P */
        __pad_0 : 16; /* phvbit[0], Flit[0], FlitOffset[0] */
        /* FieldType = K */
        ddos_metadata_ddos_service_policer_idx : 16; /* phvbit[1192], Flit[2], FlitOffset[168] */
    };
    /* FieldType = P */
    __pad_16 : 2; /* phvbit[0], Flit[0], FlitOffset[0] */
    /* FieldType = I */
    ddos_metadata_ddos_service_policer_color : 2; /* phvbit[3234], Flit[6], FlitOffset[162] */
    /* FieldType = P */
    __pad_20 : 4; /* phvbit[0], Flit[0], FlitOffset[0] */
    __pad_to_512b : 488;
};

/* K + D fields */
struct ddos_service_policer_action_ddos_service_policer_action_d {

    ddos_service_policer_saved_color : 2;
    ddos_service_policer_dropped_packets : 22;
};

struct ddos_service_policer_action_d {
    action_id : 8; /* TBD: Does this need to go after listing all actions? */
                   /* Answer depends on understanding bit layout/endianess.
                    * For now assuming action-pc in bitlayout comes before 
                    * action-data 
                    */
    union {
        struct ddos_service_policer_action_ddos_service_policer_action_d  ddos_service_policer_action_d;
    } u;
    __pad_to_512b : 480;
};



/* ASM Key Structure for p4-table 'ddos_src_vf_policer' */
/* P4-table 'ddos_src_vf_policer' is index table */

/* K + I fields */
struct ddos_src_vf_policer_k {
    /* FieldType = P */
    __pad_0 : 6; /* phvbit[0], Flit[0], FlitOffset[0] */
    /* FieldType = K */
    ddos_metadata_ddos_src_vf_policer_idx : 10; /* phvbit[3222], Flit[6], FlitOffset[150] */
    __pad_to_512b : 496;
};

/* K + D fields */




/* ASM Key Structure for p4-table 'copp' */
/* P4-table 'copp' is index table */

/* K + I fields */
struct copp_k {
    /* FieldType = P */
    __pad_0 : 8; /* phvbit[0], Flit[0], FlitOffset[0] */
    union { /* Sourced from field/hdr union */
        /* FieldType = P */
        __pad_8 : 8; /* phvbit[0], Flit[0], FlitOffset[0] */
        /* FieldType = K */
        copp_metadata_policer_index : 8; /* phvbit[1184], Flit[2], FlitOffset[160] */
    };
    /* FieldType = P */
    __pad_16 : 1; /* phvbit[0], Flit[0], FlitOffset[0] */
    /* FieldType = I */
    copp_metadata_policer_color : 2; /* phvbit[3217], Flit[6], FlitOffset[145] */
    /* FieldType = P */
    __pad_19 : 5; /* phvbit[0], Flit[0], FlitOffset[0] */
    __pad_to_512b : 488;
};

/* K + D fields */

struct copp_d {
    action_id : 8; /* TBD: Does this need to go after listing all actions? */
                   /* Answer depends on understanding bit layout/endianess.
                    * For now assuming action-pc in bitlayout comes before 
                    * action-data 
                    */
    __pad_to_512b : 504;
};



/* ASM Key Structure for p4-table 'ddos_src_vf_policer_action' */
/* P4-table 'ddos_src_vf_policer_action' is index table */

/* K + I fields */
struct ddos_src_vf_policer_action_k {
    /* FieldType = P */
    __pad_0 : 6; /* phvbit[0], Flit[0], FlitOffset[0] */
    /* FieldType = K */
    ddos_metadata_ddos_src_vf_policer_idx : 10; /* phvbit[3222], Flit[6], FlitOffset[150] */
    /* FieldType = I */
    ddos_metadata_ddos_src_vf_policer_color : 2; /* phvbit[3232], Flit[6], FlitOffset[160] */
    /* FieldType = P */
    __pad_18 : 6; /* phvbit[0], Flit[0], FlitOffset[0] */
    __pad_to_512b : 488;
};

/* K + D fields */
struct ddos_src_vf_policer_action_ddos_src_vf_policer_action_d {

    ddos_src_vf_policer_saved_color : 2;
    ddos_src_vf_policer_dropped_packets : 22;
};

struct ddos_src_vf_policer_action_d {
    action_id : 8; /* TBD: Does this need to go after listing all actions? */
                   /* Answer depends on understanding bit layout/endianess.
                    * For now assuming action-pc in bitlayout comes before 
                    * action-data 
                    */
    union {
        struct ddos_src_vf_policer_action_ddos_src_vf_policer_action_d  ddos_src_vf_policer_action_d;
    } u;
    __pad_to_512b : 480;
};



/* ASM Key Structure for p4-table 'tunnel_decap_copy_inner' */
/* P4-table 'tunnel_decap_copy_inner' is index table */

/* K + I fields */
struct tunnel_decap_copy_inner_k {
    /* FieldType = I */
    inner_ethernet_dstAddr : 48; /* phvbit[2304], Flit[4], FlitOffset[256] */
    /* FieldType = I */
    inner_ethernet_srcAddr : 48; /* phvbit[2352], Flit[4], FlitOffset[304] */
    /* FieldType = I */
    inner_ethernet_etherType : 16; /* phvbit[2400], Flit[4], FlitOffset[352] */
    union { /* Sourced from field/hdr union */
        struct {
            /* K/I = I */
            inner_ipv6_version : 4; /* phvbit[2416], Flit[4], FlitOffset[368] */
            /* K/I = I */
            inner_ipv6_trafficClass_sbit0_ebit3 : 4; /* phvbit[2420], Flit[4], FlitOffset[372] */
        };
        struct {
            /* K/I = I */
            inner_ipv4_version : 4; /* phvbit[2416], Flit[4], FlitOffset[368] */
            /* K/I = I */
            inner_ipv4_ihl : 4; /* phvbit[2420], Flit[4], FlitOffset[372] */
        };
    };
    union { /* Sourced from field/hdr union */
        struct {
            /* K/I = I */
            inner_ipv6_trafficClass_sbit4_ebit7 : 4; /* phvbit[2420], Flit[4], FlitOffset[372] */
            /* K/I = I */
            inner_ipv6_flowLabel_sbit0_ebit3 : 4; /* phvbit[2428], Flit[4], FlitOffset[380] */
        };
        /* FieldType = I */
        inner_ipv4_diffserv : 8; /* phvbit[2424], Flit[4], FlitOffset[376] */
    };
    union { /* Sourced from field/hdr union */
        /* FieldType = I */
        inner_ipv6_flowLabel_sbit4_ebit19 : 16; /* phvbit[2428], Flit[4], FlitOffset[380] */
        /* FieldType = I */
        inner_ipv4_totalLen : 16; /* phvbit[2432], Flit[4], FlitOffset[384] */
    };
    union { /* Sourced from field/hdr union */
        /* FieldType = I */
        inner_ipv6_payloadLen : 16; /* phvbit[2448], Flit[4], FlitOffset[400] */
        /* FieldType = I */
        inner_ipv4_identification : 16; /* phvbit[2448], Flit[4], FlitOffset[400] */
    };
    union { /* Sourced from field/hdr union */
        /* FieldType = I */
        inner_ipv6_nextHdr : 8; /* phvbit[2464], Flit[4], FlitOffset[416] */
        struct {
            /* K/I = I */
            inner_ipv4_flags : 3; /* phvbit[2464], Flit[4], FlitOffset[416] */
            /* K/I = I */
            inner_ipv4_fragOffset_sbit0_ebit4 : 5; /* phvbit[2467], Flit[4], FlitOffset[419] */
        };
    };
    union { /* Sourced from field/hdr union */
        /* FieldType = I */
        inner_ipv6_hopLimit : 8; /* phvbit[2472], Flit[4], FlitOffset[424] */
        /* FieldType = I */
        inner_ipv4_fragOffset_sbit5_ebit12 : 8; /* phvbit[2467], Flit[4], FlitOffset[419] */
    };
    union { /* Sourced from field/hdr union */
        /* FieldType = I */
        inner_ipv6_srcAddr_sbit0_ebit7 : 8; /* phvbit[2560], Flit[5], FlitOffset[0] */
        /* FieldType = I */
        inner_ipv4_ttl : 8; /* phvbit[2560], Flit[5], FlitOffset[0] */
    };
    union { /* Sourced from field/hdr union */
        /* FieldType = I */
        inner_ipv6_srcAddr_sbit8_ebit15 : 8; /* phvbit[2560], Flit[5], FlitOffset[0] */
        /* FieldType = I */
        inner_ipv4_protocol : 8; /* phvbit[2568], Flit[5], FlitOffset[8] */
    };
    union { /* Sourced from field/hdr union */
        /* FieldType = I */
        inner_ipv6_srcAddr_sbit16_ebit31 : 16; /* phvbit[2560], Flit[5], FlitOffset[0] */
        /* FieldType = I */
        inner_ipv4_hdrChecksum : 16; /* phvbit[2576], Flit[5], FlitOffset[16] */
    };
    union { /* Sourced from field/hdr union */
        /* FieldType = I */
        inner_ipv6_srcAddr_sbit32_ebit63 : 32; /* phvbit[2560], Flit[5], FlitOffset[0] */
        /* FieldType = I */
        inner_ipv4_srcAddr : 32; /* phvbit[2592], Flit[5], FlitOffset[32] */
    };
    union { /* Sourced from field/hdr union */
        /* FieldType = I */
        inner_ipv6_srcAddr_sbit64_ebit71 : 8; /* phvbit[2560], Flit[5], FlitOffset[0] */
        /* FieldType = I */
        inner_ipv4_dstAddr_sbit0_ebit7 : 8; /* phvbit[2624], Flit[5], FlitOffset[64] */
    };
    /* FieldType = P */
    __pad_248 : 4; /* phvbit[0], Flit[0], FlitOffset[0] */
    /* FieldType = I */
    inner_udp_valid : 1; /* phvbit[428], Flit[0], FlitOffset[428] */
    /* FieldType = I */
    inner_ipv4_valid : 1; /* phvbit[436], Flit[0], FlitOffset[436] */
    /* FieldType = I */
    inner_ipv6_valid : 1; /* phvbit[437], Flit[0], FlitOffset[437] */
    /* FieldType = I */
    inner_ethernet_valid : 1; /* phvbit[438], Flit[0], FlitOffset[438] */
    union { /* Sourced from field/hdr union */
        /* FieldType = I */
        inner_ipv6_srcAddr_sbit72_ebit95 : 24; /* phvbit[2560], Flit[5], FlitOffset[0] */
        /* FieldType = I */
        inner_ipv4_dstAddr_sbit8_ebit31 : 24; /* phvbit[2624], Flit[5], FlitOffset[64] */
    };
    /* FieldType = I */
    inner_ipv6_srcAddr_sbit96_ebit127 : 32; /* phvbit[2560], Flit[5], FlitOffset[0] */
    /* FieldType = I */
    inner_ipv6_dstAddr : 128; /* phvbit[2688], Flit[5], FlitOffset[128] */
    /* FieldType = I */
    inner_udp_srcPort : 16; /* phvbit[2816], Flit[5], FlitOffset[256] */
    /* FieldType = I */
    inner_udp_dstPort : 16; /* phvbit[2832], Flit[5], FlitOffset[272] */
    /* FieldType = I */
    inner_udp_len : 16; /* phvbit[2848], Flit[5], FlitOffset[288] */
    /* FieldType = I */
    inner_udp_checksum : 16; /* phvbit[2864], Flit[5], FlitOffset[304] */
    __pad_to_512b : 8;
};

/* K + D fields */

struct tunnel_decap_copy_inner_d {
    action_id : 8; /* TBD: Does this need to go after listing all actions? */
                   /* Answer depends on understanding bit layout/endianess.
                    * For now assuming action-pc in bitlayout comes before 
                    * action-data 
                    */
    __pad_to_512b : 504;
};



/* ASM Key Structure for p4-table 'ddos_src_vf' */
/* P4-table 'ddos_src_vf' ternary table.*/

/* K + I fields */
struct ddos_src_vf_k {
    union { /* Sourced from field/hdr union */
        /* FieldType = P */
        __pad_0 : 8; /* phvbit[0], Flit[0], FlitOffset[0] */
        /* FieldType = I */
        flow_lkp_metadata_lkp_proto : 8; /* phvbit[1224], Flit[2], FlitOffset[200] */
    };
    union { /* Sourced from field/hdr union */
        /* FieldType = P */
        __pad_8 : 16; /* phvbit[0], Flit[0], FlitOffset[0] */
        /* FieldType = K */
        control_metadata_src_lif : 16; /* phvbit[1424], Flit[2], FlitOffset[400] */
    };
    union { /* Sourced from field/hdr union */
        /* FieldType = P */
        __pad_24 : 8; /* phvbit[0], Flit[0], FlitOffset[0] */
        /* FieldType = I */
        tcp_flags : 8; /* phvbit[2848], Flit[5], FlitOffset[288] */
    };
    __pad_to_512b : 480;
};

/* K + D fields */
struct ddos_src_vf_ddos_src_vf_hit_d {

    ddos_src_vf_base_policer_idx : 10;
};

struct ddos_src_vf_d {
    action_id : 8; /* TBD: Does this need to go after listing all actions? */
                   /* Answer depends on understanding bit layout/endianess.
                    * For now assuming action-pc in bitlayout comes before 
                    * action-data 
                    */
    union {
        struct ddos_src_vf_ddos_src_vf_hit_d  ddos_src_vf_hit_d;
    } u;
    __pad_to_512b : 494;
};



/* ASM Key Structure for p4-table 'output_mapping' */
/* P4-table 'output_mapping' is index table */

/* K + I fields */
struct output_mapping_k {
    /* FieldType = P */
    __pad_0 : 4; /* phvbit[0], Flit[0], FlitOffset[0] */
    /* FieldType = I */
    capri_intrinsic_tm_oport : 4; /* phvbit[4], Flit[0], FlitOffset[4] */
    /* FieldType = P */
    __pad_8 : 13; /* phvbit[0], Flit[0], FlitOffset[0] */
    /* FieldType = K */
    capri_intrinsic_lif : 11; /* phvbit[13], Flit[0], FlitOffset[13] */
    /* FieldType = P */
    __pad_32 : 4; /* phvbit[0], Flit[0], FlitOffset[0] */
    /* FieldType = I */
    capri_intrinsic_tm_instance_type : 4; /* phvbit[84], Flit[0], FlitOffset[84] */
    /* FieldType = P */
    __pad_40 : 3; /* phvbit[0], Flit[0], FlitOffset[0] */
    /* FieldType = I */
    vlan_tag_valid : 1; /* phvbit[475], Flit[0], FlitOffset[475] */
    /* FieldType = P */
    __pad_44 : 4; /* phvbit[0], Flit[0], FlitOffset[0] */
    /* FieldType = I */
    rewrite_metadata_entropy_hash : 16; /* phvbit[512], Flit[1], FlitOffset[0] */
    /* FieldType = I */
    control_metadata_egress_mirror_session_id : 8; /* phvbit[528], Flit[1], FlitOffset[16] */
    /* FieldType = I */
    ethernet_etherType : 16; /* phvbit[1536], Flit[3], FlitOffset[0] */
    /* FieldType = I */
    vlan_tag_etherType : 16; /* phvbit[1568], Flit[3], FlitOffset[32] */
    __pad_to_512b : 408;
};

/* K + D fields */
struct output_mapping_redirect_to_cpu_d {

    tunnel_index : 10;
    egress_mirror_en : 1;
};
struct output_mapping_set_tm_oport_d {

    vlan_tag_in_skb : 1;
    nports : 4;
    egress_mirror_en : 1;
    egress_port1 : 4;
    egress_port2 : 4;
    egress_port3 : 4;
    egress_port4 : 4;
    egress_port5 : 4;
    egress_port6 : 4;
    egress_port7 : 4;
    egress_port8 : 4;
};
struct output_mapping_redirect_to_remote_d {

    tunnel_index : 10;
    tm_oport : 4;
    egress_mirror_en : 1;
};

struct output_mapping_d {
    action_id : 8; /* TBD: Does this need to go after listing all actions? */
                   /* Answer depends on understanding bit layout/endianess.
                    * For now assuming action-pc in bitlayout comes before 
                    * action-data 
                    */
    union {
        struct output_mapping_redirect_to_cpu_d  redirect_to_cpu_d;
        struct output_mapping_set_tm_oport_d  set_tm_oport_d;
        struct output_mapping_redirect_to_remote_d  redirect_to_remote_d;
    } u;
    __pad_to_512b : 466;
};



/* ASM Key Structure for p4-table 'tunnel_rewrite' */
/* P4-table 'tunnel_rewrite' is index table */

/* K + I fields */
struct tunnel_rewrite_k {
    union { /* Sourced from field/hdr union */
        /* FieldType = P */
        __pad_0 : 16; /* phvbit[0], Flit[0], FlitOffset[0] */
        /* FieldType = K */
        rewrite_metadata_tunnel_rewrite_index : 16; /* phvbit[1288], Flit[2], FlitOffset[264] */
    };
    /* FieldType = I */
    capri_intrinsic_timestamp : 48; /* phvbit[24], Flit[0], FlitOffset[24] */
    /* FieldType = I */
    capri_intrinsic_tm_span_session : 8; /* phvbit[72], Flit[0], FlitOffset[72] */
    /* FieldType = P */
    __pad_72 : 6; /* phvbit[0], Flit[0], FlitOffset[0] */
    /* FieldType = I */
    ethernet_valid : 1; /* phvbit[478], Flit[0], FlitOffset[478] */
    /* FieldType = P */
    __pad_79 : 1; /* phvbit[0], Flit[0], FlitOffset[0] */
    /* FieldType = I */
    rewrite_metadata_entropy_hash : 16; /* phvbit[512], Flit[1], FlitOffset[0] */
    union { /* Sourced from field/hdr union */
        /* FieldType = P */
        __pad_96 : 24; /* phvbit[0], Flit[0], FlitOffset[0] */
        /* FieldType = I */
        rewrite_metadata_tunnel_vnid : 24; /* phvbit[1304], Flit[2], FlitOffset[280] */
    };
    union { /* Sourced from field/hdr union */
        /* FieldType = P */
        __pad_120 : 8; /* phvbit[0], Flit[0], FlitOffset[0] */
        /* FieldType = I */
        qos_metadata_cos : 8; /* phvbit[1352], Flit[2], FlitOffset[328] */
    };
    union { /* Sourced from field/hdr union */
        /* FieldType = P */
        __pad_128 : 16; /* phvbit[0], Flit[0], FlitOffset[0] */
        /* FieldType = I */
        l3_metadata_payload_length : 16; /* phvbit[1376], Flit[2], FlitOffset[352] */
    };
    /* FieldType = I */
    ethernet_dstAddr : 48; /* phvbit[1440], Flit[2], FlitOffset[416] */
    /* FieldType = I */
    ethernet_srcAddr : 48; /* phvbit[1488], Flit[2], FlitOffset[464] */
    /* FieldType = I */
    ethernet_etherType : 16; /* phvbit[1536], Flit[3], FlitOffset[0] */
    union { /* Sourced from field/hdr union */
        /* FieldType = P */
        __pad_360 : 16; /* phvbit[0], Flit[0], FlitOffset[0] */
        /* FieldType = I */
        ipv4_totalLen : 16; /* phvbit[1600], Flit[3], FlitOffset[64] */
    };
    /* FieldType = I */
    ipv6_payloadLen : 16; /* phvbit[1616], Flit[3], FlitOffset[80] */
    /* FieldType = I */
    tunnel_metadata_inner_ip_proto : 8; /* phvbit[3240], Flit[6], FlitOffset[168] */
    __pad_to_512b : 216;
};

/* K + D fields */
struct tunnel_rewrite_encap_ipv6_ipsec_tunnel_esp_d {

    mac_sa : 48;
    mac_da : 48;
    ip_sa : 128;
    ip_da : 128;
};
struct tunnel_rewrite_encap_ipv4_ipsec_tunnel_esp_d {

    mac_sa : 48;
    mac_da : 48;
    ip_sa : 32;
    ip_da : 32;
};
struct tunnel_rewrite_encap_erspan_d {

    mac_sa : 48;
    mac_da : 48;
    ip_sa : 128;
    ip_da : 128;
    ip_type : 1;
    vlan_valid : 1;
    vlan_id : 12;
};
struct tunnel_rewrite_encap_vlan_ipv6_ipsec_tunnel_esp_d {

    mac_sa : 48;
    mac_da : 48;
    ip_sa : 128;
    ip_da : 128;
    vlan_id : 12;
};
struct tunnel_rewrite_encap_vlan_ipv4_ipsec_tunnel_esp_d {

    mac_sa : 48;
    mac_da : 48;
    ip_sa : 32;
    ip_da : 32;
    vlan_id : 12;
};
struct tunnel_rewrite_encap_gre_d {

    mac_sa : 48;
    mac_da : 48;
    ip_sa : 128;
    ip_da : 128;
    ip_type : 1;
    vlan_valid : 1;
    vlan_id : 12;
};
struct tunnel_rewrite_encap_vxlan_gpe_d {

    mac_sa : 48;
    mac_da : 48;
    ip_sa : 128;
    ip_da : 128;
    ip_type : 1;
    vlan_valid : 1;
    vlan_id : 12;
};
struct tunnel_rewrite_encap_vxlan_d {

    mac_sa : 48;
    mac_da : 48;
    ip_sa : 128;
    ip_da : 128;
    ip_type : 1;
    vlan_valid : 1;
    vlan_id : 12;
};
struct tunnel_rewrite_encap_ip_d {

    mac_sa : 48;
    mac_da : 48;
    ip_sa : 128;
    ip_da : 128;
    ip_type : 1;
    vlan_valid : 1;
    vlan_id : 12;
};
struct tunnel_rewrite_encap_mpls_d {

    mac_sa : 48;
    mac_da : 48;
    eompls : 1;
    num_labels : 2;
    label0 : 20;
    exp0 : 3;
    bos0 : 1;
    ttl0 : 8;
    label1 : 20;
    exp1 : 3;
    bos1 : 1;
    ttl1 : 8;
    label2 : 20;
    exp2 : 3;
    bos2 : 1;
    ttl2 : 8;
};
struct tunnel_rewrite_encap_genv_d {

    mac_sa : 48;
    mac_da : 48;
    ip_sa : 128;
    ip_da : 128;
    ip_type : 1;
    vlan_valid : 1;
    vlan_id : 12;
};
struct tunnel_rewrite_encap_nvgre_d {

    mac_sa : 48;
    mac_da : 48;
    ip_sa : 128;
    ip_da : 128;
    ip_type : 1;
    vlan_valid : 1;
    vlan_id : 12;
};

struct tunnel_rewrite_d {
    action_id : 8; /* TBD: Does this need to go after listing all actions? */
                   /* Answer depends on understanding bit layout/endianess.
                    * For now assuming action-pc in bitlayout comes before 
                    * action-data 
                    */
    union {
        struct tunnel_rewrite_encap_ipv6_ipsec_tunnel_esp_d  encap_ipv6_ipsec_tunnel_esp_d;
        struct tunnel_rewrite_encap_ipv4_ipsec_tunnel_esp_d  encap_ipv4_ipsec_tunnel_esp_d;
        struct tunnel_rewrite_encap_erspan_d  encap_erspan_d;
        struct tunnel_rewrite_encap_vlan_ipv6_ipsec_tunnel_esp_d  encap_vlan_ipv6_ipsec_tunnel_esp_d;
        struct tunnel_rewrite_encap_vlan_ipv4_ipsec_tunnel_esp_d  encap_vlan_ipv4_ipsec_tunnel_esp_d;
        struct tunnel_rewrite_encap_gre_d  encap_gre_d;
        struct tunnel_rewrite_encap_vxlan_gpe_d  encap_vxlan_gpe_d;
        struct tunnel_rewrite_encap_vxlan_d  encap_vxlan_d;
        struct tunnel_rewrite_encap_ip_d  encap_ip_d;
        struct tunnel_rewrite_encap_mpls_d  encap_mpls_d;
        struct tunnel_rewrite_encap_genv_d  encap_genv_d;
        struct tunnel_rewrite_encap_nvgre_d  encap_nvgre_d;
    } u;
    __pad_to_512b : 138;
};


