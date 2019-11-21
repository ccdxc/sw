# Testcase definition file.

import socket
import pdb
from infra.common.logging       import logger
import types_pb2                as types_pb2

import infra.common.defs as defs

# 
# The following definitions should match proxy.hpp
#
service_lif_cpu       = 33
service_lif_apollo    = 34
service_lif_tcp_proxy = 35
service_lif_tls_proxy = 36
service_lif_ipsec_esp = 37
service_lif_ipsec_ah  = 38
service_lif_ipfix     = 39
service_lif_app_redir = 40
service_lif_gc        = 41
service_lif_p4pt      = 42
service_lif_cpu_bypass = 43

#
# rawrcb/rawccb creation for span;
# HW always spans to qtype 0 and qid 0, lif is derived from the mirror
# session's lport_id. Once in rawrcb, P4+ will hash on flow and spray to
# appropriate ARQ.
#
app_redir_span_rawrcb_id = 0

# 
# The following definitions should match app_redir_shared.h
#
app_redir_desc_valid_bit_upd        = 0x0001
app_redir_desc_valid_bit_req        = 0x0002
app_redir_chain_doorbell_no_sched   = 0x0004
app_redir_dol_pipeline_loopbk_en    = 0x0008
app_redir_dol_sim_desc_alloc_full   = 0x0010
app_redir_dol_sim_page_alloc_full   = 0x0020
app_redir_dol_sim_chain_rxq_full    = 0x0040
app_redir_dol_sim_chain_txq_full    = 0x0080
app_redir_chain_desc_add_aol_offset = 0x0100
app_redir_dol_skip_chain_doorbell   = 0x0200

rawrcb_num_entries_max  = 4096
rawccb_num_entries_max  = 4096
rawrcb_num_entries_mask = rawrcb_num_entries_max - 1
rawccb_num_entries_mask = rawccb_num_entries_max - 1

proxyrcb_num_entries_max  = 4096
proxyccb_num_entries_max  = 4096
proxyrcb_num_entries_mask = proxyrcb_num_entries_max - 1
proxyccb_num_entries_mask = proxyccb_num_entries_max - 1

