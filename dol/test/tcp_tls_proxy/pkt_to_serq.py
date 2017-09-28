# Testcase definition file.

import pdb
import copy
import test.tcp_tls_proxy.tcp_proxy as tcp_proxy
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
    tcp_proxy.SetupProxyArgs(tc)
    id = ProxyCbServiceHelper.GetFlowInfo(tc.config.flow._FlowObject__session)
    TcpCbHelper.main(id)
    tcbid = "TcpCb%04d" % id
    # 1. Configure TCB in HBM before packet injection
    tcb = tc.infra_data.ConfigStore.objects.db[tcbid]
    tcp_proxy.init_tcb_inorder(tc, tcb)
    tcb.debug_dol |= tcp_proxy.tcp_debug_dol_pkt_to_serq
    if hasattr(tc.module.args, 'atomic_stats') and tc.module.args.atomic_stats:
        print("Testing atomic stats")
        tcb.debug_dol |= tcp_proxy.tcp_debug_dol_test_atomic_stats
    tcb.bytes_rcvd = 0
    # set tcb state to ESTABLISHED(1)
    tcb.state = 1
    tcb.SetObjValPd()

    # 2. Clone objects that are needed for verification
    rnmdr = copy.deepcopy(tc.infra_data.ConfigStore.objects.db["RNMDR"])
    rnmdr.Configure()
    rnmpr = copy.deepcopy(tc.infra_data.ConfigStore.objects.db["RNMPR"])
    rnmpr.Configure()
    serqid = "TLSCB%04d_SERQ" % id
    serq = copy.deepcopy(tc.infra_data.ConfigStore.objects.db[serqid])
    serq.Configure()
    tlscbid = "TlsCb%04d" % id
    tlscb = copy.deepcopy(tc.infra_data.ConfigStore.objects.db[tlscbid])
    tlscb.GetObjValPd()
    tcpcb = copy.deepcopy(tc.infra_data.ConfigStore.objects.db[tcbid])
    tcpcb.GetObjValPd()
    
    tc.pvtdata.Add(tlscb)
    tc.pvtdata.Add(rnmdr)
    tc.pvtdata.Add(rnmpr)
    tc.pvtdata.Add(tcpcb)
    tc.pvtdata.Add(serq)
    return

