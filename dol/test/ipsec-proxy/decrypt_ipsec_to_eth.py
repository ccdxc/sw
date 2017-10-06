# Testcase definition file.

import pdb
import copy

import types_pb2                as types_pb2
import crypto_keys_pb2          as crypto_keys_pb2
#import crypto_keys_pb2_grpc     as crypto_keys_pb2_grpc
import test.callbacks.networking.modcbs as modcbs

from config.store               import Store

rnmdr = 0
ipseccbq = 0
ipseccb = 0

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

    print("TestCaseSetup(): Sample Implementation.")
    # 1. Configure IPSECCB in HBM before packet injection
    ipseccb = tc.infra_data.ConfigStore.objects.db["IPSECCB0000"]
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
    ipseccb.barco_enc_cmd             = 0x30000000
    ipseccb.iv                        = 0xaaaaaaaaaaaaaaaa
    ipseccb.iv_salt                   = 0xbbbbbbbb
    ipseccb.esn_hi                    = 0
    ipseccb.esn_lo                    = 0
    ipseccb.spi                       = 0
    ipseccb.key_index                 = ipseccb.crypto_key.keyindex
    ipseccb.SetObjValPd()

    # 2. Clone objects that are needed for verification
    rnmdr = copy.deepcopy(tc.infra_data.ConfigStore.objects.db["RNMDR"])
    ipseccbq = copy.deepcopy(tc.infra_data.ConfigStore.objects.db["IPSECCB0000_IPSECCBQ"])
    ipseccb = tc.infra_data.ConfigStore.objects.db["IPSECCB0000"]

    return

def TestCaseVerify(tc):
    global rnmdr
    global ipseccbq
    global ipseccb

    # 1. Verify pi/ci got update got updated
    ipseccb_cur = tc.infra_data.ConfigStore.objects.db["IPSECCB0000"]
    print("pre-sync: ipseccb_cur.pi %d ipseccb_cur.ci %d" % (ipseccb_cur.pi, ipseccb_cur.ci))
    ipseccb_cur.GetObjValPd()
    print("post-sync: ipseccb_cur.pi %d ipseccb_cur.ci %d" % (ipseccb_cur.pi, ipseccb_cur.ci))
    if (ipseccb_cur.pi != ipseccb.pi or ipseccb_cur.ci != ipseccb.ci):
        print("serq pi/ci not as expected")
        return False

    # 3. Fetch current values from Platform
    rnmdr_cur = tc.infra_data.ConfigStore.objects.db["RNMDR"]
    rnmdr_cur.Configure()
    ipseccbqq_cur = tc.infra_data.ConfigStore.objects.db["IPSECCB0000_IPSECCBQ"]
    ipseccbqq_cur.Configure()

    # 4. Verify PI for RNMDR got incremented by 1
    if (rnmdr_cur.pi != rnmdr.pi+2):
        print("RNMDR pi check failed old %d new %d" % (rnmdr.pi, rnmdr_cur.pi))
        return False

    # 5. Verify descriptor
    if rnmdr.ringentries[rnmdr.pi].handle != ipseccbqq_cur.ringentries[ipseccb.pi].handle:
        print("Descriptor handle not as expected in ringentries 0x%x 0x%x" % (rnmdr.ringentries[rnmdr.pi].handle, ipseccbqq_cur.ringentries[ipseccb.pi].handle))
        #return False

    if rnmdr.swdre_list[rnmdr.pi].DescAddr != ipseccbqq_cur.swdre_list[ipseccb.pi].DescAddr:
        print("Descriptor handle not as expected in swdre_list 0x%x 0x%x" % (rnmdr.swdre_list[rnmdr.pi].DescAddr, ipseccbqq_cur.swdre_list[ipseccb.pi].DescAddr))
        #return False

    return True

def TestCaseTeardown(tc):
    print("TestCaseTeardown(): Sample Implementation.")
    return
