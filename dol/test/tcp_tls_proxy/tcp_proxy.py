# Common file for all proxy test cases

import pdb
from infra.common.logging import logger
from infra.common.logging import logger as logger

# Need to match defines in tcp-constants.h
tcp_debug_dol_pkt_to_serq = 0x1
tcp_debug_dol_test_atomic_stats = 0x2
tcp_debug_dol_dont_queue_to_serq = 0x4
tcp_debug_dol_leave_in_arq = 0x8
tcp_debug_dol_dont_send_ack = 0x10
tcp_debug_dol_del_ack_timer = 0x20
tcp_debug_dol_pkt_to_l7q = 0x40
tcp_debug_dol_bypass_barco = 0x80

tcp_tx_debug_dol_dont_send_ack = 0x1
tcp_tx_debug_dol_dont_tx = 0x2
tcp_tx_debug_dol_bypass_barco = 0x4
tcp_tx_debug_dol_dont_start_retx_timer = 0x8
tcp_tx_debug_dol_force_timer_full = 0x10
tcp_tx_debug_dol_force_tbl_setaddr = 0x20

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

TCP_OOO_CELL_SIZE = 128

l7_proxy_type_NONE = 0
l7_proxy_type_REDIR = 1
l7_proxy_type_SPAN = 2

tcp_ddol_TBLADDR_VALUE = 0x59

