#include "./p4pt.h"


struct p4pt_start_k  k;
struct p4pt_start_d  d;
struct phv_ p;

%%
    .param      p4pt_tcb_base
    .param      p4pt_update_tcb_start

   /*
    *  - stage0: parse protocol header, setup lookup p4tcb
    *    - k vector: p4_to_p4plus_p4pt_header (aka p4pt_iscsi_app_header, etc.)
    *    - d vector: qid
    *         Q: would d vector contain other things?
    *    - parse p4pt_header.iscsi_payload into p4pt_s2s; p4pt_s2s is available through all stages
    *
    *      // check if qid/protocol is recogized, 0 = iscsi, 1 = nfs, etc.
    *      if qid == 0:
    *         p4pt_parse_iscsi
    *      else:
    *         exit
    *
    *    - p4pt_parse_iscsi:
    *      iscsi_header = p4_to_p4plus_p4pt_header.payload
    *
    *      // read iscsi opcode (cmd vs. resp), if neither then exit
    *      if iscsi_header.opcode == 0x1:
    *         p4pt_s2s.req = 1
    *      else if iscsi_header.opcode == 0x21:
    *         p4pt_s2s.resp = 1
    *      else:
    *         exit
    *
    *      // update p4pt global indices that we use later in the pipelnie
    *      p4pt_global.qid = qid
    *      p4pt_global.p4pt_idx = p4pt_header.p4pt_idx
    *
    *      // if response, we only look for status to be updated in phv
    *      if p4pt_s2s.resp:
    *         p4pt_s2s.status = iscsi_header.status
    *         p4pt_s2s.tag_id = iscsi_header.tag_id
    *         goto prep_p4ptcb_lookup
    *
    *      // must be req
    *      if scsi_header.read:
    *         if iscsi_header.cmd_scsi_cdb_op & 0x1F == 0x8:
    *            p4pt_s2s.read = 1
    *         else:
    *            exit               // scsi and iscsi do not match up
    *      if scsi_header.write:
    *         if iscsi_header.cmd_scsi_cdb_op & 0x1F == 0xA:
    *            p4pt_s2s.write = 1
    *         else:
    *            exit               // scsi and iscsi do not match up
    *      else:
    *         exit                  // exit if this is neither read, nor write operation
    *
    *      p4pt_s2s.data_length = iscsi_header.data_seg_length
    *      p4pt_s2s.lun = iscsi_header.lun
    *      p4pt_s2s.tag_id = iscsi_header.tag_id
    *      goto prep_p4ptcb_lookup
    *
    *    - prep p4tcb lookup
    *         // enable table0 lookup for p4ptcb
    *         // each record is 512 bits (64 bytes)
    *         addr = P4PT_TCB_BASE_ADDR + p4pt_header.p4pt_idx * P4PT_TCB_SIZE
    *
    */

p4pt_s0_rx_start:
   /*
    *      // check if qid/protocol is recogized, 0 = iscsi, 1 = nfs, etc.
    *      if qid == 0:
    *         p4pt_parse_iscsi
    *      else:
    *         exit
    *
    */
    seq  c1, CAPRI_RXDMA_INTRINSIC_QID, 0
    b.c1 p4pt_parse_iscsi
    nop
    b    p4pt_exit
    nop


