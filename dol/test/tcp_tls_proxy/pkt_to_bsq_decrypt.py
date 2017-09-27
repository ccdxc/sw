# Testcase definition file.

import pdb
import copy
import test.tcp_tls_proxy.tcp_proxy as tcp_proxy

import types_pb2                as types_pb2
import crypto_keys_pb2          as crypto_keys_pb2
#import crypto_keys_pb2_grpc     as crypto_keys_pb2_grpc

from config.store               import Store
from config.objects.proxycb_service    import ProxyCbServiceHelper
from config.objects.tcp_proxy_cb        import TcpCbHelper
import test.callbacks.networking.modcbs as modcbs

rnmdr = 0
rnmpr = 0
tnmdr = 0
tnmpr = 0
brq = 0
tlscb = 0

def Setup(infra, module):
    print("Setup(): Sample Implementation")
    modcbs.Setup(infra, module)
    return

def Teardown(infra, module):
    print("Teardown(): Sample Implementation.")
    return

def TestCaseSetup(tc):
    global rnmdr
    global rnmpr
    global tnmdr
    global tnmpr
    global brq
    global tlscb

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
    rnmpr = copy.deepcopy(tc.infra_data.ConfigStore.objects.db["RNMPR"])

    tnmdr = copy.deepcopy(tc.infra_data.ConfigStore.objects.db["TNMDR"])
    tnmpr = copy.deepcopy(tc.infra_data.ConfigStore.objects.db["TNMPR"])

    tlscbid = "TlsCb%04d" % id
    tlscb = copy.deepcopy(tc.infra_data.ConfigStore.objects.db[tlscbid])

    brq = copy.deepcopy(tc.infra_data.ConfigStore.objects.db["BRQ_ENCRYPT"])

    # Key Setup
    key_type = types_pb2.CRYPTO_KEY_TYPE_AES128
    key_size = 16
    #key = b'\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00'
    key = b'\x19\xe4\xa3\x26\xa5\x0a\xf1\x29\x06\x3c\x11\x0c\x7f\x03\xf9\x5e'
    tlscb.crypto_key.Update(key_type, key_size, key)

    # TLS-CB Setup
    #tlscb.command = 0x30000000
    tlscb.command = 0x30100000
    tlscb.crypto_key_idx = tlscb.crypto_key.keyindex
    # Salt: 0xaf 0x98 0xc5 0xe2
    tlscb.salt = 0xe2c598af
    # IV: 
    tlscb.explicit_iv = 0x0100000000000000
    tlscb.is_decrypt_flow = True
    tlscb.SetObjValPd()
    print("IS Decrypt Flow: %s" % tlscb.is_decrypt_flow)

    return

def TestCaseVerify(tc):
    global rnmdr
    global rnmpr
    global tnmdr
    global tnmpr
    global brq
    global tlscb

    id = ProxyCbServiceHelper.GetFlowInfo(tc.config.flow._FlowObject__session)
    # 1. Fetch current values from Platform
    rnmdr_cur = tc.infra_data.ConfigStore.objects.db["RNMDR"]
    rnmdr_cur.Configure()

    rnmpr_cur = tc.infra_data.ConfigStore.objects.db["RNMPR"]
    rnmpr_cur.Configure()

    tnmdr_cur = tc.infra_data.ConfigStore.objects.db["TNMDR"]
    rnmdr_cur.Configure()

    tnmpr_cur = tc.infra_data.ConfigStore.objects.db["TNMPR"]
    rnmpr_cur.Configure()

    tlscb_cur = tc.infra_data.ConfigStore.objects.db["TlsCb0000"]
    tlscb_cur.GetObjValPd()

    brq_cur = tc.infra_data.ConfigStore.objects.db["BRQ_ENCRYPT"]
    brq_cur.Configure()

    # 2. Verify PI for RNMDR got incremented by 1
    if (rnmdr_cur.pi != rnmdr.pi+1):
        print("RNMDR pi check failed old %d new %d" % (rnmdr.pi, rnmdr_cur.pi))
        return False


    # 2. Verify PI for TNMDR got incremented by 1
    if (tnmdr_cur.pi != tnmdr.pi+1):
        print("TNMDR pi check failed old %d new %d" % (tnmdr.pi, tnmdr_cur.pi))
        #return False
        # Enable once the semaphore support is added
    #else:
    #    print("TNMDR pi check succeeded old %d new %d" % (tnmdr.pi, tnmdr_cur.pi))


    # 3. Verify descriptor on the BRQ
    if rnmdr.ringentries[rnmdr.pi].handle != brq_cur.ring_entries[0].ilist_addr:
        print("RNMDR Check: Descriptor handle not as expected in ringentries 0x%x 0x%x" % (rnmdr.ringentries[rnmdr.pi].handle, brq_cur.ring_entries[0].ilist_addr))
        return False
    #else:
    #    print("RNMDR Check: Descriptor handle as expected in ringentries 0x%x 0x%x" % (rnmdr.ringentries[rnmdr.pi].handle, brq_cur.ring_entries[0].ilist_addr))



    # 4. Verify descriptor on the BRQ
    if tnmdr.ringentries[tnmdr.pi].handle != brq_cur.ring_entries[0].olist_addr:
        print("TNMDR Check: Descriptor handle not as expected in ringentries 0x%x 0x%x" % (tnmdr.ringentries[tnmdr.pi].handle, brq_cur.ring_entries[0].olist_addr))
        return False
    #else:
    #    print("TNMDR Check: Descriptor handle as expected in ringentries 0x%x 0x%x" % (tnmdr.ringentries[tnmdr.pi].handle, brq_cur.ring_entries[0].olist_addr))



    # 5. Verify BRQ Descriptor Command field
    if brq_cur.ring_entries[0].command != tlscb.command:
        print("BRQ Command Check: Failed : Got: 0x%x, Expected: 0x%x" % (brq_cur.ring_entries[0].command, tlscb.command))
        return False
    #else:
    #    print("BRQ Command Check: Success : Got: 0x%x, Expected: 0x%x" % (brq_cur.ring_entries[0].command, tlscb.command))


    # 6. Verify BRQ Descriptor Key Index field
    if brq_cur.ring_entries[0].key_desc_index != tlscb.crypto_key_idx:
        print("BRQ Crypto Key Index Check: Failed : Got: 0x%x, Expected: 0x%x" % (brq_cur.ring_entries[0].key_desc_index, tlscb.crypto_key_idx))
        return False
    #else:
    #    print("BRQ Crypto Key Index Check: Success : Got: 0x%x, Expected: 0x%x" % (brq_cur.ring_entries[0].key_desc_index, tlscb.crypto_key_idx))

    # 3. Verify page
    #if rnmpr.ringentries[0].handle != brq_cur.swdre_list[0].Addr1:
    #    print("Page handle not as expected in brq_cur.swdre_list")
        #return False

    return True

def TestCaseTeardown(tc):
    print("TestCaseTeardown(): Sample Implementation.")
    return
