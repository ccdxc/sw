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

rawrcb_num_entries_max  = 4096
rawccb_num_entries_max  = 4096
rawrcb_num_entries_mask = rawrcb_num_entries_max - 1
rawccb_num_entries_mask = rawccb_num_entries_max - 1

proxyrcb_num_entries_max  = 4096
proxyccb_num_entries_max  = 4096
proxyrcb_num_entries_mask = proxyrcb_num_entries_max - 1
proxyccb_num_entries_mask = proxyccb_num_entries_max - 1

proxyrcb_num_entries_max_mult = 2
proxyccb_num_entries_max_mult = 2

proxyr_tcp_proxy_dir = 0
proxyr_tls_proxy_dir = 1
proxyc_tcp_proxy_dir = proxyr_tcp_proxy_dir
proxyc_tls_proxy_dir = proxyr_tls_proxy_dir

def proxyr_oper_cb_offset(dir):
    return (((dir) & 1) * proxyrcb_num_entries_max)

def proxyr_oper_cb_id(dir, cb_id):
    return proxyr_oper_cb_offset(dir) + cb_id

def proxyc_oper_cb_offset(dir):
    return (((dir) & 1) * proxyccb_num_entries_max)

def proxyc_oper_cb_id(dir, cb_id):
    return proxyc_oper_cb_offset(dir) + cb_id

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

