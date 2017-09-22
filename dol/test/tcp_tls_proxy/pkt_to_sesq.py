# Testcase definition file.

import pdb
import copy

from config.store               import Store
from config.objects.proxycb_service    import ProxyCbServiceHelper
from config.objects.tcp_proxy_cb        import TcpCbHelper
from infra.common.objects import ObjectDatabase as ObjectDatabase
import test.callbacks.networking.modcbs as modcbs
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
    tcb.debug_dol = 0
    # set tcb state to ESTABLISHED(1)
    tcb.state = 1
    tcb.SetObjValPd()

    # 2. Clone objects that are needed for verification
    rnmdr = copy.deepcopy(tc.infra_data.ConfigStore.objects.db["RNMDR"])
    rnmdr.Configure()
    rnmpr = copy.deepcopy(tc.infra_data.ConfigStore.objects.db["RNMPR"])
    rnmpr.Configure()
    tnmdr = copy.deepcopy(tc.infra_data.ConfigStore.objects.db["TNMDR"])
    tnmdr.Configure()
    tnmpr = copy.deepcopy(tc.infra_data.ConfigStore.objects.db["TNMPR"])
    tnmpr.Configure()
    sesqid = "TCPCB%04d_SESQ" % id
    sesq = copy.deepcopy(tc.infra_data.ConfigStore.objects.db[sesqid])
    sesq.Configure()
    tlscbid = "TlsCb%04d" % id
    tlscb_cur = tc.infra_data.ConfigStore.objects.db[tlscbid]
    tlscb_cur.debug_dol = 5
    tlscb_cur.SetObjValPd()
    tlscb = copy.deepcopy(tlscb_cur)
    tlscb.GetObjValPd()
    tcpcb = copy.deepcopy(tcb)
    tcpcb.GetObjValPd()

    tc.pvtdata.Add(tlscb)
    tc.pvtdata.Add(rnmdr)
    tc.pvtdata.Add(rnmpr)
    tc.pvtdata.Add(tnmdr)
    tc.pvtdata.Add(tnmpr)
    tc.pvtdata.Add(tcpcb)
    tc.pvtdata.Add(sesq)
    return

def TestCaseVerify(tc):

    id = ProxyCbServiceHelper.GetFlowInfo(tc.config.flow._FlowObject__session)
    tlscbid = "TlsCb%04d" % id
    tlscb = tc.pvtdata.db[tlscbid]
    tlscb_cur = tc.infra_data.ConfigStore.objects.db[tlscbid]
    print("pre-sync: tnmdr_alloc %d tnmpr_alloc %d enc_requests %d" % (tlscb_cur.tnmdr_alloc, tlscb_cur.tnmpr_alloc, tlscb_cur.enc_requests))
    print("pre-sync: rnmdr_free %d rnmpr_free %d enc_completions %d" % (tlscb_cur.rnmdr_free, tlscb_cur.rnmpr_free, tlscb_cur.enc_completions))
    print("pre-sync: pre_debug_stage0_7_thread 0x%x post_debug_stage0_7_thread 0x%x" % (tlscb_cur.pre_debug_stage0_7_thread, tlscb_cur.post_debug_stage0_7_thread))
    tlscb_cur.GetObjValPd()
    print("post-sync: tnmdr_alloc %d tnmpr_alloc %d enc_requests %d" % (tlscb_cur.tnmdr_alloc, tlscb_cur.tnmpr_alloc, tlscb_cur.enc_requests))
    print("post-sync: rnmdr_free %d rnmpr_free %d enc_completions %d" % (tlscb_cur.rnmdr_free, tlscb_cur.rnmpr_free, tlscb_cur.enc_completions))
    print("post-sync: pre_debug_stage0_7_thread 0x%x post_debug_stage0_7_thread 0x%x" % (tlscb_cur.pre_debug_stage0_7_thread, tlscb_cur.post_debug_stage0_7_thread))

    # 0. Verify the counters
    if ((tlscb_cur.tnmdr_alloc - tlscb.tnmdr_alloc) != (tlscb_cur.rnmdr_free - tlscb.rnmdr_free)):
        print("tnmdr alloc increment not same as rnmdr free increment")
        return False

    if ((tlscb_cur.tnmpr_alloc - tlscb.tnmpr_alloc) != (tlscb_cur.rnmpr_free - tlscb.rnmpr_free)):
        print("tnmpr alloc increment not same as rnmpr free increment")
        return False


    if ((tlscb_cur.enc_requests - tlscb.enc_requests) != (tlscb_cur.enc_completions - tlscb.enc_completions)):
        print("enc requests not equal to completions %d %d %d %d" % (tlscb_cur.enc_requests, tlscb.enc_requests, tlscb_cur.enc_completions, tlscb.enc_completions))
        return False

    # 1. Verify threading
    if (tlscb_cur.pre_debug_stage0_7_thread != 0x117711):
        print("pre crypto pipeline threading was not ok")
        return False

    if (tlscb_cur.post_debug_stage0_7_thread != 0x17111):
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
    rnmdr_cur = tc.infra_data.ConfigStore.objects.db["RNMDR"]
    rnmdr_cur.Configure()
    rnmpr_cur = tc.infra_data.ConfigStore.objects.db["RNMPR"]
    rnmpr_cur.Configure()
    rnmdr = tc.pvtdata.db["RNMDR"]
    rnmpr = tc.pvtdata.db["RNMPR"]

    tnmdr_cur = tc.infra_data.ConfigStore.objects.db["TNMDR"]
    tnmdr_cur.Configure()
    tnmpr_cur = tc.infra_data.ConfigStore.objects.db["TNMPR"]
    tnmpr_cur.Configure()
    tnmdr = tc.pvtdata.db["TNMDR"]
    tnmpr = tc.pvtdata.db["TNMPR"]

    # 4. Verify PI for RNMDR got incremented by 1
    if (rnmdr_cur.pi != rnmdr.pi+1):
        print("RNMDR pi check failed old %d new %d" % (rnmdr.pi, rnmdr_cur.pi))
        return False

    
    # 5. Verify PI for TNMDR got incremented by 1
    if (tnmdr_cur.pi != tnmdr.pi+1):
        print("TNMDR pi check failed old %d new %d" % (tnmdr.pi, tnmdr_cur.pi))
        return False

    sesqid = "TCPCB%04d_SESQ" % id
    sesq = tc.pvtdata.db[sesqid]
    sesq_cur = tc.infra_data.ConfigStore.objects.db[sesqid]
    sesq_cur.Configure()

    # 6. Verify descriptor 
    if (rnmdr.ringentries[rnmdr.pi].handle != (sesq_cur.ringentries[0].handle - 0x40)):
        print("Descriptor handle not as expected in ringentries 0x%x 0x%x" % (rnmdr.ringentries[rnmdr.pi].handle, sesq_cur.ringentries[0].handle)) 
        return False


    return True

def TestCaseTeardown(tc):
    print("TestCaseTeardown(): Sample Implementation.")
    return