def SetupProxyArgs(tc):
    logger.info("Testcase Args:")
    same_flow = 0
    bypass_barco = 0
    send_ack_flow1 = 0
    send_ack_flow2 = 0
    test_timer = 0
    test_retx_timer = 0
    test_cancel_retx_timer = 0
    test_retx_timer_full = 0
    test_mpu_tblsetaddr = 0
    ooo_seq_delta = 0
    num_pkts = 1
    test_retx = None
    sem_full = None
    test_cong_avoid = 0
    fin = 0
    fin_ack = 0
    final_fin = 0
    timestamp = 0
    pkt_alloc = 0
    pkt_free = 0
    serq_full = False
    snd_cwnd = 0
    if hasattr(tc.module.args, 'same_flow'):
        same_flow = tc.module.args.same_flow
        logger.info("- same_flow %s" % tc.module.args.same_flow)
    if hasattr(tc.module.args, 'bypass_barco'):
        bypass_barco = tc.module.args.bypass_barco
        logger.info("- bypass_barco %s" % tc.module.args.bypass_barco)
    if hasattr(tc.module.args, 'send_ack_flow1'):
        send_ack_flow1 = tc.module.args.send_ack_flow1
        logger.info("- send_ack_flow1 %s" % tc.module.args.send_ack_flow1)
    if hasattr(tc.module.args, 'send_ack_flow2'):
        send_ack_flow2 = tc.module.args.send_ack_flow2
        logger.info("- send_ack_flow2 %s" % tc.module.args.send_ack_flow2)
    if hasattr(tc.module.args, 'test_timer'):
        test_timer = tc.module.args.test_timer
        logger.info("- test_timer %s" % tc.module.args.test_timer)
    if hasattr(tc.module.args, 'test_retx_timer'):
        test_retx_timer = tc.module.args.test_retx_timer
        logger.info("- test_retx_timer %s" % tc.module.args.test_retx_timer)
    if hasattr(tc.module.args, 'test_cancel_retx_timer'):
        test_cancel_retx_timer = tc.module.args.test_cancel_retx_timer
        logger.info("- test_cancel_retx_timer %s" % tc.module.args.test_cancel_retx_timer)
    if hasattr(tc.module.args, 'ooo_seq_delta'):
        ooo_seq_delta = tc.module.args.ooo_seq_delta
        logger.info("- ooo_seq_delta %s" % tc.module.args.ooo_seq_delta)
    if hasattr(tc.module.args, 'num_pkts'):
        num_pkts = tc.module.args.num_pkts
        logger.info("- num_pkts %s" % tc.module.args.num_pkts)
    if hasattr(tc.module.args, 'test_retx'):
        test_retx = tc.module.args.test_retx
    if hasattr(tc.module.args, 'sem_full'):
        sem_full = tc.module.args.sem_full
    if hasattr(tc.module.args, 'test_cong_avoid'):
        test_cong_avoid = tc.module.args.test_cong_avoid
    if hasattr(tc.module.args, 'fin'):
        fin = tc.module.args.fin
    if hasattr(tc.module.args, 'fin_ack'):
        fin_ack = tc.module.args.fin_ack
    if hasattr(tc.module.args, 'final_fin'):
        final_fin = tc.module.args.final_fin
    if hasattr(tc.module.args, 'test_retx_timer_full'):
        test_retx_timer_full = tc.module.args.test_retx_timer_full
        logger.info("- test_retx_timer_full %s" % tc.module.args.test_retx_timer_full)
    if hasattr(tc.module.args, 'test_mpu_tblsetaddr'):
        test_mpu_tblsetaddr = tc.module.args.test_mpu_tblsetaddr
        logger.info("- test_mpu_tblsetaddr %s" % tc.module.args.test_mpu_tblsetaddr)
    if hasattr(tc.module.args, 'timestamp'):
        timestamp = tc.module.args.timestamp
        logger.info("- timestamp %s" % tc.module.args.timestamp)
    if hasattr(tc.module.args, 'pkt_alloc'):
        pkt_alloc = tc.module.args.pkt_alloc
        logger.info("- pkt_alloc %s" % tc.module.args.pkt_alloc)
    if hasattr(tc.module.args, 'pkt_free'):
        pkt_free = tc.module.args.pkt_free
        logger.info("- pkt_free %s" % tc.module.args.pkt_free)
    if hasattr(tc.module.args, 'serq_full'):
        serq_full = tc.module.args.serq_full
        logger.info("- serq_full %s" % tc.module.args.serq_full)
    if hasattr(tc.module.args, 'snd_cwnd'):
        snd_cwnd = tc.module.args.snd_cwnd
        logger.info("- snd_cwnd %s" % tc.module.args.snd_cwnd)

    logger.info("Testcase Iterators:")
    iterelem = tc.module.iterator.Get()
    if iterelem:
        if 'same_flow' in iterelem.__dict__:
            same_flow = iterelem.same_flow
            logger.info("- same_flow %s" % iterelem.same_flow)
        if 'bypass_barco' in iterelem.__dict__:
            bypass_barco = iterelem.bypass_barco
            logger.info("- bypass_barco %s" % iterelem.bypass_barco)
        if 'send_ack_flow1' in iterelem.__dict__:
            send_ack_flow1 = iterelem.send_ack_flow1
            logger.info("- send_ack_flow1 %s" % iterelem.send_ack_flow1)
        if 'send_ack_flow2' in iterelem.__dict__:
            send_ack_flow2 = iterelem.send_ack_flow2
            logger.info("- send_ack_flow2 %s" % iterelem.send_ack_flow2)
        if 'test_timer' in iterelem.__dict__:
            test_timer = iterelem.test_timer
            logger.info("- test_timer %s" % iterelem.test_timer)
        if 'test_retx_timer' in iterelem.__dict__:
            test_retx_timer = iterelem.test_retx_timer
            logger.info("- test_retx_timer %s" % iterelem.test_retx_timer)
        if 'test_cancel_retx_timer' in iterelem.__dict__:
            test_cancel_retx_timer = iterelem.test_cancel_retx_timer
            logger.info("- test_cancel_retx_timer %s" % iterelem.test_cancel_retx_timer)
        if 'ooo_seq_delta' in iterelem.__dict__:
            ooo_seq_delta = iterelem.ooo_seq_delta
            logger.info("- ooo_seq_delta %s" % iterelem.ooo_seq_delta)
        if 'num_pkts' in iterelem.__dict__:
            num_pkts = iterelem.num_pkts
            logger.info("- num_pkts %s" % iterelem.num_pkts)
        if 'sem_full' in iterelem.__dict__:
            sem_full = iterelem.sem_full
            logger.info("- sem_full %s" % iterelem.sem_full)
        if 'test_cong_avoid' in iterelem.__dict__:
            test_cong_avoid = iterelem.test_cong_avoid
            logger.info("- test_cong_avoid %s" % iterelem.test_cong_avoid)
        if 'fin' in iterelem.__dict__:
            fin = iterelem.fin
            logger.info("- fin %s" % iterelem.fin)
    tc.pvtdata.same_flow = same_flow
    tc.pvtdata.bypass_barco = bypass_barco
    tc.pvtdata.send_ack_flow1 = send_ack_flow1
    tc.pvtdata.send_ack_flow2 = send_ack_flow2
    tc.pvtdata.test_timer = test_timer
    tc.pvtdata.test_retx_timer = test_retx_timer
    tc.pvtdata.test_cancel_retx_timer = test_cancel_retx_timer
    tc.pvtdata.test_retx_timer_full = test_retx_timer_full
    tc.pvtdata.test_mpu_tblsetaddr = test_mpu_tblsetaddr
    tc.pvtdata.ooo_seq_delta = ooo_seq_delta
    tc.pvtdata.num_pkts = num_pkts
    tc.pvtdata.test_retx = test_retx
    tc.pvtdata.sem_full = sem_full
    tc.pvtdata.test_cong_avoid = test_cong_avoid
    tc.pvtdata.fin = fin
    tc.pvtdata.fin_ack = fin_ack
    tc.pvtdata.final_fin = final_fin
    tc.pvtdata.timestamp = timestamp
    tc.pvtdata.pkt_alloc = pkt_alloc
    tc.pvtdata.pkt_free = pkt_free
    tc.pvtdata.serq_full = serq_full
    tc.pvtdata.snd_cwnd = snd_cwnd

