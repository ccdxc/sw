

/*
 * ingress.h
 * Mahesh Shirshyad (Pensando Systems)
 */

/* This file contains assembly level data structures for all Ingress processing
 * needed for MPU to read and act on action data and action input. 
 *
 * Every Ingress P4table after match hit can optionally provide
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


/* ASM Key Structure for p4-table 'D_mpu_only' */
/* P4-table 'D_mpu_only' Mpu/Keyless table.*/

/* K + I fields */

/* K + D fields */




/* ASM Key Structure for p4-table 'C_idx' */
/* P4-table 'C_idx' is index table */

/* K + I fields */
struct C_idx_k {
    /* FieldType = P */
    __pad_0 : 8; /* phvbit[0], Flit[0], FlitOffset[0] */
    /* FieldType = K */
    ipv4_ttl : 8; /* phvbit[768], Flit[1], FlitOffset[256] */
    __pad_to_512b : 496;
};

/* K + D fields */
struct C_idx_update_ttl_d {

    ttl_val : 8;
};

struct C_idx_d {
    struct C_idx_update_ttl_d  update_ttl_d;
    __pad_to_512b : 504;
};



/* ASM Key Structure for p4-table 'A_hash' */
/* P4-table 'A_hash' is hash table */

/* K + I fields */
struct A_hash_k {
    /* FieldType = K */
    ethernet_dstAddr : 48; /* phvbit[512], Flit[1], FlitOffset[0] */
    /* FieldType = K */
    udp_dstPort : 16; /* phvbit[808], Flit[1], FlitOffset[296] */
    __pad_to_512b : 448;
};

/* K + D fields */




/* ASM Key Structure for p4-table 'B_tcam' */
/* P4-table 'B_tcam' ternary table.*/

/* K + I fields */
struct B_tcam_k {
    /* FieldType = K */
    ipv6_dstAddr : 128; /* phvbit[592], Flit[1], FlitOffset[80] */
    /* FieldType = K */
    ipv4_dstAddr : 32; /* phvbit[776], Flit[1], FlitOffset[264] */
    /* FieldType = K */
    ipv4_valid : 1; /* phvbit[508], Flit[0], FlitOffset[508] */
    /* FieldType = K */
    ipv6_valid : 1; /* phvbit[509], Flit[0], FlitOffset[509] */
    /* FieldType = P */
    __pad_162 : 6; /* phvbit[0], Flit[0], FlitOffset[0] */
    __pad_to_512b : 344;
};

/* K + D fields */

struct B_tcam_d {
    action_id : 8; /* TBD: Does this need to go after listing all actions? */
    __pad_to_512b : 504;
};




