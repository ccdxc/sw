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
    rnmdr.GetMeta()
    rnmdr.GetRingEntries([rnmdr.pi])
    rnmpr = copy.deepcopy(tc.infra_data.ConfigStore.objects.db["RNMPR"])
    rnmpr.GetMeta()
    tnmdr = copy.deepcopy(tc.infra_data.ConfigStore.objects.db["TNMDR"])
    tnmdr.GetMeta()
    tnmdr.GetRingEntries([tnmdr.pi])
    tnmpr = copy.deepcopy(tc.infra_data.ConfigStore.objects.db["TNMPR"])
    tnmpr.GetMeta()



    tlscbid = "TlsCb%04d" % id
    tlscb = copy.deepcopy(tc.infra_data.ConfigStore.objects.db[tlscbid])
    tlscb.debug_dol = tcp_tls_proxy.tls_debug_dol_bypass_proxy | \
                            tcp_tls_proxy.tls_debug_dol_sesq_stop

    #If 'use_random_iv' is set, set the corresponding debug-dol flag to indicate
    #datapath to pick a random value from DRBG as IV.
    if hasattr(tc.module.args, 'use_random_iv') and tc.module.args.use_random_iv == 1:
        tlscb.debug_dol |= tcp_tls_proxy.tls_debug_dol_explicit_iv_use_random
    tlscb.other_fid = 0xffff

    if tc.module.args.key_size == 16:
        tcp_tls_proxy.tls_aes128_encrypt_setup(tc, tlscb)
    elif tc.module.args.key_size == 32:
        tcp_tls_proxy.tls_aes256_encrypt_setup(tc, tlscb)
    
    tlscb.serq_pi = 0
    tlscb.serq_ci = 0
    tlscb.SetObjValPd()
    print("snapshot1: tnmdr_alloc %d tnmpr_alloc %d enc_requests %d" % (tlscb.tnmdr_alloc, tlscb.tnmpr_alloc, tlscb.enc_requests))
    print("snapshot1: rnmdr_free %d rnmpr_free %d enc_completions %d" % (tlscb.rnmdr_free, tlscb.rnmpr_free, tlscb.enc_completions))

    if tc.module.args.cipher_suite == "CCM":
        brq = copy.deepcopy(tc.infra_data.ConfigStore.objects.db["BRQ_ENCRYPT_CCM"])
    elif tc.module.args.cipher_suite == "CBC":
        brq = copy.deepcopy(tc.infra_data.ConfigStore.objects.db["BRQ_ENCRYPT_CBC"])
    else:
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
    print("pre-sync: tnmdr_alloc %d tnmpr_alloc %d enc_requests %d mac_requests %d" % (tlscb_cur.tnmdr_alloc, tlscb_cur.tnmpr_alloc, tlscb_cur.enc_requests, tlscb_cur.mac_requests))
    print("pre-sync: rnmdr_free %d rnmpr_free %d enc_completions %d mac_completions %d" % (tlscb_cur.rnmdr_free, tlscb_cur.rnmpr_free, tlscb_cur.enc_completions, tlscb_cur.mac_completions))
    print("pre-sync: pre_debug_stage0_7_thread 0x%x post_debug_stage0_7_thread 0x%x" % (tlscb_cur.pre_debug_stage0_7_thread, tlscb_cur.post_debug_stage0_7_thread))
    tlscb_cur.GetObjValPd()
    print("post-sync: tnmdr_alloc %d tnmpr_alloc %d enc_requests %d mac_requests %d" % (tlscb_cur.tnmdr_alloc, tlscb_cur.tnmpr_alloc, tlscb_cur.enc_requests, tlscb_cur.mac_requests))
    print("post-sync: rnmdr_free %d rnmpr_free %d enc_completions %d mac_completions %d" % (tlscb_cur.rnmdr_free, tlscb_cur.rnmpr_free, tlscb_cur.enc_completions, tlscb_cur.mac_completions))
    print("post-sync: pre_debug_stage0_7_thread 0x%x post_debug_stage0_7_thread 0x%x" % (tlscb_cur.pre_debug_stage0_7_thread, tlscb_cur.post_debug_stage0_7_thread))

    print("snapshot3: tnmdr_alloc %d tnmpr_alloc %d enc_requests %d mac_requests %d" % (tlscb.tnmdr_alloc, tlscb.tnmpr_alloc, tlscb.enc_requests, tlscb.mac_requests))
    print("snapshot3: rnmdr_free %d rnmpr_free %d enc_completions %d mac_completions %d" % (tlscb.rnmdr_free, tlscb.rnmpr_free, tlscb.enc_completions, tlscb.mac_completions))

    # 0. Verify the counters
    #if ((tlscb_cur.tnmdr_alloc - tlscb.tnmdr_alloc) != (tlscb_cur.rnmdr_free - tlscb.rnmdr_free)):
        #print("tnmdr alloc increment not same as rnmdr free increment")
        #return False

    #if ((tlscb_cur.tnmpr_alloc - tlscb.tnmpr_alloc) != (tlscb_cur.rnmpr_free - tlscb.rnmpr_free)):
        #print("tnmpr alloc increment not same as rnmpr free increment")
        #return False


    if ((tlscb_cur.enc_requests - tlscb.enc_requests) != (tlscb_cur.enc_completions - tlscb.enc_completions)):
        print("enc requests not equal to completions %d %d %d %d" % (tlscb_cur.enc_requests, tlscb.enc_requests, tlscb_cur.enc_completions, tlscb.enc_completions))
        return False

    if ((tlscb_cur.mac_requests - tlscb.mac_requests) != (tlscb_cur.mac_completions - tlscb.mac_completions)):
        print("mac requests not equal to completions %d %d %d %d" % (tlscb_cur.mac_requests, tlscb.mac_requests, tlscb_cur.mac_completions, tlscb.mac_completions))
        return False

    # 1. Verify threading

    # When using random value for IV, there will be an additional DRBG table read program launched
    if tc.module.args.cipher_suite == "CBC": 
        pre_debug_stage0_7_thread = 0x111911
    elif ((tlscb.debug_dol & tcp_tls_proxy.tls_debug_dol_explicit_iv_use_random) == tcp_tls_proxy.tls_debug_dol_explicit_iv_use_random):
        pre_debug_stage0_7_thread = 0x137711
    else:
        pre_debug_stage0_7_thread = 0x117711

    if (tlscb_cur.pre_debug_stage0_7_thread != pre_debug_stage0_7_thread):
        print("pre crypto pipeline threading was not ok 0x%x" % tlscb_cur.pre_debug_stage0_7_thread)
        return False

    if (tlscb_cur.post_debug_stage0_7_thread != 0x11111):
        print("post crypto pipeline threading was not ok 0x%x" % tlscb_cur.pre_debug_stage0_7_thread)
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
    rnmdr_cur.GetMeta()
    rnmpr_cur = tc.infra_data.ConfigStore.objects.db["RNMPR"]
    rnmpr_cur.GetMeta()
    rnmdr = tc.pvtdata.db["RNMDR"]
    rnmpr = tc.pvtdata.db["RNMPR"]

    tnmdr_cur = tc.infra_data.ConfigStore.objects.db["TNMDR"]
    tnmdr_cur.GetMeta()
    tnmpr_cur = tc.infra_data.ConfigStore.objects.db["TNMPR"]
    tnmpr_cur.GetMeta()

    if tc.module.args.cipher_suite == "CCM":
        brq_cur = tc.infra_data.ConfigStore.objects.db["BRQ_ENCRYPT_CCM"]
    elif tc.module.args.cipher_suite == "CBC":
        brq_cur = tc.infra_data.ConfigStore.objects.db["BRQ_ENCRYPT_CBC"]
    else:
        brq_cur = tc.infra_data.ConfigStore.objects.db["BRQ_ENCRYPT"]
    brq_cur.GetMeta()
    if brq_cur.pi > 0:
        brq_cur.GetRingEntries([brq_cur.pi - 1, brq_cur.pi])
    else:
        brq_cur.GetRingEntries([brq_cur.pi])

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

        # In case of AES-CBC-HMAC_SHA2 cipher (MAC-then-encrypt), there is software chaining of 2 barco
        # passes, HMAC in the 1st pass, and AES-CBC in the 2nd pass, hence barco command will be AES-CBC
        if tc.module.args.cipher_suite != "CBC" or brq_cur.ring_entries[brq_cur.pi-1].command != 0x1000000:
            print("BRQ Command Check: Failed : Got: 0x%x, Expected: 0x%x" % (brq_cur.ring_entries[brq_cur.pi-1].command, tlscb.command))
            return False


    # 10. Verify BRQ Descriptor Key Index field
    if brq_cur.ring_entries[brq_cur.pi-1].key_desc_index != tlscb.crypto_key_idx:
        print("BRQ Crypto Key Index Check: Failed : Got: 0x%x, Expected: 0x%x" % (brq_cur.ring_entries[brq_cur.pi-1].key_desc_index, tlscb.crypto_key_idx))
        return False

    # 11. Verify BRQ Descriptor HMAC Key Index field
    #     Activate this check when HW support for AES-CBC-HMAC-SHA2, currently we use software chaining
    #if brq_cur.ring_entries[brq_cur.pi-1].second_key_desc_index != tlscb.crypto_hmac_key_idx:
        #print("BRQ Crypto HMAC Key Index Check: Failed : Got: 0x%x, Expected: 0x%x" % (brq_cur.ring_entries[brq_cur.pi-1].second_key_desc_index, tlscb.crypto_hmac_key_idx))
        #return False

    # 12. Verify Salt
    #     In case of AES-CBC-HMAC-SHA2 ciphers, we use random IV with no salt.
    if brq_cur.ring_entries[brq_cur.pi-1].salt != tlscb.salt and tc.module.args.cipher_suite != "CBC":
        print("Salt Check Failed: Got 0x%x, Expected: 0x%x" % (brq_cur.ring_entries[brq_cur.pi-1].salt, tlscb.salt))
        return False
    print("Salt Check Success: Got 0x%x, Expected: 0x%x" % (brq_cur.ring_entries[brq_cur.pi-1].salt, tlscb.salt))
        
    # 13. Verify Explicit IV
    tls_explicit_iv = tlscb.explicit_iv

    # When using random value for IV from DRBG or AES-CBC-HMAC-SHA2 cipher, the IV field from tlscb will not match, as expected.
    if ((tlscb.debug_dol & tcp_tls_proxy.tls_debug_dol_explicit_iv_use_random) != tcp_tls_proxy.tls_debug_dol_explicit_iv_use_random):
        if brq_cur.ring_entries[brq_cur.pi-1].explicit_iv != tls_explicit_iv:
            print("Explicit IV Check Failed: Got 0x%x, Expected: 0x%x" %
                  (brq_cur.ring_entries[brq_cur.pi-1].explicit_iv, tls_explicit_iv))
            return False
    print("Explicit IV Check Success: Got 0x%x, Expected: 0x%x" %
          (brq_cur.ring_entries[brq_cur.pi-1].explicit_iv, tls_explicit_iv))

    # 14. Verify header size, this is the AAD size and is 13 bytes 
    #     In case of AES-CBC-HMAC-SHA2 (MAC-then-encrypt) ciphers, there is no header with
    #     software-chaining in the 2nd pass of barco (AES-CBC encrypt).
    #     In case of AES-CCM, TLS uses a fixed header size of 2 16-byte blocks including AAD.
    if tc.module.args.cipher_suite == "CBC":
       hdr_size = 0x0
    elif tc.module.args.cipher_suite == "CCM":
       hdr_size = 0x20
    else:
       hdr_size = 0xd
    if brq_cur.ring_entries[brq_cur.pi-1].header_size != hdr_size:
        print("Header Size Check Failed: Got 0x%x, Expected: 0x%x" %
                                (brq_cur.ring_entries[brq_cur.pi-1].header_size, hdr_size))
        return False
    else:
        print("Header Size Check Success: Got 0x%x, Expected: 0x%x" %
                                (brq_cur.ring_entries[brq_cur.pi-1].header_size, hdr_size))

    # 15. Barco Status check
    if brq_cur.ring_entries[brq_cur.pi-1].barco_status != 0:
        print("Barco Status Check Failed: Got 0x%x, Expected: 0" %
                                (brq_cur.ring_entries[brq_cur.pi-1].barco_status))
        return False
    else:
        print("Barco Status Check Success: Got 0x%x, Expected: 0" %
                                (brq_cur.ring_entries[brq_cur.pi-1].barco_status))

    # 16. Verify page
    #if rnmpr.ringentries[0].handle != brq_cur.swdre_list[0].Addr1:
    #    print("Page handle not as expected in brq_cur.swdre_list")
        #return False

    return True

def TestCaseTeardown(tc):
    print("TestCaseTeardown(): Sample Implementation.")
    return
