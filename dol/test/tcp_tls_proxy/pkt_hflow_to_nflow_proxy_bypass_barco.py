# Testcase definition file.

import pdb
import copy

from config.store               import Store
from config.objects.proxycb_service    import ProxyCbServiceHelper
from config.objects.tcp_proxy_cb        import TcpCbHelper
import test.callbacks.networking.modcbs as modcbs
from infra.common.objects import ObjectDatabase as ObjectDatabase
from infra.common.logging import logger


def Setup(infra, module):
    print("Setup(): Sample Implementation")
    modcbs.Setup(infra, module)
    return

def Teardown(infra, module):
    print("Teardown(): Sample Implementation.")
    return

def TestCaseSetup(tc):

    tc.pvtdata = ObjectDatabase(logger)
    id = ProxyCbServiceHelper.GetFlowInfo(tc.config.flow._FlowObject__session)
    other_id = ProxyCbServiceHelper.GetOtherFlowInfo(tc.config.flow._FlowObject__session)
    TcpCbHelper.main(id)
    TcpCbHelper.main(other_id)
    tcbid = "TcpCb%04d" % id
    other_tcbid = "TcpCb%04d" % other_id
    # 1. Configure TCB in HBM before packet injection
    tcb = tc.infra_data.ConfigStore.objects.db[tcbid]
    tcb.rcv_nxt = 0x1ABABABA
    tcb.snd_nxt = 0x1FEFEFF0
    tcb.snd_una = 0x1FEFEFEF
    tcb.rcv_tsval = 0x1AFAFAFA
    tcb.ts_recent = 0x1AFAFAF0
    tcb.snd_wnd = 1000
    tcb.snd_cwnd = 1000
    tcb.rcv_mss = 9216
    tcb.debug_dol = 0
    tcb.source_port = 47273
    tcb.dest_port = 80
    if tc.config.src.segment.vlan_id != 0:
        vlan_etype_bytes = bytes([0x81, 0x00]) + \
                tc.config.src.segment.vlan_id.to_bytes(2, 'big') + \
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
    tcb.SetObjValPd()
    # 2. Configure other TCB in HBM before packet injection
    other_tcb = tc.infra_data.ConfigStore.objects.db[other_tcbid]
    other_tcb.rcv_nxt = 0x2ABABABA
    other_tcb.snd_nxt = 0x2FEFEFF0
    other_tcb.snd_una = 0x2FEFEFEF
    other_tcb.rcv_tsval = 0x2AFAFAFA
    other_tcb.ts_recent = 0x2AFAFAF0
    other_tcb.snd_wnd = 1000
    other_tcb.snd_cwnd = 1000
    other_tcb.rcv_mss = 9216
    other_tcb.debug_dol = 0
    other_tcb.source_port = 47273
    other_tcb.dest_port = 80
    if tc.config.src.segment.vlan_id != 0:
        vlan_etype_bytes = bytes([0x81, 0x00]) + \
                tc.config.src.segment.vlan_id.to_bytes(2, 'big') + \
                bytes([0x08, 0x00])
    else:
        vlan_etype_bytes = bytes([0x08, 0x00])
    other_tcb.header_template = \
             tc.config.dst.endpoint.macaddr.getnum().to_bytes(6, 'big') + \
             tc.config.src.endpoint.macaddr.getnum().to_bytes(6, 'big') + \
             vlan_etype_bytes + \
             bytes([0x45, 0x08, 0x00, 0x7c, 0x00, 0x01, 0x00, 0x00]) + \
             bytes([0x40, 0x06, 0xfa, 0x71]) + \
             tc.config.flow.sip.getnum().to_bytes(4, 'big') + \
             tc.config.flow.dip.getnum().to_bytes(4, 'big')
    print("header_template = " + str(tcb.header_template))
    # set other tcb state to ESTABLISHED(1)
    other_tcb.state = 1
    other_tcb.SetObjValPd()

    # 3. Configure TLS CB in HBM before packet injection
    tlscbid = "TlsCb%04d" % id
    tlscb = tc.infra_data.ConfigStore.objects.db[tlscbid]
    tlscb.debug_dol = 1
    tlscb.other_fid = other_id
    tlscb.SetObjValPd()

    # 4. Configure other TLS CB in HBM before packet injection
    other_tlscbid = "TlsCb%04d" % other_id
    other_tlscb = tc.infra_data.ConfigStore.objects.db[other_tlscbid]
    other_tlscb.debug_dol = 1
    other_tlscb.other_fid = id
    other_tlscb.SetObjValPd()


    # 5. Clone objects that are needed for verification
    tcpcb = copy.deepcopy(tc.infra_data.ConfigStore.objects.db[tcbid])
    tcpcb.GetObjValPd()
    tc.pvtdata.Add(tcpcb)
    other_tcpcb = copy.deepcopy(tc.infra_data.ConfigStore.objects.db[other_tcbid])
    other_tcpcb.GetObjValPd()
    tc.pvtdata.Add(other_tcpcb)

    tlscb = copy.deepcopy(tc.infra_data.ConfigStore.objects.db[tlscbid])
    tlscb.GetObjValPd()
    tc.pvtdata.Add(tlscb)
    other_tlscb = copy.deepcopy(tc.infra_data.ConfigStore.objects.db[other_tlscbid])
    other_tcpcb.GetObjValPd()
    tc.pvtdata.Add(other_tlscb)
    return

