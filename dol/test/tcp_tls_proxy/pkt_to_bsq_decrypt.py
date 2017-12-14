# Testcase definition file.

import pdb
import copy
import test.tcp_tls_proxy.tcp_proxy as tcp_proxy

import types_pb2                as types_pb2
import crypto_keys_pb2          as crypto_keys_pb2
#import crypto_keys_pb2_grpc     as crypto_keys_pb2_grpc

from config.store                       import Store
from config.objects.proxycb_service     import ProxyCbServiceHelper
from config.objects.tcp_proxy_cb        import TcpCbHelper
import test.callbacks.networking.modcbs as modcbs
import test.tcp_tls_proxy.tcp_tls_proxy as tcp_tls_proxy
from infra.common.objects import ObjectDatabase as ObjectDatabase
from infra.common.logging import logger
from infra.common.glopts import GlobalOptions

def Setup(infra, module):
    print("Setup(): Sample Implementation")
    modcbs.Setup(infra, module)
    return

def Teardown(infra, module):
    print("Teardown(): Sample Implementation.")
    return

def TestCaseSetup(tc):

    print("TestCaseSetup(): Start")

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

    if tc.module.args.cipher_suite == "CCM":
        brq = copy.deepcopy(tc.infra_data.ConfigStore.objects.db["BRQ_ENCRYPT_CCM"])
    elif tc.module.args.cipher_suite == "CBC":
        brq = copy.deepcopy(tc.infra_data.ConfigStore.objects.db["BRQ_ENCRYPT_CBC"])
    else:
        brq = copy.deepcopy(tc.infra_data.ConfigStore.objects.db["BRQ_ENCRYPT"])
    brq.Configure()

    tlscbid = "TlsCb%04d" % id
    tlscb = copy.deepcopy(tc.infra_data.ConfigStore.objects.db[tlscbid])

    tlscb.debug_dol = tcp_tls_proxy.tls_debug_dol_bypass_proxy | \
                            tcp_tls_proxy.tls_debug_dol_sesq_stop
    if hasattr(tc.module.args, 'reassemble'):
        if tc.module.args.reassemble == True:
            print("Enabling reassembly support")
            tlscb.debug_dol = tlscb.debug_dol | tcp_tls_proxy.tls_debug_dol_dec_reasm_path
    tlscb.other_fid = 0xffff

    if tc.module.args.key_size == 16:
        tcp_tls_proxy.tls_aes128_decrypt_setup(tc, tlscb)
    elif tc.module.args.key_size == 32:
        tcp_tls_proxy.tls_aes256_decrypt_setup(tc, tlscb)

    tc.pvtdata.Add(tlscb)
    tc.pvtdata.Add(rnmdr)
    tc.pvtdata.Add(rnmpr)
    tc.pvtdata.Add(tnmdr)
    tc.pvtdata.Add(tnmpr)
    tc.pvtdata.Add(brq)

    return

