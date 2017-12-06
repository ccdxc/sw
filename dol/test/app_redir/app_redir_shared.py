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

app_redir_rawrcb_num_entries_max = 4096
app_redir_rawccb_num_entries_max = 4096
app_redir_rawrcb_num_entries_mask = app_redir_rawrcb_num_entries_max - 1
app_redir_rawccb_num_entries_mask = app_redir_rawccb_num_entries_max - 1

app_redir_proxyrcb_num_entries_max = 4096
app_redir_proxyccb_num_entries_max = 4096
app_redir_proxyrcb_num_entries_mask = app_redir_proxyrcb_num_entries_max - 1
app_redir_proxyccb_num_entries_mask = app_redir_proxyccb_num_entries_max - 1


# 
# The following definitions should match proxy.hpp
#
service_lif_tcp_proxy = 1001
service_lif_tls_proxy = 1002
service_lif_cpu = 1003
service_lif_ipsec_esp = 1004
service_lif_ipfix = 1005
service_lif_app_redir = 1006
service_lif_gc = 1007
service_lif_p4pt = 1008