def TestCaseVerify(tc):

    id = ProxyCbServiceHelper.GetFlowInfo(tc.config.flow._FlowObject__session)
    tcbid = "TcpCb%04d" % id
    tcpcb = tc.pvtdata.db[tcbid]
    tcpcb_cur = tc.infra_data.ConfigStore.objects.db[tcbid]
    tcpcb_cur.GetObjValPd()

    other_id = ProxyCbServiceHelper.GetOtherFlowInfo(tc.config.flow._FlowObject__session)
    other_tcbid = "TcpCb%04d" % other_id
    other_tcpcb = tc.pvtdata.db[other_tcbid]
    other_tcpcb_cur = tc.infra_data.ConfigStore.objects.db[other_tcbid]
    other_tcpcb_cur.GetObjValPd()

    tlscbid = "TlsCb%04d" % id
    tlscb = tc.pvtdata.db[tlscbid]
    tlscb_cur = tc.infra_data.ConfigStore.objects.db[tlscbid]
    tlscb_cur.GetObjValPd()

    other_tlscbid = "TlsCb%04d" % other_id
    other_tlscb = tc.pvtdata.db[other_tlscbid]
    other_tlscb_cur = tc.infra_data.ConfigStore.objects.db[other_tlscbid]
    other_tlscb_cur.GetObjValPd()


    # Print stats
    print("bytes_rcvd = %d:" % tcpcb_cur.bytes_rcvd)
    print("pkts_rcvd = %d:" % tcpcb_cur.pkts_rcvd)
    print("debug_num_phv_to_mem = %d:" % tcpcb_cur.debug_num_phv_to_mem)
    print("debug_num_pkt_to_mem = %d:" % tcpcb_cur.debug_num_pkt_to_mem)

    print("bytes_sent = %d:" % other_tcpcb_cur.bytes_sent)
    print("pkts_sent = %d:" % other_tcpcb_cur.pkts_sent)
    print("debug_num_phv_to_pkt = %d:" % other_tcpcb_cur.debug_num_phv_to_pkt)
    print("debug_num_mem_to_pkt = %d:" % other_tcpcb_cur.debug_num_mem_to_pkt)


    # 1. Verify SERQ pi got updated
    if (tlscb_cur.serq_pi != tlscb.serq_pi + 1):
        print("serq pi/ci not as expected old (%d, %d), new (%d, %d)" %
                (tlscb.serq_pi, tlscb.serq_ci,
                 tlscb_cur.serq_pi, tlscb_cur.serq_ci))
        return False

    # 2. Verify SERQ ci got updated
    if (tlscb_cur.serq_ci != tlscb.serq_ci + 1):
        print("sesq pi/ci not as expected old (%d, %d), new (%d, %d)" %
                (tlscb.serq_pi, tlscb.serq_ci,
                 tlscb_cur.serq_pi, tlscb_cur.serq_ci))
        return False

    # 3. Verify SESQ pi got updated
    if (other_tcpcb_cur.sesq_pi != other_tcpcb.sesq_pi + 1):
        print("sesq pi/ci not as expected old (%d, %d), new (%d, %d)" %
                (other_tcpcb.sesq_pi, other_tcpcb.sesq_ci,
                 other_tcpcb_cur.sesq_pi, other_tcpcb_cur.sesq_ci))
        return False

    # 4. Verify SESQ ci got updated
    if (other_tcpcb_cur.sesq_ci != other_tcpcb.sesq_ci + 1):
        print("sesq pi/ci not as expected old (%d, %d), new (%d, %d)" %
                (other_tcpcb.sesq_pi, other_tcpcb.sesq_ci,
                 other_tcpcb_cur.sesq_pi, other_tcpcb_cur.sesq_ci))
        return False

    # 5. Verify pkt rx stats
    if tcpcb_cur.pkts_rcvd != tcpcb.pkts_rcvd + 1:
        print("pkt rx stats not as expected")
        return False

    if ((tcpcb_cur.bytes_rcvd - tcpcb.bytes_rcvd) != 84):
        print("Warning! pkt rx byte stats not as expected %d %d" % (tcpcb_cur.bytes_rcvd, tcpcb.bytes_rcvd))
        return False

    # 6. Verify pkt tx stats
    if other_tcpcb_cur.pkts_sent != other_tcpcb.pkts_sent + 1:
        print("pkt tx stats not as expected")
        return False

    if ((other_tcpcb_cur.bytes_sent - other_tcpcb.bytes_sent) != 54):
        print("Warning! pkt tx byte stats not as expected %d %d" % (other_tcpcb_cur.bytes_sent, other_tcpcb.bytes_sent))
        return False
    
    # 7. Verify phv2pkt
    if other_tcpcb_cur.debug_num_phv_to_pkt != other_tcpcb.debug_num_phv_to_pkt + 2:
        print("Num phv2pkt not as expected")
        return False

    # 8. Verify mem2pkt
    if other_tcpcb_cur.debug_num_mem_to_pkt != other_tcpcb.debug_num_mem_to_pkt + 2:
        print("Num mem2pkt not as expected")
        return False

    # 9  Verify phv2mem
    if other_tcpcb_cur.snd_nxt != 0xefeff044:
        print("mem2pkt failed snd_nxt = 0x%x" % other_tcpcb_cur.snd_nxt)
        return False

    # 10. Verify pkt tx (in testspec)

    return True

def TestCaseTeardown(tc):
    print("TestCaseTeardown(): Sample Implementation.")
    return
