# Testcase definition file.

import pdb
import copy

import types_pb2                as types_pb2
import internal_pb2          as internal_pb2
#import internal_pb2_grpc     as internal_pb2_grpc

from iris.config.store               import Store
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
    print("Setup(): Sample Implementation")
    return

def TestCaseVerify(tc):
    global ipseccbq
    global ipseccb

    return True

def TestCaseTeardown(tc):
    print("TestCaseTeardown(): Sample Implementation.")
    return
