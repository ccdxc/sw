# Testcase definition file.

import pdb
import copy

import types_pb2                as types_pb2
import crypto_keys_pb2          as crypto_keys_pb2
import test.callbacks.networking.modcbs as modcbs

from config.store               import Store
from infra.common.objects import ObjectDatabase as ObjectDatabase
from infra.common.logging import logger
from infra.common.glopts import GlobalOptions
from config.objects.proxycb_service    import ProxyCbServiceHelper
from config.objects.ipsec_proxy_cb      import IpsecCbHelper

rnmdr = 0
ipseccbq = 0
ipseccb = 0
expected_seq_no = 0

def Setup(infra, module):
    print("Setup(): Sample Implementation")
    modcbs.Setup(infra, module)
    return

def Teardown(infra, module):
    print("Teardown(): Sample Implementation.")
    return

def TestCaseSetup(tc):
    global rnmdr
    global ipseccbq
    global ipseccb
    global expected_seq_no

    tc.pvtdata = ObjectDatabase()
    print("TestCaseSetup(): Sample Implementation.")
    id = ProxyCbServiceHelper.GetFlowInfo(tc.config.flow._FlowObject__session)
    IpsecCbHelper.main(id)
    ipsecid = "IPSECCB%04d" % id
    # 1. Configure IPSECCB in HBM before packet injection
    ipseccb = tc.infra_data.ConfigStore.objects.db[ipsecid]
    key_type = types_pb2.CRYPTO_KEY_TYPE_AES128
    key_size = 16
    key = b'\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00'
    ipseccb.crypto_key.Update(key_type, key_size, key)

    ipseccb.tunnel_sip4               = 0x0A010001
    ipseccb.tunnel_dip4               = 0x0A010002
    ipseccb.iv_size                   = 8
    ipseccb.icv_size                  = 16
    ipseccb.block_size                = 16
    ipseccb.key_index                 = 0
    ipseccb.barco_enc_cmd             = 0x30100000
    ipseccb.iv                        = 0xaaaaaaaaaaaaaaaa
    ipseccb.iv_salt                   = 0xbbbbbbbb
    ipseccb.esn_hi                    = 0
    ipseccb.esn_lo                    = 0
    ipseccb.spi                       = 0
    ipseccb.new_spi                   = 1
    ipseccb.key_index                 = ipseccb.crypto_key.keyindex
    ipseccb.expected_seq_no           = 0
    ipseccb.seq_no_bmp                = 0
    ipseccb.vrf_vlan                  = 0x0005
    ipseccb.is_nat_t                  = 0
    ipseccb.SetObjValPd()

    # 2. Clone objects that are needed for verification
    expected_seq_no = ipseccb.expected_seq_no
    rnmdr = copy.deepcopy(tc.infra_data.ConfigStore.objects.db["RNMDR"])
    rnmdr.GetMeta()
    rnmdr.GetRingEntries([rnmdr.pi, rnmdr.pi + 1])
    rnmdr.GetRingEntryAOL([rnmdr.pi, rnmdr.pi + 1])
    ipsec_cbq_id = ipsecid + "_IPSECCBQ"
    ipseccbq = copy.deepcopy(tc.infra_data.ConfigStore.objects.db[ipsec_cbq_id])
    ipseccb = tc.infra_data.ConfigStore.objects.db[ipsecid]

    rnmpr = copy.deepcopy(tc.infra_data.ConfigStore.objects.db["RNMPR"])
    rnmpr.GetMeta()
    rnmpr.GetRingEntries([rnmpr.pi])
    tnmdr = copy.deepcopy(tc.infra_data.ConfigStore.objects.db["TNMDR"])
    tnmdr.GetMeta()
    tnmdr.GetRingEntries([tnmdr.pi])
    tnmpr = copy.deepcopy(tc.infra_data.ConfigStore.objects.db["TNMPR"])
    tnmpr.GetMeta()
    tnmpr.GetRingEntries([tnmpr.pi])

    brq = copy.deepcopy(tc.infra_data.ConfigStore.objects.db["BRQ_DECRYPT_GCM"])
    brq.GetMeta()
    brq.GetRingEntries([brq.pi])

    tc.pvtdata.Add(rnmdr)
    tc.pvtdata.Add(rnmpr)
    tc.pvtdata.Add(tnmdr)
    tc.pvtdata.Add(tnmpr)
    tc.pvtdata.Add(brq)

    return

def TestCaseVerify(tc):
    global ipseccbq
    global ipseccb

    id = ProxyCbServiceHelper.GetFlowInfo(tc.config.flow._FlowObject__session)
    ipsecid = "IPSECCB%04d" % id
    ipsec_cbq_id = ipsecid + "_IPSECCBQ"
    ipseccbqq_cur = tc.infra_data.ConfigStore.objects.db[ipsec_cbq_id]
    ipseccbqq_cur.Configure()

    rnmdr = tc.pvtdata.db["RNMDR"]
    rnmpr = tc.pvtdata.db["RNMPR"]
    tnmdr = tc.pvtdata.db["TNMDR"]
    tnmpr = tc.pvtdata.db["TNMPR"]

    rnmdr_cur = tc.infra_data.ConfigStore.objects.db["RNMDR"]
    rnmdr_cur.GetMeta()
    rnmdr_cur.GetRingEntries([rnmdr_cur.pi, rnmdr_cur.pi + 1])
    rnmdr_cur.GetRingEntryAOL([rnmdr_cur.pi, rnmdr_cur.pi + 1])

    rnmpr_cur = tc.infra_data.ConfigStore.objects.db["RNMPR"]
    rnmpr_cur.GetMeta()
    rnmpr_cur.GetRingEntries([rnmpr.pi,rnmpr_cur.pi])

    tnmdr_cur = tc.infra_data.ConfigStore.objects.db["TNMDR"]
    tnmdr_cur.GetMeta()
    tnmdr_cur.GetRingEntries([tnmdr.pi,tnmdr_cur.pi])

    tnmpr_cur = tc.infra_data.ConfigStore.objects.db["TNMPR"]
    tnmpr_cur.GetMeta()
    tnmpr_cur.GetRingEntries([tnmpr.pi,tnmpr_cur.pi])

    # 1. Verify pi/ci got update got updated
    ipseccb_cur = tc.infra_data.ConfigStore.objects.db[ipsecid]
    print("pre-sync: ipseccb_cur.pi %d ipseccb_cur.ci %d" % (ipseccb_cur.pi, ipseccb_cur.ci))
    ipseccb_cur.GetObjValPd()
    print("post-sync: ipseccb_cur.pi %d ipseccb_cur.ci %d" % (ipseccb_cur.pi, ipseccb_cur.ci))
    if (ipseccb_cur.pi != ipseccb.pi or ipseccb_cur.ci != ipseccb.ci):
        print("serq pi/ci not as expected")
        return False

    print("Expected seq no 0x%x seq_no_bmp 0x%x" % (ipseccb_cur.expected_seq_no, ipseccb_cur.seq_no_bmp))

    # 3. Fetch current values from Platform
    ipseccbqq_cur = tc.infra_data.ConfigStore.objects.db[ipsec_cbq_id]
    ipseccbqq_cur.Configure()

    return True

def TestCaseTeardown(tc):
    print("TestCaseTeardown(): Sample Implementation.")
    return
