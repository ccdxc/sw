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
    TcpCbHelper.main(id)
    tcbid = "TcpCb%04d" % id
    # 1. Configure TCB in HBM before packet injection
    tcb = tc.infra_data.ConfigStore.objects.db[tcbid]
    tcb.rcv_nxt = 0xBABABABA
    tcb.snd_nxt = 0xEFEFEFF0
    tcb.snd_una = 0xEFEFEFEF
    tcb.rcv_tsval = 0xFAFAFAFA
    tcb.ts_recent = 0xFAFAFAF0
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
    #tcb.header_template = bytes([
        #0x00, 0xee, 0xff, 0x00, 0x00, 0x03, # dmac
        #0x00, 0xee, 0xff, 0x00, 0x00, 0x02, # smac
        #0x81, 0x00, 0xe0, 0x02, 0x08, 0x00,
        # ip header
        #0x45, 0x08, 0x00, 0x7c, 0x00, 0x01, 0x00, 0x00,
        #0x40, 0x06, 0xfa, 0x71, 0x40, 0x00, 0x00, 0x01,
        #0x40, 0x00, 0x00, 0x02])
    tcb.SetObjValPd()

    # 2. Configure TLS CB in HBM before packet injection
    tlscbid = "TlsCb%04d" % id
    tlscb = tc.infra_data.ConfigStore.objects.db[tlscbid]
    tlscb.debug_dol = 1
    tlscb.SetObjValPd()

    # 3. Clone objects that are needed for verification
    tcpcb = copy.deepcopy(tc.infra_data.ConfigStore.objects.db[tcbid])
    tc.pvtdata.Add(tcpcb)
    return

def TestCaseVerify(tc):

    id = ProxyCbServiceHelper.GetFlowInfo(tc.config.flow._FlowObject__session)
    tcbid = "TcpCb%04d" % id
    tcpcb = tc.pvtdata.db[tcbid]
    tcpcb_cur = tc.infra_data.ConfigStore.objects.db[tcbid]
    tcpcb_cur.GetObjValPd()

    # Print stats
    print("bytes_sent = %d:" % tcpcb_cur.bytes_sent)
    print("pkts_sent = %d:" % tcpcb_cur.pkts_sent)
    print("debug_num_phv_to_pkt = %d:" % tcpcb_cur.debug_num_phv_to_pkt)
    print("debug_num_mem_to_pkt = %d:" % tcpcb_cur.debug_num_mem_to_pkt)

    # 1. Verify pi got updated
    if (tcpcb_cur.sesq_pi != tcpcb.sesq_pi + 1):
        print("sesq pi/ci not as expected old (%d, %d), new (%d, %d)" %
                (tcpcb.sesq_pi, tcpcb.sesq_ci,
                 tcpcb_cur.sesq_pi, tcpcb_cur.sesq_ci))
        return False

    # 2. Verify ci got updated
    if (tcpcb_cur.sesq_ci != tcpcb.sesq_ci + 1):
        print("sesq pi/ci not as expected old (%d, %d), new (%d, %d)" %
                (tcpcb.sesq_pi, tcpcb.sesq_ci,
                 tcpcb_cur.sesq_pi, tcpcb_cur.sesq_ci))
        return False

    # 3. Verify pkt stats
    if tcpcb_cur.pkts_sent != tcpcb.pkts_sent + 1:
        print("pkt tx stats not as expected")
        return False

    if tcpcb_cur.bytes_sent != tcpcb.bytes_sent:
        print("Warning! pkt tx byte stats not as expected")
        return False
    
    # 4. Verify phv2pkt
    if tcpcb_cur.debug_num_phv_to_pkt != tcpcb.debug_num_phv_to_pkt + 2:
        print("Num phv2pkt not as expected")
        return False

    # 5. Verify mem2pkt
    if tcpcb_cur.debug_num_mem_to_pkt != tcpcb.debug_num_mem_to_pkt + 2:
        print("Num mem2pkt not as expected")
        return False

    # 6  Verify phv2mem
    if tcpcb_cur.snd_nxt != 0xefeff044:
        print("mem2pkt failed snd_nxt = 0x%x" % tcpcb_cur.snd_nxt)
        return False

    # 7. Verify pkt tx (in testspec)

    return True

def TestCaseTeardown(tc):
    print("TestCaseTeardown(): Sample Implementation.")
    return
