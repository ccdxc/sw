# Common file for all proxy test cases

import pdb

# Need to match defines in tcp-constants.h
tcp_debug_dol_pkt_to_serq = 0x1
tcp_debug_dol_test_atomic_stats = 0x2
tcp_debug_dol_dont_queue_to_serq = 0x4
tcp_debug_dol_leave_in_arq = 0x8

tcp_state_ESTABLISHED = 1
tcp_state_SYN_SENT = 2
tcp_state_SYN_RECV = 3
tcp_state_FIN_WAIT1 = 4
tcp_state_FIN_WAIT2 = 5
tcp_state_TIME_WAIT = 6
tcp_state_CLOSE = 7
tcp_state_CLOSE_WAIT = 8
tcp_state_LAST_ACK = 9
tcp_state_LISTEN = 10
tcp_state_CLOSING = 11
tcp_state_NEW_SYN_RECV = 12


def init_tcb_inorder(tc, tcb):
    tcb.rcv_nxt = 0xBABABABA
    tcb.snd_nxt = 0xEFEFEFF0
    tcb.snd_una = 0xEFEFEFEF
    tcb.rcv_tsval = 0xFAFAFAFA
    tcb.ts_recent = 0xFAFAFAF0
    tcb.snd_wnd = 1000
    tcb.snd_cwnd = 1000
    tcb.rcv_mss = 9216
    tcb.debug_dol = 0
    tcb.source_port = tc.config.flow.sport
    tcb.dest_port = tc.config.flow.dport
    if tc.config.src.endpoint.intf.type == 'UPLINK':
        vlan_id = tc.config.src.segment.vlan_id
        tcb.source_lif = 0 # TODO set uplink lif here
    else:
        vlan_id = tc.config.src.endpoint.intf.encap_vlan_id
        tcb.source_lif = tc.config.src.endpoint.intf.lif.hw_lif_id
    if vlan_id != 0:
        vlan_etype_bytes = bytes([0x81, 0x00]) + \
                vlan_id.to_bytes(2, 'big') + \
                bytes([0x08, 0x00])
    else:
        vlan_etype_bytes = bytes([0x08, 0x00])
    tcb.header_template = \
             tc.config.dst.endpoint.macaddr.getnum().to_bytes(6, 'big') + \
             tc.config.src.endpoint.macaddr.getnum().to_bytes(6, 'big') + \
             vlan_etype_bytes + \
             bytes([0x45, 0x08, 0x00, 0x7c, 0x00, 0x01, 0x00, 0x00]) + \
             bytes([0x40, 0x06, 0xfa, 0x71]) + \
             tc.config.flow.sip.getnum().to_bytes(4, 'big') + \
             tc.config.flow.dip.getnum().to_bytes(4, 'big')
    print("header_template = " + str(tcb.header_template))
    # set tcb state to ESTABLISHED(1)
    tcb.state = 1

