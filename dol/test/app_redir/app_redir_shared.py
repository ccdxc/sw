# Testcase definition file.

import pdb
from infra.common.logging       import logger
import types_pb2                as types_pb2

import infra.common.defs as defs

# 
# The following definitions should match app_redir_shared.h
#
app_redir_desc_valid_bit_upd = 0x0001
app_redir_desc_valid_bit_req = 0x0002
app_redir_chain_doorbell_no_sched = 0x0004
app_redir_dol_pipeline_loopbk_en = 0x0008
app_redir_dol_sim_desc_alloc_full = 0x0010
app_redir_dol_sim_page_alloc_full = 0x0020
app_redir_dol_sim_chain_rxq_full = 0x0040
app_redir_dol_sim_chain_txq_full = 0x0080

