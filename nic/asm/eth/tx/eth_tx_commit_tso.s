
#include "INGRESS_p.h"
#include "ingress.h"
#include "INGRESS_tx_table_s2_t0_k.h"

#include "defines.h"

struct phv_ p;
struct tx_table_s2_t0_k_ k;
struct tx_table_s2_t1_eth_tx_commit_tso_d d;

%%

.align
eth_tx_commit_tso:
  //XXX if the tx2 qstate can't be loaded in d... ignore the error :-O
  //XXX eth_tx_commit owns the table valid bits, so what can we do here?
  // bcf          [c2 | c3 | c7], eth_tx_commit_tso_error

  bbeq            k.eth_tx_global_tso_sot, 1, eth_tx_commit_tso_sot
  nop

eth_tx_commit_tso_cont:
eth_tx_commit_tso_eot:
  // update tso state in qstate
  tbladd          d.tso_ipid_delta, 1
  tbladd.e.f      d.tso_seq_delta, k.eth_tx_to_s2_tso_hdr_addr // mss
  phvwr.f         p.{eth_tx_t1_s2s_tso_hdr_addr...eth_tx_t1_s2s_tso_seq_delta}, d.{tso_hdr_addr...tso_seq_delta}

eth_tx_commit_tso_sot:
  // init tso state in qstate
  tblwr           d.{tso_hdr_addr...tso_hdr_len}, k.{eth_tx_to_s2_tso_hdr_addr...eth_tx_to_s2_tso_hdr_len}
  tblwr.e.f       d.{tso_ipid_delta...tso_seq_delta}, 0
  phvwr.f         p.{eth_tx_t1_s2s_tso_hdr_addr...eth_tx_t1_s2s_tso_seq_delta}, d.{tso_hdr_addr...tso_seq_delta}
