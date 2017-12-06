#include "p4pt.h"

struct p4pt_update_tcb_stats_k k;
struct p4pt_update_tcb_stats_d d;
struct phv_ p;

%%
    .param  p4pt_tcb_iscsi_read_latency_distribution_base
    .param  p4pt_update_read_latency_distribution_start

   /*
    * - stage3: write p4tcb_tcb_iscsi_stats, setup lookup p4pt_tcb_iscsi_read_latency_distribution
    *    - k vector: p4pt_global, p4pt_s2s
    *    - d vector: p4pt_tcb_iscsi_stats
    *    - if p4pt_s2s.read:
    *         if p4pt_s2s.req:
    *             p4pt_tcb_iscsi_stats.total_read_reqs++
    *         if p4pt_s2s.resp:
    *            if p4pt_s2s.status != 0:
    *               p4pt_tcb_iscsi_stats.read_errors++
    *            else:
    *                p4pt_tcb_iscsi_stats.total_read_resps++
    *                p4pt_tcb_iscsi_stats.read_bytes += p4pt_s2s.data_length
    *                p4pt_tcb_iscsi_stats.total_read_latency += p4pt_global.latency
    *         write p4tcb_tcb_iscsi_stats
    *      if p4pt_s2s.write:
    *         if p4pt_s2s.req:
    *             p4pt_tcb_iscsi_stats.write_reqs++
    *         if p4pt_s2s.resp:
    *            if p4pt_s2s.status != 0:
    *               p4pt_tcb_iscsi_stats.write_errors++
    *            else:
    *               p4pt_tcb_iscsi_stats.total_write_resps++
    *               p4pt_tcb_iscsi_stats.write_bytes += p4pt_s2s.data_length
    *               p4pt_tcb_iscsi_stats.total_write_latency += p4pt_global.latency
    *         write p4tcb_tcb_iscsi_stats
    *    - lookup p4pt_tcb_iscsi_read_latency_distribution
    *         addr = P4PT_TCB_ISCSI_READ_LATENCY_DISTRIBUTION_BASE_ADDR + \
    *                p4pt_global.p4pt_idx * P4PT_TCB_ISCSI_READ_LATENCY_DISTRIBUTION_SIZE
    *
    */

p4pt_update_tcb_stats:

    P4PT_CHECK_EXIT
   
   /*
    *    - if p4pt_s2s.read:
    *         if p4pt_s2s.req:
    *             p4pt_tcb_iscsi_stats.total_read_reqs++
    */
    seq      c1, k.p4pt_s2s_req, 1
    seq      c2, k.p4pt_s2s_resp, 1
    seq      c3, k.p4pt_s2s_read, 1
    seq      c4, k.p4pt_s2s_write, 1
    seq      c5, k.p4pt_s2s_status, 0

    setcf    c6, [c1 & c3]
    add.c6  r1, d.u.p4pt_update_tcb_stats_d.read_reqs, 1
    tblwr.c6 d.u.p4pt_update_tcb_stats_d.read_reqs, r1

   /*
    *         if p4pt_s2s.resp:
    *            if p4pt_s2s.status != 0:
    *               p4pt_tcb_iscsi_stats.read_errors++
    *            else:
    *                p4pt_tcb_iscsi_stats.total_read_resps++
    *                p4pt_tcb_iscsi_stats.read_bytes += p4pt_s2s.data_length
    *                p4pt_tcb_iscsi_stats.total_read_latency += p4pt_global.latency
    */

    setcf    c6, [c2 & c3 & !c5]
    add.c6   r1, d.u.p4pt_update_tcb_stats_d.read_errors, 1
    tblwr.c6 d.u.p4pt_update_tcb_stats_d.read_errors, r1
    
    setcf    c6, [c2 & c3 & c5]
    add.c6   r1, d.u.p4pt_update_tcb_stats_d.total_read_resps, 1
    tblwr.c6 d.u.p4pt_update_tcb_stats_d.total_read_resps, r1
    add.c6   r1, d.u.p4pt_update_tcb_stats_d.read_bytes, k.p4pt_s2s_data_length
    tblwr.c6 d.u.p4pt_update_tcb_stats_d.read_bytes, r1
    add.c6   r1, d.u.p4pt_update_tcb_stats_d.total_read_latency,\
                 k.{p4pt_global_latency_sbit0_ebit3...p4pt_global_latency_sbit28_ebit31}
    tblwr.c6 d.u.p4pt_update_tcb_stats_d.total_read_latency, r1

   /*
    *    - if p4pt_s2s.write:
    *         if p4pt_s2s.req:
    *             p4pt_tcb_iscsi_stats.total_write_reqs++
    */
    setcf    c6, [c1 & c4]
    add.c6   r1, d.u.p4pt_update_tcb_stats_d.write_reqs, 1
    tblwr.c6 d.u.p4pt_update_tcb_stats_d.write_reqs, r1

   /*
    *         if p4pt_s2s.resp:
    *            if p4pt_s2s.status != 0:
    *               p4pt_tcb_iscsi_stats.write_errors++
    *            else:
    *                p4pt_tcb_iscsi_stats.total_write_resps++
    *                p4pt_tcb_iscsi_stats.write_bytes += p4pt_s2s.data_length
    *                p4pt_tcb_iscsi_stats.total_write_latency += p4pt_global.latency
    */
    setcf    c6, [c2 & c4 & !c5]
    add.c6   r1, d.u.p4pt_update_tcb_stats_d.write_errors, 1
    tblwr.c6 d.u.p4pt_update_tcb_stats_d.write_errors, r1
    
    setcf    c6, [c2 & c4 & c5]
    add.c6   r1, d.u.p4pt_update_tcb_stats_d.total_write_resps, 1
    tblwr.c6 d.u.p4pt_update_tcb_stats_d.total_write_resps, r1
    add.c6   r1, d.u.p4pt_update_tcb_stats_d.write_bytes, k.p4pt_s2s_data_length
    tblwr.c6 d.u.p4pt_update_tcb_stats_d.write_bytes, r1
    add.c6   r1, d.u.p4pt_update_tcb_stats_d.total_write_latency,\
                 k.{p4pt_global_latency_sbit0_ebit3...p4pt_global_latency_sbit28_ebit31}
    tblwr.c6 d.u.p4pt_update_tcb_stats_d.total_write_latency, r1

   /*
    *    - lookup p4pt_tcb_iscsi_read_latency_distribution_base:
    *         addr = p4tcb.base_addr + p4pt_s2s.p4pt_idx * p4tcb.rec_size
    *      p4pt_global.rec_idx = curr_idx
    */
    addi    r1, r0, loword(p4pt_tcb_iscsi_read_latency_distribution_base)
    addui   r1, r1, hiword(p4pt_tcb_iscsi_read_latency_distribution_base)
    add     r1, r1, k.p4pt_global_p4pt_idx, 6
    phvwr   p.common_te0_phv_table_addr, r1
    phvwri  p.common_te0_phv_table_pc, p4pt_update_read_latency_distribution_start[33:6]
    phvwr   p.common_te0_phv_table_raw_table_size, 6
    phvwr.e p.common_te0_phv_table_lock_en, 0
    nop
