# Testcase definition file.

import pdb
import copy

import types_pb2                as types_pb2
import crypto_keys_pb2          as crypto_keys_pb2
#import crypto_keys_pb2_grpc     as crypto_keys_pb2_grpc

from config.store               import Store
from infra.common.objects import ObjectDatabase as ObjectDatabase
from infra.common.logging import logger
from infra.common.glopts import GlobalOptions

rnmdr = 0
ipseccbq = 0
ipseccb = 0
iv = 0
seq = 0
def Setup(infra, module):
    print("Setup(): Sample Implementation")
    elem = module.iterator.Get()
    module.testspec.selectors.flow.Extend(elem.flow)
    return

def Teardown(infra, module):
    print("Teardown(): Sample Implementation.")
    return

def TestCaseSetup(tc):
    global ipseccbq
    global ipseccb
    global rnmdr
    global iv
    global seq

    tc.pvtdata = ObjectDatabase()
    print("TestCaseSetup(): Sample Implementation.")
    # 2. Clone objects that are needed for verification
    rnmdr = copy.deepcopy(tc.infra_data.ConfigStore.objects.db["RNMDR"])
    rnmdr.GetMeta()
    rnmdr.GetRingEntries([rnmdr.pi, rnmdr.pi + 1])
    rnmdr.GetRingEntryAOL([rnmdr.pi, rnmdr.pi + 1])
    ipseccbq = copy.deepcopy(tc.infra_data.ConfigStore.objects.db["IPSECCB0000_IPSECCBQ"])
    ipseccb = tc.infra_data.ConfigStore.objects.db["IPSECCB0000"]

    rnmpr = copy.deepcopy(tc.infra_data.ConfigStore.objects.db["RNMPR"])
    rnmpr.GetMeta()
    rnmpr.GetRingEntries([rnmpr.pi])
    tnmdr = copy.deepcopy(tc.infra_data.ConfigStore.objects.db["TNMDR"])
    tnmdr.GetMeta()
    tnmdr.GetRingEntries([tnmdr.pi])
    tnmpr = copy.deepcopy(tc.infra_data.ConfigStore.objects.db["TNMPR"])
    tnmpr.GetMeta()
    tnmpr.GetRingEntries([tnmpr.pi])

    brq = copy.deepcopy(tc.infra_data.ConfigStore.objects.db["BRQ_ENCRYPT_GCM"])
    brq.GetMeta()
    brq.GetRingEntries([brq.pi])

    iv = ipseccb.iv
    seq = ipseccb.esn_lo

    tc.pvtdata.Add(rnmdr)
    tc.pvtdata.Add(rnmpr)
    tc.pvtdata.Add(tnmdr)
    tc.pvtdata.Add(tnmpr)
    tc.pvtdata.Add(brq)
    return

def TestCaseVerify(tc):
    return True

def TestCaseTeardown(tc):
    print("TestCaseTeardown(): Sample Implementation.")
    return