def init_flow_pvtdata(tc, tcb1, tcb2):
    tc.pvtdata.flow1_rcv_nxt = tcb1.rcv_nxt
    tc.pvtdata.flow1_snd_nxt = tcb1.snd_nxt
    tc.pvtdata.flow1_snd_una = tcb1.snd_una
    tc.pvtdata.flow1_bytes_rxed = 0
    tc.pvtdata.flow1_bytes_txed = 0

    tc.pvtdata.flow2_rcv_nxt = tcb2.rcv_nxt
    tc.pvtdata.flow2_snd_nxt = tcb2.snd_nxt
    tc.pvtdata.flow2_snd_una = tcb2.snd_una
    tc.pvtdata.flow2_bytes_rxed = 0
    tc.pvtdata.flow2_bytes_txed = 0

def init_tcb1(tcb, session):
    tcb.rcv_nxt = 0x1ABABABA
    tcb.snd_nxt = 0x1FEFEFF0
    tcb.snd_una = 0x1FEFEFF0
    tcb.rcv_tsval = 0x1AFAFAFA
    tcb.ts_recent = 0x1AFAFAF0
    tcb.snd_cwnd = 10000

    tcb.source_port = session.iflow.dport
    tcb.dest_port = session.iflow.sport

    sep = session.iflow._FlowObject__sep
    dep = session.iflow._FlowObject__dep

    vlan_id = 0
    if dep.intf.type == 'UPLINK':
        # is there a better way to find the lif?
        tcb.source_lif = dep.intf.port
        if dep.segment.native == False:
            vlan_id = dep.segment.vlan_id
    elif hasattr(dep.intf, 'encap_vlan_id'):
        vlan_id = dep.intf.encap_vlan_id
        tcb.source_lif = dep.intf.lif.hw_lif_id

    if vlan_id != 0:
        vlan_id = 0x7 << 13 | vlan_id
        vlan_etype_bytes = bytes([0x81, 0x00]) + \
                vlan_id.to_bytes(2, 'big') + \
                bytes([0x08, 0x00])
    else:
        vlan_etype_bytes = bytes([0x08, 0x00])

    # TODO: ipv6
    tcb.header_len = ETH_IP_HDR_SZE + len(vlan_etype_bytes) - 2
    if session.iflow.IsIPV4():
        tcb.header_template = \
             sep.macaddr.getnum().to_bytes(6, 'big') + \
             dep.macaddr.getnum().to_bytes(6, 'big') + \
             vlan_etype_bytes + \
             bytes([0x45, 0x07, 0x00, 0x7c, 0x00, 0x01, 0x00, 0x00]) + \
             bytes([0x40, 0x06, 0xfa, 0x71]) + \
             session.iflow.dip.getnum().to_bytes(4, 'big') + \
             session.iflow.sip.getnum().to_bytes(4, 'big')
        print("header_template = " + str(tcb.header_template))
    # set tcb state to ESTABLISHED(1)
    tcb.state = 1
    # pred_flags
    #   header len = 5 (20 bytes, assuming no timestamp)
    #   flags = ACK
    #   window size = 1000
    tcb.pred_flags = 0x501003e8

