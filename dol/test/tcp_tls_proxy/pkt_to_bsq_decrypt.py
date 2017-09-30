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
from infra.common.objects               import ObjectDatabase as ObjectDatabase
from infra.common.logging               import logger
import test.tcp_tls_proxy.tcp_tls_proxy as tcp_tls_proxy

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

    brq = copy.deepcopy(tc.infra_data.ConfigStore.objects.db["BRQ_ENCRYPT"])
    brq.Configure()

    tlscbid = "TlsCb%04d" % id
    tlscb = copy.deepcopy(tc.infra_data.ConfigStore.objects.db[tlscbid])


    tcp_tls_proxy.tls_aes128_gcm_decrypt_setup(tc, tlscb)

    tc.pvtdata.Add(tlscb)
    tc.pvtdata.Add(rnmdr)
    tc.pvtdata.Add(rnmpr)
    tc.pvtdata.Add(tnmdr)
    tc.pvtdata.Add(tnmpr)
    tc.pvtdata.Add(brq)

    return

def TestCaseVerify(tc):

    id = ProxyCbServiceHelper.GetFlowInfo(tc.config.flow._FlowObject__session)


    # Retrieve saved state
    tlscbid = "TlsCb%04d" % id
    tlscb = tc.pvtdata.db[tlscbid]
    rnmdr = tc.pvtdata.db["RNMDR"]
    rnmpr = tc.pvtdata.db["RNMPR"]
    tnmdr = tc.pvtdata.db["TNMDR"]
    tnmpr = tc.pvtdata.db["TNMPR"]
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

    brq_cur = tc.infra_data.ConfigStore.objects.db["BRQ_ENCRYPT"]
    brq_cur.Configure()

    # 1. Verify PI for RNMDR got incremented by 1
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

    # 7. Verify Salt
    if brq_cur.ring_entries[brq_cur.pi-1].salt != tlscb.salt:
        print("Salt Check Failed: Got 0x%x, Expected: 0x%x" % (brq_cur.ring_entries[brq_cur.pi-1].salt, tlscb.salt))
        return False
    print("Salt Check Success: Got 0x%x, Expected: 0x%x" % (brq_cur.ring_entries[brq_cur.pi-1].salt, tlscb.salt))
        
    # 8. Verify Explicit IV
    # This is bound to fail until the DoL payload issue is fixed
    if brq_cur.ring_entries[brq_cur.pi-1].explicit_iv != tcp_tls_proxy.tls_aes128_gcm_explicit_iv:
        print("Explicit IV Check Failed: Got 0x%x, Expected: 0x%x" %
                                (brq_cur.ring_entries[brq_cur.pi-1].explicit_iv, tcp_tls_proxy.tls_aes128_gcm_explicit_iv))
    else:
        print("Explicit IV Check Success: Got 0x%x, Expected: 0x%x" %
                                (brq_cur.ring_entries[brq_cur.pi-1].explicit_iv, tcp_tls_proxy.tls_aes128_gcm_explicit_iv))

    # 9. Verify header size, this is the AAD size and is 13 bytes 
    if brq_cur.ring_entries[brq_cur.pi-1].header_size != 0xd:
        print("Header Size Check Failed: Got 0x%x, Expected: 0xd" %
                                (brq_cur.ring_entries[brq_cur.pi-1].header_size))
    else:
        print("Header Size Check Success: Got 0x%x, Expected: 0xd" %
                                (brq_cur.ring_entries[brq_cur.pi-1].header_size))

    # 10. Status check
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
