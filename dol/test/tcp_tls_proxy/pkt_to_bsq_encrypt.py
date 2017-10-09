# Testcase definition file.
import pdb
import copy
import test.tcp_tls_proxy.tcp_proxy as tcp_proxy
import test.tcp_tls_proxy.tcp_tls_proxy as tcp_tls_proxy

import types_pb2                as types_pb2
import crypto_keys_pb2          as crypto_keys_pb2
# import crypto_keys_pb2_grpc     as crypto_keys_pb2_grpc

from config.store                       import Store
from config.objects.proxycb_service     import ProxyCbServiceHelper
from config.objects.tcp_proxy_cb        import TcpCbHelper
import test.callbacks.networking.modcbs as modcbs
from infra.common.objects               import ObjectDatabase as ObjectDatabase
from infra.common.logging               import logger
from infra.common.glopts import GlobalOptions


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



    tlscbid = "TlsCb%04d" % id
    tlscb = copy.deepcopy(tc.infra_data.ConfigStore.objects.db[tlscbid])
    tlscb.debug_dol = tcp_tls_proxy.tls_debug_dol_bypass_proxy | \
                            tcp_tls_proxy.tls_debug_dol_sesq_stop
    tlscb.other_fid = 0xffff

    if tc.module.args.key_size == 16:
        tcp_tls_proxy.tls_aes128_gcm_encrypt_setup(tc, tlscb)
    elif tc.module.args.key_size == 32:
        tcp_tls_proxy.tls_aes256_gcm_encrypt_setup(tc, tlscb)

    tlscb.GetObjValPd()
    print("snapshot1: tnmdr_alloc %d tnmpr_alloc %d enc_requests %d" % (tlscb.tnmdr_alloc, tlscb.tnmpr_alloc, tlscb.enc_requests))
    print("snapshot1: rnmdr_free %d rnmpr_free %d enc_completions %d" % (tlscb.rnmdr_free, tlscb.rnmpr_free, tlscb.enc_completions))


    brq = copy.deepcopy(tc.infra_data.ConfigStore.objects.db["BRQ_ENCRYPT"])



    tcpcb = copy.deepcopy(tcb)
    tcpcb.GetObjValPd()



    tc.pvtdata.Add(tlscb)
    tc.pvtdata.Add(rnmdr)
    tc.pvtdata.Add(rnmpr)
    tc.pvtdata.Add(tnmdr)
    tc.pvtdata.Add(tnmpr)
    tc.pvtdata.Add(tcpcb)

    return