def init_tcb_inorder(tc, tcb):
    tcb.rcv_nxt = 0x1ABABABA
    tcb.snd_nxt = 0x1FEFEFF0
    tcb.snd_una = 0x1FEFEFF0
    tc.pvtdata.flow1_rcv_nxt = tcb.rcv_nxt
    tc.pvtdata.flow1_snd_nxt = tcb.snd_nxt
    tc.pvtdata.flow1_snd_una = tcb.snd_una
    tc.pvtdata.flow1_bytes_rxed = 0
    tc.pvtdata.flow1_bytes_txed = 0
    tcb.rcv_tsval = 0x1AFAFAFA
    tcb.ts_recent = 0x1AFAFAF0
    tcb.snd_wnd = 1000
    if tc.pvtdata.snd_cwnd:
        tcb.snd_cwnd = tc.pvtdata.snd_cwnd
    else:
        tcb.snd_cwnd = 10        # snd_cwnd is in packets
    if tc.pvtdata.test_cong_avoid:
        tcb.snd_cwnd_cnt = tcb.snd_cwnd - 1
    tcb.rcv_mss = 9216
    tcb.debug_dol = 0
    if tc.pvtdata.send_ack_flow1:
        tcb.debug_dol_tx = 0
    else:
        tcb.debug_dol = tcp_debug_dol_dont_send_ack
        tcb.debug_dol_tx = tcp_tx_debug_dol_dont_send_ack
    if tc.pvtdata.test_timer:
        tcb.debug_dol |= tcp_debug_dol_del_ack_timer
    if not tc.pvtdata.test_retx_timer and not tc.pvtdata.test_retx_timer_full:
        tcb.debug_dol_tx |= tcp_tx_debug_dol_dont_start_retx_timer
    if tc.pvtdata.test_retx_timer_full:
        tcb.debug_dol_tx |= tcp_tx_debug_dol_force_timer_full
    if tc.pvtdata.test_mpu_tblsetaddr:
        tcb.debug_dol_tx |= tcp_tx_debug_dol_force_tbl_setaddr
    if tc.pvtdata.same_flow:
        tcb.source_port = tc.config.flow.sport
        tcb.dest_port = tc.config.flow.dport
    else:
        tcb.source_port = tc.config.flow.dport
        tcb.dest_port = tc.config.flow.sport
    if tc.pvtdata.bypass_barco:
        tcb.debug_dol_tx |= tcp_tx_debug_dol_bypass_barco
        tcb.debug_dol |= tcp_debug_dol_bypass_barco

    vlan_id = 0
    if tc.pvtdata.same_flow:
        if tc.config.src.endpoint.intf.type == 'UPLINK':
            # is there a better way to find the lif?
            tcb.source_lif = tc.config.src.endpoint.intf.port
            if tc.config.src.segment.native == False:
                vlan_id = tc.config.src.segment.vlan_id
        elif hasattr(tc.config.src.endpoint.intf, 'encap_vlan_id'):
            vlan_id = tc.config.src.endpoint.intf.encap_vlan_id
            tcb.source_lif = tc.config.src.endpoint.intf.lif.hw_lif_id
    else:
        if tc.config.dst.endpoint.intf.type == 'UPLINK':
            # is there a better way to find the lif?
            tcb.source_lif = tc.config.dst.endpoint.intf.port
            if tc.config.dst.segment.native == False:
                vlan_id = tc.config.dst.segment.vlan_id
        elif hasattr(tc.config.dst.endpoint.intf, 'encap_vlan_id'):
            vlan_id = tc.config.dst.endpoint.intf.encap_vlan_id
            tcb.source_lif = tc.config.dst.endpoint.intf.lif.hw_lif_id
    if vlan_id != 0:
        vlan_id = 0x7 << 13 | vlan_id
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
             bytes([0x45, 0x07, 0x00, 0x7c, 0x00, 0x01, 0x00, 0x00]) + \
             bytes([0x40, 0x06, 0xfa, 0x71]) + \
             tc.config.flow.sip.getnum().to_bytes(4, 'big') + \
             tc.config.flow.dip.getnum().to_bytes(4, 'big')
        print("header_template = " + str(tcb.header_template))
    elif tc.config.flow.IsIPV4():
        tcb.header_template = \
             tc.config.src.endpoint.macaddr.getnum().to_bytes(6, 'big') + \
             tc.config.dst.endpoint.macaddr.getnum().to_bytes(6, 'big') + \
             vlan_etype_bytes + \
             bytes([0x45, 0x07, 0x00, 0x7c, 0x00, 0x01, 0x00, 0x00]) + \
             bytes([0x40, 0x06, 0xfa, 0x71]) + \
             tc.config.flow.dip.getnum().to_bytes(4, 'big') + \
             tc.config.flow.sip.getnum().to_bytes(4, 'big')
        print("header_template = " + str(tcb.header_template))
    # set tcb state to ESTABLISHED(1)
    tcb.state = 1
    tcb.serq_pi = 0
    tcb.serq_ci = 0
    # pred_flags
    #   header len = 5 (20 bytes, assuming no timestamp)
    #   flags = ACK
    #   window size = 1000
    tcb.pred_flags = 0x501003e8

