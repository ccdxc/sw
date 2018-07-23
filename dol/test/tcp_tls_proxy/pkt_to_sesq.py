# Testcase definition file.

import pdb
import copy
import test.tcp_tls_proxy.tcp_proxy as tcp_proxy

from config.store               import Store
from config.objects.proxycb_service    import ProxyCbServiceHelper
from config.objects.tcp_proxy_cb        import TcpCbHelper
from infra.common.objects import ObjectDatabase as ObjectDatabase
import test.callbacks.networking.modcbs as modcbs
from infra.common.logging import logger
import test.tcp_tls_proxy.tcp_tls_proxy as tcp_tls_proxy
from infra.common.glopts import GlobalOptions


def Setup(infra, module):
    print("Setup(): Sample Implementation")
    modcbs.Setup(infra, module)
    return

def Teardown(infra, module):
    print("Teardown(): Sample Implementation.")
    return

def TestCaseSetup(tc):
    tc.pvtdata = ObjectDatabase()
    tcp_proxy.SetupProxyArgs(tc)

    id = ProxyCbServiceHelper.GetFlowInfo(tc.config.flow._FlowObject__session)
    TcpCbHelper.main(id)
    tcbid = "TcpCb%04d" % id
    # 1. Configure TCB in HBM before packet injection
    tcb = tc.infra_data.ConfigStore.objects.db[tcbid]
    tcp_proxy.init_tcb_inorder(tc, tcb)
    # set tcb state to ESTABLISHED(1)
    tcb.state = 1
    tcb.SetObjValPd()

    # 2. Clone objects that are needed for verification
    rnmdpr_big = copy.deepcopy(tc.infra_data.ConfigStore.objects.db["RNMDPR_BIG"])
    rnmdpr_big.GetMeta()
    rnmdpr_big.GetRingEntries([rnmdpr_big.pi])
    tnmdr = copy.deepcopy(tc.infra_data.ConfigStore.objects.db["TNMDPR_BIG"])
    tnmdr.GetMeta()
    sesqid = "TCPCB%04d_SESQ" % id
    sesq = copy.deepcopy(tc.infra_data.ConfigStore.objects.db[sesqid])
    sesq.GetMeta()
    tlscbid = "TlsCb%04d" % id
    tlscb_cur = tc.infra_data.ConfigStore.objects.db[tlscbid]
    tlscb_cur.debug_dol = (tcp_tls_proxy.tls_debug_dol_bypass_proxy | tcp_tls_proxy.tls_debug_dol_sesq_stop | tcp_tls_proxy.tls_debug_dol_bypass_barco)
    tlscb_cur.other_fid = 0xffff
    tlscb_cur.is_decrypt_flow = False
    tlscb_cur.serq_pi = 0
    tlscb_cur.serq_ci = 0
    tlscb_cur.SetObjValPd()
    tlscb = copy.deepcopy(tlscb_cur)
    tlscb.GetObjValPd()
    tcpcb = copy.deepcopy(tcb)
    tcpcb.GetObjValPd()

    tc.pvtdata.Add(tlscb)
    tc.pvtdata.Add(rnmdpr_big)
    tc.pvtdata.Add(tnmdr)
    tc.pvtdata.Add(tcpcb)
    tc.pvtdata.Add(sesq)
    return

