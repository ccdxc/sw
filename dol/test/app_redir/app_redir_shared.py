# Testcase definition file.

import socket
import pdb
from infra.common.logging       import logger
import types_pb2                as types_pb2

import infra.common.defs as defs

# 
# The following definitions should match proxy.hpp
#
service_lif_tcp_proxy = 1001
service_lif_tls_proxy = 1002
service_lif_cpu       = 1003
service_lif_ipsec_esp = 1004
service_lif_ipfix     = 1005
service_lif_app_redir = 1006
service_lif_gc        = 1007
service_lif_p4pt      = 1008


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

#
# Build flow key for proxyrcb from tc configs
#
def proxyrcb_flow_key_build(tc, proxyrcb):
    proxyrcb.vrf = tc.config.flow._FlowObject__session.initiator.ep.tenant.id
    proxyrcb.sport = tc.config.flow.sport
    proxyrcb.dport = tc.config.flow.dport
    proxyrcb.ip_proto = socket.IPPROTO_TCP

    # Assume IPv4 for now until we figure out how to get the
    # flow filter configs from the tc
    proxyrcb.af = socket.AF_INET
    #proxyrcb.ip_sa.v4_addr = tc.config.flow.sip
    #proxyrcb.ip_da.v4_addr = tc.config.flow.dip

#
# Print rawrcb statistics
#
def rawrcb_stats_print(tc, rawrcb):
    print("RAWRCB %d stat_pkts_redir %d" % 
          (rawrcb.id, rawrcb.stat_pkts_redir))
    print("RAWRCB %d stat_pkts_discard %d" % 
          (rawrcb.id, rawrcb.stat_pkts_discard))
    print("RAWRCB %d stat_cb_not_ready %d" % 
          (rawrcb.id, rawrcb.stat_cb_not_ready))
    print("RAWRCB %d stat_qstate_cfg_err %d" % 
          (rawrcb.id, rawrcb.stat_qstate_cfg_err))
    print("RAWRCB %d stat_pkt_len_err %d" % 
          (rawrcb.id, rawrcb.stat_pkt_len_err))
    print("RAWRCB %d stat_rxq_full %d" % 
          (rawrcb.id, rawrcb.stat_rxq_full))
    print("RAWRCB %d stat_txq_full %d" % 
          (rawrcb.id, rawrcb.stat_txq_full))
    print("RAWRCB %d stat_desc_sem_alloc_full %d" % 
          (rawrcb.id, rawrcb.stat_desc_sem_alloc_full))
    print("RAWRCB %d stat_mpage_sem_alloc_full %d" % 
          (rawrcb.id, rawrcb.stat_mpage_sem_alloc_full))
    print("RAWRCB %d stat_ppage_sem_alloc_full %d" % 
          (rawrcb.id, rawrcb.stat_ppage_sem_alloc_full))
    print("RAWRCB %d stat_sem_free_full %d" % 
          (rawrcb.id, rawrcb.stat_sem_free_full))

#
# Print rawccb statistics
#
def rawccb_stats_print(tc, rawccb):
    print("RAWCCB %d stat_pkts_chain %d" % 
          (rawccb.id, rawccb.stat_pkts_chain))
    print("RAWCCB %d stat_pkts_discard %d" % 
          (rawccb.id, rawccb.stat_pkts_discard))
    print("RAWCCB %d stat_cb_not_ready %d" % 
          (rawccb.id, rawccb.stat_cb_not_ready))
    print("RAWCCB %d stat_my_txq_empty %d" % 
          (rawccb.id, rawccb.stat_my_txq_empty))
    print("RAWCCB %d stat_aol_err %d" % 
          (rawccb.id, rawccb.stat_aol_err))
    print("RAWCCB %d stat_txq_full %d" % 
          (rawccb.id, rawccb.stat_txq_full))
    print("RAWCCB %d stat_desc_sem_free_full %d" % 
          (rawccb.id, rawccb.stat_desc_sem_free_full))
    print("RAWCCB %d stat_page_sem_free_full %d" % 
          (rawccb.id, rawccb.stat_page_sem_free_full))

#
# Print proxyrcb statistics
#
def proxyrcb_stats_print(tc, proxyrcb):
    print("PROXYRCB %d stat_pkts_redir %d" % 
          (proxyrcb.id, proxyrcb.stat_pkts_redir))
    print("PROXYRCB %d stat_pkts_discard %d" % 
          (proxyrcb.id, proxyrcb.stat_pkts_discard))
    print("PROXYRCB %d stat_cb_not_ready %d" % 
          (proxyrcb.id, proxyrcb.stat_cb_not_ready))
    print("PROXYRCB %d stat_null_ring_indices_addr %d" % 
          (proxyrcb.id, proxyrcb.stat_null_ring_indices_addr))
    print("PROXYRCB %d stat_aol_err %d" % 
          (proxyrcb.id, proxyrcb.stat_aol_err))
    print("PROXYRCB %d stat_rxq_full %d" % 
          (proxyrcb.id, proxyrcb.stat_rxq_full))
    print("PROXYRCB %d stat_txq_empty %d" % 
          (proxyrcb.id, proxyrcb.stat_txq_empty))
    print("PROXYRCB %d stat_sem_alloc_full %d" % 
          (proxyrcb.id, proxyrcb.stat_sem_alloc_full))
    print("PROXYRCB %d stat_sem_free_full %d" % 
          (proxyrcb.id, proxyrcb.stat_sem_free_full))


#
# Print proxyccb statistics
#
def proxyccb_stats_print(tc, proxyccb):
    print("PROXYCCB %d stat_pkts_chain %d" % 
          (proxyccb.id, proxyccb.stat_pkts_chain))
    print("PROXYCCB %d stat_pkts_discard %d" % 
          (proxyccb.id, proxyccb.stat_pkts_discard))
    print("PROXYCCB %d stat_cb_not_ready %d" % 
          (proxyccb.id, proxyccb.stat_cb_not_ready))
    print("PROXYCCB %d stat_my_txq_empty %d" % 
          (proxyccb.id, proxyccb.stat_my_txq_empty))
    print("PROXYCCB %d stat_aol_err %d" % 
          (proxyccb.id, proxyccb.stat_aol_err))
    print("PROXYCCB %d stat_txq_full %d" % 
          (proxyccb.id, proxyccb.stat_txq_full))
    print("PROXYCCB %d stat_desc_sem_free_full %d" % 
          (proxyccb.id, proxyccb.stat_desc_sem_free_full))
    print("PROXYCCB %d stat_page_sem_free_full %d" % 
          (proxyccb.id, proxyccb.stat_page_sem_free_full))