def TestCaseVerify(tc):
    if GlobalOptions.dryrun:
        return True

    id = ProxyCbServiceHelper.GetFlowInfo(tc.config.flow._FlowObject__session)


    # Retrieve saved state
    tlscbid = "TlsCb%04d" % id
    tlscb = tc.pvtdata.db[tlscbid]
    rnmdr = tc.pvtdata.db["RNMDR"]
    rnmpr = tc.pvtdata.db["RNMPR"]
    tnmdr = tc.pvtdata.db["TNMDR"]
    tnmpr = tc.pvtdata.db["TNMPR"]
    if tc.module.args.cipher_suite == "CCM":
        brq = tc.pvtdata.db["BRQ_ENCRYPT_CCM"]
    elif tc.module.args.cipher_suite == "CBC":
        brq = tc.pvtdata.db["BRQ_ENCRYPT_CBC"]
    else:
        brq = tc.pvtdata.db["BRQ_ENCRYPT"]

    #  Fetch current values from Platform
    rnmdr_cur = tc.infra_data.ConfigStore.objects.db["RNMDR"]
    rnmdr_cur.Configure()

    rnmpr_cur = tc.infra_data.ConfigStore.objects.db["RNMPR"]
    rnmpr_cur.Configure()

    tnmdr_cur = tc.infra_data.ConfigStore.objects.db["TNMDR"]
    tnmdr_cur.Configure()

    tnmpr_cur = tc.infra_data.ConfigStore.objects.db["TNMPR"]
    tnmpr_cur.Configure()

    tlscb_cur = tc.infra_data.ConfigStore.objects.db[tlscbid]
    tlscb_cur.GetObjValPd()

    if tc.module.args.cipher_suite == "CCM":
        brq_cur = tc.infra_data.ConfigStore.objects.db["BRQ_ENCRYPT_CCM"]
    elif tc.module.args.cipher_suite == "CBC":
        brq_cur = tc.infra_data.ConfigStore.objects.db["BRQ_ENCRYPT_CBC"]
    else:
        brq_cur = tc.infra_data.ConfigStore.objects.db["BRQ_ENCRYPT"]
    brq_cur.Configure()

    # 1. Verify PI for RNMDR got incremented by respective amount
    # This will be done only for non-reassemble scenarios
    if (not hasattr(tc.module.args, 'reassemble')) or (tc.module.args.reassemble != True):
        if (rnmdr_cur.pi != rnmdr.pi+1):
            print("RNMDR pi check failed old %d new %d" % (rnmdr.pi, rnmdr_cur.pi))
            return False
        print("Old RNMDR PI: %d, New RNMDR PI: %d" % (rnmdr.pi, rnmdr_cur.pi))


    # 2. Verify PI for TNMDR got incremented by 1
    if (tnmdr_cur.pi != tnmdr.pi+1):
        print("TNMDR pi check failed old %d new %d" % (tnmdr.pi, tnmdr_cur.pi))
        return False
    print("Old TNMDR PI: %d, New TNMDR PI: %d" % (tnmdr.pi, tnmdr_cur.pi))

    # 3. Verify PI for TNMPR got incremented by 1
    if (tnmpr_cur.pi != tnmpr.pi+1):
        print("TNMPR pi check failed old %d new %d" % (tnmpr.pi, tnmpr_cur.pi))
        return False
    print("Old TNMPR PI: %d, New TNMPR PI: %d" % (tnmpr.pi, tnmpr_cur.pi))

    print("BRQ: Current PI %d" % brq_cur.pi)

    # 3. Verify input descriptor on the BRQ
    if rnmdr.ringentries[rnmdr.pi].handle != (brq_cur.ring_entries[brq_cur.pi-1].ilist_addr - 0x40):
        print("Barco ilist Check: Descriptor handle not as expected in ringentries 0x%x 0x%x" % (rnmdr.ringentries[rnmdr.pi].handle, brq_cur.ring_entries[brq_cur.pi-1].ilist_addr - 0x40))
        return False
    print("Barco ilist Check: Descriptor handle as expected in ringentries 0x%x 0x%x" % (rnmdr.ringentries[rnmdr.pi].handle, brq_cur.ring_entries[brq_cur.pi-1].ilist_addr - 0x40))


    # 4. Verify output descriptor on the BRQ
    if tnmdr.ringentries[tnmdr.pi].handle != (brq_cur.ring_entries[brq_cur.pi-1].olist_addr - 0x40):
        print("Barco olist Check: Descriptor handle not as expected in ringentries 0x%x 0x%x" % (tnmdr.ringentries[tnmdr.pi].handle, brq_cur.ring_entries[brq_cur.pi-1].olist_addr - 0x40))
        return False
    print("Barco olist Check: Descriptor handle as expected in ringentries 0x%x 0x%x" % (tnmdr.ringentries[tnmdr.pi].handle, brq_cur.ring_entries[brq_cur.pi-1].olist_addr - 0x40))


    # 5. Verify BRQ Descriptor Command field
    if brq_cur.ring_entries[brq_cur.pi-1].command != tlscb.command:
        print("BRQ Command Check: Failed : Got: 0x%x, Expected: 0x%x" % (brq_cur.ring_entries[brq_cur.pi-1].command, tlscb.command))
        return False
    print("BRQ Command Check: Success : Got: 0x%x, Expected: 0x%x" % (brq_cur.ring_entries[brq_cur.pi-1].command, tlscb.command))


    # 6. Verify BRQ Descriptor Key Index field
    if brq_cur.ring_entries[brq_cur.pi-1].key_desc_index != tlscb.crypto_key_idx:
        print("BRQ Crypto Key Index Check: Failed : Got: 0x%x, Expected: 0x%x" % (brq_cur.ring_entries[brq_cur.pi-1].key_desc_index, tlscb.crypto_key_idx))
        return False
    print("BRQ Crypto Key Index Check: Success : Got: 0x%x, Expected: 0x%x" % (brq_cur.ring_entries[brq_cur.pi-1].key_desc_index, tlscb.crypto_key_idx))

    # 7. Verify BRQ Descriptor HMAC Key Index field
    #     Activate this check when HW support for AES-CBC-HMAC-SHA2, currently we use software chaining
    #if brq_cur.ring_entries[brq_cur.pi-1].second_key_desc_index != tlscb.crypto_hmac_key_idx:
        #print("BRQ Crypto HMAC Key Index Check: Failed : Got: 0x%x, Expected: 0x%x" % (brq_cur.ring_entries[0].second_key_desc_index, tlscb.crypto_hmac_key_idx))
        #return False

    # 8. Verify Salt
    if brq_cur.ring_entries[brq_cur.pi-1].salt != tlscb.salt:
        print("Salt Check Failed: Got 0x%x, Expected: 0x%x" % (brq_cur.ring_entries[brq_cur.pi-1].salt, tlscb.salt))
        return False
    print("Salt Check Success: Got 0x%x, Expected: 0x%x" % (brq_cur.ring_entries[brq_cur.pi-1].salt, tlscb.salt))
        
    # 9. Verify Explicit IV
    # This is bound to fail until the DoL payload issue is fixed
    tls_explicit_iv = tcp_tls_proxy.tls_explicit_iv(tc.module.args.key_size)
    if brq_cur.ring_entries[brq_cur.pi-1].explicit_iv != tls_explicit_iv:
        print("Explicit IV Check Failed: Got 0x%x, Expected: 0x%x" %
                                (brq_cur.ring_entries[brq_cur.pi-1].explicit_iv, tls_explicit_iv))
        return False
    print("Explicit IV Check Success: Got 0x%x, Expected: 0x%x" %
                            (brq_cur.ring_entries[brq_cur.pi-1].explicit_iv, tls_explicit_iv))

    # 10. Verify header size, this is the AAD size and is 13 bytes 
    if brq_cur.ring_entries[brq_cur.pi-1].header_size != 0xd:
        print("Header Size Check Failed: Got 0x%x, Expected: 0xd" %
                                (brq_cur.ring_entries[brq_cur.pi-1].header_size))
        return False
    print("Header Size Check Success: Got 0x%x, Expected: 0xd" %
                            (brq_cur.ring_entries[brq_cur.pi-1].header_size))

    # 11. Barco Status check
    if brq_cur.ring_entries[brq_cur.pi-1].barco_status != 0:
        print("Barco Status Check Failed: Got 0x%x, Expected: 0" %
                                (brq_cur.ring_entries[brq_cur.pi-1].barco_status))
    else:
        print("Barco Status Check Success: Got 0x%x, Expected: 0" %
                                (brq_cur.ring_entries[brq_cur.pi-1].barco_status))

    # 3. Verify page
    #if rnmpr.ringentries[0].handle != brq_cur.swdre_list[0].Addr1:
    #    print("Page handle not as expected in brq_cur.swdre_list")
        #return False

    # Pending Checks
    #   - Header size
    #   - Barco Operation Status
    #   - Key in memory
    #   - Decrypted content

    return True

def TestCaseTeardown(tc):
    print("TestCaseTeardown(): Sample Implementation.")
    return
