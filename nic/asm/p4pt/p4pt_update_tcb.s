#include "p4pt.h"

struct p4pt_update_tcb_k k;
struct p4pt_update_tcb_d d;
struct phv_ p;

%%
    .param  p4pt_tcb_iscsi_rec_base
    .param  p4pt_update_tcb_rec_start

   /*
    * - stage1: update p4tcb, setup lookup p4tcb_rec
    *    - k vector: p4pt_global, p4pt_s2s
    *    - d vector: p4tcb
    *    - if k.p4pt_s2s.req:
    *         curr_idx = p4tcb.next_idx
    *         if curr_idx == 9:
    *            curr_idx = 0
    *         p4tcb.next_idx = curr_idx
    *         p4tcb.key[curr_idx] = p4pt_s2s.tag_id
    *         write p4ptcb
    *    - if k.p4pt_s2s.resp:
    *         curr_idx = match k.p4pt_s2s.tag_id in [ d.p4ptcb.rec_key0 - d.p4ptcb.rec_key9 ]
    *         if !found:
    *            exit            // Q: if we exit, how do we collect other stats
    *    - p4pt_global.rec_idx = curr_idx
    *    - lookup p4pt_tcb_rec:
    *         addr = p4tcb.base_addr + curr_idx * p4tcb.rec_size
    *
    */

p4pt_update_tcb_start:
   /*
    *    - if k.p4pt_s2s.req:
    *         curr_idx = p4tcb.next_idx
    *         if curr_idx == 9:
    *            curr_idx = 0
    *         p4tcb.next_idx = curr_idx
    *         p4tcb.key[curr_idx] = p4pt_s2s.tag_id
    *         write p4ptcb
    */

    P4PT_CHECK_EXIT

    seq      c1, k.p4pt_s2s_resp, TRUE
    b.c1     p4pt_update_tcb_process_resp

    add      r7, r0, d.u.p4pt_update_tcb_d.next_idx

    seq      c1, r7, 9
    tblwr.c1 d.u.p4pt_update_tcb_d.rec_key9, k.p4pt_s2s_tag_id
    b.c1     p4pt_update_idx

    seq      c1, r7, 8
    tblwr.c1 d.u.p4pt_update_tcb_d.rec_key8, k.p4pt_s2s_tag_id
    b.c1     p4pt_update_idx

    seq      c1, r7, 7
    tblwr.c1 d.u.p4pt_update_tcb_d.rec_key7, k.p4pt_s2s_tag_id
    b.c1     p4pt_update_idx

    seq      c1, r7, 6
    tblwr.c1 d.u.p4pt_update_tcb_d.rec_key6, k.p4pt_s2s_tag_id
    b.c1     p4pt_update_idx

    seq      c1, r7, 5
    tblwr.c1 d.u.p4pt_update_tcb_d.rec_key5, k.p4pt_s2s_tag_id
    b.c1     p4pt_update_idx

    seq      c1, r7, 4
    tblwr.c1 d.u.p4pt_update_tcb_d.rec_key4, k.p4pt_s2s_tag_id
    b.c1     p4pt_update_idx

    seq      c1, r7, 3
    tblwr.c1 d.u.p4pt_update_tcb_d.rec_key3, k.p4pt_s2s_tag_id
    b.c1     p4pt_update_idx

    seq      c1, r7, 2
    tblwr.c1 d.u.p4pt_update_tcb_d.rec_key2, k.p4pt_s2s_tag_id
    b.c1     p4pt_update_idx

    seq      c1, r7, 1
    tblwr.c1 d.u.p4pt_update_tcb_d.rec_key1, k.p4pt_s2s_tag_id
    b.c1     p4pt_update_idx
    nop

    tblwr    d.u.p4pt_update_tcb_d.rec_key0, k.p4pt_s2s_tag_id

p4pt_update_idx:
   /*
    *    - p4pt_global.rec_idx = curr_idx
    */
    phvwr    p.p4pt_global_rec_idx, r7

    add      r1, r7, 1
    seq      c2, r1, 10
    addi.c2  r1, r0, 0
    nop
    tblwr    d.u.p4pt_update_tcb_d.next_idx, r1

    b        p4pt_tcb_rec_lookup
    nop

p4pt_update_tcb_process_resp:
   /*
    *    - if k.p4pt_s2s.resp:
    *         curr_idx = match k.p4pt_s2s.tag_id in [ d.p4ptcb.rec_key0 - d.p4ptcb.rec_key9 ]
    *         if !found:
    *            exit            // Q: if we exit, how do we collect other stats
    */
    addi     r7, r0, 10

    seq      c1, k.p4pt_s2s_tag_id, d.u.p4pt_update_tcb_d.rec_key0
    add.c1   r7, r0, 0
    b.c1     p4pt_resp_update_idx

    seq      c1, k.p4pt_s2s_tag_id, d.u.p4pt_update_tcb_d.rec_key1
    add.c1   r7, r0, 1
    b.c1     p4pt_resp_update_idx

    seq      c1, k.p4pt_s2s_tag_id, d.u.p4pt_update_tcb_d.rec_key2
    add.c1   r7, r0, 2
    b.c1     p4pt_resp_update_idx

    seq      c1, k.p4pt_s2s_tag_id, d.u.p4pt_update_tcb_d.rec_key3
    add.c1   r7, r0, 3
    b.c1     p4pt_resp_update_idx

    seq      c1, k.p4pt_s2s_tag_id, d.u.p4pt_update_tcb_d.rec_key4
    add.c1   r7, r0, 4
    b.c1     p4pt_resp_update_idx

    seq      c1, k.p4pt_s2s_tag_id, d.u.p4pt_update_tcb_d.rec_key5
    add.c1   r7, r0, 5
    b.c1     p4pt_resp_update_idx

    seq      c1, k.p4pt_s2s_tag_id, d.u.p4pt_update_tcb_d.rec_key6
    add.c1   r7, r0, 6
    b.c1     p4pt_resp_update_idx

    seq      c1, k.p4pt_s2s_tag_id, d.u.p4pt_update_tcb_d.rec_key7
    add.c1   r7, r0, 7
    b.c1     p4pt_resp_update_idx

    seq      c1, k.p4pt_s2s_tag_id, d.u.p4pt_update_tcb_d.rec_key8
    add.c1   r7, r0, 8
    b.c1     p4pt_resp_update_idx

    seq      c1, k.p4pt_s2s_tag_id, d.u.p4pt_update_tcb_d.rec_key9
    add.c1   r7, r0, 9
    b.c1     p4pt_resp_update_idx

    seq      c1, r7, 10
    b.c1     p4pt_exit
    nop

p4pt_resp_update_idx:
    phvwr    p.p4pt_global_rec_idx, r7

p4pt_tcb_rec_lookup:
   /*
    *    - lookup p4pt_tcb_rec:
    *         addr = p4tcb.base_addr + curr_idx * p4tcb.rec_size
    */
    addi     r1, r0, loword(p4pt_tcb_iscsi_rec_base)
    addui    r1, r1, hiword(p4pt_tcb_iscsi_rec_base)
    add      r2, r0, r7
    mul      r2, r2, 64
    nop
    add      r2, k.p4pt_global_p4pt_idx, r2
    add      r1, r1, r2, 6
    phvwr    p.common_te0_phv_table_addr, r1
    phvwri   p.common_te0_phv_table_pc, p4pt_update_tcb_rec_start[33:6]
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