def init_tcb2(tcb, session):
    tcb.rcv_nxt = 0x2ABABABA
    tcb.snd_nxt = 0x2FEFEFF0
    tcb.snd_una = 0x2FEFEFF0
    tcb.rcv_tsval = 0x2AFAFAFA
    tcb.ts_recent = 0x2AFAFAF0
    tcb.snd_cwnd = 10000

    tcb.source_port = session.iflow.sport
    tcb.dest_port = session.iflow.dport

    dep = session.iflow._FlowObject__sep
    sep = session.iflow._FlowObject__dep

    vlan_id = 0
    if dep.intf.type == 'UPLINK':
        # is there a better way to find the lif?
        tcb.source_lif = dep.intf.port
        if dep.segment.native == False:
            vlan_id = dep.segment.vlan_id
    elif hasattr(dep.intf, 'encap_vlan_id'):
        vlan_id = dep.intf.encap_vlan_id
        tcb.source_lif = dep.intf.lif.hw_lif_id

    if vlan_id != 0:
        vlan_id = 0x7 << 13 | vlan_id
        vlan_etype_bytes = bytes([0x81, 0x00]) + \
                vlan_id.to_bytes(2, 'big') + \
                bytes([0x08, 0x00])
    else:
        vlan_etype_bytes = bytes([0x08, 0x00])

    # TODO: ipv6
    tcb.header_len = ETH_IP_HDR_SZE + len(vlan_etype_bytes) - 2
    if session.iflow.IsIPV4():
        tcb.header_template = \
             sep.macaddr.getnum().to_bytes(6, 'big') + \
             dep.macaddr.getnum().to_bytes(6, 'big') + \
             vlan_etype_bytes + \
             bytes([0x45, 0x07, 0x00, 0x7c, 0x00, 0x01, 0x00, 0x00]) + \
             bytes([0x40, 0x06, 0xfa, 0x71]) + \
             session.iflow.sip.getnum().to_bytes(4, 'big') + \
             session.iflow.dip.getnum().to_bytes(4, 'big')
        print("header_template = " + str(tcb.header_template))
    # set tcb state to ESTABLISHED(1)
    tcb.state = 1
    # pred_flags
    #   header len = 5 (20 bytes, assuming no timestamp)
    #   flags = ACK
    #   window size = 1000
    tcb.pred_flags = 0x501003e8

