
#include "INGRESS_p.h"
#include "ingress.h"
#include "INGRESS_tx_table_s3_t0_k.h"

#include "../../asm/adminq/defines.h"

struct phv_ p;
struct tx_table_s3_t0_k_ k;
struct tx_table_s3_t0_adminq_commit_d d;

%%

#define     _r_num_desc         r1
#define     _r_cq_desc_addr     r5

.align
adminq_commit:

  bcf             [c2 | c3 | c7], adminq_commit_error
  nop

  seq             c1, d.{c_index0}.hx, k.{adminq_to_s3_adminq_ci}.hx
  bcf             [!c1], adminq_commit_abort
  nop

  // Save consumer index
  phvwr           p.nicmgr_req_desc_comp_index, d.c_index0

  // Claim the descriptor
  tblmincri       d.{c_index0}.hx, d.{ring_size}.hx, 1

  // Eval doorbell when pi == ci
  seq             c3, d.{p_index0}.hx, d.{c_index0}.hx
  bcf             [!c3], adminq_commit_done
  nop.!c3.e

adminq_commit_eval_db:
  CAPRI_RING_DOORBELL_ADDR(0, DB_IDX_UPD_NOP, DB_SCHED_UPD_EVAL, k.adminq_t0_s2s_qtype, k.adminq_t0_s2s_lif)   // R4 = ADDR
  CAPRI_RING_DOORBELL_DATA(0, k.adminq_t0_s2s_qid, 0, 0)   // R3 = DATA
  memwr.dx.e      r4, r3

adminq_commit_done:
  phvwri.f        p.{app_header_table0_valid...app_header_table3_valid}, 0

adminq_commit_abort:
  tblwr.f         d.ci_fetch, d.c_index0

adminq_commit_error:
  phvwri.e        p.{app_header_table0_valid...app_header_table3_valid}, 0
  phvwri.f        p.p4_intr_global_drop, 1