def TestCaseVerify(tc):

    num_pkts = 1
    if hasattr(tc.module.args, 'num_pkts'):
        num_pkts = int(tc.module.args.num_pkts)
    id = ProxyCbServiceHelper.GetFlowInfo(tc.config.flow._FlowObject__session)
    tcbid = "TcpCb%04d" % id
    # 1. Verify rcv_nxt got updated
    tcpcb = tc.pvtdata.db[tcbid]
    tcb_cur = tc.infra_data.ConfigStore.objects.db[tcbid]
    print("rcv_nxt value pre-sync from HBM 0x%x" % tcb_cur.rcv_nxt)
    tcb_cur.GetObjValPd()
    print("rcv_nxt value post-sync from HBM 0x%x" % tcb_cur.rcv_nxt)
    if tcb_cur.rcv_nxt != 0x1abababa + num_pkts * 84: # fix this hack later
        print("rcv_nxt not as expected")
        return False
    print("rcv_nxt as expected")

    tlscbid = "TlsCb%04d" % id
    # 2. Verify pi/ci got update got updated
    tlscb = tc.pvtdata.db[tlscbid]
    tlscb_cur = tc.infra_data.ConfigStore.objects.db[tlscbid]
    print("pre-sync: tlscb_cur.serq_pi %d tlscb_cur.serq_ci %d" % (tlscb_cur.serq_pi, tlscb_cur.serq_ci))
    tlscb_cur.GetObjValPd()
    print("post-sync: tlscb_cur.serq_pi %d tlscb_cur.serq_ci %d" % (tlscb_cur.serq_pi, tlscb_cur.serq_ci))
    if (tlscb_cur.serq_pi != tlscb.serq_pi or tlscb_cur.serq_ci != tlscb.serq_ci):
        print("serq pi/ci not as expected")
        return False

    # 3. Fetch current values from Platform
    rnmdr = tc.pvtdata.db["RNMDR"]
    rnmpr = tc.pvtdata.db["RNMPR"]
    rnmdr_cur = tc.infra_data.ConfigStore.objects.db["RNMDR"]
    rnmdr_cur.Configure()
    rnmpr_cur = tc.infra_data.ConfigStore.objects.db["RNMPR"]
    rnmpr_cur.Configure()
    serqid = "TLSCB%04d_SERQ" % id
    serq = tc.pvtdata.db[serqid]
    serq_cur = tc.infra_data.ConfigStore.objects.db[serqid]
    serq_cur.Configure()

    # 4. Verify PI for RNMDR got incremented by 1 
    if (rnmdr_cur.pi != rnmdr.pi+num_pkts):
        print("RNMDR pi check failed old %d new %d" % (rnmdr.pi, rnmdr_cur.pi))
        return False

    # 5. Verify descriptor 
    if rnmdr.ringentries[rnmdr.pi].handle != serq_cur.ringentries[tlscb.serq_pi].handle and rnmdr.ringentries[rnmdr.pi+1].handle != serq_cur.ringentries[tlscb.serq_pi].handle:
        print("Descriptor handle not as expected in ringentries 0x%x 0x%x" % (rnmdr.ringentries[rnmdr.pi].handle, serq_cur.ringentries[tlscb.serq_pi].handle)) 
        return False

    if rnmdr.swdre_list[rnmdr.pi].DescAddr != serq_cur.swdre_list[tlscb.serq_pi].DescAddr and rnmdr.swdre_list[rnmdr.pi+1].DescAddr != serq_cur.swdre_list[tlscb.serq_pi].DescAddr:
        print("Descriptor handle not as expected in swdre_list 0x%x 0x%x" % (rnmdr.swdre_list[rnmdr.pi].DescAddr, serq_cur.swdre_list[tlscb.serq_pi].DescAddr))
        return False

    # 6. Verify page
    if rnmpr.ringentries[0].handle != serq_cur.swdre_list[0].Addr1:
        print("Page handle not as expected in serq_cur.swdre_list")
        #return False

    # Print stats
    print("bytes_rcvd = %d:" % tcb_cur.bytes_rcvd)
    print("pkts_rcvd = %d:" % tcb_cur.pkts_rcvd)
    print("pages_alloced = %d:" % tcb_cur.pages_alloced)
    print("desc_alloced = %d:" % tcb_cur.desc_alloced)

    #7 Verify pkt stats
    if tcb_cur.pkts_rcvd != tcpcb.pkts_rcvd + num_pkts:
        print("pkt rx stats not as expected, %d vs received %d" %
                (tcpcb.pkts_rcvd + num_pkts, tcb_cur.pkts_rcvd))
        return False
    print("%d %d" %(tcb_cur.bytes_rcvd, tcpcb.bytes_rcvd))
    if tcb_cur.bytes_rcvd != tcpcb.bytes_rcvd + num_pkts * 84:
        print("Warning! pkt rx byte stats not as expected")
        return False

    #8 Verify page stats
    if tcb_cur.pages_alloced != tcpcb.pages_alloced + num_pkts:
        print("pages alloced stats not as expected, %d vs received %d" %
                (tcpcb.pages_alloced + num_pkts, tcb_cur.pages_alloced))
        return False
    
    #9 Verify descr stats
    if tcb_cur.desc_alloced != tcpcb.desc_alloced + num_pkts:
        print("desc alloced stats not as expected, %d vs received %d" %
                (tcpcb.desc_alloced + num_pkts, tcb_cur.desc_alloced))
        return False

    # 10. Verify phv2mem counter
    if (tcb_cur.debug_num_phv_to_mem != tcpcb.debug_num_phv_to_mem+4*num_pkts):
        print("phv2mem not as expected, %d vs received %d" %
                (tcpcb.debug_num_phv_to_mem + 4*num_pkts, tcb_cur.debug_num_phv_to_mem))
        return False

    return True

def TestCaseTeardown(tc):
    print("TestCaseTeardown(): Sample Implementation.")
    return