def init_tcb_inorder2(tc, tcb):
    tcb.rcv_nxt = 0x2ABABABA
    tcb.snd_nxt = 0x2FEFEFF0
    tcb.snd_una = 0x2FEFEFF0
    tc.pvtdata.flow2_rcv_nxt = tcb.rcv_nxt
    tc.pvtdata.flow2_snd_nxt = tcb.snd_nxt
    tc.pvtdata.flow2_snd_una = tcb.snd_una
    tc.pvtdata.flow2_bytes_rxed = 0
    tc.pvtdata.flow2_bytes_txed = 0
    tcb.rcv_tsval = 0x2AFAFAFA
    tcb.ts_recent = 0x2AFAFAF0
    tcb.snd_wnd = 1000
    if tc.pvtdata.snd_cwnd:
        tcb.snd_cwnd = tc.pvtdata.snd_cwnd
    else:
        tcb.snd_cwnd = 10        # snd_cwnd is in packets
    if tc.pvtdata.test_cong_avoid:
        tcb.snd_cwnd_cnt = tcb.snd_cwnd - 1
    tcb.rcv_mss = 9216
    tcb.debug_dol = 0
    if tc.pvtdata.send_ack_flow2:
        tcb.debug_dol_tx = 0
    else:
        tcb.debug_dol = tcp_debug_dol_dont_send_ack
        tcb.debug_dol_tx = tcp_tx_debug_dol_dont_send_ack
    if tc.pvtdata.test_timer:
        tcb.debug_dol |= tcp_debug_dol_del_ack_timer
    if not tc.pvtdata.test_retx_timer and not tc.pvtdata.test_retx_timer_full:
        tcb.debug_dol_tx |= tcp_tx_debug_dol_dont_start_retx_timer
    if tc.pvtdata.test_retx_timer_full:
        tcb.debug_dol_tx |= tcp_tx_debug_dol_force_timer_full
    if tc.pvtdata.test_mpu_tblsetaddr:
        tcb.debug_dol_tx |= tcp_tx_debug_dol_force_tbl_setaddr
    if tc.pvtdata.bypass_barco:
        tcb.debug_dol_tx |= tcp_tx_debug_dol_bypass_barco
        tcb.debug_dol |= tcp_debug_dol_bypass_barco

    tcb.source_port = tc.config.flow.sport
    tcb.dest_port = tc.config.flow.dport
    vlan_id = 0
    if tc.config.src.endpoint.intf.type == 'UPLINK':
        # is there a better way to find the lif?
        tcb.source_lif = tc.config.src.endpoint.intf.port
        if tc.config.src.segment.native == False:
            vlan_id = tc.config.src.segment.vlan_id
    elif hasattr(tc.config.src.endpoint.intf, 'encap_vlan_id'):
        vlan_id = tc.config.src.endpoint.intf.encap_vlan_id
        tcb.source_lif = tc.config.src.endpoint.intf.lif.hw_lif_id
    if vlan_id != 0:
        vlan_id = 0x7 << 13 | vlan_id
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
             bytes([0x45, 0x07, 0x00, 0x7c, 0x00, 0x01, 0x00, 0x00]) + \
             bytes([0x40, 0x06, 0xfa, 0x71]) + \
             tc.config.flow.sip.getnum().to_bytes(4, 'big') + \
             tc.config.flow.dip.getnum().to_bytes(4, 'big')
        print("header_template = " + str(tcb.header_template))
    # set tcb state to ESTABLISHED(1)
    tcb.state = 1
    tcb.serq_pi = 0
    tcb.serq_ci = 0
    # pred_flags
    #   header len = 5 (20 bytes, assuming no timestamp)
    #   flags = ACK
    #   window size = 1000
    tcb.pred_flags = 0x501003e8
