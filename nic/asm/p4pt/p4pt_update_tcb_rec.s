#include "p4pt.h"

struct p4pt_update_tcb_rec_k k;
struct p4pt_update_tcb_rec_d d;
struct phv_ p;

%%
    .param p4pt_tcb_iscsi_stats_base
    .param p4pt_update_tcb_stats

   /*
    * - stage2: update p4tcb_rec, setup lookup p4tcb_tcb_iscsi_stats
    *    - k vector: p4pt_global, p4pt_s2s
    *    - d vector: p4pt_iscsi_rec
    *    - if p4pt_s2s.resp:
    *         p4pt_iscsi_rec.status = p4pt_s2s.status
    *         p4pt_global.latency = p4pt_iscsi_rec.req_timestamp - time.now()
    *      if p4pt_s2s.req:
    *         p4pt_iscsi_rec.tag_id == p4pt_s2s.tag_id
    *         p4pt_iscsi_rec.lun == p4pt_s2s.lun
    *         p4pt_iscsi_rec.req_timestamp = curr_time()
    *         p4pt_iscsi_rec.data_length = p4pt_s2s.data_length
    *    - lookup p4pt_tcb_iscsi_stats::
    *      addr = P4PT_TCB_ISCSI_STATS_BASE_ADDR + p4pt_global.p4pt_idx * P4PT_TCB_ISCSI_STATS_SIZE
    */

p4pt_update_tcb_rec_start:
    P4PT_CHECK_EXIT

   /*
    *      if p4pt_s2s.req:
    *         p4pt_iscsi_rec.tag_id == p4pt_s2s.tag_id
    *         p4pt_iscsi_rec.lun == p4pt_s2s.lun
    *         p4pt_iscsi_rec.req_timestamp = curr_time()
    *         p4pt_iscsi_rec.data_length = p4pt_s2s.data_length
    */
    seq      c1, k.p4pt_s2s_req, 1
    b.!c1    process_response
    nop

    tblwr    d.u.p4pt_update_tcb_rec_d.tag_id, k.p4pt_s2s_tag_id
    tblwr    d.u.p4pt_update_tcb_rec_d.lun, k.p4pt_s2s_lun
    tblwr    d.u.p4pt_update_tcb_rec_d.data_length, k.p4pt_s2s_data_length
    tblwr    d.u.p4pt_update_tcb_rec_d.read, k.p4pt_s2s_read
    tblwr    d.u.p4pt_update_tcb_rec_d.write, k.p4pt_s2s_write
#ifdef CAPRI_IGNORE_TIMESTAMP
    mfspr    r4, spr_time
    tblwr    d.u.p4pt_update_tcb_rec_d.req_timestamp, r4
#else
    add      r1, r0, 0
    tblwr    d.u.p4pt_update_tcb_rec_d.req_timestamp, r0
#endif
    b        prep_next_lookup
    nop

process_response:
   /*
    *    - if p4pt_s2s.resp:
    *         p4pt_iscsi_rec.status = p4pt_s2s.status
    *         p4pt_global.latency = p4pt_iscsi_rec.req_timestamp - time.now()
    */
    seq      c1, k.p4pt_s2s_resp, 1
    b.!c1    p4pt_exit

    tblwr    d.u.p4pt_update_tcb_rec_d.status, k.p4pt_s2s_status
    add      r1, r0, d.u.p4pt_update_tcb_rec_d.req_timestamp
#ifdef CAPRI_IGNORE_TIMESTAMP
    mfspr    r4, spr_time
    sub      r1, r1, r4
#else
    add      r1, r0, 0
#endif
    phvwr    p.p4pt_global_latency, r1
    phvwr    p.p4pt_s2s_write, d.u.p4pt_update_tcb_rec_d.write
    phvwr    p.p4pt_s2s_read, d.u.p4pt_update_tcb_rec_d.read

prep_next_lookup:
   /*
    *    - lookup p4pt_tcb_iscsi_stats::
    *      addr = P4PT_TCB_ISCSI_STATS_BASE_ADDR + p4pt_global.p4pt_idx * P4PT_TCB_ISCSI_STATS_SIZE
    */
    addi     r1, r0, loword(p4pt_tcb_iscsi_stats_base)
    addui    r1, r1, hiword(p4pt_tcb_iscsi_stats_base)
    add      r1, r1, k.p4pt_global_p4pt_idx, 6
    phvwr    p.common_te0_phv_table_addr, r1
    phvwri   p.common_te0_phv_table_pc, p4pt_update_tcb_stats[33:6]
    phvwr    p.common_te0_phv_table_raw_table_size, 6
    phvwr.e  p.common_te0_phv_table_lock_en, 0
    phvwr    p.app_header_table0_valid, 1

p4pt_exit:
    /*
     * Done with parsing, no further pipeline action needed
     * This can happen because
     *   1) port/protocol is not supported
     *   2) all processing is done and remaining steps are not to be executed
     */
    P4PT_EXIT