def TestCaseVerify(tc):
    if GlobalOptions.dryrun:
        return True

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

    print("snapshot3: tnmdr_alloc %d tnmpr_alloc %d enc_requests %d" % (tlscb.tnmdr_alloc, tlscb.tnmpr_alloc, tlscb.enc_requests))
    print("snapshot3: rnmdr_free %d rnmpr_free %d enc_completions %d" % (tlscb.rnmdr_free, tlscb.rnmpr_free, tlscb.enc_completions))

    # 0. Verify the counters
    if ((tlscb_cur.tnmdr_alloc - tlscb.tnmdr_alloc) != (tlscb_cur.rnmdr_free - tlscb.rnmdr_free)):
        print("tnmdr alloc increment not same as rnmdr free increment")
        #return False

    if ((tlscb_cur.tnmpr_alloc - tlscb.tnmpr_alloc) != (tlscb_cur.rnmpr_free - tlscb.rnmpr_free)):
        print("tnmpr alloc increment not same as rnmpr free increment")
        #return False


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
    brq_cur = tc.infra_data.ConfigStore.objects.db["BRQ_ENCRYPT"]
    brq_cur.Configure()
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


    # 6. Verify PI for TNMPR got incremented by 1
    if (tnmpr_cur.pi != tnmpr.pi+1):
        print("TNMPR pi check failed old %d new %d" % (tnmpr.pi, tnmpr_cur.pi))
        return False
    print("Old TNMPR PI: %d, New TNMPR PI: %d" % (tnmpr.pi, tnmpr_cur.pi))

    print("BRQ: Current PI %d" % brq_cur.pi)


    # 7. Verify descriptor on the BRQ
    if (rnmdr.ringentries[rnmdr.pi].handle != (brq_cur.ring_entries[brq_cur.pi-1].ilist_addr - 0x40)):
        print("RNMDR Check: Descriptor handle not as expected in ringentries 0x%x 0x%x" % (rnmdr.ringentries[rnmdr.pi].handle, brq_cur.ring_entries[0].ilist_addr))
        return False


    # 8. Verify descriptor on the BRQ
    if (tnmdr.ringentries[tnmdr.pi].handle != (brq_cur.ring_entries[brq_cur.pi-1].olist_addr - 0x40)):
        print("TNMDR Check: Descriptor handle not as expected in ringentries 0x%x 0x%x" % (tnmdr.ringentries[tnmdr.pi].handle, brq_cur.ring_entries[0].olist_addr))
        return False



    # 9. Verify BRQ Descriptor Command field
    if brq_cur.ring_entries[brq_cur.pi-1].command != tlscb.command:
        print("BRQ Command Check: Failed : Got: 0x%x, Expected: 0x%x" % (brq_cur.ring_entries[0].command, tlscb.command))
        return False


    # 10. Verify BRQ Descriptor Key Index field
    if brq_cur.ring_entries[brq_cur.pi-1].key_desc_index != tlscb.crypto_key_idx:
        print("BRQ Crypto Key Index Check: Failed : Got: 0x%x, Expected: 0x%x" % (brq_cur.ring_entries[0].key_desc_index, tlscb.crypto_key_idx))
        return False

    # 11. Verify Salt
    if brq_cur.ring_entries[brq_cur.pi-1].salt != tlscb.salt:
        print("Salt Check Failed: Got 0x%x, Expected: 0x%x" % (brq_cur.ring_entries[brq_cur.pi-1].salt, tlscb.salt))
        return False
    print("Salt Check Success: Got 0x%x, Expected: 0x%x" % (brq_cur.ring_entries[brq_cur.pi-1].salt, tlscb.salt))
        
    # 12. Verify Explicit IV
    tls_explicit_iv = tlscb.explicit_iv
    if brq_cur.ring_entries[brq_cur.pi-1].explicit_iv != tls_explicit_iv:
        print("Explicit IV Check Failed: Got 0x%x, Expected: 0x%x" %
                                (brq_cur.ring_entries[brq_cur.pi-1].explicit_iv, tls_explicit_iv))
        return False
    else:
        print("Explicit IV Check Success: Got 0x%x, Expected: 0x%x" %
                                (brq_cur.ring_entries[brq_cur.pi-1].explicit_iv, tls_explicit_iv))

    # 13. Verify header size, this is the AAD size and is 13 bytes 
    if brq_cur.ring_entries[brq_cur.pi-1].header_size != 0xd:
        print("Header Size Check Failed: Got 0x%x, Expected: 0xd" %
                                (brq_cur.ring_entries[brq_cur.pi-1].header_size))
        return False
    else:
        print("Header Size Check Success: Got 0x%x, Expected: 0xd" %
                                (brq_cur.ring_entries[brq_cur.pi-1].header_size))

    # 14. Barco Status check
    if brq_cur.ring_entries[brq_cur.pi-1].barco_status != 0:
        print("Barco Status Check Failed: Got 0x%x, Expected: 0" %
                                (brq_cur.ring_entries[brq_cur.pi-1].barco_status))
        return False
    else:
        print("Barco Status Check Success: Got 0x%x, Expected: 0" %
                                (brq_cur.ring_entries[brq_cur.pi-1].barco_status))

    # 15. Verify page
    #if rnmpr.ringentries[0].handle != brq_cur.swdre_list[0].Addr1:
    #    print("Page handle not as expected in brq_cur.swdre_list")
        #return False

    return True

def TestCaseTeardown(tc):
    print("TestCaseTeardown(): Sample Implementation.")
    return
