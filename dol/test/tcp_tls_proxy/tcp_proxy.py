# Common file for all proxy test cases

import pdb
from infra.common.logging import logger

# Need to match defines in tcp-constants.h
tcp_debug_dol_pkt_to_serq = 0x1
tcp_debug_dol_test_atomic_stats = 0x2
tcp_debug_dol_dont_queue_to_serq = 0x4
tcp_debug_dol_leave_in_arq = 0x8
tcp_debug_dol_dont_ring_tx_doorbell = 0x10

tcp_tx_debug_dol_dont_send_ack = 0x1

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

ETH_IP_HDR_SZE = 34
ETH_IP_VLAN_HDR_SZE = 38

def SetupProxyArgs(tc):
    tc.module.logger.info("Testcase Args:")
    same_flow = 0
    bypass_barco = 0
    send_ack = 0
    if hasattr(tc.module.args, 'same_flow'):
        same_flow = tc.module.args.same_flow
        tc.module.logger.info("- same_flow %s" % tc.module.args.same_flow)
    if hasattr(tc.module.args, 'bypass_barco'):
        bypass_barco = tc.module.args.bypass_barco
        tc.module.logger.info("- bypass_barco %s" % tc.module.args.bypass_barco)
    if hasattr(tc.module.args, 'send_ack'):
        send_ack = tc.module.args.send_ack
        tc.module.logger.info("- send_ack %s" % tc.module.args.send_ack)

    tc.module.logger.info("Testcase Iterators:")
    iterelem = tc.module.iterator.Get()
    if iterelem:
        if 'same_flow' in iterelem.__dict__:
            same_flow = iterelem.same_flow
            tc.module.logger.info("- same_flow %s" % iterelem.same_flow)
        if 'bypass_barco' in iterelem.__dict__:
            bypass_barco = iterelem.bypass_barco
            tc.module.logger.info("- bypass_barco %s" % iterelem.bypass_barco)
        if 'send_ack' in iterelem.__dict__:
            send_ack = iterelem.send_ack
            tc.module.logger.info("- send_ack %s" % iterelem.send_ack)
    tc.pvtdata.same_flow = same_flow
    tc.pvtdata.bypass_barco = bypass_barco
    tc.pvtdata.send_ack = send_ack