p4pt_parse_iscsi:

   /*
    *      // read iscsi opcode (cmd vs. resp), if neither then exit
    *      if iscsi_header.opcode == 0x1:
    *         p4pt_s2s.req = 1
    *      else if iscsi_header.opcode == 0x21:
    *         p4pt_s2s.resp = 1
    *      else:
    *         exit
    *
    */
    seq      c1, k.p4pt_iscsi_app_header_opcode, 0x1
    phvwr.c1 p.p4pt_s2s_req, TRUE  

    seq      c2, k.p4pt_iscsi_app_header_opcode, 0x21
    phvwr.c2 p.p4pt_s2s_resp, TRUE  

    bcf      [!c1 & !c2], p4pt_exit
    nop

   /*
    *      // update p4pt global indices that we use later in the pipelnie
    *      p4pt_global.qid = qid
    *      p4pt_global.p4pt_idx = p4pt_header.p4pt_idx
    */
    phvwr p.p4pt_global_qid, CAPRI_RXDMA_INTRINSIC_QID 
    phvwr p.p4pt_global_p4pt_idx, k.p4pt_iscsi_app_header_p4pt_idx

   /*
    *      // if response, we only look for status to be updated in phv
    *      if p4pt_s2s.resp:
    *         p4pt_s2s.status = iscsi_header.status
    *         prep_p4ptcb_lookup
    *
    */
    phvwr.c2 p.p4pt_s2s_status, k.p4pt_iscsi_app_header_resp_status
    b.c2      prep_p4ptcb_lookup

   /*
    *      // must be req
    *      if scsi_header.read:
    *         if iscsi_header.cmd_scsi_cdb_op & 0x1F == 0x8:
    *            p4pt_s2s.read = 1
    *         else:
    *            exit               // scsi and iscsi do not match up
    *      if scsi_header.write:
    *         if iscsi_header.cmd_scsi_cdb_op & 0x1F == 0xA:
    *            p4pt_s2s.write = 1
    *         else:
    *            exit               // scsi and iscsi do not match up
    *      else:
    *         exit                  // exit if this is neither read, nor write operation
    *
    */
    seq      c1, k.p4pt_iscsi_app_header_cmd_read, TRUE
    and      r1, k.p4pt_iscsi_app_header_cmd_scsi_cdb_op, 0xF
    seq      c3, r1, 0x8
    setcf    c4, [c1 & c3]
    phvwr.c4 p.p4pt_s2s_read, TRUE
    bcf      [c1 & !c3], p4pt_exit
    nop

    seq      c2, k.p4pt_iscsi_app_header_cmd_write, TRUE
    seq      c3, r1, 0xA
    setcf    c4, [c2 & c3]
    phvwr.c4 p.p4pt_s2s_write, TRUE
    bcf      [c2 & !c3], p4pt_exit
    nop

    bcf [!c1 & !c2], p4pt_exit
    nop

   /*
    *      p4pt_s2s.data_length = iscsi_header.data_seg_length
    *      p4pt_s2s.lun = iscsi_header.lun
    *      prep_p4ptcb_lookup
    *
    */
    phvwr    p.p4pt_s2s_data_length, k.p4pt_iscsi_app_header_cmd_data_length
    phvwr    p.p4pt_s2s_lun, k.{p4pt_iscsi_app_header_cmd_lun_sbit0_ebit7, p4pt_iscsi_app_header_cmd_lun_sbit8_ebit63}

prep_p4ptcb_lookup:
   /*
    *      p4pt_s2s.tag_id = iscsi_header.tag_id
    */
    phvwr    p.p4pt_s2s_tag_id[31:16], k.p4pt_iscsi_app_header_tag_id_sbit0_ebit15
    phvwr    p.p4pt_s2s_tag_id[15:0], k.p4pt_iscsi_app_header_tag_id_sbit16_ebit31

   /*
    *         // enable table0 lookup for p4ptcb
    *         // each record is 512 bits (64 bytes)
    *         addr = P4PT_TCB_BASE_ADDR + p4pt_header.p4pt_idx * P4PT_TCB_SIZE
    */
    addi     r1, r0, loword(p4pt_tcb_base)
    addui    r1, r1, hiword(p4pt_tcb_base)
    add      r1, r1, k.p4pt_iscsi_app_header_p4pt_idx, 6  // 64 bytes record
    phvwr    p.common_te0_phv_table_addr, r1
    phvwri   p.common_te0_phv_table_pc, p4pt_update_tcb_start[33:6]
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
