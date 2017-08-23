/* TLS P4 definitions */

#define tx_table_s0_t0_action       read_tls_stg0
#define tx_table_s0_t1_action       tls_dummy_action
#define tx_table_s0_t2_action       tls_dummy_action
#define tx_table_s0_t3_action       tls_dummy_action

#define tx_table_s1_t0_action       read_tls_stg1_7
#define tx_table_s1_t1_action       tls_dummy_action
#define tx_table_s1_t2_action       tls_dummy_action
#define tx_table_s1_t3_action       tls_dummy_action

#define tx_table_s2_t0_action       read_tls_stg1_7
#define tx_table_s2_t1_action       tls_dummy_action
#define tx_table_s2_t2_action       tls_dummy_action
#define tx_table_s2_t3_action       tls_dummy_action

#define tx_table_s3_t0_action       read_tls_stg1_7
#define tx_table_s3_t1_action       tls_dummy_action
#define tx_table_s3_t2_action       tls_dummy_action
#define tx_table_s3_t3_action       tls_dummy_action

#define tx_table_s4_t0_action       read_tls_stg1_7
#define tx_table_s4_t1_action       tls_dummy_action
#define tx_table_s4_t2_action       tls_dummy_action
#define tx_table_s4_t3_action       tls_dummy_action

#define tx_table_s5_t0_action       read_tls_stg1_7
#define tx_table_s5_t1_action       tls_dummy_action
#define tx_table_s5_t2_action       tls_dummy_action
#define tx_table_s5_t3_action       tls_dummy_action

#define tx_table_s6_t0_action       read_tls_stg1_7
#define tx_table_s6_t1_action       tls_dummy_action
#define tx_table_s6_t2_action       tls_dummy_action
#define tx_table_s6_t3_action       tls_dummy_action

#define tx_table_s7_t0_action       read_tls_stg1_7
#define tx_table_s7_t1_action       tls_dummy_action
#define tx_table_s7_t2_action       tls_dummy_action
#define tx_table_s7_t3_action       tls_dummy_action

#include "../../common-p4+/common_txdma.p4"


#define CAPRI_QSTATE_HEADER_COMMON \
        pc                  : 8;\
        rsvd                : 8;\
        cosA                : 4;\
        cosB                : 4;\
        cos_sel             : 8;\
        eval_last           : 8;\
        host                : 4;\
        total               : 4;\
        pid                 : 16;\



#define CAPRI_QSTATE_HEADER_RING(_x)		\
        pi_##_x                           : 16;\
        ci_##_x                           : 16;\

/* Per stage D-vector Definitions */

header_type tls_stg0_d_t {
    fields {
        // 8 Bytes intrinsic header
        CAPRI_QSTATE_HEADER_COMMON
        // 4 Bytes SERQ ring
        CAPRI_QSTATE_HEADER_RING(0)
        // 4 Bytes BSQ ring
        CAPRI_QSTATE_HEADER_RING(1)

        serq_base                       : ADDRESS_WIDTH;
        fid                             : 16;
        dec_flow                        : 1;

        // TBD: Total used   : 209 bits, pending: 303
        pad                             : 303;
    }
}

header_type tls_stg1_7_d_t {
    fields {
        cipher_type                     : 8;
        ver_major                       : 4;
        key_addr                        : ADDRESS_WIDTH;
        iv_addr                         : ADDRESS_WIDTH;
        qhead                           : ADDRESS_WIDTH;
        qtail                           : ADDRESS_WIDTH;
        una_desc                        : ADDRESS_WIDTH;
        una_desc_idx                    : 2;
        una_data_offset                 : 16;
        una_data_len                    : 16;
        nxt_desc                        : ADDRESS_WIDTH;
        nxt_desc_idx                    : 2;
        nxt_data_offset                 : 16;
        nxt_data_len                    : 16;
        next_tls_hdr_offset             : 16;
        cur_tls_data_len                : 16;
        ofid                            : 16;

        // Total used   : 512 bits, pending: 0
    }
}


@pragma scratch_metadata
metadata tls_stg0_d_t tls_stg0_d;

@pragma scratch_metadata
metadata tls_stg1_7_d_t tls_stg1_7_d;



action read_tls_stg0(pc, rsvd, cosA, cosB, cos_sel, eval_last, host, total, pid, pi_0, ci_0, pi_1, ci_1, serq_base, fid, dec_flow, pad) {
    modify_field(tls_stg0_d.pc, pc);
    modify_field(tls_stg0_d.rsvd, rsvd);
    modify_field(tls_stg0_d.cosA, cosA);
    modify_field(tls_stg0_d.cosB, cosB);
    modify_field(tls_stg0_d.cos_sel, cos_sel);
    modify_field(tls_stg0_d.eval_last, eval_last);
    modify_field(tls_stg0_d.host, host);
    modify_field(tls_stg0_d.total, total);
    modify_field(tls_stg0_d.pid, pid);

    modify_field(tls_stg0_d.pi_0, pi_0);
    modify_field(tls_stg0_d.ci_0, ci_0);

    modify_field(tls_stg0_d.pi_1, pi_1);
    modify_field(tls_stg0_d.ci_1, ci_1);

    modify_field(tls_stg0_d.serq_base, serq_base);
    modify_field(tls_stg0_d.fid, fid);
    modify_field(tls_stg0_d.dec_flow, dec_flow);
    //modify_field(tls_stg0_d.pad, pad);
}


action read_tls_stg1_7(cipher_type, ver_major, key_addr, iv_addr, qhead, qtail, una_desc, una_desc_idx, una_data_offset, una_data_len, nxt_desc, nxt_desc_idx, nxt_data_offset, nxt_data_len, next_tls_hdr_offset, cur_tls_data_len, ofid) {

    modify_field(tls_stg1_7_d.cipher_type, cipher_type);
    modify_field(tls_stg1_7_d.ver_major, ver_major);
    modify_field(tls_stg1_7_d.key_addr, key_addr);
    modify_field(tls_stg1_7_d.iv_addr, iv_addr);
    modify_field(tls_stg1_7_d.qhead, qhead);
    modify_field(tls_stg1_7_d.qtail, qtail);
    modify_field(tls_stg1_7_d.una_desc, una_desc);
    modify_field(tls_stg1_7_d.una_desc_idx, una_desc_idx);
    modify_field(tls_stg1_7_d.una_data_offset, una_data_offset);
    modify_field(tls_stg1_7_d.una_data_len, una_data_len);
    modify_field(tls_stg1_7_d.nxt_desc, nxt_desc);
    modify_field(tls_stg1_7_d.nxt_desc_idx, nxt_desc_idx);
    modify_field(tls_stg1_7_d.nxt_data_offset, nxt_data_offset);
    modify_field(tls_stg1_7_d.nxt_data_len, nxt_data_len);
    modify_field(tls_stg1_7_d.next_tls_hdr_offset, next_tls_hdr_offset);
    modify_field(tls_stg1_7_d.cur_tls_data_len, cur_tls_data_len);
    modify_field(tls_stg1_7_d.ofid, ofid);
}







action tls_dummy_action(data0, data1, data2, data3, data4, data5, data6, data7) {
    modify_field(scratch_metadata0.data0, data0);
    modify_field(scratch_metadata0.data1, data1);
    modify_field(scratch_metadata0.data2, data2);
    modify_field(scratch_metadata0.data3, data3);
    modify_field(scratch_metadata0.data4, data4);
    modify_field(scratch_metadata0.data5, data5);
    modify_field(scratch_metadata0.data6, data6);
    modify_field(scratch_metadata0.data7, data7);
}