def init_tcb_inorder(tc, tcb):
    tcb.rcv_nxt = 0x1ABABABA
    tcb.snd_nxt = 0x1FEFEFF0
    tcb.snd_una = 0x1FEFEFEF
    tc.pvtdata.flow1_rcv_nxt = tcb.rcv_nxt
    tc.pvtdata.flow1_snd_nxt = tcb.snd_nxt
    tc.pvtdata.flow1_snd_una = tcb.snd_una
    tcb.rcv_tsval = 0x1AFAFAFA
    tcb.ts_recent = 0x1AFAFAF0
    tcb.snd_wnd = 1000
    tcb.snd_cwnd = 1000
    tcb.rcv_mss = 9216
    tcb.debug_dol = 0
    if tc.pvtdata.send_ack:
        tcb.debug_dol_tx = 0 
    else:
        tcb.debug_dol = tcp_debug_dol_dont_ring_tx_doorbell
        tcb.debug_dol_tx = tcp_tx_debug_dol_dont_send_ack 
    if tc.pvtdata.same_flow:
        tcb.source_port = tc.config.flow.sport
        tcb.dest_port = tc.config.flow.dport
    else:
        tcb.source_port = tc.config.flow.dport
        tcb.dest_port = tc.config.flow.sport

    vlan_id = 0
    if tc.pvtdata.same_flow:
        if tc.config.src.endpoint.intf.type == 'UPLINK':
            # is there a better way to find the lif?
            tcb.source_lif = tc.config.src.endpoint.intf.port - 1
            if tc.config.src.segment.native == False:
                vlan_id = tc.config.src.segment.vlan_id
        elif hasattr(tc.config.src.endpoint.intf, 'encap_vlan_id'):
            vlan_id = tc.config.src.endpoint.intf.encap_vlan_id
            tcb.source_lif = tc.config.src.endpoint.intf.lif.hw_lif_id
    else:
        if tc.config.dst.endpoint.intf.type == 'UPLINK':
            # is there a better way to find the lif?
            tcb.source_lif = tc.config.dst.endpoint.intf.port - 1
            if tc.config.dst.segment.native == False:
                vlan_id = tc.config.dst.segment.vlan_id
        elif hasattr(tc.config.dst.endpoint.intf, 'encap_vlan_id'):
            vlan_id = tc.config.dst.endpoint.intf.encap_vlan_id
            tcb.source_lif = tc.config.dst.endpoint.intf.lif.hw_lif_id
    if vlan_id != 0:
        vlan_etype_bytes = bytes([0x81, 0x00]) + \
                vlan_id.to_bytes(2, 'big') + \
                bytes([0x08, 0x00])
    else:
        vlan_etype_bytes = bytes([0x08, 0x00])

    # TODO: ipv6
    tcb.header_len = ETH_IP_HDR_SZE + len(vlan_etype_bytes) - 2
    if tc.pvtdata.same_flow and tc.config.flow.IsIPV4():
        tcb.header_template = \
             tc.config.dst.endpoint.macaddr.getnum().to_bytes(6, 'big') + \
             tc.config.src.endpoint.macaddr.getnum().to_bytes(6, 'big') + \
             vlan_etype_bytes + \
             bytes([0x45, 0x08, 0x00, 0x7c, 0x00, 0x01, 0x00, 0x00]) + \
             bytes([0x40, 0x06, 0xfa, 0x71]) + \
             tc.config.flow.sip.getnum().to_bytes(4, 'big') + \
             tc.config.flow.dip.getnum().to_bytes(4, 'big')
        print("header_template = " + str(tcb.header_template))
    elif tc.config.flow.IsIPV4():
        tcb.header_template = \
             tc.config.src.endpoint.macaddr.getnum().to_bytes(6, 'big') + \
             tc.config.dst.endpoint.macaddr.getnum().to_bytes(6, 'big') + \
             vlan_etype_bytes + \
             bytes([0x45, 0x08, 0x00, 0x7c, 0x00, 0x01, 0x00, 0x00]) + \
             bytes([0x40, 0x06, 0xfa, 0x71]) + \
             tc.config.flow.dip.getnum().to_bytes(4, 'big') + \
             tc.config.flow.sip.getnum().to_bytes(4, 'big')
        print("header_template = " + str(tcb.header_template))
    # set tcb state to ESTABLISHED(1)
    tcb.state = 1

def init_tcb_inorder2(tc, tcb):
    tcb.rcv_nxt = 0x2ABABABA
    tcb.snd_nxt = 0x2FEFEFF0
    tcb.snd_una = 0x2FEFEFEF
    tc.pvtdata.flow2_rcv_nxt = tcb.rcv_nxt
    tc.pvtdata.flow2_snd_nxt = tcb.snd_nxt
    tc.pvtdata.flow2_snd_una = tcb.snd_una
    tcb.rcv_tsval = 0x2AFAFAFA
    tcb.ts_recent = 0x2AFAFAF0
    tcb.snd_wnd = 1000
    tcb.snd_cwnd = 1000
    tcb.rcv_mss = 9216
    tcb.debug_dol = 0
    if tc.pvtdata.send_ack:
        tcb.debug_dol_tx = 0 
    else:
        tcb.debug_dol = tcp_debug_dol_dont_ring_tx_doorbell
        tcb.debug_dol_tx = tcp_tx_debug_dol_dont_send_ack 
    tcb.source_port = tc.config.flow.sport
    tcb.dest_port = tc.config.flow.dport
    vlan_id = 0
    if tc.config.src.endpoint.intf.type == 'UPLINK':
        # is there a better way to find the lif?
        tcb.source_lif = tc.config.src.endpoint.intf.port - 1
        if tc.config.src.segment.native == False:
            vlan_id = tc.config.src.segment.vlan_id
    elif hasattr(tc.config.src.endpoint.intf, 'encap_vlan_id'):
        vlan_id = tc.config.src.endpoint.intf.encap_vlan_id
        tcb.source_lif = tc.config.src.endpoint.intf.lif.hw_lif_id
    if vlan_id != 0:
        vlan_etype_bytes = bytes([0x81, 0x00]) + \
                vlan_id.to_bytes(2, 'big') + \
                bytes([0x08, 0x00])
    else:
        vlan_etype_bytes = bytes([0x08, 0x00])
    if tc.config.flow.IsIPV4():
        tcb.header_len = ETH_IP_HDR_SZE + len(vlan_etype_bytes) - 2
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
