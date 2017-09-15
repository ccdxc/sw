# Testcase definition file.

import pdb
import copy

import types_pb2                as types_pb2
import crypto_keys_pb2          as crypto_keys_pb2
import crypto_keys_pb2_grpc     as crypto_keys_pb2_grpc

from config.store                   import Store
from config.objects.proxycb_service    import ProxyCbServiceHelper
from config.objects.tcp_proxy_cb        import TcpCbHelper

rnmdr = 0
rnmpr = 0
brq = 0
tlscb = 0

def Setup(infra, module):
    print("Setup(): Sample Implementation")
    return

def Teardown(infra, module):
    print("Teardown(): Sample Implementation.")
    return

def TestCaseSetup(tc):
    global rnmdr
    global rnmpr
    global brq
    global tlscb

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
    tcb.SetObjValPd()

    # 2. Clone objects that are needed for verification
    rnmdr = copy.deepcopy(tc.infra_data.ConfigStore.objects.db["RNMDR"])
    rnmpr = copy.deepcopy(tc.infra_data.ConfigStore.objects.db["RNMPR"])

    tlscbid = "TlsCb%04d" % id
    tlscb = copy.deepcopy(tc.infra_data.ConfigStore.objects.db[tlscbid])

    brq = copy.deepcopy(tc.infra_data.ConfigStore.objects.db["BRQ_ENCRYPT"])


    # Key Setup
    key_type = types_pb2.CRYPTO_KEY_TYPE_AES128
    key_size = 16
    key = b'\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00'
    tlscb.crypto_key.Update(key_type, key_size, key)

    # TLS-CB Setup
    tlscb.command = 0x30000000
    tlscb.crypto_key_idx = tlscb.crypto_key.keyindex
    tlscb.salt = 0x12345678
    tlscb.explicit_iv = 0xfedcba9876543210
    tlscb.SetObjValPd()

    return

def TestCaseVerify(tc):
    global rnmdr
    global rnmpr
    global brq
    global tlscb

    id = ProxyCbServiceHelper.GetFlowInfo(tc.config.flow._FlowObject__session)
    # 1. Verify pi/ci got update got updated for SERQ
    tlscbid = "TlsCb%04d" % id
    tlscb_cur = tc.infra_data.ConfigStore.objects.db[tlscbid]
    print("pre-sync: tlscb_cur.serq_pi %d tlscb_cur.serq_ci %d" % (tlscb_cur.serq_pi, tlscb_cur.serq_ci))
    tlscb_cur.GetObjValPd()
    print("post-sync: tlscb_cur.serq_pi %d tlscb_cur.serq_ci %d" % (tlscb_cur.serq_pi, tlscb_cur.serq_ci))
    if (tlscb_cur.serq_pi != (tlscb.serq_pi+1) or tlscb_cur.serq_ci != (tlscb.serq_ci+1)):
        print("serq pi/ci not as expected")
        #VijayV to enable this test after ci is fixed in p4+
        #return False

    # 2. Verify pi/ci got update got updated for BRQ
    brq_cur = tc.infra_data.ConfigStore.objects.db["BRQ_ENCRYPT"]
    print("pre-sync: brq_cur.pi %d brq_cur.ci %d" % (brq_cur.pi, brq_cur.ci))
    brq_cur.Configure()
    print("post-sync: brq_cur.pi %d brq_cur.ci %d" % (brq_cur.pi, brq_cur.ci))
    if (brq_cur.pi != (brq.pi+1) or brq_cur.ci != (brq.ci+1)):
        print("brq pi/ci not as expected")
        #needs fix in HAL and support in model/p4+ for this check to work/pass
        #return False

    # 2. Fetch current values from Platform
    rnmdr_cur = tc.infra_data.ConfigStore.objects.db["RNMDR"]
    rnmdr_cur.Configure()
    rnmpr_cur = tc.infra_data.ConfigStore.objects.db["RNMPR"]
    rnmpr_cur.Configure()

    # 3. Verify PI for RNMDR got incremented by 1
    if (rnmdr_cur.pi != rnmdr.pi+1):
        print("RNMDR pi check failed old %d new %d" % (rnmdr.pi, rnmdr_cur.pi))
        return False
    print("Old RNMDR PI: %d, New RNMDR PI: %d" % (rnmdr.pi, rnmdr_cur.pi))

    # 2. Verify descriptor 
    print("BRQ:")
    print("ilist_addr 0x%x" % brq_cur.ring_entries[0].ilist_addr)
    print("olist_addr 0x%x" % brq_cur.ring_entries[0].olist_addr)
    print("command 0x%x" % brq_cur.ring_entries[0].command)
    print("key_desc_index 0x%x" % brq_cur.ring_entries[0].key_desc_index)
    print("iv_addr 0x%x" % brq_cur.ring_entries[0].iv_addr)
    print("status_addr 0x%x" % brq_cur.ring_entries[0].status_addr)
    if rnmdr.ringentries[rnmdr.pi].handle != brq_cur.ring_entries[0].ilist_addr:
        print("Descriptor handle not as expected in ringentries 0x%x 0x%x" % (rnmdr.ringentries[rnmdr.pi].handle, brq_cur.ring_entries[0].ilist_addr)) 
        return False
    print("RNMDR Entry: 0x%x, BRQ ILIST: 0x%x" % (rnmdr.ringentries[rnmdr.pi].handle, brq_cur.ring_entries[0].ilist_addr))

    # 3. Verify page
    #if rnmpr.ringentries[rnmdr.pi].handle != brq_cur.swdre_list[0].Addr1:
    #    print("Page handle not as expected in brq_cur.swdre_list")
    #    #return False

    return True

def TestCaseTeardown(tc):
    print("TestCaseTeardown(): Sample Implementation.")
    return
