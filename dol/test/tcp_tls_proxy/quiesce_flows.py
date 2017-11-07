# Testcase definition file.
import pdb
import copy
import binascii

import types_pb2                as types_pb2

from config.store               import Store
from infra.common.objects       import ObjectDatabase as ObjectDatabase
from infra.common.logging       import logger
from infra.common.glopts        import GlobalOptions
import infra.config.base        as base
import config.hal.defs          as haldefs
import config.hal.api           as halapi
import quiesce_pb2              as quiesce_pb2

def quience_msg_send():

    stub = quiesce_pb2.QuiesceStub(halapi.HalChannel)
    print("Invoking QuiesceMsgSnd API");
    req = types_pb2.Empty()
    stub.QuiesceMsgSnd(req)
    return

def Setup(infra, module):
    return

def Teardown(infra, module):
    print("Teardown(): Sample Implementation.")
    return

def TestCaseSetup(tc):
    return

def TestCaseStepVerify(tc, step):
    if halapi.IsHalDisabled():
        return True
    print("step")
    print(step.step_id)
    if step.step_id == 0:
        resp = quience_msg_send()
    return True

def TestCaseVerify(tc):
    return True

def TestCaseTeardown(tc):
    print("TestCaseTeardown(): Sample Implementation.")
    return