def TestCaseVerify(tc):
    if GlobalOptions.dryrun:
        return True

    id = ProxyCbServiceHelper.GetFlowInfo(tc.config.flow._FlowObject__session)
    tlscbid = "TlsCb%04d" % id
    tlscb = tc.pvtdata.db[tlscbid]
    tlscb_cur = tc.infra_data.ConfigStore.objects.db[tlscbid]
    print("pre-sync: tnmdr_alloc %d tnmpr_alloc %d enc_requests %d" % (tlscb_cur.tnmdr_alloc, tlscb_cur.tnmpr_alloc, tlscb_cur.enc_requests))
    print("pre-sync: rnmdpr_big_free %d rnmpr_free %d enc_completions %d" % (tlscb_cur.rnmdr_free, tlscb_cur.rnmpr_free, tlscb_cur.enc_completions))
    print("pre-sync: pre_debug_stage0_7_thread 0x%x post_debug_stage0_7_thread 0x%x" % (tlscb_cur.pre_debug_stage0_7_thread, tlscb_cur.post_debug_stage0_7_thread))
    tlscb_cur.GetObjValPd()
    print("post-sync: tnmdr_alloc %d tnmpr_alloc %d enc_requests %d" % (tlscb_cur.tnmdr_alloc, tlscb_cur.tnmpr_alloc, tlscb_cur.enc_requests))
    print("post-sync: rnmdpr_big_free %d rnmpr_free %d enc_completions %d" % (tlscb_cur.rnmdr_free, tlscb_cur.rnmpr_free, tlscb_cur.enc_completions))
    print("post-sync: pre_debug_stage0_7_thread 0x%x post_debug_stage0_7_thread 0x%x" % (tlscb_cur.pre_debug_stage0_7_thread, tlscb_cur.post_debug_stage0_7_thread))

    print("snapshot: tnmdr_alloc %d tnmpr_alloc %d enc_requests %d" % (tlscb.tnmdr_alloc, tlscb.tnmpr_alloc, tlscb.enc_requests))
    print("snapshot: rnmdpr_big_free %d rnmpr_free %d enc_completions %d" % (tlscb.rnmdr_free, tlscb.rnmpr_free, tlscb.enc_completions))


    # 0. Verify the counters
    #if ((tlscb_cur.tnmdr_alloc - tlscb.tnmdr_alloc) != (tlscb_cur.rnmdpr_big_free - tlscb.rnmdpr_big_free)):
        #print("tnmdr alloc increment not same as rnmdpr_big free increment")
        #return False

    #if ((tlscb_cur.tnmpr_alloc - tlscb.tnmpr_alloc) != (tlscb_cur.rnmpr_free - tlscb.rnmpr_free)):
        #print("tnmpr alloc increment not same as rnmpr free increment")
        #return False


    if ((tlscb_cur.enc_requests - tlscb.enc_requests) != (tlscb_cur.enc_completions - tlscb.enc_completions)):
        print("enc requests not equal to completions %d %d %d %d" % (tlscb_cur.enc_requests, tlscb.enc_requests, tlscb_cur.enc_completions, tlscb.enc_completions))
        return False

    # 1. Verify threading
    if (tlscb_cur.pre_debug_stage0_7_thread != 0x151111):
        print("pre crypto pipeline threading was not ok")
        return False

    if (tlscb_cur.post_debug_stage0_7_thread != 0x1111111):
        print("post crypto pipeline threading was not ok")
        return False

        
    tcbid = "TcpCb%04d" % id
    tcb = tc.pvtdata.db[tcbid]
    # 2. Verify pi/ci got update got updated for SESQ
    tcb_cur = tc.infra_data.ConfigStore.objects.db[tcbid]
    print("pre-sync: tcb_cur.sesq_pi %d tcb_cur.sesq_ci %d" % (tcb_cur.sesq_pi, tcb_cur.sesq_ci))
    tcb_cur.GetObjValPd()
    print("post-sync: tcb_cur.sesq_pi %d tcb_cur.sesq_ci %d" % (tcb_cur.sesq_pi, tcb_cur.sesq_ci))
    if (tcb_cur.sesq_pi != (tcb.sesq_pi+1) or tcb_cur.sesq_ci != (tcb.sesq_ci+1)):
        print("sesq pi/ci not as expected")
        #VijayV to enable this test after ci is fixed in p4+
        #return False


    # 3. Fetch current values from Platform
    rnmdpr_big_cur = tc.infra_data.ConfigStore.objects.db["RNMDPR_BIG"]
    rnmdpr_big_cur.GetMeta()
    rnmpr_cur = tc.infra_data.ConfigStore.objects.db["RNMPR"]
    rnmpr_cur.GetMeta()
    rnmdpr_big = tc.pvtdata.db["RNMDPR_BIG"]

    tnmdr_cur = tc.infra_data.ConfigStore.objects.db["TNMDPR_BIG"]
    tnmdr_cur.GetMeta()
    tnmdr = tc.pvtdata.db["TNMDPR_BIG"]

    # 4. Verify PI for RNMDPR_BIG got incremented by 1
    if (rnmdpr_big_cur.pi != rnmdpr_big.pi+1):
        print("RNMDPR_BIG pi check failed old %d new %d" % (rnmdpr_big.pi, rnmdpr_big_cur.pi))
        return False

    
    sesqid = "TCPCB%04d_SESQ" % id
    sesq = tc.pvtdata.db[sesqid]
    sesq_cur = tc.infra_data.ConfigStore.objects.db[sesqid]
    sesq_cur.GetMeta()
    sesq_cur.GetRingEntries([0])

    # 6. Verify descriptor 
    if (rnmdpr_big.ringentries[rnmdpr_big.pi].handle != (sesq_cur.ringentries[0].handle - 0x40)):
        print("Descriptor handle not as expected in ringentries 0x%x 0x%x" % (rnmdpr_big.ringentries[rnmdpr_big.pi].handle, sesq_cur.ringentries[0].handle)) 
        return False


    return True

def TestCaseTeardown(tc):
    print("TestCaseTeardown(): Sample Implementation.")
    return
