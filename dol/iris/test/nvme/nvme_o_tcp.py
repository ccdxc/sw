# Common file for all nvme test cases

from infra.common.logging import logger
from infra.common.logging import logger as logger

ETH_IP_HDR_SZE = 34

def init_tcb2(tcb, session):
    tcb.rcv_nxt = 0x2ABABABA
    tcb.rcv_wup = 0x2ABABABA
    tcb.snd_nxt = 0x2FEFEFF0
    tcb.snd_una = 0x2FEFEFF0
    tcb.rcv_tsval = 0x2AFAFAFA
    tcb.ts_recent = 0x2AFAFAF0
    tcb.snd_cwnd = 50000
    tcb.initial_window = 50000
    tcb.snd_wnd = 50000
    tcb.rcv_mss = 9000
    tcb.smss = 9000
    tcb.snd_ssthresh = 4000

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
