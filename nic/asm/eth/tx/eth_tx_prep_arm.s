
#include "INGRESS_p.h"
#include "ingress.h"

#include "defines.h"

struct phv_ p;

%%

.param  eth_tx_commit_arm

.align
eth_tx_prep_arm:
  phvwri.e        p.common_te0_phv_table_raw_table_size, LG2_TX_QSTATE_SIZE
  phvwri.f        p.common_te0_phv_table_pc, eth_tx_commit_arm[38:6]
