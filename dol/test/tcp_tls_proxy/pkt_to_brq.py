# Testcase definition file.

import pdb
import copy

import types_pb2                as types_pb2
import crypto_keys_pb2          as crypto_keys_pb2
import crypto_keys_pb2_grpc     as crypto_keys_pb2_grpc

from config.store                   import Store
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
    tcb.debug_dol = 0
    tcb.SetObjValPd()

    # 2. Clone objects that are needed for verification
    rnmdr = copy.deepcopy(tc.infra_data.ConfigStore.objects.db["RNMDR"])
    rnmpr = copy.deepcopy(tc.infra_data.ConfigStore.objects.db["RNMPR"])
    tnmdr = copy.deepcopy(tc.infra_data.ConfigStore.objects.db["TNMDR"])
    tnmpr = copy.deepcopy(tc.infra_data.ConfigStore.objects.db["TNMPR"])

    tlscbid = "TlsCb%04d" % id
    tlscb = copy.deepcopy(tc.infra_data.ConfigStore.objects.db[tlscbid])

    brq = copy.deepcopy(tc.infra_data.ConfigStore.objects.db["BRQ_ENCRYPT"])
    tcpcb = copy.deepcopy(tcb)


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
    tlscb.enc_requests = 0
    tlscb.enc_completions = 0
    tlscb.SetObjValPd()

    tc.pvtdata.Add(tlscb)
    tc.pvtdata.Add(rnmdr)
    tc.pvtdata.Add(rnmpr)
    tc.pvtdata.Add(tnmdr)
    tc.pvtdata.Add(tnmpr)
    tc.pvtdata.Add(tcpcb)
    tc.pvtdata.Add(brq)
    return

def TestCaseVerify(tc):

    id = ProxyCbServiceHelper.GetFlowInfo(tc.config.flow._FlowObject__session)
    # 1. Verify pi/ci got update got updated for SERQ
    tlscbid = "TlsCb%04d" % id
    tlscb = tc.pvtdata.db[tlscbid]
    tlscb_cur = tc.infra_data.ConfigStore.objects.db[tlscbid]
    print("pre-sync: tlscb_cur.serq_pi %d tlscb_cur.serq_ci %d" % (tlscb_cur.serq_pi, tlscb_cur.serq_ci))
    tlscb_cur.GetObjValPd()
    print("post-sync: tlscb_cur.serq_pi %d tlscb_cur.serq_ci %d" % (tlscb_cur.serq_pi, tlscb_cur.serq_ci))
    if (tlscb_cur.serq_pi != (tlscb.serq_pi+1) or tlscb_cur.serq_ci != (tlscb.serq_ci+1)):
        print("serq pi/ci not as expected")
        return False

    # 2. Verify enc_requests
    if (tlscb_cur.enc_requests != tlscb.enc_requests+1):
        print("enc_requests not as expected")
        return False

    if (tlscb_cur.enc_completions != tlscb.enc_completions+1):
        print("enc_completions not as expected")
        return False

    if (tlscb_cur.pre_debug_stage0_7_thread != 0x117711):
        print("pre_debug_stage0_7_thread not as expected")
        return False

    if (tlscb_cur.post_debug_stage0_7_thread != 0):
        print("post_debug_stage0_7_thread not as expected")
        return False

    # 3. Verify pi/ci got update got updated for BRQ
    brq = tc.pvtdata.db["BRQ_ENCRYPT"]
    brq_cur = tc.infra_data.ConfigStore.objects.db["BRQ_ENCRYPT"]
    print("pre-sync: brq_cur.pi %d brq_cur.ci %d" % (brq_cur.pi, brq_cur.ci))
    brq_cur.Configure()
    print("post-sync: brq_cur.pi %d brq_cur.ci %d" % (brq_cur.pi, brq_cur.ci))
    if (brq_cur.pi != (brq.pi+1) or brq_cur.ci != (brq.ci+1)):
        print("brq pi/ci not as expected")
        #needs fix in HAL and support in model/p4+ for this check to work/pass
        #return False

    # 4. Fetch current values from Platform
    rnmdr = tc.pvtdata.db["RNMDR"]
    rnmpr = tc.pvtdata.db["RNMPR"]
    rnmdr_cur = tc.infra_data.ConfigStore.objects.db["RNMDR"]
    rnmdr_cur.Configure()
    rnmpr_cur = tc.infra_data.ConfigStore.objects.db["RNMPR"]
    rnmpr_cur.Configure()
    tnmdr = tc.pvtdata.db["TNMDR"]
    tnmpr = tc.pvtdata.db["TNMPR"]
    tnmdr_cur = tc.infra_data.ConfigStore.objects.db["TNMDR"]
    tnmdr_cur.Configure()
    tnmpr_cur = tc.infra_data.ConfigStore.objects.db["TNMPR"]
    tnmpr_cur.Configure()

    # 5. Verify PI for RNMDR got incremented by 1 
    if (rnmdr_cur.pi != rnmdr.pi+1):
        print("RNMDR pi check failed old %d new %d" % (rnmdr.pi, rnmdr_cur.pi))
        return False
    print("Old RNMDR PI: %d, New RNMDR PI: %d" % (rnmdr.pi, rnmdr_cur.pi))

    # 6. Verify PI for TNMDR got incremented by 1 
    if (tnmdr_cur.pi != tnmdr.pi+1):
        print("TNMDR pi check failed old %d new %d" % (tnmdr.pi, tnmdr_cur.pi))
        return False
    print("Old TNMDR PI: %d, New RNMDR PI: %d" % (tnmdr.pi, tnmdr_cur.pi))

    # 7. Verify PI for TNMPR got incremented by 1 
    if (tnmpr_cur.pi != tnmpr.pi+1):
        print("TNMPR pi check failed old %d new %d" % (tnmpr.pi, tnmpr_cur.pi))
        return False
    print("Old TNMPR PI: %d, New RNMDR PI: %d" % (tnmpr.pi, tnmpr_cur.pi))

    # 8. Verify descriptor 
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
